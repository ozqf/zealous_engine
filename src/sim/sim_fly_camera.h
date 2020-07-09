#include "sim_internal.h"

extern "C" void Sim_TickDebugCamera(Transform* t, SimActorInput input, f32 moveSpeed, timeFloat delta)
{
	
    // Apply Rotate
    M3x3_SetToIdentity(t->rotation.cells);
    M3x3_RotateY(t->rotation.cells, input.degrees.y * DEG2RAD);
    M3x3_RotateX(t->rotation.cells, input.degrees.x * DEG2RAD);

	Vec3 dirInput = {};
	if (input.buttons & ACTOR_INPUT_MOVE_FORWARD)
	{
		dirInput.z -= 1;
	}
	if (input.buttons & ACTOR_INPUT_MOVE_BACKWARD)
	{
		dirInput.z += 1;
	}
	if (input.buttons & ACTOR_INPUT_MOVE_LEFT)
	{
		dirInput.x -= 1;
	}
	if (input.buttons & ACTOR_INPUT_MOVE_RIGHT)
	{
		dirInput.x += 1;
	}
	if (input.buttons & ACTOR_INPUT_MOVE_UP)
	{
		dirInput.y += 1;
	}
	if (input.buttons & ACTOR_INPUT_MOVE_DOWN)
	{
		dirInput.y -= 1;
	}
	Vec3 forward = {};
	Vec3 left = {};
	Vec3 up = {};
	Vec3 move = {};
	forward.x = t->rotation.zAxis.x * dirInput.z;
	forward.y = t->rotation.zAxis.y * dirInput.z;
	forward.z = t->rotation.zAxis.z * dirInput.z;

	left.x = t->rotation.xAxis.x * dirInput.x;
	left.y = t->rotation.xAxis.y * dirInput.x;
	left.z = t->rotation.xAxis.z * dirInput.x;

	up.x = t->rotation.yAxis.x * dirInput.y;
	up.y = t->rotation.yAxis.y * dirInput.y;
	up.z = t->rotation.yAxis.z * dirInput.y;

	move.x = forward.x + left.x + up.x;
	move.y = forward.y + left.y + up.y;
	move.z = forward.z + left.z + up.z;

	Vec3_SetMagnitude(&move, moveSpeed);

	t->pos.x += (move.x * (f32)delta);
	t->pos.y += (move.y * (f32)delta);
	t->pos.z += (move.z * (f32)delta);
}
