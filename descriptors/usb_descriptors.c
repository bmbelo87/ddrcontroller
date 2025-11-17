#include "tusb.h"
#include "usb_descriptors.h"
#include "class/hid/hid_device.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]       MIDI | HID | MSC | CDC          [LSB]
 */

#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

// Definições GamePad
#define HID_ENDPOINT_SIZE 64
#define ENDPOINT0_SIZE    64
#define HID_PROTOCOL_NONE 0x00
#define HID_SUBCLASS_BOOT 0x01
#define HID_DESC_TYPE_HID 0x21
#define HID_DESC_TYPE_REPORT 0x22
#define GAMEPAD_INTERFACE    0
#define GAMEPAD_ENDPOINT    1
#define GAMEPAD_SIZE        64

// Ajustes de endpoint/tamanho
#define EPNUM_GAMEPAD_IN  0x81   // 0x80 | 1 (IN endpoint 1)
#define JOY_CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

uint8_t const desc_joy_report[] = {
    0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,  // USAGE (Joystick)
    0xA1, 0x01,  // COLLECTION (Application)

    // Botões (12 botões)
    0x05, 0x09,  //   USAGE_PAGE (Button)
    0x19, 0x01,  //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,  //   USAGE_MAXIMUM (Button 16)
    0x15, 0x00,  //   LOGICAL_MINIMUM (0)
    0x25, 0x01,  //   LOGICAL_MAXIMUM (1)
    0x95, 0x10,  //   REPORT_COUNT (16)
    0x75, 0x01,  //   REPORT_SIZE (1)
    0x81, 0x02,  //   INPUT (Data,Var,Abs)

    // Eixos (X e Y)
    0x05, 0x01,  //   USAGE_PAGE (Generic Desktop)
    0x09, 0x30,  //   USAGE (X)
    0x09, 0x31,  //   USAGE (Y)
    0x15, 0x81,  //   LOGICAL_MINIMUM (-127)
    0x25, 0x7F,  //   LOGICAL_MAXIMUM (127)
    0x75, 0x08,  //   REPORT_SIZE (8 bits por eixo)
    0x95, 0x02,  //   REPORT_COUNT (2 eixos)
    0x81, 0x02,  //   INPUT (Data,Var,Abs)

    0xC0         // END_COLLECTION
  };

// CALLBACK do report descriptor (ok)
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance) {
    (void) instance;
    return desc_joy_report;
}

// STRINGS (ajustado)
char const* slvrio_string_desc_arr [] =
{
    (const char[]) { 0x09, 0x04 }, // 0: LANGID (0x0409)
    "SilverTech",                   // 1: Manufacturer
    "Dance Dance Revolution Pad",   // 2: Product
    "000001",                       // 3: Serial
    "",                             // 4: (vazio se não usar)
    "SLVRIO"                        // 5: Vendor Interface (ou ajuste índice)
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    if ( index == 0 )
    {
        // LANGID descriptor (0x0409)
        _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2*1 + 2));
        _desc_str[1] = 0x0409;
        return _desc_str;
    }

    // valida índice
    uint32_t string_count = sizeof(slvrio_string_desc_arr) / sizeof(slvrio_string_desc_arr[0]);
    if ( index >= string_count ) return NULL;

    const char* str = slvrio_string_desc_arr[index];
    uint8_t chr_count = (uint8_t) strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    for ( uint8_t i = 0; i < chr_count; i++ )
    {
        _desc_str[1 + i] = (uint16_t) str[i];
    }

    _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8) | (2*chr_count + 2));
    return _desc_str;
}

// DEVICE DESCRIPTOR
uint8_t const * tud_descriptor_device_cb(void)
{
    static tusb_desc_device_t hid_desc_device = {
 // Aplica o hid_desc_device para controle
            .bLength            = sizeof(tusb_desc_device_t),
            .bDescriptorType    = TUSB_DESC_DEVICE,
            .bcdUSB             = 0x0200,

            .bDeviceClass       = 0x00,
            .bDeviceSubClass    = 0x00,
            .bDeviceProtocol    = 0x00,
            .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

            .idVendor           = 0x0547,
            .idProduct          = 0x1003,
            .bcdDevice          = 0x0100,

            .iManufacturer      = 0x01,
            .iProduct           = 0x02,
            .iSerialNumber      = 0x00,

            .bNumConfigurations = 0x01
    };
    return (uint8_t const *) &hid_desc_device;
}

// CONFIGURATION DESCRIPTOR — CRIEI O ARRAY E RETORNEI
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;

    static uint8_t const desc_configuration[] = {
        TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_GAMEPAD_TOTAL, 0, JOY_CONFIG_TOTAL_LEN, 0x00, 500),
        TUD_HID_DESCRIPTOR(ITF_NUM_GAMEPAD, 5 /*string index*/, HID_ITF_PROTOCOL_NONE, sizeof(desc_joy_report), EPNUM_GAMEPAD_IN, 64, 10)
    };

    return desc_configuration;
}

// Callback para lidar com solicitações GET_REPORT 
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) 
{ 
    (void) instance; 
    (void) report_id; 
    (void) report_type; 
    (void) buffer; 
    (void) reqlen; 
    return 0; 
}

// Callback para lidar com solicitações SET_REPORT 
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) 
{ 
    (void) instance; 
    (void) report_id; 
    (void) report_type; 
    (void) buffer; 
    (void) bufsize; 
}