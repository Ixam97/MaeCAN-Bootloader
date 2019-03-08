#include "mcp2515_basic.h"


/************************************************************************/
/* SPI                                                                  */
/************************************************************************/

void spi_init(void) {
	//Activating SPI pins
	DDRB |= 0B00101100;
	PORTB &= ~0B00111100;
	
	//Activating SPI master interface, fosc = fclk / 2
	SPCR = (1 << SPE) | (1 << MSTR);
	SPSR = (1 << SPI2X);
}


uint8_t spi_putc(uint8_t data) {
	//Send a byte:
	SPDR = data;
	
	//Wait until byte is sent:
	while (!(SPSR & (1 << SPIF))) {};
	
	return SPDR;
}


/************************************************************************/
/* MCP2515                                                              */
/************************************************************************/

/* Basic */

void mcp1525_write_register(uint8_t adress, uint8_t data) {
	PORTCS &= ~(1 << PCS);
	
	spi_putc(SPI_WRITE);
	spi_putc(adress);
	spi_putc(data);
	
	PORTCS |= (1 << PCS);
}

uint8_t mcp2515_read_register(uint8_t adress) {
	uint8_t data;
	
	PORTCS &= ~(1 << PCS);
	
	spi_putc(SPI_READ);
	spi_putc(adress);
	
	data = spi_putc(0xff);
	
	PORTCS |= (1 << PCS);
	
	return data;
}

void mcp2515_bit_modify(uint8_t adress, uint8_t mask, uint8_t data) {
	PORTCS &= ~(1 << PCS);
	
	spi_putc(SPI_BIT_MODIFY);
	spi_putc(adress);
	spi_putc(mask);
	spi_putc(data);
	
	PORTCS |= (1 << PCS);
}

uint8_t mcp2515_read_rx_status(void) {
	uint8_t data;
	
	PORTCS &= ~(1 << PCS);
	
	spi_putc(SPI_RX_STATUS);
	data = spi_putc(0xff);
	spi_putc(0xff);
	
	PORTCS |= (1 << PCS);
	
	return data;
}

void mcp2515_init(void) {
	spi_init();
	
	PORTCS &= ~(1 << PCS);
	spi_putc(SPI_RESET);
	_delay_ms(1);
	PORTCS |= (1 << PCS);
	
	_delay_ms(10);
	
	mcp1525_write_register(CNF1, 0x41);
	mcp1525_write_register(CNF2, 0xf1);
	mcp1525_write_register(CNF3, 0x85);
	mcp1525_write_register(CANINTE, (1 << RX1IE) | (1 << RX0IE));
	mcp1525_write_register(RXB0CTRL, (1 << RXM1) | (1 << RXM0));
	mcp1525_write_register(RXB1CTRL, (1 << RXM1) | (1 << RXM0));
	
	mcp1525_write_register(RXM0SIDH, 0);
	mcp1525_write_register(RXM0SIDL, 0);
	mcp1525_write_register(RXM0EID8, 0);
	mcp1525_write_register(RXM0EID0, 0);
	
	mcp1525_write_register(RXM1SIDH, 0);
	mcp1525_write_register(RXM1SIDL, 0);
	mcp1525_write_register(RXM1EID8, 0);
	mcp1525_write_register(RXM1EID0, 0);
	
	mcp1525_write_register(BFPCTRL, 0);
	mcp1525_write_register(TXRTSCTRL, 0);
	mcp2515_bit_modify(CANCTRL, 0xe0, 0);
}


void sendCanFrame(canFrame *frame) {
	
	EIMSK &= ~(1 << INT0); // Disable INT0
	
	uint32_t txID = (((uint32_t)frame->cmd << 17) | ((uint32_t)frame->resp << 16) | frame->hash);
	
	PORTCS &= ~(1 << PCS);
	spi_putc(SPI_WRITE_TX);
	
	// Set frame ID
	spi_putc((uint8_t)((txID >> 16) >> 5));
	spi_putc(((uint8_t)((txID >> 16) & 0x03)) | 0x08 | (uint8_t)(((txID >> 16) & 0x1c) << 3));
	spi_putc((uint8_t)(txID >> 8));
	spi_putc((uint8_t)(txID & 0xff));
	
	// Set frame dlc
	spi_putc(frame->dlc);
	
	// Set frame data
	for (uint8_t i = 0; i < 8; i++) {
		spi_putc(frame->data[i]);
	}
	PORTCS |= (1 << PCS);
	
	//Send CAN Frame:
	PORTCS &= ~(1 << PCS);
	spi_putc(SPI_RTS | 0x01);
	PORTCS |= (1 << PCS);
	
	EIMSK |= (1 << INT0); // Enable INT0
	
	_delay_ms(1);
}

void getCanFrame(canFrame *frame) {
	
	uint32_t rxID;
	uint8_t status = mcp2515_read_rx_status();
	
	if(status & (1 << 6)) {
		PORTCS &= ~(1 << PCS);
		spi_putc(SPI_READ_RX);
		} else if (status & (1 << 7)) {
		PORTCS &= ~(1 << PCS);
		spi_putc(SPI_READ_RX | 0x04);
		} else {
		return 0xff;
	}
	
	rxID = (uint32_t) (spi_putc(0xff) << 3);
	uint8_t sidl = spi_putc(0xff);
	rxID |= sidl >> 5;
	rxID = (rxID << 2) | (sidl & 0x03);
	rxID = (rxID << 8) | spi_putc(0xff);
	rxID = (rxID << 8) | spi_putc(0xff);
	
	frame->cmd = (uint8_t)(rxID >> 17);
	frame->resp = (uint8_t)(rxID >> 16) & 0x01;
	frame->hash = (uint16_t)(rxID & 0xffff);
	
	frame->dlc = spi_putc(0xff) & 0x0f;
	
	for (uint8_t i = 0; i < 8; i++) {
		frame->data[i] = spi_putc(0xff);
	}
	
	PORTCS |= (1 << PCS);
	
	if (status & ( 1 << 6)) {
		mcp2515_bit_modify(CANINTF, (1 << RX0IF), 0);
		} else {
		mcp2515_bit_modify(CANINTF, (1 << RX1IF), 0);
	}
}

void mcp2515_reset_int(uint8_t status) {
	
}
