#pragma once

#include "../../headers/common/ze_common_full.h"

struct FakeSocketInfo
{
    i32 mode;
    i32 minMS;
    i32 maxMS;
    f32 lossNormal;
    i32 randomIndex;

    i32 RollDropPacket()
    {
        if (lossNormal == 0) { return NO; }
        //f32 num = COM_Randf32(&this->randomIndex);
        f32 num = COM_STDRandf32();
        //printf("Drop? %.2f vs %.2f\n", num, this->lossNormal);
        return (num < lossNormal);
    }

    i32 RollDelay(i32 currentDelayTime)
    {
        //f32 num = ZE_Randf32(this->randomIndex++);
        #if 1 // Realistic variable ping
        f32 num = COM_STDRandf32();
        return (i32)((f32)(maxMS - minMS) * num) + minMS;
        #endif
        #if 0 // Ping will oscilate between two values:
        if (currentDelayTime == maxMS)
        { return minMS; }
        else
        { return maxMS; }
        #endif
    }
};

#define FAKE_SOCKET_STATUS_NONE 0
#define FAKE_SOCKET_STATUS_ACTIVE 1
#define FAKE_SOCKET_STATUS_FREEABLE 2

#define FAKE_SOCKET_MAX_HANDLES 2048

struct FakeSocketPacketHeader
{
	i32 status;
	i32 socketIndex;
    ZNetAddress address;
    i32 size;
    timeFloat tick;
};

struct FakeSocket
{
    i32 isActive;
    FakeSocketInfo info;
    FakeSocketPacketHeader* handles[FAKE_SOCKET_MAX_HANDLES];
    timeFloat delayRecalcTime;
    timeFloat delayRecalcMax;
    timeFloat dropRecalcTime;
    i32 nextDelayTimeMS;
    timeFloat nextDropTime;

    void SetLagStats(i32 minLagMS, i32 maxLagMS, f32 normalisedPacketLossChance)
    {
        if (minLagMS > maxLagMS) { minLagMS = maxLagMS; }
        ZE_ClampF32(&normalisedPacketLossChance, 0, 0.9f);
        // Half the given round trip time to get the delay for
        // an individual packet
        info.minMS = minLagMS / 2;
        info.maxMS = maxLagMS / 2;
        info.lossNormal = normalisedPacketLossChance;
    }

    void Init(i32 minLagMS, i32 maxLagMS, f32 normalisedPacketLossChance)
    {
        delayRecalcTime = 0;
        delayRecalcMax = 0;
        ZE_SET_ZERO((u8*)this, sizeof(FakeSocket));
        SetLagStats(minLagMS, maxLagMS, normalisedPacketLossChance);
    }

    i32 GetFreeHandleIndex()
    {
        for (i32 i = 0; i < FAKE_SOCKET_MAX_HANDLES; ++i)
        {
            if (this->handles[i] == 0)
            {
                return i;
            }
        }
        return -1;
    }

    void FreeHandle(i32 i)
    {
        if (this->handles[i] == NULL)
        { return; }
        if (this->handles[i]->status != FAKE_SOCKET_STATUS_FREEABLE)
        { return; }
        FakeSocketPacketHeader* h = this->handles[i];
        this->handles[i] = NULL;
        free((void*)h);
    }

    void Destroy()
    {
        for (i32 i = 0; i < 256; ++i)
        {
            this->FreeHandle(i);
        }
    }

    void SendPacket(i32 socketIndex, ZNetAddress* address, u8* data, i32 numBytes)
    {
        timeFloat delay = (f32)nextDelayTimeMS / 1000.f;
        #if 1 // Report internal packets:
        if (address->port == APP_CLIENT_LOOPBACK_PORT)
        {
            APP_LOG(64, "SV -> CL %d bytes, delay %.3f\n", numBytes, delay);
        }
        else if (address->port == APP_SERVER_LOOPBACK_PORT)
        {
            APP_LOG(64, "CL -> SV %d bytes, delay %.3f\n", numBytes, delay);
        }
        #endif
        if (this->info.RollDropPacket())
        {
            APP_LOG(16, "GULP\n");
            return;
        }
        i32 i = this->GetFreeHandleIndex();
        if (i == -1)
        {
            // need more space!
            ILLEGAL_CODE_PATH
            return;
        }
        
        i32 space = sizeof(FakeSocketPacketHeader) + numBytes;
        FakeSocketPacketHeader* h = (FakeSocketPacketHeader*)malloc(space);
        ZE_ASSERT(h, "Failed to allocate packet buffer");
        h->address = *address;
		h->socketIndex = socketIndex;
        h->size = numBytes;
        h->status = FAKE_SOCKET_STATUS_ACTIVE;
        h->tick = delay;
        
        handles[i] = h;
        u8* ptr = (u8*)h + sizeof(FakeSocketPacketHeader);
        ZE_COPY(data, ptr, numBytes);
        //printf("Fake Socket storing %d bytes. Packet delay: %.2f\n", h->size, h->tick);
    }

    void Tick(timeFloat deltaTime)
    {
        if (delayRecalcTime <= 0)
        {
            delayRecalcTime = delayRecalcMax;
            i32 newDelayTimeMS = this->info.RollDelay(nextDelayTimeMS);
            #if 0
            APP_PRINT(128, "APP - FAKE SOCKET DELAY CHANGE %d to %d\n",
                nextDelayTimeMS, newDelayTimeMS);
            APP_LOG(128, "APP - FAKE SOCKET DELAY CHANGE %d to %d\n",
                nextDelayTimeMS, newDelayTimeMS);
            #endif
            nextDelayTimeMS = newDelayTimeMS;
        }
        else
        {
            delayRecalcTime -= deltaTime;
        }
        for (i32 i = 0; i < 256; ++i)
        {
            FakeSocketPacketHeader* h = this->handles[i];
            if (h == NULL) { continue; }
            if (h->status == FAKE_SOCKET_STATUS_FREEABLE)
            {
                FreeHandle(i);
                continue;
            }
            h->tick -= deltaTime;
            //if (h->tick > 0) { continue; }
            //u8* ptr = (u8*)h + sizeof(FakeSocketPacketHeader);
            //ZNet_SendActual(net, &h->address, ptr, (u16)h->size);
            //this->FreeHandle(i);
        }
    }

    // Call repeatedly to pull packets that are read out
    // one by one
    void Read(i32* socketIndex, u8** ptr, i32* numBytes, ZNetAddress* from)
    {
        for (i32 i = 0; i < 256; ++i)
        {
            FakeSocketPacketHeader* h = this->handles[i];
            if (h == NULL) { continue; }
            if (h->status != FAKE_SOCKET_STATUS_ACTIVE) { continue; }
            if (h->tick > 0) { continue; }
            
            //
            h->status = FAKE_SOCKET_STATUS_FREEABLE;
			*socketIndex = h->socketIndex;
            *numBytes = h->size;
            *from = h->address;
            *ptr = (u8*)h + sizeof(FakeSocketPacketHeader);
            //printf("FAKESOCK: Reading %d bytes\n", h->size);
            return;
            
            //u8* ptr = (u8*)h + sizeof(FakeSocketPacketHeader);
            //ZNet_SendActual(net, &h->address, ptr, (u16)h->size);
            //this->FreeHandle(i);
        }
        *ptr = NULL;
        *numBytes = 0;
		*socketIndex = 0;
		*from = {};
    }
};
