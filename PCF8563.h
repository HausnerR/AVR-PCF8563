/*
 * PCF8563.h
 *
 * Created: 2014-11-18 20:00:42
 *  Author: Jakub Pachciarek
 */ 


#ifndef PCF8563_H_
#define PCF8563_H_


#define PCF8563_READ_ADDR				0xA3
#define PCF8563_WRITE_ADDR				0xA2

#define PCF_ALARM_FLAG					(1<<3)
#define PCF_TIMER_FLAG					(1<<2)
#define PCF_ALARM_INTERRUPT_ENABLE		(1<<1)
#define PCF_TIMER_INTERRUPT_ENABLE		(1<<0)

#define PCF_CLKOUT_32768HZ				0b10000000
#define PCF_CLKOUT_1024HZ				0b10000001
#define PCF_CLKOUT_32HZ					0b10000010
#define PCF_CLKOUT_1HZ					0b10000011
#define PCF_CLKOUT_DISABLED				0b00000000

#define PCF_TIMER_4096HZ				0b10000000
#define PCF_TIMER_64HZ					0b10000001
#define PCF_TIMER_1HZ					0b10000010
#define PCF_TIMER_1_60HZ				0b10000011
#define PCF_TIMER_DISABLED				0b00000011

#define PCF_DISABLE_ALARM				80


typedef struct {
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t weekday;
} PCF_Alarm;

typedef struct {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t weekday;
	uint8_t month;
	uint16_t year;
} PCF_DateTime;


void PCF_Write(uint8_t addr, uint8_t *data, uint8_t count);
void PCF_Read(uint8_t addr, uint8_t *data, uint8_t count);

void PCF_Init(uint8_t mode);
uint8_t PCF_GetAndClearFlags(void);

void PCF_SetClockOut(uint8_t mode);

void PCF_SetTimer(uint8_t mode, uint8_t count);
uint8_t PCF_GetTimer(void);

uint8_t PCF_SetAlarm(PCF_Alarm *alarm);
uint8_t PCF_GetAlarm(PCF_Alarm *alarm);

uint8_t PCF_SetDateTime(PCF_DateTime *dateTime);
uint8_t PCF_GetDateTime(PCF_DateTime *dateTime);


#endif /* PCF8563_H_ */