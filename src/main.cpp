#include "pico/stdlib.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "hardware/i2c.h"

#include "PCF8575.h"

void gpio_callback(uint gpio, uint32_t event_mask) {
	printf("INTERRUPT\n");
}
 
void vInputHandler(void* unused_arg) {	
	PCF8575 inputs(20, 21, i2c0, 0x21);

	PCF8575 relays(20, 21, i2c0, 0x20);

        // Set all pins to high (input/off)
	inputs.write(1, 0, 16);
	relays.write(1, 0, 16);
	
	int loops = 0;

	for (;;) {
		printf("Running for %d seconds\nPin 16 state: %d\n", loops, gpio_get(16));
		loops++;
		vTaskDelay(1000);
	}
}

int main() {
	stdio_init_all();

	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

	gpio_init(16);
	gpio_set_dir(16, GPIO_IN);

	gpio_set_irq_enabled_with_callback(16, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

	xTaskCreate(vInputHandler, "Read relay states", 8192, NULL, 1, NULL);

	vTaskStartScheduler();
}
