/* date = January 8th 2021 8:15 pm */

#ifndef SGE_NETWORK_H
#define SGE_NETWORK_H

#include <windows.h>
#include <WinSock2.h>

// TODO(MIGUEL): Read on UDP and network protocols
internal void
win32_network_init()
{
    SOCKET client_socket;
    WSADATA wsad; // TODO(MIGUEL): rename to socket_start_data
    
    if(WSAStartup(MAKEWORD(2, 0), &wsad) != 0)
    {
        printf("cannot support sockets");
    }
    
    //Berkley sock like api
    
    if(socket(AF_INET, SOCK_STREAM, 0))
    {
        printf("failed socket() call");
    }
    
    return;
}

#endif //SGE_NETWORK_H
