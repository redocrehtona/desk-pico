#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/i2c.h"

#include "PCF8575.h"

const bool IO_OFF = 1;
const bool IO_ON = 0;

const uint8_t IO_ROOM_ON = 5;
const uint8_t IO_ROOM_OFF = 6;
const uint8_t IO_DESK_TOGGLE = 7;

const uint8_t RELAY_ROOM = 14;
const uint8_t RELAY_DESK = 1;

bool room_on = 0;
bool desk_on = 0;
 
void vMainTask(void* unused_arg) {	
	PCF8575 inputs(20, 21, i2c0, 0x21);

	PCF8575 relays(20, 21, i2c0, 0x20);

        // Set all pins to high (input/off)
	inputs.write(1, 0, 16);
	relays.write(1, 0, 16);

	for (;;) {

		/// Room light
		if (inputs.read()[IO_ROOM_ON] == IO_ON && !room_on) {
			relays.write(IO_ON, RELAY_ROOM, 1);
			room_on = 1;
		}
		else if (inputs.read()[IO_ROOM_OFF] == IO_ON && room_on) {
			relays.write(IO_OFF, RELAY_ROOM, 1);
			room_on = 0;
		}

		/// Desk light
		if (inputs.read()[IO_DESK_TOGGLE] == IO_ON && !desk_on) {
			relays.write(IO_ON, RELAY_DESK, 1);
			desk_on = 1;
		}
		else if (inputs.read()[IO_DESK_TOGGLE] == IO_OFF && desk_on) {
			relays.write(IO_OFF, RELAY_DESK, 1);
			desk_on = 0;
		}



		vTaskDelay(10);
	}
}

int main() {
	stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	xTaskCreate(vMainTask, "Main task", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
