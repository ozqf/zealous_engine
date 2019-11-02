#ifndef SERVER_DEBUG_H
#define SERVER_DEBUG_H

#include "server.cpp"

internal void SV_PrintMsgSizes()
{
    APP_PRINT(64, "SV sizeof ping: %d\n",
        sizeof(CmdPing))
    APP_PRINT(64, "SV sizeof Command: %d\n",
        sizeof(Command))
    APP_PRINT(64, "SV sizeof S2C_EntitySync: %d\n",
        sizeof(S2C_EntitySync))
    APP_PRINT(64, "SV sizeof S2C_BulkSpawn: %d\n",
        sizeof(S2C_BulkSpawn))
    APP_PRINT(64, "SV sizeof S2C_RestoreEntity: %d\n",
        sizeof(S2C_RestoreEntity))
}

void SV_WriteDebugString(CharBuffer* str)
{
    //char* chars = str->chars;
    //i32 written = 0;
    if (g_debugFlags & SV_DEBUG_TIMING)
    {
        str->cursor += sprintf_s(str->cursor, str->Space(),
            "SERVER:\nTick: %d\nElapsed: %.3f\nMax Rate %d\nNext remote ent: %d\n",
            g_sim.tick, g_elapsed, g_maxSyncRate, g_sim.remoteEntitySequence
        );
    }
    
    // TODO: Just showing local user for now
    ZNetAddress addr = {};
    addr.port = APP_CLIENT_LOOPBACK_PORT;
    User* user = User_FindByAddress(&g_users, &addr);
    if (user && (g_debugFlags & SV_DEBUG_USER_BANDWIDTH))
    {
        AckStream* acks = &user->acks;
        
        // title
        str->cursor += sprintf_s(
            str->cursor,
            str->Space(),
            "-- Local Client %d --\n",
            user->ids.privateId
		);
        // Bandwidth
        if (g_debugFlags & SV_DEBUG_USER_BANDWIDTH)
        {
            #if 1
            StreamStats stats;
            User_SumPacketStats(user, &stats);
            if (stats.numPackets > 0)
            {
                i32 kbpsTotal = (stats.totalBytes * 8) / 1024;
                i32 reliableKbps = (stats.reliableBytes * 8) / 1024;
                i32 unreliableKbps = (stats.unreliableBytes * 8) / 1024;
                f32 lossEstimate = Ack_EstimateLoss(&user->acks);
                ReliableCmdQueueStats queueStats = Stream_CountCommands(
                    &user->reliableStream.outputBuffer);
                str->cursor += sprintf_s(
                    str->cursor,
                    str->Space(),
                    "-Bandwidth -\nRate: %d\nPer Second: %dkbps (%.3f KB)\nReliable: %d kbps\nUnreliable: %d kbps\nLoss %.1f%%\nEnqueued: %d (%d Bytes)\nSequence rage %d\n",
                    user->syncRateHertz,
                    kbpsTotal,
                    (f32)stats.totalBytes / 1024.0f,
                    reliableKbps,
                    unreliableKbps,
                    lossEstimate,
                    queueStats.count,
                    user->reliableStream.outputBuffer.Written(),
                    queueStats.highestSeq - queueStats.lowestSeq
		        );
                #if 1
                // Sequencing/jitter
                str->cursor += sprintf_s(
                    str->cursor,
                    str->Space(),
                    "- Latency -\nOutput seq: %d\nAck Seq: %d\nDelay: %.3f\nJitter: %.3f\n",
                    acks->outputSequence,
                    acks->remoteSequence,
                    user->ping,
		        	user->jitter
                );
                #endif
                i32 numLinks = user->entSync.numLinks;
                i32 numDeadLinks = Priority_TallyDeadLinks(
                    user->entSync.links, user->entSync.numLinks);
                str->cursor += sprintf_s(
                    str->cursor,
                    str->Space(),
                    "- Cmds/Sync Links -\nLinks %d\nDead Links %d\nLifetime max %.3f\nCurrent max %.3f\n--Per packet averages--\nReliable Cmds %d\nUnreliable Cmds %d\nPacket Size %d\n",
                    numLinks,
                    numDeadLinks,
                    user->entSync.highestMeasuredPriority,
                    user->entSync.currentHighest,
                    stats.numReliableMessages / stats.numPackets,
                    stats.numUnreliableMessages / stats.numPackets,
                    stats.totalBytes / stats.numPackets
		        );
                
                str->cursor += sprintf_s(
                    str->cursor,
                    str->Space(),
                    "--Last second totals --\nReliable Cmds %d\nUnreliable Cmds %d\n",
                    stats.numReliableMessages,
                    stats.numUnreliableMessages
		        );
            }
            for (i32 i = 0; i < 256; ++i)
            {
                i32 count = stats.commandCounts[i];
                if (count == 0) { continue; }
                str->cursor += sprintf_s(
                    str->cursor,
                    str->Space(),
                    "\tType %d: %d\n",
                    i, count);
            }
            #endif
            #if 0
		    // currently overflows debug text buffer:
            for (i32 j = 0; j < ACK_CAPACITY; ++j)
            {
                AckRecord* rec = &acks->awaitingAck[j];
                if (rec->acked)
                {
                    f32 time = rec->receivedTime - rec->sentTime;
                    written += sprintf_s(chars + written, str->maxLength - written,
                        "%.3f Sent: %.3f Rec: %.3f\n", time, rec->sentTime, rec->receivedTime
                    );
                }
            }
		    #endif
        }
        
    }
    else
    {
        str->cursor += sprintf_s(
            str->cursor,
            str->Space(),
            "No local client found\n");
    }
}


#endif // SERVER_DEBUG_H
