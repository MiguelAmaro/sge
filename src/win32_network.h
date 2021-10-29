/* date = January 8th 2021 8:15 pm */

#ifndef SGE_NETWORK_H
#define SGE_NETWORK_H

#include <windows.h>
#include <WinSock2.h>


struct socketader_in g_server_address;
struct socketader_in g_connected_client_address;
WSADATA              g_socket_start_data;

// TODO(MIGUEL): Read on UDP and network protocols
internal void
win32_network_init()
{
    SOCKET client_socket;
    
    if(WSAStartup(MAKEWORD(2, 0), &g_socket_start_data) != 0)
    {
        printf("cannot support sockets");
    }
    
    g_server_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    if (g_server_socket == INVALID_SOCKET) 
    {
		printf(L"Failed server socket() call");
    }
    
    g_server_address.sin_family      = AF_INET;	
    g_server_address.sin_addr.s_addr = htonl( INADDR_ANY );    
    g_server_address.sin_port        = htons( SERVER_PORT );	
    
    if (bind (g_server_socket, (struct sockaddr *)&g_server_address, sizeof(g_server_address)) == SOCKET_ERROR)
    {
		printf(L"Failed server bind() call");
    }
    
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
    return;
}

#endif //SGE_NETWORK_H
