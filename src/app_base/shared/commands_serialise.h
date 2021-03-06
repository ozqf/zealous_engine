#pragma once

#include "commands.h"
#include "../../../headers/common/ze_bitpack.h"
#include "../../../headers/common/ze_memory_utils.h"

// Returns bytes written
internal i32 Cmd_Serialise(
    QuantiseSet* quantise,
    u8* write,
    Command* h,
    CmdSeq sequenceOffset)
{
    const u8* bufStart = write;
    switch (h->type)
    {
        case CMD_TYPE_S2C_SYNC_ENTITY:
        {
            #ifdef SIM_QUANTISE_SYNC
            S2C_EntitySync* cmd = (S2C_EntitySync*)h;
            // Command type
            *write = h->type; write++;
            // Sub-type
            *write = cmd->subType; write++;
            if (cmd->subType == S2C_ENTITY_SYNC_TYPE_DEATH)
            {
                write += COM_WriteI32(cmd->networkId, write);
                return (write - bufStart);
            }
            else
            {
                // Network Id, targetId, pos, vel
                write += COM_WriteI32(cmd->networkId, write);
                write += COM_WriteI32(cmd->update.targetId, write);

                write += COM_WriteU16((u16)COM_QuantiseF2I(
                    cmd->update.pos.x, &quantise->pos), write);
                write += COM_WriteU16((u16)COM_QuantiseF2I(
                    cmd->update.pos.y, &quantise->pos), write);
                write += COM_WriteU16((u16)COM_QuantiseF2I(
                    cmd->update.pos.z, &quantise->pos), write);
                
                write += COM_WriteU16(
                    (u16)COM_QuantiseF2I(cmd->update.vel.x, &quantise->vel), write);
                write += COM_WriteU16(
                    (u16)COM_QuantiseF2I(cmd->update.vel.y, &quantise->vel), write);
                write += COM_WriteU16(
                    (u16)COM_QuantiseF2I(cmd->update.vel.z, &quantise->vel), write);
                return (write - bufStart);
            }
            #else
            return ZE_COPY_STRUCT(h, bufStart, S2C_EntitySync);
            #endif
        } break;
        case CMD_TYPE_S2C_BULK_SPAWN:
        {
            #ifdef SIM_QUANTISE_SPAWNS
            
            /*
            1 u8 type
            1 u8 sequenceOffset
            4 i32 tick
            4 i32 firstSerial
            4 i32 sourceSerial
            2 u16 x
            2 u16 y
            2 u16 z
            4 i32 forwardNormal
            4 i32 leftNormal
            4 i32 upNormal
            2 u16 radius
            1 u8 patternIndex
            1 u8 numItems
            1 u8 seedIndex
            */
            S2C_BulkSpawn* cmd = (S2C_BulkSpawn*)h;
            // cmdType, seqOffset, tick, firstSerial, source serial
            *write = CMD_TYPE_S2C_BULK_SPAWN; write++;
            *write = (u8)sequenceOffset; write++;
            write += COM_WriteI32(cmd->header.tick, write);
            write += COM_WriteI32(cmd->def.base.firstSerial, write);
            write += COM_WriteI32(cmd->def.base.sourceSerial, write);

            // Position
            Vec3* pos = &cmd->def.base.xForm.pos;
            write += COM_WriteU16((u16)COM_QuantiseF2I(
                pos->x, &quantise->pos), write);
            write += COM_WriteU16((u16)COM_QuantiseF2I(
                pos->y, &quantise->pos), write);
            write += COM_WriteU16((u16)COM_QuantiseF2I(
                pos->z, &quantise->pos), write);

            // Rotation - 3 packed normals
            Vec3* forward = &cmd->def.base.xForm.rotation.zAxis;
            Vec3* left = &cmd->def.base.xForm.rotation.xAxis;
            Vec3* up = &cmd->def.base.xForm.rotation.yAxis;

            i32 packedForward = ZE_PackVec3NormalToI32(forward->parts);
            i32 packedLeft = ZE_PackVec3NormalToI32(left->parts);
            i32 packedUp = ZE_PackVec3NormalToI32(up->parts);
            write += COM_WriteI32(packedForward, write);
            write += COM_WriteI32(packedLeft, write);
            write += COM_WriteI32(packedUp, write);

            // printf("Serialised transform\n");
            // Transform_Printf(&cmd->def.base.xForm);

            // radius etc...
            write += COM_WriteU16((u16)COM_QuantiseF2I(
                cmd->def.patternDef.radius, &quantise->pos), write);
            write += COM_WriteU16((u16)COM_QuantiseF2I(
                cmd->def.patternDef.arc, &quantise->rot), write);
            
            // patternId, factoryType, numItems, seedIndex
            *write = (u8)cmd->def.patternDef.patternId; write++;
            *write = (u8)cmd->def.factoryType; write++;
            *write = (u8)cmd->def.patternDef.numItems; write++;
            *write = cmd->def.base.seedIndex; write++;
            write += COM_WriteU16((u16)0xBEEF, write);
            //printf("Serialised %d of bulk spawn data\n", write - bufStart);
           return (write - bufStart);
           #else
           return ZE_COPY_STRUCT(h, write, S2C_BulkSpawn);
           #endif
        } break;

        case CMD_TYPE_IMPULSE:
        return ZE_COPY_STRUCT(h, write, CmdImpulse);
        // Just dumb copy the bytes
        default:
        return ZE_COPY(h, write, h->size);
    }
}
