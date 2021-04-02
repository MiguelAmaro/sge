/* date = January 9th 2021 8:02 pm */

#ifndef _PIPES_H
#define _PIPES_H


HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

HANDLE g_hInputFile = NULL;



void ErrorExit(PTSTR lpszFunction) 

// Format a readable error message, display a message box, 
// and exit from the application.
{ 
    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 
    
    FormatMessage(
                  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  dw,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR) &lpMsgBuf,
                  0, NULL );
    
    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
                                      (lstrlen((LPCTSTR)lpMsgBuf)+lstrlen((LPCTSTR)lpszFunction)+40)*sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
                    LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                    TEXT("%s failed with error %d: %s"), 
                    lpszFunction, dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 
    
    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(1);
}

void WriteToPipe(void) 

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{ 
    DWORD dwRead, dwWritten; 
    CHAR chBuf[DEBUG_CONSOLE_BUFFER_SIZE];
    BOOL bSuccess = FALSE;
    
    for (;;) 
    { 
        bSuccess = ReadFile(g_hInputFile, chBuf, DEBUG_CONSOLE_BUFFER_SIZE, &dwRead, NULL);
        if ( ! bSuccess || dwRead == 0 ) break; 
        
        bSuccess = WriteFile(g_hChildStd_IN_Wr, chBuf, dwRead, &dwWritten, NULL);
        if ( ! bSuccess ) break; 
    } 
    
    // Close the pipe handle so the child process stops reading. 
    
    if ( ! CloseHandle(g_hChildStd_IN_Wr) ) 
        ErrorExit(TEXT("StdInWr CloseHandle")); 
} 

void ReadFromPipe(void) 

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{ 
    DWORD dwRead, dwWritten; 
    CHAR chBuf[DEBUG_CONSOLE_BUFFER_SIZE]; 
    BOOL bSuccess = FALSE;
    HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    for (;;) 
    { 
        bSuccess = ReadFile( g_hChildStd_OUT_Rd, chBuf, DEBUG_CONSOLE_BUFFER_SIZE, &dwRead, NULL);
        if( ! bSuccess || dwRead == 0 ) break; 
        
        bSuccess = WriteFile(hParentStdOut, chBuf, 
                             dwRead, &dwWritten, NULL);
        if (! bSuccess ) break; 
    } 
} 

internal void
win32_create_Debug_Console(void)
{
    b32 debug_console_success = false;
    
    PROCESS_INFORMATION debug_console_process_info = { 0 };
    STARTUPINFO         debug_console_startup_info = 
    {
        .cb         = sizeof(STARTUPINFO),
        .dwXSize    = 200,
        .dwYSize    = 800,
        .hStdError  = g_hChildStd_OUT_Wr,
        .hStdOutput = g_hChildStd_OUT_Wr,
        .hStdInput  = g_hChildStd_IN_Rd,
        .dwFlags    = STARTF_USESIZE | STARTF_USESTDHANDLES
    };
    
    CreateProcessA(NULLPTR,
                   "child",
                   NULLPTR,
                   NULLPTR,
                   true,
                   0,
                   NULLPTR,
                   NULLPTR,
                   &debug_console_startup_info,
                   &debug_console_process_info
                   );
    
    if(!debug_console_success)
    {
        ErrorExit(TEXT("CreateProcess"));
    }
    else
    {
        
        CloseHandle( debug_console_process_info.hProcess);
        CloseHandle( debug_console_process_info.hThread );
        
        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_IN_Rd);
    }
    
    return;
}

internal void 
win32_create_Debug_Pipes(void)
{
    SECURITY_ATTRIBUTES saAttr; 
    
    printf("\n->Start of parent execution.\n");
    
    // Set the bInheritHandle flag so pipe handles are inherited. 
    
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
    saAttr.bInheritHandle = TRUE; 
    saAttr.lpSecurityDescriptor = NULL; 
    
    // Create a pipe for the child process's STDOUT. 
    
    if ( ! CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0) ) 
        ErrorExit(TEXT("StdoutRd CreatePipe")); 
    
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    
    if ( ! SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdout SetHandleInformation")); 
    
    // Create a pipe for the child process's STDIN. 
    
    if (! CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) 
        ErrorExit(TEXT("Stdin CreatePipe")); 
    
    // Ensure the write handle to the pipe for STDIN is not inherited. 
    
    if ( ! SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0) )
        ErrorExit(TEXT("Stdin SetHandleInformation")); 
    
    // Create the child process. 
    
    win32_create_Debug_Console();
    
    return;
}


#endif //_PIPES_H
