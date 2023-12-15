#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include "hardware/i2c.h"

#include "PCF8575.h"

/*
void setRelay(int relay, bool state) {

	const uint sda_pin = 20;
	const uint scl_pin = 21;

	i2c_inst_t *i2c = i2c0;

	stdio_init_all();

	i2c_init(i2c, 400 * 1000);

	gpio_set_function(sda_pin, GPIO_FUNC_I2C);
	gpio_set_function(scl_pin, GPIO_FUNC_I2C);


	uint16_t data = 0xffff;
	uint8_t msg[2] = { 0x00, 0x00 };
	uint16_t mask = 0x0000;

	mask |= ( state << relay );
	
	data ^= mask;

	msg[0] = (uint8_t)((data & 0xff00) >> 8); msg[1] = (uint8_t)(data & 0x00ff);

	i2c_write_blocking(i2c, 0x20, msg, 2, true);
}
*/

void vWriteTask(void* unused_arg) {
	int relay = 0;

	i2c_inst_t *relays_i2c_bus = i2c0;
	uint8_t relays_i2c_address = 0x20;
	PCF8575 relays(20, 21, relays_i2c_bus, relays_i2c_address);

	for (;;) {
		gpio_put(PICO_DEFAULT_LED_PIN, 1);
		relays.write(relay, 1);
		vTaskDelay(250);
		gpio_put(PICO_DEFAULT_LED_PIN, 0);
		relays.write(relay, 0);
		vTaskDelay(250);
		relay++;
		if (relay >= 16) {
			relay = 0;
		}
	}
}

void vReadTask(void* unused_arg) {

	i2c_inst_t *inputs_i2c_bus = i2c0;
	uint8_t inputs_i2c_address = 0x20;
	PCF8575 inputs(20, 21, inputs_i2c_bus, inputs_i2c_address);

	int* ptr; 
	

	for (;;) {

		ptr = inputs.read();

		for ( int i = 0; i < 16; i++ ) {
			printf("%d", inputs.read()[i]);
			printf("  ");
		}
		printf("\n");
		vTaskDelay(250);

	}}

int main() {
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	xTaskCreate(vWriteTask, "Write relay states", 4096, NULL, 1, NULL);
	xTaskCreate(vReadTask, "Read relay states", 4096, NULL, 1, NULL);

	vTaskStartScheduler();
}
