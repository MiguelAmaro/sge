/* date = January 8th 2021 8:15 pm */

#ifndef SGE_NETWORK_H
#define SGE_NETWORK_H

#include <windows.h>
#include <WinSock2.h>

// TODO(MIGUEL): Read on UDP and network protocols
internal void
win32_network_inti()
{
    SOCKET client_socket;
    WSADATA wsad; // TODO(MIGUEL): rename to socket_start_data
    
    if(WSAStartup(MAKEWORD(2, 0), &wsad) != 0)
    {
        
    }
    
    if(socket(AF_INET, SOCK_STREAM, 0))
    {
        
    }
    
    
    
    return;
}

#endif //SGE_NETWORK_H
