#pragma once

#include "server.cpp"

internal i32 SVP_WriteUnreliableSection(
    SimScene* sim,
    User* user,
    ZEByteBuffer* packet,
    TransmissionRecord* rec,
    PacketStats* stats)
{
    i32 capacity = packet->Space();
    u8* start = packet->cursor;
    // write the unreliable section header
    // - sim tick for these commands
    packet->cursor += COM_WriteI32(sim->tick, packet->cursor);

    // send input confirmation
    SimEntity* avatar = Sim_GetEntityBySerial(&g_sim, user->entSerial);
    Vec3 pos = {};
    if (avatar) { pos = avatar->body.previousPos; }
    S2C_InputResponse response = {};
    Cmd_InitInputResponse(
        &response,
        sim->tick,
        user->userInputSequence,
        pos
        );
    packet->cursor += Cmd_Serialise(
        &sim->quantise, packet->cursor, &response.header, 0);
    stats->numUnreliableMessages += 1;
    // ENTITY SYNC
    #if 1
    PriorityLink* links = user->entSync.links;
    i32 numLinks = user->entSync.numLinks;
    // most of the time there will be too many entities
    // for one packet and this for loop will never complete
    for (i32 i = 0; i < numLinks; ++i)
    {
        if (packet->Space() < sizeof(S2C_EntitySync))
        { break; }
        stats->numUnreliableMessages += 1;

        PriorityLink* link = &links[i];
        i32 serial = link->id;
        SimEntity* ent = Sim_GetEntityBySerial(sim, serial);

        // Prepare sync data
        S2C_EntitySync cmd = {};
        if (ent)
        {
            Cmd_WriteEntitySyncAsUpdate(&cmd, sim->tick, ent);
        }
        else
        {
            // No entity? must be dead!
            Cmd_WriteEntitySyncAsDeath(&cmd, sim->tick, link->id);
        }

        // Write sync to packet
        packet->cursor += Cmd_Serialise(
            &sim->quantise, packet->cursor, &cmd.header, 0);
        
        // Is this a new baseline for unreliable ack?
        if (link->baselineSequence == 0)
        {
            link->baselineSequence = rec->sequence;
        }

        // Reset importance, add to transmission record
        link->importance = 0;
        rec->syncIds[rec->numSyncMessages] = serial;
        rec->numSyncMessages += 1;
        if (rec->numSyncMessages == MAX_PACKET_SYNC_MESSAGES)
        { break; }
    }
    #endif
    i32 written = (packet->cursor - start);
    stats->unreliableBytes = written;
    return written;
}

internal i32 SVP_WriteReliableSection(
    User* user,
    ZEByteBuffer* packet,
    TransmissionRecord* rec,
    QuantiseSet* quantise,
    PacketStats* stats)
{
    // Commands are serialised to this buffer to measure them before
    // writing to the packet.
    u8 staging[CMD_MAX_SIZE];
    ZEByteBuffer* cmds = &user->reliableStream.outputBuffer;
    u8* read = cmds->start;
    u8* end = cmds->cursor;
    i32 bBaseSequenceSet = 0;
    CmdSeq baseSequence = 0;
    //i32 numCommandsNotWritten = 0;
    while(read < end)
    {
        Command* cmd = (Command*)read;
        ZE_ASSERT(
			Cmd_Validate(cmd) == ZE_ERROR_NONE,
			"Invalid reliable command")
        read += cmd->size;
        //i32 size = cmd->size;
        //read += size;
        //if (cmd->size > space) { numCommandsNotWritten++; continue; }
        if (cmd->sendTicks > 0) { cmd->sendTicks--; continue; }
        
        i32 cmdBytesWritten = 0;
        CmdSeq seqOffset = 0;
        // if first, use this command's sequence as the base others
        // will offset themselves from
        if (bBaseSequenceSet == NO)
        {
            bBaseSequenceSet = YES;
            baseSequence = cmd->sequence;
            packet->cursor += Cmd_WriteSequence(
                packet->cursor, baseSequence);
            //size += sizeof(CmdSeq);
            // We assume the buffer can always fit at least ONE reliable
            // command and so don't check the size of the first
            cmdBytesWritten = Cmd_Serialise(
                quantise, staging, cmd, 0);
        }
        else
        {
            // If this command's sequence is out of the one byte range
            // of an offset it cannot be written to this packet!
            i32 seqDiff = cmd->sequence - baseSequence;
            if (Cmd_IsSequenceDiffOkay(seqDiff) == NO)
            {
                printf("SV Seq diff %d out of range!\n", seqDiff);
                continue;
            }
            seqOffset = (CmdSeq)seqDiff;
            cmdBytesWritten = Cmd_Serialise(
                quantise, staging, cmd, seqOffset);
            // Will it fit?
            if (cmdBytesWritten > packet->Space())
            continue;
        }
        
        // Flow control to avoid filling packets with the same
        // commands redundantly
        cmd->timesSent++;
        // k this thing just ain't getting through
        if (cmd->timesSent > SV_CMD_SLOW_RESEND_ATTEMPTS)
        {
            // Spam the bloody thing out
            cmd->sendTicks = 0;
        }
        else
        {
            // Resend just incase
            // TODO: Vary resend wait based on packet loss?
            cmd->sendTicks = SV_CMD_RESEND_WAIT_TICKS;
        }
        
        //packet->cursor += Cmd_Serialise(packet->cursor, cmd, seqOffset);
        // Everything is okay, copy from staging to packet
        packet->cursor += ZE_COPY(staging, packet->cursor, cmdBytesWritten);
        stats->numReliableMessages += 1;
		
		// Record message
		rec->reliableMessageIds[rec->numReliableMessages++] = cmd->sequence;
        stats->commandCounts[cmd->type] += 1;
		ZE_ASSERT(
            rec->numReliableMessages < MAX_PACKET_TRANSMISSION_MESSAGES,
            "Too many messages in packet to record")
    }
    /*if (numCommandsNotWritten > 0)
    {
        APP_LOG(128, "SV no space for %d commands to user %d (%d bytes in output)\n",
            numCommandsNotWritten, user->ids.privateId, cmds->Written());
    }*/
    //stats->numReliableSkipped = numCommandsNotWritten;
    return packet->Written();
}

internal PacketStats SVP_WriteUserPacket(SimScene* sim, User* user, timeFloat time)
{
	// enqueue
	PacketStats stats = {};
	const i32 packetSize = SV_PACKET_MAX_BYTES;
	// unreliable may use whatever space is remaining, but
	// we always want to send *some* unreliable sync info.
	// so leave some space.
	const i32 reliableAllocation = SV_PACKET_RELIABLE_MAX_BYTES;
	
	// Record packet transmission for ack
	u32 packetSequence = user->acks.outputSequence++;
	Ack_RecordPacketTransmission(&user->acks, packetSequence, time);

	u8 buf[packetSize];
    ZEByteBuffer packet = Buf_FromBytes(buf, packetSize);
	
	// -- header --
	u32 ack = user->acks.remoteSequence;
    u32 ackBits = Ack_BuildOutgoingAckBits(&user->acks);
    Packet_StartWrite(&packet, user->ids.privateId, packetSequence, ack, ackBits, 0, 0, 0);
	
	// -- record packet payload and load reliable commands -- 
	TransmissionRecord* rec = Stream_AssignTransmissionRecord(
		user->reliableStream.transmissions, packetSequence);
    
    // Create sub-section for reliableBuffer
    ZEByteBuffer reliableBuf = Buf_FromBytes(
        packet.cursor, reliableAllocation);
    // Write reliable stream
    i32 reliableWritten = SVP_WriteReliableSection(
        user, &reliableBuf, rec, &sim->quantise, &stats);
    // step packet buffer forward
    packet.cursor += reliableWritten;

    // -- write mid-packet deserialise check -- 
    packet.cursor += COM_WriteI32(ZE_SENTINEL, packet.cursor);
    // fill remaining space with unreliable sync data
    i32 unreliableWritten = SVP_WriteUnreliableSection(
        sim, user, &packet, rec, &stats);
    
	// -- Finish --
    Packet_FinishWrite(&packet, reliableWritten, unreliableWritten);
    i32 total = packet.Written();
    App_SendTo(0, &user->address, buf, total);

    stats.totalBytes = total;
    stats.reliableBytes = reliableWritten;
    stats.unreliableBytes = unreliableWritten;
    return stats;
}

/*internal void SVP_WriteTestPacket()
{
    // Make a packet, no messages just a header
    u8 buf[1400];
    ZEByteBuffer b = Buf_FromBytes(buf, 1400);
    Packet_StartWrite(&b, 0, 0, 0, 0, g_ticks, g_elapsed, 0);
    b.cursor += COM_WriteI32(ZE_SENTINEL, b.cursor);
    Packet_FinishWrite(&b, 0, 0);
    i32 written = b.Written();

    // loopback address
    ZNetAddress addr = {};
    addr.port = APP_CLIENT_LOOPBACK_PORT;

    App_SendTo(0, &addr, buf, written);
}*/

internal void SVP_ReadUnreliableSection(
    User* user, ZEByteBuffer* b, QuantiseSet* quantise)
{
    u8 readBuffer[CMD_MAX_SIZE];
    u8* read = b->start;
    u8* end = b->cursor;
    i32 baseTick = COM_ReadI32(&read);
    while (read < end)
    {
        read += Cmd_Deserialise(
            quantise, read, readBuffer, CMD_MAX_SIZE, 0, baseTick);
        Command* header = (Command*)readBuffer;
        i32 err = Cmd_Validate(header);
        if (err)
        {
            APP_LOG(128, "SV read unreliable cmd failed %d\n", err);
            return;
        }
        read += header->size;
        switch (header->type)
        {
            case CMD_TYPE_C2S_INPUT:
            {
                C2S_Input* cmd = (C2S_Input*)header;
                if (cmd->userInputSequence > user->userInputSequence)
                {
                    user->userInputSequence = cmd->userInputSequence;
                }
                if (cmd->header.tick > user->latestServerTick)
                {
                    user->latestServerTick = cmd->header.tick;
                }
                Sim_SetActorInput(&g_sim, &cmd->input, user->entSerial);
            } break;

            default:
            {
                APP_LOG(64, "SV Unknown unreliable cmd type %d\n",
                    header->type);
            } break;
        }
    }
}

internal void SVP_ReadPacket(
    SysPacketEvent* ev, QuantiseSet* quantise, timeFloat time)
{
	i32 headerSize = sizeof(SysPacketEvent);
    i32 dataSize = ev->header.size - headerSize;
    u8* data = (u8*)(ev) + headerSize;
    APP_LOG(64, "SV Read %d Packet bytes from %d\n", dataSize, ev->sender.port);

    PacketDescriptor p;
    i32 err = Packet_InitDescriptor(
        &p, data, dataSize);
	if (err != ZE_ERROR_NONE)
	{
		printf("  Error %d deserialising packet\n", err);
		return;
	}
    /*printf("  Seq %d Tick %d Time %.3f\n",
        p.packetSequence,
        p.transmissionSimFrameNumber,
        p.transmissionSimTime);*/
	ev->header.type = SYS_EVENT_SKIP;

    User* user = User_FindByPrivateId(&g_users, p.id);
    if (user)
    {
        //printf("\tSV packet from user %d\n", p.id);
    }
    else
    {
        printf("SV Couldn't find user %d for packet\n", p.id);
        return;
    }
	
	// -- Ack packets + commands --
    Ack_RecordPacketReceived(&user->acks, p.packetSequence);
    u32 packetAcks[ACK_RESULTS_CAPACITY];
    i32 numPacketAcks = Ack_CheckIncomingAcks(
        &user->acks, p.ackSequence, p.ackBits, packetAcks, time);
	
    for (i32 i = 0; i < numPacketAcks; ++i)
    {
        i32 sequence = packetAcks[i];
        TransmissionRecord* rec = 
            Stream_ClearReceivedOutput(&user->reliableStream, sequence);
        Priority_UpdateSyncAcks(
            &user->entSync, sequence, rec->syncIds, rec->numSyncMessages);

    }
	// Stream_ProcessPacketAcks(
    //     &user->reliableStream, packetAcks, numPacketAcks);
	
	// -- reliable section --
	
	
	// -- unreliable section --
	ZEByteBuffer b = {};
    b.start = data + p.unreliableOffset;
    b.cursor = b.start + p.numUnreliableBytes;
    b.capacity = p.numUnreliableBytes;
    
    SVP_ReadUnreliableSection(user, &b, quantise);
}
