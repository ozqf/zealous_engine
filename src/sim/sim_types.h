#pragma once

#include "../ze_common/ze_common.h"
#include "../ze_common/ze_bitpack.h"
#include "../zr_draw_types.h"
#include "../physics/physics.h"

#define ACTOR_BASE_SPEED 12.f
#define ACTOR_MOVE_PUSH_SPEED 120.f
#define ACTOR_EVADE_SPEED 30
#define ACTOR_EVADE_SECONDS 0.25f
#define ACTOR_EVADE_RESET_SECONDS 0.5f

#define SIM_PLAYER_PROJECTILE_SPEED 70.f
#define SIM_PLAYER_SHOTGUN_PELLETS 20

// #define SIM_PLAYER_PROJECTILE_SPEED 30.f
// #define SIM_PLAYER_SHOTGUN_PELLETS 4

#define ACTOR_INPUT_MOVE_FORWARD (1 << 0)
#define ACTOR_INPUT_MOVE_BACKWARD (1 << 1)
#define ACTOR_INPUT_MOVE_LEFT (1 << 2)
#define ACTOR_INPUT_MOVE_RIGHT (1 << 3)
#define ACTOR_INPUT_MOVE_UP (1 << 4)
#define ACTOR_INPUT_MOVE_DOWN (1 << 5)
#define ACTOR_INPUT_ATTACK (1 << 6)
#define ACTOR_INPUT_ATTACK2 (1 << 7)
#define ACTOR_INPUT_MOVE_SPECIAL1 (1 << 8)
// inventory slots
#define ACTOR_INPUT_SLOT_1 (1 << 9)
#define ACTOR_INPUT_SLOT_2 (1 << 10)
#define ACTOR_INPUT_SLOT_3 (1 << 11)
#define ACTOR_INPUT_SLOT_4 (1 << 12)

#define ACTOR_INPUT_SHOOT_LEFT (1 << 27)
#define ACTOR_INPUT_SHOOT_RIGHT (1 << 28)
#define ACTOR_INPUT_SHOOT_UP (1 << 29)
#define ACTOR_INPUT_SHOOT_DOWN (1 << 30)
struct SimActorInput
{
    u32 buttons;
	u32 prevButtons;
    Vec3 degrees;

    i32 HasBitToggledOff(i32 bit)
    {
        return ((buttons & bit) == 0
            && (prevButtons & bit) != 0
        );
    }
};

#pragma pack(push, 1)
struct SimSaveFileInfo
{
    i32 infoOffset;

    i32 entsOffset;
    i32 numEnts;

    i32 players;
    i32 numPlayers;

    i32 read;
    i32 numReadBytes;

    i32 write;
    i32 numWriteBytes;

    // data not related to the Sim itself:
    i32 server;
    i32 numServerBytes;

    i32 client;
    i32 numClientBytes;

    i32 sentinel;
};
#pragma pack(pop)

struct SimScene;
struct SimEntity;
typedef void (*SimEnt_Tick)(SimScene* sim, SimEntity* ent, timeFloat deltaTime, i32 bIsServer);

struct SimAvoidInfo
{
    Vec3 dir;
    i32 numNeighbours;
};

struct SimInventoryItem
{
    char* name;
    char* model;
    char* skin;
    i32 eventType;      // what happens
    u8 factoryType;    // if spawning something, what type?
    i32 eventCount;     // eg projectile count
    f32 duration;       // eg refire time
};

#pragma pack(push, 1)
union SimEntIndex
{
    struct
    {
        u16 iteration;
        u16 index;
    };
    u16 arr[2];
    u32 value;
};
#pragma pack(pop)

// Note: A serial of 0 should be considered null
// and that this Entity Id relates to no entity, even if
// the slot is set.
struct SimEntId
{
    // Location in local entity memory
    // Will differ between/ client and server!
    SimEntIndex slot;
    // unique, replicated Id.
    i32 serial;
};

#define SIM_DISPLAY_FLAG_DISABLED (1 << 0)

struct SimEntDisplay
{
    u32 flags;
    Vec3 scale;
    Colour colourA;
    Colour colourB;
    ZRDrawObjData data;
};

#define SIM_ENT_MOVE_BIT_GROUNDED (1 << 0)
#define SIM_ENT_MOVE_BIT_BOUNDARY_BOUNCE (1 << 1)
#define SIM_ENT_MOVE_BIT_IGNORE_BOUNDARY (1 << 2)

struct SimEntMovement
{
    f32 speed;
    Vec3 velocity;
    i32 moveMode;
    // current condition flags eg grounded
    i32 flags;
    // for recording time-limits for specific movements, eg evading
    timeFloat moveTime;
    // Move is either the direction an AI wants to move in
    // or an abritrary position this entity is moving toward
    // depending on mode
    Vec3 move;
};

// Fields replicated to clients
struct SimEntSyncData
{
    Vec3 pos;
	Vec3 rot;
	Vec3 vel;
    i32 targetId;
    i32 tickType;
    timeFloat thinkTime;
};

struct SimEntBody
{
    Transform t;
    Vec3 previousPos;
    Vec3 error;
    f32 errorRate;
    f32 pitchDegrees;
    f32 yawDegrees;
    // use this to store the default scale of an entity, as the transform
    // scale may be changed by game logic
    Vec3 baseHalfSize;
};

struct SimEntRelationships
{
    // TODO: For the client only the serial of targetid is safe.
    // Slot is not!!
    SimEntId targetId;  // current enemy // REPLICATED
    SimEntId parentId;  // Who spawned this entity
    simFactoryType childFactoryType;
    i32 childSpawnCount;
    i32 liveChildren;
    i32 maxLiveChildren;
    i32 totalChildren;
    u8 patternType;
};

struct SimEntTiming
{
    frameInt lastThink;
    frameInt nextThink;
    // even if an ent was technically born in the current tick
    // birth tick may be in the past for lag compensation,
    frameInt birthTick;
    // the real, uncompensated birth tick
    frameInt realBirthTick;
    // This entity was spawned in the past this many ticks ago
    // and needs to catch up
	frameInt fastForwardTicks;
};

struct SimEntHealth
{
    i32 health;
    i32 healthMax;
    i32 healthOverchargeMax;
    i32 stunThreshold;
    timeFloat stunDuration;
};

struct SimEntThink
{
    i32 tickType; // REPLICATED
    i32 coreTickType; // the default tick type to return to DO NOT CHANGE!
    i32 subMode; // internal state within current ticker
};

// TODO - breakup into ECS...?
struct SimEntity
{
    i32 status;
    i32 isLocal;
    SimEntId id;
	i32 playerId;	// if a player owns this
    u8 teamId;
    
    simFactoryType factoryType; // identifies 'class' of entity. DO NOT CHANGE!
    SimEntThink think;
    //ZShapeDef shape;

    SimEntRelationships relationships;
    
    SimEntTiming timing;

    timeFloat attackTick;
    timeFloat attackTime;

    SimEntDisplay display;
    i32 lightType;
    u8 deathType;

    u32 flags;
    u32 localFlags;

    SimEntMovement movement;
    
    // physical
    SimEntBody body;
    
    SimEntHealth life;
    
	SimActorInput input;

    struct
    {
        i32 index;
        i32 pendingIndex;
    } inventory;
    

    i32 touchDamage;

    f32 priority;
    f32 basePriority;
    struct
    {
        i32 lastSync;
    } clientOnly;
};

struct SimSpawnPatternItem
{
	i32 entSerial;
	Vec3 pos;
	Vec3 forward;
};

struct SimSpawnPatternDef
{
	i32 patternId;
	i32 numItems;
	f32 radius;
    f32 arc;
};

struct SimProjectileType
{
    // Characterics of each projectile
    f32 speed;
    f32 lifeTime;
    f32 arcDegrees;

    ColourU32 colour;
    Vec3 scale;

    // Characterics of how this projectile is spawned
    SimSpawnPatternDef patternDef;
};

#if 0
// For block allocated entity storage
struct SimEntBlock
{
    i32 index;
    SimEntity* ents;
    i32 capacity;
};
#endif

struct SimRaycastResult
{
    f32 fraction;
    Vec3 hitPos;
    Vec3 normal;
    f32 dist;
    SimEntity* ent;
};

#define SIM_MAX_PLAYERS 4

struct SimPlayer
{
	i32 id;
	i32 state;
    i32 lastStateChangeTick;
	i32 avatarId;
    SimActorInput input;
};

#define SIM_SCENE_BIT_IS_SERVER (1 << 0)
#define SIM_SCENE_BIT_IS_CLIENT (1 << 1)

/**
 * Data external to the sim instance
 */
struct SimSceneData
{
    SimEntity* ents;    
    SimPlayer players[SIM_MAX_PLAYERS];
	// physics
    WorldHandle* world;
    ZRAssetDB* db;
    
    // pointer to buffer for game logic commands from the current tick
    ZEBuffer* outputBuf;
    ZEBuffer* soundOutputBuf;
};

/**
 * Data internal to the sim instance
 * All data in this struct should be within the struct
 * No pointers!
 */
struct SimSceneInfo
{
    i32 tickRate;
    
    i32 maxEnts;
    u32 flags;
    i32 gameRules;
    // Ent Id of local player's avatar
    // required so it can be hidden during draw, and so that projectiles
    // the player fires can be hidden for a few frames
    i32 localAvatarId;

    i32 gameState;
    // count of players actually in-game
    i32 numActivePlayers;
    
    // sequential, unrelated to blocks
    i32 remoteEntitySequence;
    i32 localEntitySequence;
    //i32 cmdSequence;
    i32 tick;
    timeFloat time;
	
	timeFloat timeInAABBSearch;

    // for client. server has remote sequence anyway
    i32 highestAssignedSequence;

    Vec3 groundOrigin;
    Vec3 groundNormal;
    Vec3 gravity;

    Vec3 boundaryMin;
    Vec3 boundaryMax;

	i32 nextPlayerId;
	i32 maxPlayers;
    Vec3 playerStartPos;
	
    Transform observePos;
    Vec3 observeTarget;

    QuantiseSet quantise;

    i32 bVerbose;

    char mapName[Z_MAX_PATH];
};

struct SimScene
{
    SimSceneData data;
    SimSceneInfo info;
};
