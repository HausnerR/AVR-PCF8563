# AVR PCF8563 #

This is C library for communication between PCF8563 and AVR uC. Written for Atmega88PA, but should work with other microprocessors without modifications (if it have hardware TWI/I2C interface).

It's implement almost all PCF features:

* Date and time
* Alarm
* Clock out
* Timer
* Interrupt flags

### Setup ###

Just copy library to your project and use it.

Functions are well named so I think long description are not required.

If you have any questions - feel free to ask me by mail: **kuba [at] pachciarek [dot] pl**

### Examples ###

Pre: All constants are defined in header file. If something is not well described, look into PCF documentation. This library use standard Atmel TWI interface to communicate with PCF8563.

To initialize communication with PCF, use function:

```
#!c
PCF_Init();
```

You can initialize PCF with one or many interrupts enabled:

```
#!c
PCF_Init(PCF_ALARM_INTERRUPT_ENABLE | PCF_TIMER_INTERRUPT_ENABLE);
```

To set clock output use function:

```
#!c
PCF_SetClockOut(PCF_CLKOUT_1024HZ);
```

To set PCF date and time you need to create defined structure and pass it to function that send it to PCF:

```
#!c
PCF_DateTime dateTime;
dateTime.second = 43;
dateTime.minute = 59;
dateTime.hour = 20;
dateTime.day = 15;
dateTime.weekday = 6;
dateTime.month = 8;
dateTime.year = 2015;

PCF_SetDateTime(&dateTime);
```

Getting date and time information from PCF looks similar. Create structure and pass it to function to get data:

```
#!c
PCF_DateTime pcfDateTime;
PCF_GetDateTime(&pcfDateTime);
```

Setting and disabling timer:

```
#!c
PCF_SetTimer(PCF_TIMER_1HZ, 100); //Sets 1Hz ticks and counting to 100. Max 255
PCF_SetTimer(PCF_TIMER_DISABLED, 0); //Disables timer
```

Alarm settings looks like date time setting. Create structure, fill them and pass to function. PCF_DISABLE_ALARM makes given parameter "transparent". So in this example alarm turns on every day at 22:22:

```
#!c
PCF_Alarm pcfAlarm;
pcfAlarm.minute = 22;
pcfAlarm.hour = 22;
pcfAlarm.day = PCF_DISABLE_ALARM;
pcfAlarm.weekday = PCF_DISABLE_ALARM;
PCF_SetAlarm(&pcfAlarm);
```

And if you are use PCF interrupts, use something like that to get information about interrupt source:

```
#!c
uint8_t flags = PCF_GetAndClearFlags();

//Alarm enabled
if (flags & PCF_ALARM_FLAG)
{
}

//Timer countdown end
if (flags & PCF_TIMER_FLAG)
{
}
```