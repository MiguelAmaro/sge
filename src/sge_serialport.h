// NOTE(MIGUEL): Should This code go in the windows platfom layer?? and i just keep abastractions and generics here
#include <windows.h>
#include "RingBuffer.h"
#include "LAL.h"

typedef struct
{
    HANDLE comm;
    RingBuffer transmit_queue;
    RingBuffer recieve_queue;
    u16 padding;
} Device_Comm;

extern Device_Comm global_Device;

void
win32_serial_Port_Init(void);

void
win32_send_Message(u8 *bytes, u32 size);

void
win32_read_Message(void);


