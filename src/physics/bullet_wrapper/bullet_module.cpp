#ifndef ZPHYSICS_MODULE_CPP
#define ZPHYSICS_MODULE_CPP

#include <stdio.h>
#include "../../../headers/common/ze_common_full.h"
// header, contains data structures and function definitions
#include "../physics.h"
/**
 * !NO BULLET PHYSICS LIBRARY ABOVE THIS POINT!
 * 
 */
#include "../../../lib/bullet/btBulletCollisionCommon.h"
#include "../../../lib/bullet/btBulletDynamicsCommon.h"

#include "../../../lib/bullet/BulletCollision/CollisionDispatch/btGhostObject.h"
//#include "btGhostObject.h"

// For debug output ONLY
#include <windows.h>

// Internal data structures used to interact with Bullet Physics
//#include "physics_internal_types.h"
#include "bullet_internal_interface.h"

// Global variables used by the rest of the internal system
#include "bullet_globals.h"

// Implement public interface

#define PHYS_DEFAULT_FRICTION 1.0
#define PHYS_DEFAULT_RESTITUTION 0.0

// Handling public interface
#include "bullet_external.h"

#include "bullet_query.h"
#include "bullet_factory.h"
#include "bullet_overlaps.h"
#include "bullet_main.h"

#endif