#ifndef UTIL_H
#define UTIL_H

#include "Arduino.h"
#include <stdio.h>
#include <avr/pgmspace.h>

#define ADD_PRINTF_SUPPORT()												\
	static FILE uartout = {0};												\
	static int uart_putchar(char c, FILE* stream)							\
	{																		\
		(void) stream;														\
		Serial.write(c);													\
		return c;															\
	}																		\
	static void printf_setup()												\
	{																		\
		fdev_setup_stream(&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);	\
		stdout = &uartout;													\
	}

#define P(string_literal) PSTR(string_literal "\n")

#define ANALOG_HIGH 511

#ifdef _DEBUG

#define assert(cond, ...)		\
	while (cond)				\
		printf_P(__VA_ARGS__);

#define trace(...)				\
	printf_P(__VA_ARGS__)

#else

#define assert(cond, ...)
#define trace(...)

#endif

bool boolAnalogRead(byte pin);
float floatAnalogRead(byte pin);
void floatAnalogWrite(byte pin, float value);

#endif
