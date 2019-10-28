#pragma once

#include "../ze_common/ze_common_full.h"

struct FakeSocketInfo
{
    i32 mode;
    i32 minMS;
    i32 maxMS;
    f32 lossNormal;
    i32 randomIndex;

    i32 RollDropPacket()
    {
        //f32 num = COM_Randf32(&this->randomIndex);
        f32 num = COM_STDRandf32();
        //printf("Drop? %.2f vs %.2f\n", num, this->lossNormal);
        return (num < lossNormal);
    }

    i32 RollDelay()
    {
        f32 num = COM_Randf32(this->randomIndex++);
        return (i32)((f32)(maxMS - minMS) * num) + minMS;
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
    f32 tick;
};

struct FakeSocket
{
    i32 isActive;
    FakeSocketInfo info;
    FakeSocketPacketHeader* handles[FAKE_SOCKET_MAX_HANDLES];
    f32 delayRecalcTime;
    f32 dropRecalcTime;
    f32 nextDelayTime;
    f32 nextDropTime;

    void SetLagStats(i32 minLagMS, i32 maxLagMS, f32 normalisedPacketLossChance)
    {
        if (minLagMS > maxLagMS) { minLagMS = maxLagMS; }
        COM_ClampF32(&normalisedPacketLossChance, 0, 0.9f);
        // Half the given round trip time to get the delay for
        // an individual packet
        info.minMS = minLagMS / 2;
        info.maxMS = maxLagMS / 2;
        info.lossNormal = normalisedPacketLossChance;
    }

    void Init(i32 minLagMS, i32 maxLagMS, f32 normalisedPacketLossChance)
    {
        delayRecalcTime = 0;
        COM_ZeroMemory((u8*)this, sizeof(FakeSocket));
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
        f32 delay = nextDelayTime;
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
            //APP_PRINT(16, "GULP\n");
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
        COM_COPY(data, ptr, numBytes);
        //printf("Fake Socket storing %d bytes. Packet delay: %.2f\n", h->size, h->tick);
    }

    void Tick(f32 deltaTime)
    {
        if (delayRecalcTime <= 0)
        {
            delayRecalcTime = 1;
            nextDelayTime = (f32)this->info.RollDelay() / 1000.0f;
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
