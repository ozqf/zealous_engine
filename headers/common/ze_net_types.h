#ifndef ZE_NET_TYPES_H
#define ZE_NET_TYPES_H

#include "ze_common.h"

struct ZNetAddress
{
    u16 ip4Bytes[4]; // TODO: Is this meant to be u16s..? surely it is u8?
    u16 port;
};

internal ZNetAddress ZE_LocalHost(u16 port)
{
	ZNetAddress addr;
    addr.ip4Bytes[0] = 127;
    addr.ip4Bytes[1] = 0;
    addr.ip4Bytes[2] = 0;
    addr.ip4Bytes[3] = 1;
    addr.port = port;
    return addr;
}

#endif // ZE_NET_TYPES_H