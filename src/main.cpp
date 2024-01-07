#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/i2c.h"

#include "PCF8575.h"


/// I/O expanders have inverted 0/1
const bool IO_OFF = 1;
const bool IO_ON = 0;

/// Main I/O expander pins
const uint8_t IO_ROOM_ON = 5;
const uint8_t IO_ROOM_OFF = 6;
const uint8_t IO_DESK_TOGGLE = 7;
const uint8_t IO_MAIN_TOGGLE = 0;
const uint8_t IO_CANCEL = 1;
const uint8_t IO_CONFIRM = 2;

/// Relay expander pins
const uint8_t RELAY_COMP = 13;

const uint8_t RELAY_SOCKET_WALL_ON = 7;
const uint8_t RELAY_SOCKET_WALL_OFF = 10;
const uint8_t RELAY_SOCKET_MONITOR_ON = 8;
const uint8_t RELAY_SOCKET_MONITOR_OFF = 4;
const uint8_t RELAY_SOCKET_PROJECTOR_ON = 6;
const uint8_t RELAY_SOCKET_PROJECTOR_OFF = 11;
const uint8_t RELAY_SOCKET_EXTENSION_ON = 9;
const uint8_t RELAY_SOCKET_EXTENSION_OFF = 15;
const uint8_t RELAY_SOCKET_SPEAKERS_ON = 5;
const uint8_t RELAY_SOCKET_SPEAKERS_OFF = 0;
const uint8_t RELAY_ROOM = 14;
const uint8_t RELAY_DESK = 1;


/// Initialise I/O expanders
PCF8575 inputs(20, 21, i2c0, 0x21);
PCF8575 relays(20, 21, i2c0, 0x20);



void vMainTask(void* unused_arg) {
	int comp_state = 0;
	// 0 = off
	// 1 = ready to turn on
	// 2 = waiting for startup
	// 3 = on
	// 4 = ready to turn off
	// 5 = waiting for poweroff

	while (1) {
		while (inputs.read()[IO_MAIN_TOGGLE] == IO_ON) {
			if (comp_state == 0) {
				comp_state = 1;
			}
			else if (comp_state == 1) {
				/// Startup sequence
				/// Power
				relays.write(IO_ON, RELAY_SOCKET_WALL_ON, 1);
				vTaskDelay(250);
				relays.write(IO_OFF, RELAY_SOCKET_WALL_ON, 1);
				relays.write(IO_ON, RELAY_SOCKET_MONITOR_ON, 1);
				vTaskDelay(250);
				relays.write(IO_OFF, RELAY_SOCKET_MONITOR_ON, 1);
				vTaskDelay(250);
				
				/// Computer
				relays.write(IO_ON, RELAY_COMP, 1);
				vTaskDelay(250);
				relays.write(IO_OFF, RELAY_COMP, 1);

				vTaskDelay(250);
				
				comp_state = 2;
			}
			else if (comp_state == 2) {
				vTaskDelay(10000);
				comp_state = 3;
			}
			else if (comp_state == 3) {
				vTaskDelay(1000);
			}
			else if (comp_state == 4) {
				comp_state = 3;
			}
			else if (comp_state == 5) {
				vTaskDelay(20000);
				comp_state = 0;
				break;
			}

			vTaskDelay(250);
		}
		while (inputs.read()[IO_MAIN_TOGGLE] == IO_OFF) {
			if (comp_state == 0) {
				vTaskDelay(1000);
			}
			else if (comp_state == 1) {
				comp_state = 0;
			}
			else if (comp_state == 2) {
				vTaskDelay(10000);
				comp_state = 3;
			}
			else if (comp_state == 3) {
				comp_state = 4;
			}
			else if (comp_state == 4) {
				/// Shutdown sequence
				/// Cancel (reset state to 'on' and break) if not confirmed
				while (relays.read()[IO_CONFIRM] == IO_OFF) {
					if (IO_CANCEL == IO_ON) {
						comp_state = 3;
						break;
					}
					vTaskDelay(50);
				}
				
				vTaskDelay(500);
	
				/// If not canceled, power off.
				/// Press button
				relays.write(IO_ON, RELAY_COMP, 1);
				vTaskDelay(250);
				relays.write(IO_OFF, RELAY_COMP, 1);
	
				/// Wait 20 seconds
				vTaskDelay(20000);
	
				/// Power off at wall
				relays.write(IO_ON, RELAY_SOCKET_WALL_OFF, 1);
				vTaskDelay(250);
				relays.write(IO_OFF, RELAY_SOCKET_WALL_OFF, 1);
				vTaskDelay(250);
	
				/// Set to 'waiting for poweroff' and break.  
				comp_state = 5;
			}
			else if (comp_state == 5) {
				vTaskDelay(20000);
				comp_state = 0;
			}
			vTaskDelay(250);
		}

		vTaskDelay(1000);
	}
}


/// Lights
void vLightsTask(void* unused_arg) {
	bool room_on = 0;
	bool desk_on = 0;

	while (1) {

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

		vTaskDelay(100);
	}
}

int main() {
	stdio_init_all();

	/// Set all I/O expander values to 1 (open/off)
	inputs.write(1, 0, 16);
	relays.write(1, 0, 16);

//	xTaskCreate(vMainTask, "Main task", 4096, NULL, 1, NULL);
	xTaskCreate(vLightsTask, "Lights task", 4096, NULL, 1, NULL);

	vTaskStartScheduler();
}
