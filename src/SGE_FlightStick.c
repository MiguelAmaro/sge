#define DIRECTINPUT_VERSION 0x0800
#include "SGE_Platform.h"
#include "LAL.h"
#include <windows.h>
#include <dinput.h>
#include <dinputd.h>

global LPDIRECTINPUT8       Device_Interface;
global LPDIRECTINPUTDEVICE8 global_Joystick;
global DIDEVCAPS            Capabilities;



//TODO: Include "direct input" in func name
BOOL Win32ErrorChecking(HRESULT result, const char* expectation){
    printf("%s - ", expectation);
    switch(result)
    {
        case DI_OK:
        {
            printf("Successful ! \n");
            return TRUE;
        }
        break;
        case DIERR_BETADIRECTINPUTVERSION:
        {
            printf("Error: %s", "Beta Direct Input version \n");
            return FALSE;
        }
        break;
        case DIERR_INVALIDPARAM:
        {
            printf("Error: %s", "Invalid Parameter \n  ");
            return FALSE;
        }
        break;
        case DIERR_OLDDIRECTINPUTVERSION:
        {
            printf("Error: %s", "Old Direct Input Version \n");
            return FALSE;
        }
        break;
        case DIERR_NOTINITIALIZED:
        {
            printf("Error: %s", "Not initialized\n");
            return FALSE;
        }
        break;
        case DIERR_OUTOFMEMORY:
        {
            printf("Error: %s", "Out of Memory \n");
            return FALSE;
        }
        break;
        case DIERR_DEVICENOTREG:
        {
            printf("Error: %s", "Error: Device Not Registered \n");
            return FALSE;
        }
        break;
        case E_HANDLE:
        {
            printf("Error: %s", "Error with the handle \n");
            return FALSE;
        }
        break;
        default:
        {
            printf("Error: %s", "Error Log Failed \n");
            return FALSE;
        }
    }
}


BOOL CALLBACK DirectInput_Enum_Callback(LPCDIDEVICEINSTANCE device, LPVOID prevInstance) {
    HRESULT result;
    
    if(device->dwDevType & ~DI8DEVTYPE_FLIGHT)
    {
        // Only Instantiating Stick for now
        
        printf("Flight Stuff: %s \n\r", device->tszInstanceName);
        
        result = Device_Interface->lpVtbl->CreateDevice(Device_Interface,
                                                        &device->guidInstance,
                                                        &global_Joystick,
                                                        NULL);
        
        Win32ErrorChecking(result, "Create a Device");
        
        
        return DIENUM_STOP;
    }
    
    return DIENUM_CONTINUE;
}




// TODO(Miguel): Replace args with something more generic that could work on Qt
void CreateDevice(HWND Window, HINSTANCE Instance) {
    
    //******************************
    // INPUT DEVICE SETUP
    //
    // CREATING A DEVICE INTERFACE
    //******************************
    
    
    HRESULT result;
    result = DirectInput8Create(Instance,
                                DIRECTINPUT_VERSION,
                                &IID_IDirectInput8,
                                (VOID**)&Device_Interface,
                                NULL);
    
    
    Win32ErrorChecking(result, "Create Device Interface");
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // QUERYING DEVICES ON SYSTEM FOR STICK
    //**************************************
    
    result = Device_Interface->lpVtbl->EnumDevices(
                                                   Device_Interface,
                                                   DI8DEVCLASS_GAMECTRL,
                                                   DirectInput_Enum_Callback,
                                                   NULL,
                                                   DIEDFL_ATTACHEDONLY);
    
    Win32ErrorChecking(result, "Enumerate Devices And Fetch Desired One");
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // TELL THE OS WHAT DATA FORMAT TO EXPECT
    //**************************************
    
    result = global_Joystick->lpVtbl->SetDataFormat(global_Joystick,
                                                    &c_dfDIJoystick2);
    
    Win32ErrorChecking(result, "Set Device Data Format");
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // UHHHMMM
    //**************************************
    
    result = global_Joystick->lpVtbl->SetCooperativeLevel(global_Joystick,
                                                          Window,
                                                          DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    
    Win32ErrorChecking(result, "Set Device Cooperative Levels");
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // FIND OUT THE CAPABILETIES OF THE STICK
    //**************************************
    
    // Try to print capabilities later
    Capabilities.dwSize = sizeof(DIDEVCAPS);
    
    result = global_Joystick->lpVtbl->GetCapabilities(global_Joystick,
                                                      &Capabilities);
    
    Win32ErrorChecking(result, "Get Device Capabilities");
    
    
    //**************************************
    // INPUT DEVICE SETUP
    //
    // CONFIGURE AXES AND BUTTONS
    //**************************************
    //https://www.cs.cmu.edu/~jparise/directx/joystick/
    // Use Defaults configure later ^ (Enumerating Axes)
    //Joystick->lpVtbl->EnumObjects(Joystick, enumAxesCallback, NULL, DIDFT_AXIS)
    
    
    return;
}

HRESULT Joystick_Poll(Platform *platform) 
{
    HRESULT result;
    DIJOYSTATE2 joystick_state;
    
    if(global_Joystick == NULL)
    {
        return S_OK;
    }
    
    result = global_Joystick->lpVtbl->Poll(global_Joystick);
    
    if(FAILED(result))
    {
        result = global_Joystick->lpVtbl->Acquire(global_Joystick);
        
        while (result == DIERR_INPUTLOST) 
        {
            result = global_Joystick->lpVtbl->Acquire(global_Joystick);
        }
        
        if ((result == DIERR_INVALIDPARAM) || (result == DIERR_NOTINITIALIZED)) 
        {
            return E_FAIL;
        }
        
        if (result == DIERR_OTHERAPPHASPRIO) 
        {
            return S_OK;
        }
    }
    if (FAILED(result = global_Joystick->lpVtbl->GetDeviceState(global_Joystick, sizeof(DIJOYSTATE2), &joystick_state))) 
    {
        return result; // The device should have been acquired during the Poll()
    }
    
    platform->stick_x = joystick_state.lX;
    platform->stick_y = joystick_state.lY;
    
    return S_OK;
}

