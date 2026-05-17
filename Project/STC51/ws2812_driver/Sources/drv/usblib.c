//<<AICUBE_USER_HEADER_REMARK_BEGIN>>
////////////////////////////////////////
// File name: usblib.c
// File desc: Minimal USB control for ws2812 test
// File ver : V1.0
////////////////////////////////////////
//<<AICUBE_USER_HEADER_REMARK_END>>

#include "config.h"

//<<AICUBE_USER_INCLUDE_BEGIN>>
#include "gp_led_matrix_bt_debug.h"
#include "gp_led_matrix_usb_debug.h"
//<<AICUBE_USER_INCLUDE_END>>

//<<AICUBE_USER_GLOBAL_DEFINE_BEGIN>>
//<<AICUBE_USER_GLOBAL_DEFINE_END>>

////////////////////////////////////////
// USB init
////////////////////////////////////////
void USBLIB_Init(void)
{
    usb_init();
    USB_SetIntPriority(0);
    set_usb_OUT_callback(USBLIB_OUT_Callback);
    set_usb_ispcmd("@STCISP#");
}

////////////////////////////////////////
// Wait USB configured
////////////////////////////////////////
void USBLIB_WaitConfiged(void)
{
    while (DeviceState != DEVSTATE_CONFIGURED)
    {
        WDT_Clear();
    }
}

////////////////////////////////////////
// USB OUT callback
////////////////////////////////////////
void USBLIB_OUT_Callback(void)
{
    if ((OutNumber >= 5U)
        && (UsbOutBuffer[0] == 'D')
        && (UsbOutBuffer[1] == 'E')
        && (UsbOutBuffer[2] == 'B')
        && (UsbOutBuffer[3] == 'U')
        && (UsbOutBuffer[4] == 'G'))
    {
        /* Temporary: USB "DEBUG" command enters row-test debug loop. */
        GpLedMatrixUsbDebug_Enter();
        return;
    }

    if ((OutNumber >= 3U)
        && (UsbOutBuffer[0] == 'B')
        && (UsbOutBuffer[1] == 'T'))
    {
        /* Route the single BT text command to the HC-05 debug helper. */
        GpLedMatrixBtDebug_HandleUsbCommand(&UsbOutBuffer[2], (uint8_t)(OutNumber - 2U));
        return;
    }
}

//<<AICUBE_USER_FUNCTION_IMPLEMENT_BEGIN>>
//<<AICUBE_USER_FUNCTION_IMPLEMENT_END>>
