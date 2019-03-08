#define F_CPU 16000000UL

#include "mcp2515_defs.h"
#include <avr/io.h>
#include <util/delay.h>

#define PORTCS PORTB
#define DDRCS  DDRB
#define PCS	   2

typedef struct {
	uint8_t cmd;
	uint8_t resp;
	uint16_t hash;
	uint8_t dlc;
	uint8_t data[8];
} canFrame;
void spi_init(void);
uint8_t spi_putc(uint8_t data);
void mcp1525_write_register(uint8_t adress, uint8_t data);
uint8_t mcp2515_read_register(uint8_t adress);
void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data);
uint8_t mcp2515_read_rx_status(void);
void mcp2515_init(void);
void sendCanFrame(canFrame *frame);
void getCanFrame(canFrame *frame);
