#include "bsp/board.h"
#include "device/usbd.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "tusb.h"
#include "usb_descriptors.h"
#include "tusb_config.h"

// Button and LED mapping (change if needed)
static const uint BUTTON_PINS[4] = {2, 3, 4, 5};
static const uint LED_PINS[4]    = {6, 7, 8, 9};

void processJoystickMode(uint16_t *buttons) {
    // Monta a bitmask dos botões: bit 0 -> BUTTON_PINS[0], bit 1 -> BUTTON_PINS[1], ...
    *buttons = 0; // Zera todos os botões antes de atribuir novos
    uint16_t mask = 0;
    for (int i = 0; i < 4; i++) {
        // pull-up está ativo; gpio_get() == 0 quando pressionado
        if (!gpio_get(BUTTON_PINS[i])) {
            mask |= (1u << i);
        }
    }
    *buttons = mask;
}

void joy_task(void) {
    // Atualiza LEDs de acordo com os botões (feedback visual)
    for (int i = 0; i < 4; i++) {
        bool button_pressed = !gpio_get(BUTTON_PINS[i]);
        gpio_put(LED_PINS[i], button_pressed ? 1 : 0);
    }

    if (!tud_hid_ready()) return;

    JoystickReport report = {0}; // Zera a estrutura antes de preencher
    processJoystickMode(&report.buttons); // Atualiza os botões pressionados

    tud_hid_report(0, &report, sizeof(JoystickReport));
}

int main(void) {
    board_init();
    sleep_ms(2000);

    // init buttons
    for (int i = 0; i < 4; i++) {
        gpio_init(BUTTON_PINS[i]);
        gpio_set_dir(BUTTON_PINS[i], GPIO_IN);
        gpio_pull_up(BUTTON_PINS[i]);
    }

    // init leds
    for (int i = 0; i < 4; i++) {
        gpio_init(LED_PINS[i]);
        gpio_set_dir(LED_PINS[i], GPIO_OUT);
        gpio_put(LED_PINS[i], 0);
    }

    tud_init(BOARD_TUD_RHPORT);

    while (true) {
        tud_task();
        joy_task();
        // pequeno delay para evitar busy-loop excessivo (opcional)
        sleep_ms(1);
    }

    return 0;
}
