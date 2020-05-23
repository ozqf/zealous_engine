#pragma once

#include "commands.h"
#include "../../ze_common/ze_bitpack.h"
#include "../../ze_common/ze_memory_utils.h"

// returns bytes read
internal i32 Cmd_Deserialise(
    QuantiseSet* quantise,
    u8* read,
    u8* buffer,
    i32 capacity,
    CmdSeq baseSequence,
    i32 baseTick)
{
    const u8* source = read;
    // If dumb copying the bytes, don't step forward yet
    u8 type = *source;
    //u8* read = source;
    switch (type)
    {
        case CMD_TYPE_S2C_SYNC_ENTITY:
        {
            #ifdef SIM_QUANTISE_SYNC
            // Prepare result
            S2C_EntitySync* cmd = (S2C_EntitySync*)buffer;
            *cmd = {};
            // Step over type
            read++;
            // read sub-type
            u8 subType = *read; read++;
            if (subType == S2C_ENTITY_SYNC_TYPE_DEATH)
            {
                i32 serial = COM_ReadI32(&read);
                Cmd_WriteEntitySyncAsDeath(cmd, baseTick, serial);
                i32 totalRead = (read - source);
                //xprintf("CMD read %d bytes for death\n", written);
                return totalRead;
            }
            else if (subType == S2C_ENTITY_SYNC_TYPE_UPDATE)
            {
                //i32 totalRead = ZE_COPY_STRUCT(source, buffer, S2C_EntitySync);
                //return totalRead;

                i32 serial = COM_ReadI32(&read);
                i32 targetSerial = COM_ReadI32(&read);
                Vec3 pos = {};
                pos.x = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->pos);
                pos.y = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->pos);
                pos.z = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->pos);
                Vec3 vel = {};
                vel.x = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->vel);
                vel.y = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->vel);
                vel.z = COM_DequantiseI2F(
                    COM_ReadU16(&read), &quantise->vel);
                
                Cmd_ReadEntSyncUpdate(cmd,
                    baseTick, serial, targetSerial, pos, vel);

                return (read - source);
            }
            else
            {
                //return ZE_COPY_STRUCT(source, buffer, S2C_EntitySync);
                ZE_ASSERT(0, "Unknown entity sync type");
                return 0;
            }
            #else
            return ZE_COPY_STRUCT(source, buffer, S2C_EntitySync);
            #endif
        } break;
        case CMD_TYPE_S2C_BULK_SPAWN:
        {
            #ifdef SIM_QUANTISE_SPAWNS
            // cmdType, seqOffset, tick, firstSerial, source serial
            u8 cmdType = *read; read++;
            u8 seqOffset = *read; read++;
            i32 tick = COM_ReadI32(&read);
            i32 firstSerial = COM_ReadI32(&read);
            i32 sourceSerial = COM_ReadI32(&read);

            // Position
            Transform t;
            Transform_SetToIdentity(&t);
            
            t.pos.x = COM_DequantiseI2F(
                COM_ReadU16(&read), &quantise->pos);
            t.pos.y = COM_DequantiseI2F(
                COM_ReadU16(&read), &quantise->pos);
            t.pos.z = COM_DequantiseI2F(
                COM_ReadU16(&read), &quantise->pos);
            
            // Rotation
            i32 packedNormal = COM_ReadI32(&read);
            i32 packedLeft = COM_ReadI32(&read);
            i32 packedUp = COM_ReadI32(&read);
            
            t.rotation.zAxis = ZE_UnpackVec3Normal(
                packedNormal);
            t.rotation.xAxis = ZE_UnpackVec3Normal(
                packedLeft);
            t.rotation.yAxis  = ZE_UnpackVec3Normal(
                packedUp);
            
            printf("Deserialised transform\n");
            Transform_Printf(&t);

            // radius etc...
            f32 radius = COM_DequantiseI2F(
                COM_ReadU16(&read), &quantise->pos);
            f32 arc = COM_DequantiseI2F(
                COM_ReadU16(&read), &quantise->rot);
            
            // patternId, factoryType, numItems, seedIndex
            u8 patternId = *read; read++;
            u8 factoryType = *read; read++;
            u8 numItems = *read; read++;
            u8 seedIndex = *read; read++;

            u16 sentinel = COM_ReadU16(&read);
            ZE_ASSERT(sentinel == 0xBEEF, "Bulk spawn deserialise failed")

            // Build command
            SimBulkSpawnEvent ev = {};
            Sim_SetBulkSpawn(&ev, firstSerial, sourceSerial,
                t, tick, factoryType, patternId, numItems,
                seedIndex, radius, arc);

            S2C_BulkSpawn* cmd = (S2C_BulkSpawn*)buffer;
            *cmd = {};
            Cmd_InitBulkSpawn(cmd, &ev, tick);
            // TODO: Automate set sequence here somehow?
            cmd->header.sequence = baseSequence + seqOffset;
            //printf("Deserialised Bulk spawn seq %d size %d\n",
            //    cmd->header.sequence, read - source);
            //ZE_ASSERT(0, "foo")
            return (read - source);
            #else
            return ZE_COPY_STRUCT(source, buffer, S2C_BulkSpawn);
            #endif
        } break;
        case CMD_TYPE_IMPULSE:
        return ZE_COPY_STRUCT(source, buffer, CmdImpulse);
        break;
        case CMD_TYPE_S2C_HANDSHAKE:
        return ZE_COPY_STRUCT(source, buffer, S2C_Handshake);
        break;
        case CMD_TYPE_S2C_SET_SCENE:
        return ZE_COPY_STRUCT(source, buffer, CmdSetScene);
        break;
        case CMD_TYPE_S2C_RESTORE_ENTITY:
        return ZE_COPY_STRUCT(source, buffer, S2C_RestoreEntity);
        break;
        case CMD_TYPE_S2C_SESSION_SYNC:
        return ZE_COPY_STRUCT(source, buffer, S2C_Sync);
        break;
        case CMD_TYPE_C2S_INPUT:
        return ZE_COPY_STRUCT(source, buffer, C2S_Input);
        break;
        case CMD_TYPE_PING:
        return ZE_COPY_STRUCT(source, buffer, CmdPing);
        break;
        case CMD_TYPE_S2C_INPUT_RESPONSE:
        return ZE_COPY_STRUCT(source, buffer, S2C_InputResponse);
        break;
        case CMD_TYPE_S2C_REMOVE_ENTITY:
        return ZE_COPY_STRUCT(source, buffer, S2C_RemoveEntity);
        break;
        case CMD_TYPE_S2C_REMOVE_ENTITY_GROUP:
        return ZE_COPY_STRUCT(source, buffer, S2C_RemoveEntityGroup);
        break;
        default:
        // Don't know size of command automatically, so
        // just give up
        ZE_ASSERT(0, "No read function for command");
        break;
    }
    return 0;
}
