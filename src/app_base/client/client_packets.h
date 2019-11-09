#pragma
/*
Read/Write packets for client
*/
#include "../../ze_common/ze_common_full.h"
#include "../packet.h"
#include "../../sys_events.h"

internal void CL_LogCommandBuffer(ZEByteBuffer* b, char* label)
{
    APP_LOG(64, "CL %s contents (%d bytes)\n", label, b->Written());
    u8* read = b->start;
    u8* end = b->cursor;
    while (read < end)
    {
        Command* cmd = (Command*)read;
        i32 err = Cmd_Validate(cmd);
        if (err != ZE_ERROR_NONE)
        {
            APP_LOG(64, "  cmd validate failed with code %d\n", err);
            return;
        }
        read += cmd->size;
        APP_LOG(128, "  Type %d Tick %d Size %d\n",
            cmd->type, cmd->tick, cmd->size);
    }
}

/**
 * Input passed in may be null
 */
internal i32 CL_WriteUnreliableSection(
    QuantiseSet* quantise, ZEByteBuffer* packet, C2S_Input* userInput)
{
    u8* start = packet->cursor;
    // Write header
    packet->cursor += COM_WriteI32(CL_GetServerTick(), packet->cursor);
    // Send ping
	CmdPing ping = {};
	// TODO: Stream enqueue will set the sequence for us
	// so remove sending 0 here.
	Cmd_Prepare(&ping.header, CL_GetServerTick());
	ping.sendTime = g_elapsed;
    packet->cursor += Cmd_Serialise(
        quantise, packet->cursor, &ping.header, 0);
	
    if (userInput != NULL)
    {
        APP_LOG(64, "CL Write input\n");
        packet->cursor += Cmd_Serialise(
            quantise, packet->cursor, &userInput->header, 0);
    }
	
    return (packet->cursor - start);
}

/**
 * Input passed in may be null
 */
internal void CL_WritePacket(
    QuantiseSet* quantise, timeFloat time, C2S_Input* userInput)
{
    AppTimer timer(APP_STAT_CL_OUTPUT, g_sim.tick);
    #if 1
	//printf("CL Write packet for user %d\n", g_ids.privateId);
	//Stream_EnqueueOutput(&user->reliableStream, &ping.header);
	
	// enqueue
	//ZEByteBuffer* buf = App_GetLocalClientPacketForWrite();
	
	u8 buf[1400];
    ZEByteBuffer packet = Buf_FromBytes(buf, 1400);
    u32 packetSequence = g_acks.outputSequence++;
	Ack_RecordPacketTransmission(&g_acks, packetSequence, time);
	
    u32 ack = g_acks.remoteSequence;
    u32 ackBits = Ack_BuildOutgoingAckBits(&g_acks);
    Packet_StartWrite(&packet, g_ids.privateId, packetSequence, ack, ackBits, 0, 0, 0);
	
	//TransmissionRecord* rec = Stream_AssignTransmissionRecord(
	//	g_reliableStream.transmissions, packetSequence);
    packet.cursor += COM_WriteI32(ZE_SENTINEL, packet.cursor);
    i32 unreliableWritten = CL_WriteUnreliableSection(
        quantise, &packet, userInput);
    Packet_FinishWrite(&packet, 0, unreliableWritten);
    i32 total = packet.Written();
	
    App_SendTo(0, &g_serverAddress, buf, total);
    
	//Packet_WriteFromStream(
    //    &user->reliableStream,
    //    &user->unreliableStream,
    //    buf, 1400, g_elapsed, g_ticks, 0);
    #endif
}

internal i32 CL_ReadPacketUnreliableInput(
    ZEByteBuffer* buf, NetStream* stream, QuantiseSet* quantise)
{
    //CL_LogCommandBuffer(buf, "Unreliable packet section");
    u8* read = buf->start;
    u8* end = buf->cursor;
    u8 readBuffer[CMD_MAX_SIZE];
    APP_LOG(128, "CL Reading %d unreliable bytes\n", (end -read));
    i32 baseTick = COM_ReadI32(&read);
    i32 cmdsRead = 0;
    while (read < end)
    {
        read += Cmd_Deserialise(
            quantise, read, readBuffer, CMD_MAX_SIZE, 0, baseTick);
        Command* h = (Command*)readBuffer;
        i32 err = Cmd_Validate(h);
        if (err != ZE_ERROR_NONE)
        {
            return err;
        }
        cmdsRead++;
        i32 wrote = Stream_EnqueueUnreliableInput(stream, h);
        if (wrote)
        {
            //APP_LOG(128, "CL Enqueue unreliable type %d size %d\n",
            //    h->type, wrote);
        }
    }
    return ZE_ERROR_NONE;
}

internal i32 CL_ReadPacketReliableInput(
    ZEByteBuffer* buf, NetStream* stream, QuantiseSet* quantise)
{
    u8* read = buf->start;
    u8* end = buf->cursor;
    u8 readBuffer[CMD_MAX_SIZE];
    i32 cmdsRead = 0;
    // Read sequence that cmds will offset from
    CmdSeq baseSequence = Cmd_ReadSequence(&read);
    while (read < end)
    {
        read += Cmd_Deserialise(
            quantise, read, readBuffer, CMD_MAX_SIZE, baseSequence, 0);
        Command* h = (Command*)readBuffer;
        ErrorCode err = Cmd_Validate(h);
        ZE_ASSERT(err == ZE_ERROR_NONE, "Invalid cmd")
        
        cmdsRead++;
        Stream_EnqueueReliableInput(stream, h);
    }
    return ZE_ERROR_NONE;
}

internal i32 CL_ReadPacket(
    SysPacketEvent* ev,
    NetStream* reliableStream,
    NetStream* unreliableStream,
    QuantiseSet* quantise,
    timeFloat time)
{
    // -- Descriptor --
    i32 headerSize = sizeof(SysPacketEvent);
    i32 dataSize = ev->header.size - headerSize;
    u8* data = (u8*)(ev) + headerSize;
    //printf("CL %d Packet bytes from %d\n", dataSize, ev->sender.port);
	
    PacketDescriptor p;
    i32 err = Packet_InitDescriptor(
        &p, data, dataSize);
	if (err != ZE_ERROR_NONE)
	{
		printf("  Error %d deserialising packet\n", err);
        ZE_ASSERT(0, "Packet Deserialise failed");
		return ZE_ERROR_DESERIALISE_FAILED;
	}
    /*printf("  Seq %d Tick %d Time %.3f\n",
        p.packetSequence,
        p.transmissionSimFrameNumber,
        p.transmissionSimTime);*/
    
    // -- ack --
    Ack_RecordPacketReceived(&g_acks, p.packetSequence);
	u32 packetAcks[ACK_RESULTS_CAPACITY]; 
	i32 numPacketAcks = Ack_CheckIncomingAcks(
		&g_acks, p.ackSequence, p.ackBits, packetAcks, time
	);
    for (i32 i = 0; i < numPacketAcks; ++i)
    {
        Stream_ClearReceivedOutput(
            reliableStream, packetAcks[i]);
    }
	//Stream_ProcessPacketAcks(reliableStream, packetAcks, numPacketAcks);
    
    // -- reliable section --
    // TODO: Put this byte buffer on the descriptor and initialise it there
    // to save all this faff in multiple places
    ZEByteBuffer reliableSection = {};
    //printf("  Reliable bytes: %d\n", p.numReliableBytes);
    reliableSection.start = data + Packet_GetHeaderSize();
    reliableSection.cursor = reliableSection.start + p.numReliableBytes;
    reliableSection.capacity = p.numReliableBytes;
    CL_ReadPacketReliableInput(&reliableSection, reliableStream, quantise);
    //Cmd_PrintBuffer(reliableSection.start, reliableSection.Written());
	
	// -- unreliable section --
    ZEByteBuffer unreliableSection = {};
    unreliableSection.start = data + p.unreliableOffset;
    unreliableSection.cursor =
        unreliableSection.start +
        p.numUnreliableBytes;
    err = CL_ReadPacketUnreliableInput(
        &unreliableSection, unreliableStream, quantise);
    if (err)
    {
        APP_PRINT(64, "CL err %d reading unreliable", err) 
    }

    return ZE_ERROR_NONE;
}
