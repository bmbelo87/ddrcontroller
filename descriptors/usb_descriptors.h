#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include "tusb.h"

enum
{
    REPORT_ID_KEYBOARD,
    REPORT_ID_MOUSE,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_GAMEPAD = 1,
    REPORT_ID_COUNT
};

typedef enum {
    ITF_NUM_GAMEPAD = 0,
    ITF_NUM_GAMEPAD_TOTAL = 1
} interface_numbers_t;

// usb_descriptors.h (Original)
typedef struct {
    uint16_t buttons; // 2 bytes
    int8_t x;         // 1 byte
    int8_t y;         // 1 byte
} JoystickReport; // 4 bytes

#endif /* USB_DESCRIPTORS_H_ */