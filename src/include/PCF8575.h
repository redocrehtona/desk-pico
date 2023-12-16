#ifndef PCF8575_H
#define PCF8575_H

#include "pico/stdlib.h"

#include <stdio.h>
#include "hardware/i2c.h"

class PCF8575{
	private:
	i2c_inst_t *I2C;
	uint8_t I2C_ADDR;
	uint16_t data = 0x0000;

	public:
	PCF8575(int sda_pin, int scl_pin, i2c_inst_t *i2c_bus, uint8_t expander_i2c_address) {
		I2C = i2c_bus;
		I2C_ADDR = expander_i2c_address;
	
		i2c_init(I2C, 400 * 1000);
	
		gpio_set_function(sda_pin, GPIO_FUNC_I2C);
		gpio_set_function(scl_pin, GPIO_FUNC_I2C);

	}

	void write(bool state, int first_pin, int pin_count) {
		// If we are setting the bit(s)
		if (state) {
			for (int i = 0; i < pin_count; i++) {
				data |= ((uint8_t)1 << first_pin + i);
			}
		}
		// Else we are clearing the bit(s)
		else {
			for (int i = 0; i < pin_count; i++) {
				data &= ~((uint8_t)1 << first_pin + i);
			}
		}

		uint8_t msg[2] = { 0x00, 0x00 };

		msg[0] = (uint8_t)((data & 0xff00) >> 8); msg[1] = (uint8_t)(data & 0x00ff);

		i2c_write_blocking(I2C, I2C_ADDR, msg, 2, true);
	}

	int *read() {
		static int read_values[16] = {
			0, 0, 0, 0, 0, 0, 0, 0, 
			0, 0, 0, 0, 0, 0, 0, 0
		};

		uint8_t read[2] = { 0x00, 0x00 };

		i2c_read_blocking(I2C, I2C_ADDR, read, 2, true);

		// Copy read values to array
		// First 8-bit integer (pins 0 to 7)
		for ( int i = 0; i < 8; i++ ) {
			read_values[i] = ( ( read[0] & (1 << i) ) ? 1 : 0 );
		}
		// Second 8-bit integer (pins 8 to 15)
		for ( int i = 0; i < 8; i++ ) {
			read_values[i + 8] = ( ( read[1] & (1 << i) ) ? 1 : 0 );
		}


		return read_values;
	};
};

#endif
