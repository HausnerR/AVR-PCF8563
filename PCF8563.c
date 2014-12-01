/*
 * PCF8563.c
 *
 * Created: 2014-11-18 20:00:32
 *  Author: Jakub Pachciarek
 *
 * Not implemented:
 * - TEST1, STOP, TESTC, TI_TP bits - can be accessed trough raw PCF_Write and PCF_Read
 */ 

#include <avr/io.h>
#include "PCF8563.h"


//PHYSICAL LAYER

void TWI_Init()
{
	//About 100kHz for 1.6MHz clock
	TWBR = 0;										//Set bitrate factor to 0
	TWSR &= ~((1<<TWPS1) | (1<<TWPS0));				//Set prescaler to 1
}

void TWI_Start()
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTA);
	while (!(TWCR & (1<<TWINT)));
}

void TWI_Stop()
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	while ((TWCR & (1<<TWSTO)));
}

uint8_t TWI_Read(uint8_t ack)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (((ack ? 1 : 0)<<TWEA));
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}

void TWI_Write(uint8_t byte)
{
	TWDR = byte;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}


//COMMUNICATION LAYER

void PCF_Write(uint8_t addr, uint8_t *data, uint8_t count) {
	TWI_Start();

	TWI_Write(PCF8563_WRITE_ADDR);
	TWI_Write(addr);

	while (count) {
		count--;

		TWI_Write(*data);
		data++;
	}

	TWI_Stop();

}

void PCF_Read(uint8_t addr, uint8_t *data, uint8_t count) {
	TWI_Start();

	TWI_Write(PCF8563_WRITE_ADDR);
	TWI_Write(addr);

	TWI_Stop();
	TWI_Start();

	TWI_Write(PCF8563_READ_ADDR);

	while (count)
	{
		count--;

		*data = TWI_Read(count);
		data++;
	}

	TWI_Stop();

}


//APPLICATION LAYER

//0 - 99 binary to BCD code conversion
#define BinToBCD(bin) ((((bin) / 10) << 4) + ((bin) % 10))


void PCF_Init(uint8_t mode)
{
	TWI_Init();

	uint8_t tmp = 0b00000000;
	PCF_Write(0x00, &tmp, 1);				//Control_status_1

	mode &= 0b00010011;						//Mask unnecessary bits
	PCF_Write(0x01, &mode, 1);				//Control_status_2
}

uint8_t PCF_GetAndClearFlags()
{
	uint8_t flags;
	PCF_Read(0x01, &flags, 1);				//Control_status_2

	uint8_t cleared = flags & 0b00010011;	//Mask only configuration bits
	PCF_Write(0x01, &cleared, 1);			//Control_status_2

	return flags & 0x0C;					//Mask unnecessary bits
}

void PCF_SetClockOut(uint8_t mode)
{
	mode &= 0b10000011;
	PCF_Write(0x0D, &mode, 1);				//CLKOUT_control
}

void PCF_SetTimer(uint8_t mode, uint8_t count)
{
	mode &= 0b10000011;
	PCF_Write(0x0E, &mode, 1);				//Timer_control

	PCF_Write(0x0F, &count, 1);				//Timer
}

uint8_t PCF_GetTimer()
{
	uint8_t count;
	PCF_Read(0x0F, &count, 1);				//Timer

	return count;
}

uint8_t PCF_SetAlarm(PCF_Alarm *alarm)
{
	if ((alarm->minute >= 60 && alarm->minute != 80) || (alarm->hour >= 24 && alarm->hour != 80) || (alarm->day > 32 && alarm->day != 80) || (alarm->weekday > 6 && alarm->weekday != 80))
	{
		return 1;
	}

	uint8_t buffer[4];

	buffer[0] = BinToBCD(alarm->minute) & 0xFF;
	buffer[1] = BinToBCD(alarm->hour) & 0xBF;
	buffer[2] = BinToBCD(alarm->day) & 0xBF;
	buffer[3] = BinToBCD(alarm->weekday) & 0x87;

	PCF_Write(0x09, buffer, sizeof(buffer));

	return 0;
}

uint8_t PCF_GetAlarm(PCF_Alarm *alarm)
{
	uint8_t buffer[4];

	PCF_Read(0x09, buffer, sizeof(buffer));

	alarm->minute = (((buffer[0] >> 4) & 0x0F) * 10) + (buffer[0] & 0x0F);
	alarm->hour = (((buffer[1] >> 4) & 0x0B) * 10) + (buffer[1] & 0x0F);
	alarm->day = (((buffer[2] >> 4) & 0x0B) * 10) + (buffer[2] & 0x0F);
	alarm->weekday = (((buffer[3] >> 4) & 0x08) * 10) + (buffer[3] & 0x07);

	return 0;
}

uint8_t PCF_SetDateTime(PCF_DateTime *dateTime)
{
	if (dateTime->second >= 60 || dateTime->minute >= 60 || dateTime->hour >= 24 || dateTime->day > 32 || dateTime->weekday > 6 || dateTime->month > 12 || dateTime->year < 1900 || dateTime->year >= 2100)
	{
		return 1;
	}

	uint8_t buffer[7];

	buffer[0] = BinToBCD(dateTime->second) & 0x7F;
	buffer[1] = BinToBCD(dateTime->minute) & 0x7F;
	buffer[2] = BinToBCD(dateTime->hour) & 0x3F;
	buffer[3] = BinToBCD(dateTime->day) & 0x3F;
	buffer[4] = BinToBCD(dateTime->weekday) & 0x07;
	buffer[5] = BinToBCD(dateTime->month) & 0x1F;

	if (dateTime->year >= 2000)
	{
		buffer[5] |= 0x80;
		buffer[6] = BinToBCD(dateTime->year - 2000);
	}
	else
	{
		buffer[6] = BinToBCD(dateTime->year - 1900);
	}

	PCF_Write(0x02, buffer, sizeof(buffer));

	return 0;
}

uint8_t PCF_GetDateTime(PCF_DateTime *dateTime)
{
	uint8_t buffer[7];

	PCF_Read(0x02, buffer, sizeof(buffer));

	dateTime->second = (((buffer[0] >> 4) & 0x07) * 10) + (buffer[0] & 0x0F);
	dateTime->minute = (((buffer[1] >> 4) & 0x07) * 10) + (buffer[1] & 0x0F);
	dateTime->hour = (((buffer[2] >> 4) & 0x03) * 10) + (buffer[2] & 0x0F);
	dateTime->day = (((buffer[3] >> 4) & 0x03) * 10) + (buffer[3] & 0x0F);
	dateTime->weekday = (buffer[4] & 0x07);
	dateTime->month = ((buffer[5] >> 4) & 0x01) * 10 + (buffer[5] & 0x0F);
	dateTime->year = 1900 + ((buffer[6] >> 4) & 0x0F) * 10 + (buffer[6] & 0x0F);

	if (buffer[5] &  0x80)
	{
		dateTime->year += 100;
	}

	if (buffer[0] & 0x80) //Clock integrity not guaranted
	{
		return 1;
	}

	return 0;
}