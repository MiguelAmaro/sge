#include "SGE_SerialPort.h"

// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here
#include "RingBuffer.h"
#include "LAL.h"

global Device_Comm global_Device = {0};

void
win32_serial_Port_Init(void)
{
    // Created An IO Stream to talk with micro controller
    
    HANDLE Comm_Handle;
    
    // NOTE(MIGUEL): How do I know which com it is??
    // TODO(MIGUEL): Find out why comm get initialize with garbage
    global_Device.comm = CreateFile("\\\\.\\COM11",
                                    GENERIC_READ | GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL);
    
    
    if(global_Device.comm == INVALID_HANDLE_VALUE)
    {
        printf("Error in opening serial port \n");
        
    }
    else
    {
        printf("Opening serial port successful \n");
    }
    
    DCB DeviceSerialParams       = { 0 }; // Initializing DCB structure
    DeviceSerialParams.DCBlength = sizeof(DeviceSerialParams);
    DeviceSerialParams.BaudRate  = 115200    ;    
    DeviceSerialParams.ByteSize  = 8         ;         
    DeviceSerialParams.StopBits  = ONESTOPBIT;
    DeviceSerialParams.Parity    = NOPARITY  ;
    
    SetCommState(global_Device.comm, &DeviceSerialParams);
    
    
    COMMTIMEOUTS TimeoutsComm = { 0 };
    TimeoutsComm.ReadIntervalTimeout         = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutConstant    = 50; // in milliseconds
    TimeoutsComm.ReadTotalTimeoutMultiplier  = 10; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutConstant   = 50; // in milliseconds
    TimeoutsComm.WriteTotalTimeoutMultiplier = 10; // in milliseconds
    
    SetCommTimeouts(global_Device.comm, &TimeoutsComm);
    
    DWORD dwEventMask;
    b32 status = 0;
    
    status = SetCommMask(global_Device.comm, EV_RXCHAR);
    
    ASSERT(status);
    
    status = WaitCommEvent(global_Device.comm, &dwEventMask, NULL);
    
    ASSERT(status);
    
    return;
}

void
win32_send_Message(u8 *bytes, u32 size)
{
    u32 num_bytes_written = 0;
    b32 tx_status         = 0; // Transmission
    
    tx_status = WriteFile(global_Device.comm, // Handle to the Serial port
                          bytes,              // Data to be written to the port
                          size,               // Number of bytes to write
                          &num_bytes_written, // Bytes written
                          NULLPTR);
    ASSERT(tx_status);
    
    return;
}

void
win32_read_Message(void)
{
    // Reading
    //RoundBuffer* SerialBuffer = RoundBuffer_Create(256);
    
    u8 TempChar;
    u8 SerialBuffer[256];
    DWORD NoBytesRead;
    int i = 0;
    
    /*
    // What does this do?
    do
    {
        ReadFile( global_Device.comm,           //Handle of the Serial port
                 &TempChar,       //Temporary character
                 sizeof(TempChar),//Size of TempChar
                 &NoBytesRead,    //Number of bytes read
                 NULL);
        
        SerialBuffer[i] = TempChar;
        
        //RoundBuffer_Enqueue(SerialBuffer, &TempChar);// Store Tempchar into buffer
        i++;
    }
    while (NoBytesRead > 0);
    */
    //printf((char*)&(RoundBuffer_Dequeue(SerialBuffer)));
    //printf(SerialBuffer);
    
    
    
    //CloseHandle(global_Device.comm);//Closing the Serial Port
    
    
    return;
}


