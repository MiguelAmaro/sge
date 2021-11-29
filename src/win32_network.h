/* date = January 8th 2021 8:15 pm */

#ifndef SGE_NETWORK_H
#define SGE_NETWORK_H

#include <ws2tcpip.h>  // Address conversion function and other auxileries
#include <winsock2.h>
#include <windows.h>
//#include "sge_platform.h"
#define SERVER_PORT 80 // Well known port.


// NOTE(MIGUEL): Network layer packets need to define src and dest addresses
//               and src and dest port for packets wraping the transport layer.

// NOTE(MIGUEL): sockaddr_in is a convinience structure that lets u
//               package/define the address data easily. It just gets 
//               casted to sockaddr. sockaddr is harder to define
struct sockaddr_in g_server_address;
struct sockaddr_in g_connected_client_address;
WSADATA            g_socket_start_data;

SOCKET g_server_socket;
SOCKET g_connected_client_socket;

#if 0
// TODO(MIGUEL): Read on UDP and network protocols
internal win32_ipv4_from_string(u8 string, u32 sting_length)
{
    // NOTE(MIGUEL): Multiplayer games page 77
    u8 host;
    u8 service;
    
    if(0)
    {
        
    }
    else
    {
        
    }
    
    
    
    addrinfo hint; 
    memset(&hint, 0, sizeof(hint)); 
    hint.ai_family = AF_INET;
    
    addrinfo *result; 
    
    s32 error_code = getaddrinfo(host, service,
                                 &hint, &result);
    
    /* // NOTE(MIGUEL): For geting address from a domain name

    
void freeaddrinfo(addrinfo* ai); // releasing memory
alternative: alows for asynchronous use
 GetAddrInfoEx()
    */
    return;
}

#endif
internal void
win32_network_client_init()
{
    SOCKET client_socket     = INVALID_SOCKET;
    DWORD  connection_status = 0;
    
    
    ASSERT(WSAStartup(MAKEWORD(2, 0), &g_socket_start_data) != 0);
    
    // SOCK_STREAM for TCP socket
    g_server_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    ASSERT(g_server_socket == INVALID_SOCKET) 
        
#if 1
    memset(g_server_address.sin_zero, 0, sizeof(g_server_address.sin_zero)); 
    g_server_address.sin_family = AF_INET; // Address is for Ipv4
    g_server_address.sin_port = htons(80); 
    g_server_address.sin_addr.S_un.S_un_b.s_b1 = 65; 
    g_server_address.sin_addr.S_un.S_un_b.s_b2 = 254; 
    g_server_address.sin_addr.S_un.S_un_b.s_b3 = 248; 
    g_server_address.sin_addr.S_un.S_un_b.s_b4 = 180;
#else
    g_server_address.sin_family      = AF_INET; // Address is for Ipv4
    g_server_address.sin_port        = htons( SERVER_PORT );	
    InetPton(AF_INET, "65.254.248.180", &g_server_address.sin_addr); 
    //g_server_address.sin_addr.s_addr = htonl( INADDR_ANY );
    //g_server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
#endif
    
#if 1
    if (bind(g_server_socket, (struct sockaddr *)&g_server_address, sizeof(g_server_address)) == SOCKET_ERROR)
    {
		printf("Failed server bind() call");
    }
    
#else
	connection_status = connect (client_socket, (struct sockaddr *)&client_address, sizeof(client_address));
	
    if (connection_status == SOCKET_ERROR)
    {
        report_error_utf16(L"Failed client connect() call)", 3, TRUE);
    }
#endif
    
    
    /*
        if (listen (g_server_socket, MAX_CLIENTS) != 0) 
        {
            printf(L"Server listen() error");
        }
        
        /// Server loop
        while(g_running)
        {
            
        }
        */
    
    shutdown(client_socket, SD_BOTH);
    
    u32 error_code;
    
    if(SOCKET_ERROR == WSACleanup())
    {
        error_code = WSAGetLastError();
    }
    
    return;
}

internal void
win32_network_server_init()
{
    int queued_connection_limit = 2;
    
    if (bind(g_server_socket,
             (struct sockaddr *)&g_server_address,
             sizeof(g_server_address)) == SOCKET_ERROR)
    {
		printf("Failed server bind() call");
    }
    listen(g_server_socket, queued_connection_limit);
    
    accept(g_connected_client_socket, sockaddr* addr, int* addrlen);
    
    int connect(SOCKET sock, const sockaddr *addr, int addrlen); 
    
    
    return;
}

#endif //SGE_NETWORK_H
