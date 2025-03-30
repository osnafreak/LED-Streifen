
#ifndef MyArduino_h
#define MyArduino_h

#include <avr/io.h>
#include <avr/pgmspace.h>

typedef unsigned int word;
typedef uint8_t byte;

#define LED_BUILTIN 13
#define NOT_A_PIN 0
#define NOT_A_PORT 0
#define NOT_ON_TIMER 0

#define PA 1
#define PB 2
#define PC 3
#define PD 4

#define TIMER0A 1
#define TIMER0B 2
#define TIMER1A 3
#define TIMER1B 4
#define TIMER1C 5
#define TIMER2  6
#define TIMER2A 7
#define TIMER2B 8

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER 2.718281828459045235360287471352

//#define _BV(b) (1UL << (b))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

//extern const uint16_t PROGMEM port_to_mode_PGM[];
//extern const uint16_t PROGMEM port_to_input_PGM[];
//extern const uint16_t PROGMEM port_to_output_PGM[];
//
//extern const uint8_t PROGMEM digital_pin_to_port_PGM[];
//extern const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[];
//extern const uint8_t PROGMEM digital_pin_to_timer_PGM[];

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PORT,
	NOT_A_PORT,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};


const uint8_t PROGMEM digital_pin_to_port_PGM[] = {
	PD, /* 0 */
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PD,
	PB, /* 8 */
	PB,
	PB,
	PB,
	PB,
	PB,
	PC, /* 14 */
	PC,
	PC,
	PC,
	PC,
	PC,
};

const uint8_t PROGMEM digital_pin_to_bit_mask_PGM[] = {
	_BV(0), /* 0, port D */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(6),
	_BV(7),
	_BV(0), /* 8, port B */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
	_BV(0), /* 14, port C */
	_BV(1),
	_BV(2),
	_BV(3),
	_BV(4),
	_BV(5),
};

const uint8_t PROGMEM digital_pin_to_timer_PGM[] = {
	NOT_ON_TIMER, /* 0 - port D */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	// on the ATmega168, digital pin 3 has hardware pwm
	#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
	#else
	TIMER2B,
	#endif
	NOT_ON_TIMER,
	// on the ATmega168, digital pins 5 and 6 have hardware pwm
	#if defined(__AVR_ATmega8__)
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	#else
	TIMER0B,
	TIMER0A,
	#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 8 - port B */
	TIMER1A,
	TIMER1B,
	#if defined(__AVR_ATmega8__)
	TIMER2,
	#else
	TIMER2A,
	#endif
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER, /* 14 - port C */
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
	NOT_ON_TIMER,
};


#define analogInPinToBit(P) (P)
#define analogInputToDigitalPin(p)  ((p < 6) ? (p) + 14 : -1)

#define digitalPinToPort(P) ( pgm_read_byte(digital_pin_to_port_PGM + (P)) )
#define digitalPinToBitMask(P) ( pgm_read_byte(digital_pin_to_bit_mask_PGM + (P)) )
#define digitalPinToTimer(P) ( pgm_read_byte(digital_pin_to_timer_PGM + (P)) )

#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word(port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word(port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word(port_to_mode_PGM + (P))) )

#endif