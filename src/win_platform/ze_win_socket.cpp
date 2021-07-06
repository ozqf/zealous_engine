#ifndef ZE_WIN_SOCKET_CPP
#define ZE_WIN_SOCKET_CPP

///////////////////////////////////////
// Win32 Network compile module
///////////////////////////////////////

#include <stdio.h>

#include "../../headers/common/ze_common.h"

#include "ze_win_socket.h"

// https://beej.us/guide/bgnet/html/single/bgnet.html
// https://docs.microsoft.com/en-us/windows/desktop/WinSock/getting-started-with-winsock
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#define NET_NON_BLOCKING_ERROR 10035
#define UDP_BUFFER_LENGTH 4096
#define LOCALHOST_ADDRESS "127.0.0.1"

#define NET_ERROR_RESET_BY_PEER 10054
#define NET_ERROR_ADDRESS_NOT_AVAILABLE 10049

#define NET_ERROR_CASE(errorCode, msg) case errorCode##: printf(##msg##); break;

// Add new error codes as you run into them!
// https://docs.microsoft.com/en-us/windows/desktop/winsock/windows-sockets-error-codes-2
void Net_PrintNetworkError(i32 code)
{
    switch (code)
    {
        NET_ERROR_CASE(10014, "Bad Address\n");
        NET_ERROR_CASE(10022, "Invalid parameter\n");
        NET_ERROR_CASE(10035, "Non-blocking socket had nothing available\n");
        NET_ERROR_CASE(10040, "Message too long.\n");
        NET_ERROR_CASE(10047, "Address family not supported by protocol family.\n");
        NET_ERROR_CASE(10048, "Address in use\n");
        NET_ERROR_CASE(10049, "Address not available\n");
        NET_ERROR_CASE(NET_ERROR_RESET_BY_PEER, "Connection reset by peer\n");
        default: printf("Unknown error code... Sorry\n"); break;
    }
}

internal i32 Net_IsErrorIgnorable(i32 code)
{
    return (code == NET_NON_BLOCKING_ERROR
        || code == NET_ERROR_RESET_BY_PEER);
}

////////////////////////////////////////////////////////////////////////////////
// TYPES
////////////////////////////////////////////////////////////////////////////////

struct Win32_Socket
{
	i32 isActive;
    WSADATA wsaData;
    SOCKET socket;
    sockaddr_in server;
    i32 slen;
    u16 port;
};


////////////////////////////////////////////////////////////////////////////////
// GLOBALS
////////////////////////////////////////////////////////////////////////////////

#define MAX_SOCKETS 2
internal Win32_Socket g_connections[MAX_SOCKETS];

static Win32_Socket* WNet_GetActiveSocket(i32 socketIndex)
{
    ZE_ASSERT((socketIndex >= 0 && socketIndex < MAX_SOCKETS),
        "Socket index out of bounds");
    Win32_Socket* winSock = &g_connections[socketIndex];
	ZE_ASSERT(winSock->isActive,
        "Winsock is not active")
    return winSock;
}

static Win32_Socket* WNet_GetFreeSocket(i32* socketIndexResult)
{
    for (i32 i = 0; i < MAX_SOCKETS; ++i)
    {
        Win32_Socket* s = &g_connections[i];
        if (!s->isActive)
        {
            s->isActive = 1;
            *socketIndexResult = i;
            return s;
        }
    }
    ILLEGAL_CODE_PATH
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// SEND
//////////////////////////////////////////////////////////////////////////
extern "C"
i32 Net_SendTo(
    i32 transmittingSocketIndex,
    ZNetAddress* address,// u16 port,
	u8* data, i32 dataSize)
{
    char asciAddress[32];
    sprintf_s(asciAddress, 32, "%d.%d.%d.%d",
        address->ip4Bytes[0],
        address->ip4Bytes[1],
        address->ip4Bytes[2],
        address->ip4Bytes[3]
    );

    Win32_Socket* winSock = WNet_GetActiveSocket(transmittingSocketIndex);
    //printf("Port %d Sending %d bytes to %s:%d\n", winSock->port, dataSize, address, port);
	sockaddr_in toAddress;
    toAddress.sin_port = htons(address->port);
    toAddress.sin_family = AF_INET;
    //toAddress.sin_addr.S_un.S_addr = 
    InetPton(AF_INET, asciAddress, &toAddress.sin_addr.S_un.S_addr);
    
    i32 toAddressSize = sizeof(toAddress);
    
    //i32 sendLength = COM_StrLen(data) + 1;

    i32 sendResult = sendto(
        winSock->socket,
        (char*)data,
        dataSize,
        0,
        (sockaddr*)&toAddress,
        toAddressSize
    );
    if (sendResult == SOCKET_ERROR)
    {
        i32 errorCode = WSAGetLastError();
        printf("Send error %d\n", errorCode);
        Net_PrintNetworkError(errorCode);
        if (errorCode == NET_ERROR_ADDRESS_NOT_AVAILABLE)
        {
            printf("    %s\n", asciAddress);
        }
    }
    //printf("-");
    return sendResult;
}
#if 0
i32 WNet_ReadSocket_OldTest(i32 socketIndex)
{
    Win32_Socket* winSock = WNet_GetActiveSocket(socketIndex);
    Assert(winSock);
    // optional struct to store the sender's address
    sockaddr_in fromAddress;
    i32 fromAddressSize = sizeof(fromAddress);

    i32 recv_len;
    char buf[UDP_BUFFER_LENGTH];
    memset(buf, '\0', UDP_BUFFER_LENGTH);
    i32 flags = 0;
    recv_len = recvfrom(
        winSock->socket,
        buf,
        UDP_BUFFER_LENGTH,
        flags,
        (struct sockaddr *) &fromAddress,
        &fromAddressSize
    );
    
    if (recv_len == SOCKET_ERROR)
    {
        i32 errorCode = WSAGetLastError();
        if (errorCode != NET_NON_BLOCKING_ERROR)
        {
            printf("recvfrom() failed with error code %d\n", errorCode);
            Net_PrintNetworkError(errorCode);
        }
        else
        {
            printf(".");
        }
    }
    else
    {
        printf("\nReceived %d bytes: %s\n", recv_len, buf);
    }
    return recv_len > 0 ? recv_len : 0;
}
#endif
//////////////////////////////////////////////////////////////////////////
// READ
//////////////////////////////////////////////////////////////////////////
i32 Net_Read(i32 socketIndex, ZNetAddress* sender, u8** buf, i32 bufSize)
{
    Win32_Socket* winSock = WNet_GetActiveSocket(socketIndex);
    ZE_ASSERT(winSock, "Failed to acquire socket");
    // optional struct to store the sender's address
    sockaddr_in fromAddress;

    i32 fromAddressSize = sizeof(fromAddress);
	
    i32 recv_len;
    //char buf[UDP_BUFFER_LENGTH];
    //memset(buf, '\0', UDP_BUFFER_LENGTH);
    i32 flags = 0;
    recv_len = recvfrom(
        winSock->socket,
        (char*)*buf,
        bufSize,
        flags,
        (struct sockaddr *) &fromAddress,
        &fromAddressSize
    );
    
    if (recv_len == SOCKET_ERROR)
    {
        i32 errorCode = WSAGetLastError();
        if (!Net_IsErrorIgnorable(errorCode))
        {
            printf("recvfrom() on port %d failed with error code %d\n", winSock->port, errorCode);
            Net_PrintNetworkError(errorCode);
        }
    }

    if (recv_len > 0)
    {
        //printf(".");
        // debug
        //printf("RECV: ");
        //COM_PrintBytes(buffer->ptrMemory, buffer->size, 16);

        // translate address
        if (sender)
        {
            /*
		struct in_addr {

			union {
			
			struct
			{
				
				unsigned char s_b1,
				
				s_b2,
				
				s_b3,
				
				s_b4;
				
			} S_un_b;
			
			struct
			{
			
				unsigned short s_w1,
				
				s_w2;
			
			} S_un_w;
			
			unsigned long S_addr;
			
			} S_un;
			
		};
		sockaddr_in struct:
		https://msdn.microsoft.com/en-us/library/zx63b042.aspx
		struct sockaddr_in {
			short   sin_family;
			u_short sin_port;
			struct  in_addr sin_addr;
			char    sin_zero[8];
		};
		*/
		// Grab individual bytes of address
		sender->ip4Bytes[0] = fromAddress.sin_addr.S_un.S_un_b.s_b1;
		sender->ip4Bytes[1] = fromAddress.sin_addr.S_un.S_un_b.s_b2;
		sender->ip4Bytes[2] = fromAddress.sin_addr.S_un.S_un_b.s_b3;
		sender->ip4Bytes[3] = fromAddress.sin_addr.S_un.S_un_b.s_b4;
        sender->port = ntohs(fromAddress.sin_port);
        #if 0
        printf("Win32 net %d bytes from \"%d.%d.%d.%d:%d\"\n",
             recv_len,
             sender->ip4Bytes[0], sender->ip4Bytes[1], sender->ip4Bytes[2], sender->ip4Bytes[3],
             sender->port
             
             );
        #endif
		//printf("addr_in chars: %s\n", fromAddress.sa_data);
        }
    }
    return recv_len > 0 ? recv_len : 0;
}

/**
 * Returns socket index on success, -1 otherwise
 */
i32 Net_OpenSocket(u16 port, u16* portResult)
{
    i32 sockIndex;
    Win32_Socket* winSock = WNet_GetFreeSocket(&sockIndex);
    if (winSock == NULL)
    {
        // no free sockets?
        return ZE_ERROR_BAD_INDEX;
    }
    
    winSock->port = port;

    
    int iResult;
    iResult = WSAStartup(MAKEWORD(2, 2), &winSock->wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return ZE_ERROR_BAD_INDEX;
    }
    printf("Winsock initialised\n");
    
    if ((winSock->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
    {
        i32 errorCode = WSAGetLastError();
        printf("Could not create socket: %d\n", errorCode);
        return ZE_ERROR_BAD_INDEX;
    }

    winSock->server.sin_family = AF_INET;
    winSock->server.sin_addr.s_addr = htons(INADDR_ANY);
    winSock->server.sin_port = htons(winSock->port);

    u_long mode = 1;  // 1 to enable non-blocking socket
    iResult = ioctlsocket(winSock->socket, FIONBIO, &mode);
    if (iResult == SOCKET_ERROR)
    {
        i32 errorCode = WSAGetLastError();
        printf("Error setting socket to unblockable %d\n", errorCode);
        return ZE_ERROR_BAD_INDEX;
    }

    iResult = bind(winSock->socket, (sockaddr*)&winSock->server, sizeof(winSock->server));
    if (iResult == SOCKET_ERROR)
    {
        i32 errorCode = WSAGetLastError();
        printf("Failed to bind socket with code: %d\n", errorCode);
        Net_PrintNetworkError(errorCode);
        return ZE_ERROR_BAD_INDEX;
    }
    
	// Retreive socket port number
	struct sockaddr_in sin;
	int addrLen = sizeof(sin);
	u16 portNumber = 0;
	if (getsockname(winSock->socket, (struct sockaddr*)&sin, &addrLen) == 0 &&
		sin.sin_family == AF_INET &&
		addrLen == sizeof(sin))
	{
		portNumber = ntohs(sin.sin_port);
	}
	else
	{
		printf("FAILED TO ACQUIRE SOCKET NAME!\n");
		return ZE_ERROR_BAD_INDEX;
	}
	
	if (portResult)
	{
		*portResult = portNumber;
	}
	winSock->port = portNumber;
	printf("Socket bound on port %d\n", portNumber);

	winSock->isActive = 1;
	return sockIndex;
}

i32 Net_CloseSocket(i32 socketIndex)
{
    ZE_ASSERT((socketIndex >= 0 && socketIndex < MAX_SOCKETS),
        "Close socket - index out of bounds")
    Win32_Socket* sock = &g_connections[socketIndex];
    i32 result = -1;
    if (sock->isActive)
    {
        sock->isActive = 0;
        result = closesocket(sock->socket);
        printf("Closing socket %d result: %d\n", socketIndex, result);
    }
    return result;
}

i32 Net_Shutdown()
{
    printf("\nShutting down...\n");
    for (i32 i = 0; i < MAX_SOCKETS; ++i)
    {
        Win32_Socket* sock = &g_connections[i];
        if (!sock->isActive) { continue; }

        i32 result = closesocket(sock->socket);
        printf("Closing socket %d result: %d\n", i, result);
    }
    WSACleanup();
    printf("Done\n");
    return 0;
}

i32 Net_Init()
{
    return 0;
}
#if 0
void Net_RunLoopbackTest()
{
	//#define DEFAULT_PORT_SERVER 23232
	//#define DEFAULT_PORT_CLIENT 23233
	
    i32 serverSocket;
    i32 clientSocket;
    serverSocket = Net_OpenSocket(DEFAULT_PORT_SERVER);
    if (serverSocket < 0)
    {
        printf("Failed to open socket: %d\n", serverSocket);
        return;
    }
    clientSocket = Net_OpenSocket(DEFAULT_PORT_CLIENT);
    if (clientSocket < 0)
    {
        printf("Failed to open socket: %d\n", clientSocket);
        return;
    }

    //Net_SendTo(clientSocket, LOCALHOST_ADDRESS, DEFAULT_PORT_SERVER, "foo", COM_StrLen("foo"));
    Net_SendTo(serverSocket, LOCALHOST_ADDRESS, DEFAULT_PORT_SERVER, "foo", COM_StrLen("foo"));
    printf("Reading from socket: ");
    
    u8 dataBuffer[1024];
    MemoryBlock mem = { dataBuffer, 1024 };
    i32 i = 0;
    while (i++ < 100)
    {
        i32 bytes = Net_Read(serverSocket, NULL, &mem);
        if (bytes > 0)
        {
            printf("READ %d bytes... holy shit!\n", bytes);
            break;
        }
        Sleep(100);
    }
    
    Net_Shutdown();
}
#endif

#endif // ZE_WIN_SOCKET_CPP
