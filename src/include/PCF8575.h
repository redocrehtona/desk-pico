#ifndef PCF8575_H
#define PCF8575_H

#include "pico/stdlib.h"

#include <stdio.h>
#include "hardware/i2c.h"

class PCF8575{
	private:
	i2c_inst_t *I2C;
	uint8_t I2C_ADDR;

	public:
	PCF8575(int sda_pin, int scl_pin, i2c_inst_t *i2c_bus, uint8_t expander_i2c_address) {
		I2C = i2c_bus;
		I2C_ADDR = expander_i2c_address;
	
		i2c_init(I2C, 400 * 1000);
	
		gpio_set_function(sda_pin, GPIO_FUNC_I2C);
		gpio_set_function(scl_pin, GPIO_FUNC_I2C);

	}

	void write(int pin, bool state) {

		uint16_t data = 0xffff;
		uint16_t mask = 0x0000;

		uint8_t msg[2] = { 0x00, 0x00 };

		mask |= ( state << pin );
	
		data ^= mask;

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
