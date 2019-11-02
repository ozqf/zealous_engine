#pragma once
///////////////////////////////////////////////////////////
// Header only - commands shared between client and server
///////////////////////////////////////////////////////////

#include "commands_base.h"

#define CMD_MAX_SIZE 512

#define CMD_TYPE_IMPULSE 255
#define CMD_TYPE_S2C_HANDSHAKE 254
#define CMD_TYPE_S2C_SET_SCENE 253
#define CMD_TYPE_S2C_RESTORE_ENTITY 252
#define CMD_TYPE_S2C_BULK_SPAWN 251
#define CMD_TYPE_S2C_SESSION_SYNC 250
#define CMD_TYPE_C2S_INPUT 249
#define CMD_TYPE_S2C_SYNC_ENTITY 248
#define CMD_TYPE_PING 247
#define CMD_TYPE_S2C_INPUT_RESPONSE 246
#define CMD_TYPE_S2C_REMOVE_ENTITY 245
#define CMD_TYPE_S2C_REMOVE_ENTITY_GROUP 244

struct CmdPing
{
	Command header;
	i32 pingSequence;
    f32 sendTime;
};

internal void Cmd_InitPing(
    CmdPing* cmd, i32 tick, f32 time)
{
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.type = CMD_TYPE_PING;
    cmd->header.size = sizeof(CmdPing);
    cmd->sendTime = time;
}

struct S2C_Sync
{
    Command header;
    // ticks client should delay themselves by to avoid jitter
    i32 jitterTickCount;
    i32 avatarEntityId;
};

internal void Cmd_InitSync(
    S2C_Sync* cmd,
    i32 simTick,
    i32 jitterTickCount,
    i32 avatarEntityId)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, simTick);
    cmd->header.type = CMD_TYPE_S2C_SESSION_SYNC;
    cmd->header.size = sizeof(S2C_Sync);
    cmd->jitterTickCount = jitterTickCount;
    cmd->avatarEntityId = avatarEntityId;
}

//////////////////////////////////////////////////////////////////////
// Client movement sync
//////////////////////////////////////////////////////////////////////

// Server confirmation of a specific input command.
// Must only be sent in response to a client input tick,
// as the client will look this tick up to perform prediction
// and corrections
struct S2C_InputResponse
{
    Command header;
    i32 lastUserInputSequence;
    Vec3 latestAvatarPos;
};

internal void Cmd_InitInputResponse(
    S2C_InputResponse* cmd,
    i32 tick,
    i32 lastInputSequence,
    Vec3 avatarPos
)
{
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.type = CMD_TYPE_S2C_INPUT_RESPONSE;
    cmd->header.size = sizeof(S2C_InputResponse);
    cmd->lastUserInputSequence = lastInputSequence;
    cmd->latestAvatarPos = avatarPos;
}

struct C2S_Input
{
	Command header;
    i32 userInputSequence;
	SimActorInput input;
	Vec3 avatarPos;
    f32 deltaTime;
};

internal void Cmd_InitClientInput(
	C2S_Input* cmd,
    i32 userInputSequence,
	SimActorInput* input,
	Vec3* avatarPos,
	i32 tick,
    f32 deltaTime
	)
{
	*cmd = {};
	Cmd_Prepare(&cmd->header, tick);
	cmd->header.type = CMD_TYPE_C2S_INPUT;
	cmd->header.size = sizeof(C2S_Input);
    cmd->userInputSequence = userInputSequence;
    cmd->deltaTime = deltaTime;
	if (input)
	{
		cmd->input = *input;
	}
	if (avatarPos)
	{
		cmd->avatarPos = *avatarPos;
	}
}

///////////////////////////////////////////////////////////////////////////
// Individual Entity State commands
///////////////////////////////////////////////////////////////////////////
/**
 * Restore the extact client state of a specific Entity.
 * Try to use bulk spawn commands instead if possible
 */
struct S2C_RestoreEntity
{
    Command header;
    // All data that must be replicated to spawn this entity
    // have to be here. The rest is client side
    i32 networkId;
    u8 factoryType;
    Vec3 pos;
	Vec3 vel;
    f32 pitch;
    f32 yaw;
};

// TODO: Any entity specific spawning stuff here
internal void Cmd_InitRestoreEntity(
    S2C_RestoreEntity* cmd, i32 tick, SimEntity* ent)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.size = sizeof(S2C_RestoreEntity);
    cmd->header.type = CMD_TYPE_S2C_RESTORE_ENTITY;
    cmd->factoryType = (u8)ent->factoryType;
    cmd->networkId = ent->id.serial;
    cmd->pos = ent->body.t.pos;
    cmd->vel = ent->body.velocity;
    cmd->pitch = ent->body.pitch;
    cmd->yaw = ent->body.yaw;
}

struct S2C_RemoveEntity
{
    Command header;
    i32 entityId;
};

internal void Cmd_InitRemoveEntity(
    S2C_RemoveEntity* cmd, i32 tick, i32 entId)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.size = sizeof(S2C_RemoveEntity);
    cmd->header.type = CMD_TYPE_S2C_REMOVE_ENTITY;
    cmd->entityId = entId;
}

struct S2C_RemoveEntityGroup
{
    Command header;
    i32 firstId;
    u8 numIds;
};

internal void Cmd_InitRemoveEntityGroup(
    S2C_RemoveEntityGroup* cmd, i32 tick, i32 firstId, u8 numIds)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.size = sizeof(S2C_RemoveEntityGroup);
    cmd->header.type = CMD_TYPE_S2C_REMOVE_ENTITY_GROUP;
    cmd->firstId = firstId;
    cmd->numIds = numIds;

}

#define S2C_ENTITY_SYNC_TYPE_UPDATE 0
#define S2C_ENTITY_SYNC_TYPE_DEATH 1

struct S2C_EntitySync
{
	Command header;
	i32 networkId;
    u8 subType;
    union
    {
        struct
        {
            Vec3 pos;
	        Vec3 rot;
	        Vec3 vel;
            i32 targetId;
            f32 priority;
        } update;
        struct
        {
            Vec3 pos;
        } death;
        
    };
};

inline void Cmd_EntSyncSetUpdate(
    S2C_EntitySync* cmd,
    i32 tick, i32 serial, i32 targetSerial, Vec3 pos, Vec3 vel
)
{
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.type = CMD_TYPE_S2C_SYNC_ENTITY;
    cmd->header.size = sizeof(S2C_EntitySync);
    cmd->networkId = serial;
    cmd->subType = S2C_ENTITY_SYNC_TYPE_UPDATE;
	cmd->update.pos = pos;
	cmd->update.vel = vel;
    cmd->update.targetId = targetSerial;
}

internal void Cmd_WriteEntitySyncAsUpdate(
    S2C_EntitySync* cmd,
    i32 tick,
    SimEntity* ent)
{
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.type = CMD_TYPE_S2C_SYNC_ENTITY;
    cmd->header.size = sizeof(S2C_EntitySync);
    cmd->networkId = ent->id.serial;
    cmd->subType = S2C_ENTITY_SYNC_TYPE_UPDATE;
	cmd->update.pos = ent->body.t.pos;
	cmd->update.vel = ent->body.velocity;
    cmd->update.priority = ent->priority;
    cmd->update.targetId = ent->relationships.targetId.serial;
}

internal void Cmd_WriteEntitySyncAsDeath(
    S2C_EntitySync* cmd,
    i32 tick,
    i32 entitySerial)
{
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.type = CMD_TYPE_S2C_SYNC_ENTITY;
    cmd->header.size = sizeof(S2C_EntitySync);
    cmd->subType = S2C_ENTITY_SYNC_TYPE_DEATH;
    cmd->networkId = entitySerial;
}

// Return bytes written
internal i32 Cmd_EntSyncDeserialise(u8* source, u8* dest, i32 baseTick)
{
    S2C_EntitySync* cmd = (S2C_EntitySync*)dest;
    u8 type = *source; source++;
    i8 seqOffset = *source; source++;
    
    
    return sizeof(S2C_EntitySync);
}

///////////////////////////////////////////////////////////////////////////
// Bulk Entity events
///////////////////////////////////////////////////////////////////////////
#if 1
struct S2C_BulkSpawn
{
    Command header;
    SimBulkSpawnEvent def;
};

internal void Cmd_InitBulkSpawn(
    S2C_BulkSpawn* cmd,
    SimBulkSpawnEvent* event,
    i32 tick)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.size = sizeof(S2C_BulkSpawn);
    cmd->header.type = CMD_TYPE_S2C_BULK_SPAWN;
    cmd->def = *event;
}
#endif
///////////////////////////////////////////////////////////////////////////
// Connection control and sync
///////////////////////////////////////////////////////////////////////////
struct S2C_Handshake
{
    Command header;
    i32 privateId;
};

internal void Cmd_InitHandshake(
    S2C_Handshake* cmd, i32 tick, i32 privateId)
{
    *cmd = {};
    Cmd_Prepare(&cmd->header, tick);
    cmd->header.size = sizeof(S2C_Handshake);
    cmd->header.type = CMD_TYPE_S2C_HANDSHAKE;
    cmd->privateId = privateId;
}

struct CmdImpulse
{
	Command header;
	u8 impulse;
};

struct CmdSetScene
{
    Command header;
    i32 sceneId;
};

internal void Cmd_InitSetScene(
    CmdSetScene* cmd, i32 tick, i32 sceneId)
{
    *cmd = {};
    Cmd_Prepare((Command*)&cmd->header, tick);
    cmd->header.size = sizeof(CmdSetScene);
    cmd->header.type = CMD_TYPE_S2C_SET_SCENE;
    cmd->sceneId = sceneId;
}

struct CmdSetPrivateUser
{
    Command header;
    i32 connectionId;
    i32 userId;
};

struct CmdUserState
{
    Command header;
    i32 userId;
    i32 state;
};
