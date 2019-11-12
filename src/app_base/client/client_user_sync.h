#ifndef CLIENT_USER_SYNC_H
#define CLIENT_USER_SYNC_H

#include "client.h"
#include "client_input.h"
#include "client_game.h"


internal void CLG_SyncAvatar(
    SimScene* sim,
    i32 avatarSerialNumber,
    S2C_InputResponse* cmd,
    C2S_Input* sentCommands,
    i32 localInputSequence,
    timeFloat latency,
    timeFloat time)
{
    i32 verbose = NO;
    
    // gather pre-replay position to detect skips
    SimEntity* ent = Sim_GetEntityBySerial(
        sim, avatarSerialNumber);
    if (!ent) { return; }

    Vec3 prePredictionEntPos = ent->body.t.pos;
    f32 skipReportDistance = SIM_ENT_STAT_ACTOR_SPEED * (f32)App_GetSimFrameInterval();

    APP_LOG(256, "CL - Sync Avatar on sv tick %d (input seq %d) vs SV response tick %d (seq %d)\n",
        CL_GetServerTick(), localInputSequence, cmd->header.tick, cmd->lastUserInputSequence);
    
    i32 inputsSinceResponse = (localInputSequence - 1) - cmd->lastUserInputSequence;
    APP_LOG(256, "CL - %d inputs since response (CL %d vs SV response %d)\n",
        inputsSinceResponse, localInputSequence, cmd->lastUserInputSequence);
    
    // Calculate replay frames

    C2S_Input* timestampInput = CL_RecallSentInputCommand(
        sentCommands, cmd->lastUserInputSequence);
    if (timestampInput == NULL)
    {
        APP_LOG(128, "CL ERROR No input for seq %d\n", cmd->lastUserInputSequence);
        return;
    }
    if (Vec3_AreDifferent(&timestampInput->avatarPos, &cmd->latestAvatarPos, F32_EPSILON) == NO)
    {
        //printf("CL - No correction required!\n");
        return;
    }

    // calculate latency - half RTT for sever response
    //timeFloat latency = (time - timestampInput->time) / 2;

    // calculate time via timestamp of command
    latency = (time - timestampInput->time);

    // Calculate replay time via average latency param
    timeFloat replayTime = time - latency;
    
    // timeFloat delay = (time - cmd->clientTimestamp) * 0.5f;
    // i32 timeFrames = (i32)(delay / App_GetSimFrameInterval());
    // timeFrames += 1;
    // APP_LOG(256, "    Time diff %.5f replay frames count %d\n", delay, timeFrames);

    C2S_Input* timeInput = CLI_RecallOldestInputAfterTimestamp(
        sentCommands, replayTime);
    if (timeInput != NULL)
    {
        APP_LOG(128, "CL latest input - seq %d, timestamp %.5f\n",
            timeInput->userInputSequence, timeInput->time);
    }
    else
    {
        APP_LOG(128, "CL ERROR No input found after timestamp %.5f - ellapsed %.5f\n",
            replayTime, time);
        return;
    }
    
    i32 replayEnd = localInputSequence;
    //i32 replaySeq = cmd->lastUserInputSequence;
    i32 replaySeq = timeInput->userInputSequence;
    APP_LOG(256, "CL - replay seq %d to %d\n", replaySeq, replayEnd - 1);
    APP_LOG(256, "\tOrigin %.3f, %.3f, %.3f\n",
        cmd->latestAvatarPos.x, cmd->latestAvatarPos.y, cmd->latestAvatarPos.z);

    // replay
    ent->body.t.pos = cmd->latestAvatarPos;
    for (; replaySeq < replayEnd; ++replaySeq)
    {
        C2S_Input* input = CL_RecallSentInputCommand(
            sentCommands, replaySeq);
        if (!input) { continue; }
		Vec3 before = ent->body.t.pos;
		CLG_StepActor(sim, ent, &input->input, input->deltaTime);
		Vec3 after = ent->body.t.pos;
    }
    
    // calculate smoothing for corrections
    Vec3 postPredictionEntPos = ent->body.t.pos;
	ent->body.errorRate = 0.8f;
	#if 0
	 // TODO: This calculation is currently resetting any errors
    // that have not finished interpolating.
    // instead of taking prePrediction pos, take prePredictionPos - error
    // like when rendering
    ent->body.error.x = postPredictionEntPos.x - prePredictionEntPos.x;
    ent->body.error.y = postPredictionEntPos.y - prePredictionEntPos.y;
    ent->body.error.z = postPredictionEntPos.z - prePredictionEntPos.z;
	#endif
	#if 1
	Vec3 errPos = ent->body.error;
	Vec3 rendPos;
	rendPos.x = prePredictionEntPos.x - errPos.x;
	rendPos.y = prePredictionEntPos.y - errPos.y;
	rendPos.z = prePredictionEntPos.z - errPos.z;
	ent->body.error.x = postPredictionEntPos.x - rendPos.x;
    ent->body.error.y = postPredictionEntPos.y - rendPos.y;
    ent->body.error.z = postPredictionEntPos.z - rendPos.z;
	#endif

    // For debugging - report mis-predictions
    #if 0
    f32 diffX = postPredictionEntPos.x - prePredictionEntPos.x;
    ZABS(diffX);
    f32 diffY = postPredictionEntPos.y - prePredictionEntPos.y;
    ZABS(diffY);
    if (diffX > skipReportDistance
        || diffY > skipReportDistance)
    {
        APP_PRINT(128, "CL - POSITION SKIP!\n");
        APP_LOG(128, "CL - POSITION SKIP DETECTED\n");
    }
    #endif
}
#if 0
internal void CLG_SyncAvatar_Broken(SimScene* sim, S2C_InputResponse* cmd)
{
    f32 skipReportDistance = SIM_ENT_STAT_ACTOR_SPEED * (f32)App_GetSimFrameInterval();
    //printf("CL Skip report dist: %.5f\n", skipReportDistance);

    const i32 verbose = YES;
    if (g_latestUserInputAck > cmd->lastUserInputSequence)
    {
        if (verbose)
        {
            APP_LOG(64, "CL Ignore response %d (current %d)\n",
                cmd->lastUserInputSequence,
                g_latestUserInputAck);
        }
        return;
    }
    SimEntity* ent = Sim_GetEntityBySerial(
        &g_sim, g_avatarSerial);
    if (!ent) { return; }

    Vec3 prePredictionEntPos = ent->body.t.pos;

    APP_LOG(256, "CL - Sync Avatar on sv tick %d (input seq %d) vs SV response tick %d (seq %d)\n",
        CL_GetServerTick(), localInputSequence, cmd->header.tick, cmd->lastUserInputSequence);
    
    g_latestUserInputAck = cmd->lastUserInputSequence;
    g_latestAvatarPos = cmd->latestAvatarPos;
    i32 framesSinceResponse = localInputSequence - cmd->lastUserInputSequence;
    
    if (verbose)
    {
        APP_PRINT(128, "CL Replay %d frames (%d to %d)\n",
            framesSinceResponse,
            cmd->lastUserInputSequence,
            localInputSequence);
        APP_LOG(128, "CL Replay %d frames (%d to %d)\n",
            framesSinceResponse,
            cmd->lastUserInputSequence,
            localInputSequence);
    }
	
	////////////////////////////////////
    // DEBUG Dump stored inputs to log for analysis
	#if 0
	CL_DumpSentInputs(g_sentCommands, CL_MAX_SENT_INPUT_COMMANDS);
	#endif
    
    ////////////////////////////////////
    // DEBUG Compare by server tick
	#if 1
    C2S_Input* tickBasedInput = CL_RecallSentInputCommandByServerTick(
        g_sentCommands, cmd->header.tick);
    if (tickBasedInput != NULL)
    {
        Vec3 posTickBased = tickBasedInput->avatarPos;
        // APP_LOG(256, "CL compare pos by server tick\n\tCL %.3f, %.3f, %.3f vs SV %.3f, %.3f, %.3f\n",
        //     posTickBased.x, posTickBased.y, posTickBased.z, g_latestAvatarPos.x, g_latestAvatarPos.y, g_latestAvatarPos.z);
    }
    else
    {
        APP_LOG(128, "CL cannot compare by server tick CL %d vs SV %d...\n",
            CL_GetServerTick(), cmd->header.tick);
    }
	#endif
    ////////////////////////////////////
    
    // this is the input sequence matching the response. Replay will
    // occur from this point.
    C2S_Input* sourceInput = CL_RecallSentInputCommand(
        g_sentCommands, cmd->lastUserInputSequence);
	if (sourceInput == NULL)
	{
		//APP_LOG(128, "CL unable to recall input seq %d\n", cmd->lastUserInputSequence);
		return;
	}
    
    Vec3 originalLocalPos = sourceInput->avatarPos;
    Vec3 currentLocalPos = ent->body.t.pos;
    Vec3 remotePos = cmd->latestAvatarPos;

    i32 bIsCorrecting = NO;
    i32 bPositionsDiffer = Vec3_AreDifferent(&originalLocalPos, &remotePos, F32_EPSILON);
    if (g_bClientAlwaysRepredict == YES
        || bPositionsDiffer == NO)
    {
        // Server disagrees with our recorded position at this time.
        // Therefore we must correct.
        ent->body.t.pos = remotePos;
        ent->body.previousPos = remotePos;
        bIsCorrecting = YES;
        //ILLEGAL_CODE_PATH
        #if 0
        if (verbose)
        {
            if (bPositionsDiffer == YES)
            {
                APP_LOG(256,
                    "  Correcting CL vs SV: %.3f, %.3f, %.3f vs %.3f, %.3f, %.3f\n",
                    originalLocalPos.x, originalLocalPos.y, originalLocalPos.z,
                    remotePos.x, remotePos.y, remotePos.z);
            }
            else
            {
                APP_LOG(256,
                    "  No correction repredict CL vs SV: %.3f, %.3f, %.3f vs %.3f, %.3f, %.3f\n",
                    originalLocalPos.x, originalLocalPos.y, originalLocalPos.z,
                    remotePos.x, remotePos.y, remotePos.z);
            }
        }
        #endif
        // Check that there is a timing issue. Does the client have a record at this position?
        C2S_Input* matchingInput =
            CL_FindSentInputByPosition(g_sentCommands, remotePos, F32_EPSILON);
        if (matchingInput != NULL)
        {
            APP_LOG(128, "Client has matching record on tick %d vs SV response tick %d!\n",
                matchingInput->userInputSequence, cmd->lastUserInputSequence);
        }
    }
    else
    {
        if (verbose)
        {
            APP_LOG(256,
                "  No correction for local vs server positions: %.3f, %.3f, %.3f vs %.3f, %.3f, %.3f\n",
                originalLocalPos.x, originalLocalPos.y, originalLocalPos.z,
                remotePos.x, remotePos.y, remotePos.z);
        }
        
        //ent->body.t.pos = originalLocalPos;
        //ent->body.previousPos = originalLocalPos;
        return;
    }

    // Mark that this entity has been moved this frame already
    //ent->hasBeenPredicted = 1;
    
    // Replay frames
    i32 replaySequence = cmd->lastUserInputSequence;
    if (replaySequence < 0) { replaySequence = 0;  }
    i32 lastSequence = localInputSequence - 1;
    //framesSinceResponse = 0;
    #if 1
    for (i32 i = 0; i <= framesSinceResponse; ++i)
    {
        replaySequence = cmd->lastUserInputSequence + i;
        C2S_Input* input = CL_RecallSentInputCommand(
            g_sentCommands, replaySequence);
        if (!input) { continue; }
		Vec3 before = ent->body.t.pos;
		CLG_StepActor(sim, ent, &input->input, input->deltaTime);
		Vec3 after = ent->body.t.pos;
        #if 0
        if (verbose)
        {
            APP_LOG(256, "\t\tSeq %d, svtick %d, Buttons %d: %.3f, %.3f, %.3f to %.3f, %.3f, %.3f\n",
			    replaySequence,
                input->header.tick,
                input->input.buttons,
			    before.x, before.y, before.z,
			    after.x, after.y, after.z
			);
        }
		#endif
    }
    #endif
    #if 0
    // While loop version
    while (replaySequence < lastSequence)
    {
        C2S_Input* input = CL_RecallSentInputCommand(
            g_sentCommands, replaySequence);
		
		CLG_StepActor(ent, &input->input, input->deltaTime);
        
        replaySequence++;
    }
    #endif
    //printf("\n");

    Vec3 postPredictionEntPos = ent->body.t.pos;
    f32 diffX = postPredictionEntPos.x - prePredictionEntPos.x;
    ZABS(diffX);
    f32 diffY = postPredictionEntPos.y - prePredictionEntPos.y;
    ZABS(diffY);
    if (diffX > skipReportDistance
        || diffY > skipReportDistance)
    {
        APP_PRINT(128, "CL - POSITION SKIP!\n");
        APP_LOG(128, "CL - POSITION SKIP DETECTED\n");
    }
}
#endif

#endif // CLIENT_USER_SYNC_H