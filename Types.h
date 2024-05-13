#ifndef TYPES_H
#define TYPES_H

	#include "stm32f4xx.h"
	#include <stdio.h>
	#include <string.h>


	typedef uint8_t U8;
	typedef uint16_t U16;
	typedef uint32_t U32;
	
	typedef int16_t I16;

#define MAX_PACKET_STR_SIZE ((U8)(15u))
#define MAX_NR_OF_MSG_DATA_BYTES ((U8)(8u))

#define SECONDS_TO_COUNT_U8 ((U8)(10u))

#define MAX_NR_OF_CHANNELS ((U8)(18u))

#define PERIODIC_TIMER_VALUE_500US_U16	((U16)(500u))

	struct UART_MSG_STRUCT
	{
		U8 len;
		U8 id;
		U8 data[MAX_NR_OF_MSG_DATA_BYTES - 1];
	};

	typedef struct UART_MSG_STRUCT UART_MSG_T;

	enum IDs_ENUM {
		SW_VERSION = 0,
		PORT_INPUT = 1,
		PORT_OUTPUT = 2,
		PORT_HEX = 3,
		TIMERS = 4,
		ADC_DAC = 5,
		IRQ = 6,
		SPI = 7,
		I2C = 8
	};

	
	#define EMPTY_CELL ' '
	#define POSITION_INPUT 0xA0
	
	typedef enum IDs_ENUM ID_TYPE;

	enum TIMER_IDs_ENUM {
		GENERAL_TIM3 = 0,
		TIM3_CH1_COMPARE = 1,
		TIM8_CH3_CAPTURE = 2,
		TIM8_CH1_PWM = 3,
		SYS_TICK_TIMER = 4 
	};

	typedef enum TIMER_IDs_ENUM TIMER_ID_TYPE;

	typedef enum{
		PLAYER_X,
		PLAYER_O
	}Player;
	
	typedef struct {
		char cells[3][3];
	}Board;
	
	typedef struct {
		int row;
		int col;
		Player player;
	}Move;

#endif /* TYPES_H */
