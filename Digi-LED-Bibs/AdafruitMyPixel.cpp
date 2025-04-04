/*!
 * @file AdafruitMyPixel.cpp
 *
 * @mainpage Arduino Library for driving Adafruit NeoPixel addressable LEDs,
 * FLORA RGB Smart Pixels and compatible devicess -- WS2811, WS2812, WS2812B,
 * SK6812, etc.
 *
 * @section intro_sec Introduction
 *
 * This is the documentation for Adafruit's NeoPixel library for the
 * Arduino platform, allowing a broad range of microcontroller boards
 * (most AVR boards, many ARM devices, ESP8266 and ESP32, among others)
 * to control Adafruit NeoPixels, FLORA RGB Smart Pixels and compatible
 * devices -- WS2811, WS2812, WS2812B, SK6812, etc.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing products
 * from Adafruit!
 *
 * @section author Author
 *
 * Written by Phil "Paint Your Dragon" Burgess for Adafruit Industries,
 * with contributions by PJRC, Michael Miller and other members of the
 * open source community.
 *
 * @section license License
 *
 * This file is part of the AdafruitMyPixel library.
 *
 * AdafruitMyPixel is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * AdafruitMyPixel is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with NeoPixel. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include "AdafruitMyPixel.h"

#include <util/delay.h>

// Interrupt is only disabled if there is no PWM device available
// Note: Adafruit Bluefruit nrf52 does not use this option
//#define NRF52_DISABLE_INT


/*!
  @brief   "Empty" NeoPixel constructor when length, pin and/or pixel type
           are not known at compile-time, and must be initialized later with
           updateType(), updateLength() and setPin().
  @return  AdafruitMyPixel object. Call the begin() function before use.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax (length, pin,
           type).
*/
AdafruitMyPixel::AdafruitMyPixel(void)
    :
#if defined(NEO_KHZ400)
      is800KHz(true),
#endif
      begun(false), numLEDs(0), numBytes(0), pin(-1), brightness(0),
      pixels(0), rOffset(1), gOffset(0), bOffset(2), wOffset(1), endTime(0)
{
}

/*!
  @brief   NeoPixel constructor when length, pin and pixel type are known
           at compile-time.
  @param   n  Number of NeoPixels in strand.
  @param   p  Arduino pin number which will drive the NeoPixel data in.
  @param   t  Pixel type -- add together NEO_* constants defined in
              AdafruitMyPixel.h, for example NEO_GRB+NEO_KHZ800 for
              NeoPixels expecting an 800 KHz (vs 400 KHz) data stream
              with color bytes expressed in green, red, blue order per
              pixel.
  @return  AdafruitMyPixel object. Call the begin() function before use.
*/
AdafruitMyPixel::AdafruitMyPixel(uint16_t n, int16_t p, neoPixelType t)
    : 
#if defined(NEO_KHZ400)
	is800KHz(true),
#endif
	begun(false), brightness(0), pixels(0), endTime(0)
{
  updateType(t);
  updateLength(n);
  setPin(p);
}

/*!
  @brief   Deallocate AdafruitMyPixel object, set data pin back to INPUT.
*/
AdafruitMyPixel::~AdafruitMyPixel()
{
	free(pixels);
	if (pin >= 8)
		//pinMode(pin, INPUT);
		DDRB &= ~(1 << (pin - 8));
	else if (pin >= 0)
		DDRD &= ~(1 << pin);
}

/*!
  @brief   Configure NeoPixel pin for output.
*/
void AdafruitMyPixel::begin(void)
{
	//pinMode(pin, OUTPUT);
	//digitalWrite(pin, LOW);
	if (pin >= 8) {
		DDRB |= (1 << (pin - 8));
		PORTB &= ~(1 << (pin - 8));
	}
	else if (pin >= 0) {
		DDRD |= (1 << pin);
		PORTD &= ~(1 << pin);
	}
	begun = true;
}

/*!
  @brief   Change the length of a previously-declared AdafruitMyPixel
           strip object. Old data is deallocated and new data is cleared.
           Pin number and pixel format are unchanged.
  @param   n  New length of strip, in pixels.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax (length, pin,
           type).
*/
void AdafruitMyPixel::updateLength(uint16_t n)
{
	free(pixels);	// Free existing data (if any)

	// Allocate new data -- note: ALL PIXELS ARE CLEARED
	// numBytes = n * ((wOffset == rOffset) ? 3 : 4);
	numBytes = n * 2;
	if ((pixels = (uint8_t*)malloc(numBytes))) {
		//memset(pixels, 0, numBytes);
		numLEDs = n;
	} else {
		numLEDs = numBytes = 0;
	}
}

/*!
  @brief   Change the pixel format of a previously-declared
           AdafruitMyPixel strip object. If format changes from one of
           the RGB variants to an RGBW variant (or RGBW to RGB), the old
           data will be deallocated and new data is cleared. Otherwise,
           the old data will remain in RAM and is not reordered to the
           new format, so it's advisable to follow up with clear().
  @param   t  Pixel type -- add together NEO_* constants defined in
              AdafruitMyPixel.h, for example NEO_GRB+NEO_KHZ800 for
              NeoPixels expecting an 800 KHz (vs 400 KHz) data stream
              with color bytes expressed in green, red, blue order per
              pixel.
  @note    This function is deprecated, here only for old projects that
           may still be calling it. New projects should instead use the
           'new' keyword with the first constructor syntax
           (length, pin, type).
*/
void AdafruitMyPixel::updateType(neoPixelType t)
{
	bool oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

	wOffset = (t >> 6) & 0b11; // See notes in header file
	rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
	gOffset = (t >> 2) & 0b11;
	bOffset = t & 0b11;
#if defined(NEO_KHZ400)
	is800KHz = (t < 256); // 400 KHz flag is 1<<8
#endif

	// If bytes-per-pixel has changed (and pixel data was previously
	// allocated), re-allocate to new size. Will clear any data.
	if (pixels) {
		bool newThreeBytesPerPixel = (wOffset == rOffset);
		if (newThreeBytesPerPixel != oldThreeBytesPerPixel)
		updateLength(numLEDs);
	}
}

/*!
  @brief   Transmit pixel data in RAM to NeoPixels.
  @note    On most architectures, interrupts are temporarily disabled in
           order to achieve the correct NeoPixel signal timing. This means
           that the Arduino millis() and micros() functions, which require
           interrupts, will lose small intervals of time whenever this
           function is called (about 30 microseconds per RGB pixel, 40 for
           RGBW pixels). There's no easy fix for this, but a few
           specialized alternative or companion libraries exist that use
           very device-specific peripherals to work around it.
*/
void AdafruitMyPixel::show(void)
{
	if (!pixels) {
		PORTB |= (1 << PINB5);
		_delay_ms(50.0);
		PORTB &= ~(1 << PINB5);
		_delay_ms(50.0);
		PORTB |= (1 << PINB5);
		_delay_ms(50.0);
		PORTB &= ~(1 << PINB5);
		return;
	}
  // Data latch = 300+ microsecond pause in the output stream. Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed. This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
	while (!canShow()) ;
    // endTime is a private member (rather than global var) so that multiple
    // instances on different pins can be quickly issued in succession (each
    // instance doesn't delay the next).

    // In order to make this code runtime-configurable to work with any pin,
    // SBI/CBI instructions are eschewed in favor of full PORT writes via the
    // OUT or ST instructions. It relies on two facts: that peripheral
    // functions (such as PWM) take precedence on output pins, so our PORT-
    // wide writes won't interfere, and that interrupts are globally disabled
    // while data is being issued to the LEDs, so no other code will be
    // accessing the PORT. The code takes an initial 'snapshot' of the PORT
    // state, computes 'pin high' and 'pin low' values, and writes these back
    // to the PORT register as needed.

    // NRF52 may use PWM + DMA (if available), may not need to disable interrupt
    // ESP32 may not disable interrupts because espShow() uses RMT which tries to acquire locks
#if !(defined(NRF52) || defined(NRF52_SERIES) || defined(ESP32))
	//noInterrupts(); // Need 100% focus on instruction timing
	cli();	// Interrupts ausschalten
#endif

#if defined(__AVR__)
  // AVR MCUs -- ATmega & ATtiny (no XMEGA) ---------------------------------

  volatile uint16_t i = numBytes; 	// Loop counter
  volatile uint8_t *ptr = pixels, 	// Pointer to next byte
      b = *ptr,                 	// Current byte value
      hi,                         	// PORT w/output bit set high
      lo;                         	// PORT w/output bit set low

  // Hand-tuned assembly code issues data to the LED drivers at a specific
  // rate. There's separate code for different CPU speeds (8, 12, 16 MHz)
  // for both the WS2811 (400 KHz) and WS2812 (800 KHz) drivers. The
  // datastream timing for the LED drivers allows a little wiggle room each
  // way (listed in the datasheets), so the conditions for compiling each
  // case are set up for a range of frequencies rather than just the exact
  // 8, 12 or 16 MHz values, permitting use with some close-but-not-spot-on
  // devices (e.g. 16.5 MHz DigiSpark). The ranges were arrived at based
  // on the datasheet figures and have not been extensively tested outside
  // the canonical 8/12/16 MHz speeds; there's no guarantee these will work
  // close to the extremes (or possibly they could be pushed further).
  // Keep in mind only one CPU speed case actually gets compiled; the
  // resulting program isn't as massive as it might look from source here.


// 16 MHz(ish) AVR --------------------------------------------------------
#if (F_CPU >= 15400000UL) && (F_CPU <= 19000000L)

    // WS2811 and WS2812 have different hi/lo duty cycles; this is
    // similar but NOT an exact copy of the prior 400-on-8 code.

    // 20 inst. clocks per bit: HHHHHHxxxxxxxxLLLLLL
    // ST instructions:         ^     ^       ^       (T=0,6,14)

    volatile uint8_t next, bbit, tmp, tmp2;	// nächster Wert, akt. Bytebit und Farbbit

    hi = *port | pinMask;
    lo = *port & ~pinMask;
    // next = lo;
    bbit = 7;

	asm volatile(
		"h20farbe1:\n"						// Tkt	Erklärungen			(T =  0)
		"st   	%a[port],  %[hisig]\n"		// 2	Port = hi			(T =  2)
		"mov	%[next], %[losig]\n"		// 1	next = lo			(T =  3)
		"sbrc	%[byte], 7\n"				// 1-2	if (byte & 0x80)	
		"mov 	%[next], %[hisig]\n"		// 0-1	next = hi			(T =  5)
		"dec	%[bit]\n"					// 1	bit--				(T =  6)
		"st		%a[port], %[next]\n"		// 2	Port = next			(T =  8)
		"breq	h20farbe1rest\n"			// 1-2	if (bit == 0)
		"lsl	%[byte]\n"					// 0-1						(T = 10)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 12)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 18)
		"rjmp	h20farbe1\n"				// 2	-> h20farbe1		(T = 20)
		
		"h20farbe1rest:\n"					//							(T = 10)
		"ld		%[tmp], %a[ptr]+\n"			// 2	tmp = *ptr++		(T = 12)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 13)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 17)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 18)
		"ld		%[tmp2], %a[ptr]\n"			// 2	tmp2 = *ptr			(T = 20)
		"st   	%a[port],  %[hisig]\n"		// 2	Port = hi			(T =  2)
		"mov	%[next], %[losig]\n"		// 1	next = lo			(T =  3)
		"sbrc	%[byte], 6\n"				// 1-2	if (byte & 0x40)	
		"mov 	%[next], %[hisig]\n"		// 0-1	next = hi			(T =  5)
		"ldi	%[bit], 7\n"				// 1	bit = 7				(T =  6)
		"st		%a[port], %[next]\n"		// 2	Port = next			(T =  8)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T =  9)
		"lsr	%[tmp2]\n"					// 1	tmp2 >>= 1			(T = 10)
		"lsr	%[tmp2]\n"					// 1	tmp2 >>= 1			(T = 11)
		"lsr	%[tmp2]\n"					// 1	tmp2 >>= 1			(T = 12)
		"andi	%[tmp2], 0xF8\n"			// 1	tmp2 &= 0xF8		(T = 13)
		"add	%[tmp], %[tmp2]\n"			// 1	tmp += tmp2			(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"mov	%[byte], %[tmp]\n"			// 1 	byte = tmp			(T = 17)
		"ld		%[tmp], %a[ptr]+\n"			// 2	tmp = *ptr++		(T = 19)
		"nop\n"								// 1						(T = 20)
	//	"ld		%[tmp], %[tmp2]\n"			// 1	tmp = tmp2			(T = 14)
		
		"h20farbe2:\n"						// 
		"st   	%a[port],  %[hisig]\n"		// 2	Port = hi			(T =  2)
		"mov	%[next], %[losig]\n"		// 1	next = lo			(T =  3)
		"sbrc	%[byte], 7\n"				// 1-2	if (byte & 0x80)	
		"mov 	%[next], %[hisig]\n"		// 0-1	next = hi			(T =  5)
		"dec	%[bit]\n"					// 1	bit--				(T =  6)
		"st		%a[port], %[next]\n"		// 2	Port = next			(T =  8)
		"brmi	h20farbe2rest\n"			// 1-2	if (bit < 0)
		"lsl	%[byte]\n"					// 0-1						(T = 10)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 12)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 18)
		"rjmp	h20farbe2\n"				// 2	-> h20farbe2		(T = 20)
		
		"h20farbe2rest:\n"					// 							(T = 10)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 11)
		"lsl	%[tmp]\n"					// 1	tmp <<= 1			(T = 12)
	//	"rjmp	.+0\n"						// 2	nop,nop				(T = 12)
		"mov	%[byte], %[tmp]\n"			// 1 	byte = tmp			(T = 13)
		"ldi	%[bit], 7\n"				// 1	bit = 7				(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"sbiw 	%[count], 1\n" 				// 2    i--					(T = 18)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 20)

		"h20farbe3:\n"
		"st   	%a[port],  %[hisig]\n"		// 2	Port = hi			(T =  2)
		"mov	%[next], %[losig]\n"		// 1	next = lo			(T =  3)
		"sbrc	%[byte], 7\n"				// 1-2	if (byte & 0x80)	
		"mov 	%[next], %[hisig]\n"		// 0-1	next = hi			(T =  5)
		"dec	%[bit]\n"					// 1	bit--				(T =  6)
		"st		%a[port], %[next]\n"		// 2	Port = next			(T =  8)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 10)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 12)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"breq	h20farbe3rest\n"			// 1-2	if (bit == 0)
		"lsl	%[byte]\n"					// 0-1						(T = 18)
		"rjmp	h20farbe3\n"				// 2	-> h20farbe3		(T = 20)
		
		"h20farbe3rest:\n"					// 							(T = 18)
		"sbiw 	%[count], 1\n" 				// 2    i--					(T = 20)
		"st   	%a[port],  %[hisig]\n"		// 2	Port = hi			(T =  2)
		"mov	%[next], %[losig]\n"		// 1	next = lo			(T =  3)
		"sbrc	%[byte], 6\n"				// 1-2	if (byte & 0x40)	
		"mov 	%[next], %[hisig]\n"		// 0-1	next = hi			(T =  5)
		"ldi	%[bit], 7\n"				// 1	bit = 7				(T =  6)
		"st		%a[port], %[next]\n"		// 2	Port = next			(T =  8)
		"breq	ende\n"						// 1-2	if (i = 0)
		"nop\n"								// 0-1						(T = 10)
		"ld		%[tmp2], %a[ptr]\n"			// 2	tmp2 = *ptr			(T = 12)
		"andi	%[tmp2], 0xF8\n"			// 1	tmp2 &= 0xF8		(T = 13)
		"mov	%[byte], %[tmp2]\n"			// 1	byte = tmp2			(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 18)
		"rjmp	h20farbe1\n"				// 2	-> h20farbe1		(T = 20)

		"ende:\n"
		"rjmp	.+0\n"						// 2	nop,nop				(T = 12)
		"rjmp	.+0\n"						// 2	nop,nop				(T = 14)
		"st   	%a[port],  %[losig]\n"		// 2	Port = lo			(T = 16)
		
		: [port] "+e"(port), [byte] "+r"(b), [bit] "+r"(bbit),
		  [next] "+r"(next), [count] "+w"(i), [tmp] "+r"(tmp), [tmp2] "+d"(tmp2)
		: [ptr] "e"(ptr), [hisig] "r"(hi), [losig] "r"(lo));


#else
#error "CPU SPEED NOT SUPPORTED"
#endif // end F_CPU ifdefs on __AVR__

  // END AVR ----------------------------------------------------------------


#elif defined(__ARDUINO_ARC__)

    // Arduino 101  -----------------------------------------------------------

#define NOPx7                                                                  \
  {                                                                            \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
    __builtin_arc_nop();                                                       \
  }

  PinDescription *pindesc = &g_APinDescription[pin];
  register uint32_t loop =
      8 * numBytes; // one loop to handle all bytes and all bits
  register uint8_t *p = pixels;
  register uint32_t currByte = (uint32_t)(*p);
  register uint32_t currBit = 0x80 & currByte;
  register uint32_t bitCounter = 0;
  register uint32_t first = 1;

  // The loop is unusual. Very first iteration puts all the way LOW to the wire
  // - constant LOW does not affect NEOPIXEL, so there is no visible effect
  // displayed. During that very first iteration CPU caches instructions in the
  // loop. Because of the caching process, "CPU slows down". NEOPIXEL pulse is
  // very time sensitive that's why we let the CPU cache first and we start
  // regular pulse from 2nd iteration
  if (pindesc->ulGPIOType == SS_GPIO) {
    register uint32_t reg = pindesc->ulGPIOBase + SS_GPIO_SWPORTA_DR;
    uint32_t reg_val = __builtin_arc_lr((volatile uint32_t)reg);
    register uint32_t reg_bit_high = reg_val | (1 << pindesc->ulGPIOId);
    register uint32_t reg_bit_low = reg_val & ~(1 << pindesc->ulGPIOId);

    loop += 1; // include first, special iteration
    while (loop--) {
      if (!first) {
        currByte <<= 1;
        bitCounter++;
      }

      // 1 is >550ns high and >450ns low; 0 is 200..500ns high and >450ns low
      __builtin_arc_sr(first ? reg_bit_low : reg_bit_high,
                       (volatile uint32_t)reg);
      if (currBit) { // ~400ns HIGH (740ns overall)
        NOPx7 NOPx7
      }
      // ~340ns HIGH
      NOPx7 __builtin_arc_nop();

      // 820ns LOW; per spec, max allowed low here is 5000ns */
      __builtin_arc_sr(reg_bit_low, (volatile uint32_t)reg);
      NOPx7 NOPx7

          if (bitCounter >= 8) {
        bitCounter = 0;
        currByte = (uint32_t)(*++p);
      }

      currBit = 0x80 & currByte;
      first = 0;
    }
  } else if (pindesc->ulGPIOType == SOC_GPIO) {
    register uint32_t reg = pindesc->ulGPIOBase + SOC_GPIO_SWPORTA_DR;
    uint32_t reg_val = MMIO_REG_VAL(reg);
    register uint32_t reg_bit_high = reg_val | (1 << pindesc->ulGPIOId);
    register uint32_t reg_bit_low = reg_val & ~(1 << pindesc->ulGPIOId);

    loop += 1; // include first, special iteration
    while (loop--) {
      if (!first) {
        currByte <<= 1;
        bitCounter++;
      }
      MMIO_REG_VAL(reg) = first ? reg_bit_low : reg_bit_high;
      if (currBit) { // ~430ns HIGH (740ns overall)
        NOPx7 NOPx7 __builtin_arc_nop();
      }
      // ~310ns HIGH
      NOPx7

          // 850ns LOW; per spec, max allowed low here is 5000ns */
          MMIO_REG_VAL(reg) = reg_bit_low;
      NOPx7 NOPx7

          if (bitCounter >= 8) {
        bitCounter = 0;
        currByte = (uint32_t)(*++p);
      }

      currBit = 0x80 & currByte;
      first = 0;
    }
  }

#else
#error Architecture not supported
#endif

  // END ARCHITECTURE SELECT ------------------------------------------------

#if !(defined(NRF52) || defined(NRF52_SERIES) || defined(ESP32))
	//interrupts();
	sei();	// Interrupts wieder aktivieren
#endif

	//endTime = clock(); // Save EOD time for latch on next call
	endTime = time(NULL);
}

/*!
  @brief   Set/change the NeoPixel output pin number. Previous pin,
           if any, is set to INPUT and the new pin is set to OUTPUT.
  @param   p  Arduino pin number (-1 = no pin).
*/
void AdafruitMyPixel::setPin(int16_t p)
{
	if (begun && (pin >= 0))
	//pinMode(pin, INPUT); // Disable existing out pin
		(pin >= 8 ? DDRB : DDRD) &= ~pinMask;
		
#if defined(__AVR__)
	//port = portOutputRegister(digitalPinToPort(p));
	//pinMask = digitalPinToBitMask(p);
	port = (p >= 8 ? &PORTB : &PORTD);
	pinMask = (1 << (p % 8));
#endif
	
	pin = p;
	if (begun) {
		//pinMode(p, OUTPUT);
		//digitalWrite(p, LOW);
		(pin >= 8 ? DDRB : DDRD) |= pinMask;
		*port |= pinMask;
	}

}

/*!
  @brief   Set a pixel's color using separate red, green and blue
           components. If using RGBW pixels, white will be set to 0.
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
*/
void AdafruitMyPixel::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
  if (n < numLEDs)
  {
    if (brightness) { // See notes in setBrightness()
      r = ((uint16_t)r * brightness) >> 8;
      g = ((uint16_t)g * brightness) >> 8;
      b = ((uint16_t)b * brightness) >> 8;
    }
    uint8_t *p = &pixels[n * 2];
    uint16_t farbe = 0;
    farbe = ((rOffset == 0 ? r : gOffset == 0 ? g : b) & 0xF8) << 8;
    farbe |= ((rOffset == 1 ? r : gOffset == 1 ? g : b) & 0xF8) << 3;
    farbe |= ((rOffset == 2 ? r : gOffset == 2 ? g : b) & 0xF8) >> 2;
    p[0] = farbe >> 8;
    p[1] = farbe;
    // p[0] = (r & 0xF8) | (g >> 5);
    // p[1] = ((g & 0xF8) << 3) | (b >> 2);
  }
}

/*!
  @brief   Set a pixel's color using separate red, green, blue and white
           components (for RGBW NeoPixels only).
  @param   n  Pixel index, starting from 0.
  @param   r  Red brightness, 0 = minimum (off), 255 = maximum.
  @param   g  Green brightness, 0 = minimum (off), 255 = maximum.
  @param   b  Blue brightness, 0 = minimum (off), 255 = maximum.
  @param   w  White brightness, 0 = minimum (off), 255 = maximum, ignored
              if using RGB pixels.
*/
void AdafruitMyPixel::setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
	setPixelColor(n, r, g, b);
}

/*!
  @brief   Set a pixel's color using a 32-bit 'packed' RGB or RGBW value.
  @param   n  Pixel index, starting from 0.
  @param   c  32-bit color value. Most significant byte is white (for RGBW
              pixels) or ignored (for RGB pixels), next is red, then green,
              and least significant byte is blue.
*/
void AdafruitMyPixel::setPixelColor(uint16_t n, uint32_t c)
{
  if (n < numLEDs) {
    uint8_t r = (uint8_t)(c >> 16), g = (uint8_t)(c >> 8), b = (uint8_t)c;
    setPixelColor(n, r, g, b);
  }
}

/*!
  @brief   Fill all or part of the NeoPixel strip with a color.
  @param   c      32-bit color value. Most significant byte is white (for
                  RGBW pixels) or ignored (for RGB pixels), next is red,
                  then green, and least significant byte is blue. If all
                  arguments are unspecified, this will be 0 (off).
  @param   first  Index of first pixel to fill, starting from 0. Must be
                  in-bounds, no clipping is performed. 0 if unspecified.
  @param   count  Number of pixels to fill, as a positive value. Passing
                  0 or leaving unspecified will fill to end of strip.
*/
void AdafruitMyPixel::fill(uint32_t c, uint16_t first, uint16_t count)
{
  uint16_t i, end;

  if (first >= numLEDs) {
    return; // If first LED is past end of strip, nothing to do
  }

  // Calculate the index ONE AFTER the last pixel to fill
  if (count == 0) {
    // Fill to end of strip
    end = numLEDs;
  } else {
    // Ensure that the loop won't go past the last pixel
    end = first + count;
    if (end > numLEDs)
      end = numLEDs;
  }

  for (i = first; i < end; i++) {
    this->setPixelColor(i, c);
  }
}

/*!
  @brief   Convert hue, saturation and value into a packed 32-bit RGB color
           that can be passed to setPixelColor() or other RGB-compatible
           functions.
  @param   hue  An unsigned 16-bit value, 0 to 65535, representing one full
                loop of the color wheel, which allows 16-bit hues to "roll
                over" while still doing the expected thing (and allowing
                more precision than the wheel() function that was common to
                prior NeoPixel examples).
  @param   sat  Saturation, 8-bit value, 0 (min or pure grayscale) to 255
                (max or pure hue). Default of 255 if unspecified.
  @param   val  Value (brightness), 8-bit value, 0 (min / black / off) to
                255 (max or full brightness). Default of 255 if unspecified.
  @return  Packed 32-bit RGB with the most significant byte set to 0 -- the
           white element of WRGB pixels is NOT utilized. Result is linearly
           but not perceptually correct, so you may want to pass the result
           through the gamma32() function (or your own gamma-correction
           operation) else colors may appear washed out. This is not done
           automatically by this function because coders may desire a more
           refined gamma-correction function than the simplified
           one-size-fits-all operation of gamma32(). Diffusing the LEDs also
           really seems to help when using low-saturation colors.
*/
uint32_t AdafruitMyPixel::ColorHSV(uint16_t hue, uint8_t sat, uint8_t val)
{
  uint8_t r, g, b;

  // Remap 0-65535 to 0-1529. Pure red is CENTERED on the 64K rollover;
  // 0 is not the start of pure red, but the midpoint...a few values above
  // zero and a few below 65536 all yield pure red (similarly, 32768 is the
  // midpoint, not start, of pure cyan). The 8-bit RGB hexcone (256 values
  // each for red, green, blue) really only allows for 1530 distinct hues
  // (not 1536, more on that below), but the full unsigned 16-bit type was
  // chosen for hue so that one's code can easily handle a contiguous color
  // wheel by allowing hue to roll over in either direction.
  hue = (hue * 1530L + 32768) / 65536;
  // Because red is centered on the rollover point (the +32768 above,
  // essentially a fixed-point +0.5), the above actually yields 0 to 1530,
  // where 0 and 1530 would yield the same thing. Rather than apply a
  // costly modulo operator, 1530 is handled as a special case below.

  // So you'd think that the color "hexcone" (the thing that ramps from
  // pure red, to pure yellow, to pure green and so forth back to red,
  // yielding six slices), and with each color component having 256
  // possible values (0-255), might have 1536 possible items (6*256),
  // but in reality there's 1530. This is because the last element in
  // each 256-element slice is equal to the first element of the next
  // slice, and keeping those in there this would create small
  // discontinuities in the color wheel. So the last element of each
  // slice is dropped...we regard only elements 0-254, with item 255
  // being picked up as element 0 of the next slice. Like this:
  // Red to not-quite-pure-yellow is:        255,   0, 0 to 255, 254,   0
  // Pure yellow to not-quite-pure-green is: 255, 255, 0 to   1, 255,   0
  // Pure green to not-quite-pure-cyan is:     0, 255, 0 to   0, 255, 254
  // and so forth. Hence, 1530 distinct hues (0 to 1529), and hence why
  // the constants below are not the multiples of 256 you might expect.

  // Convert hue to R,G,B (nested ifs faster than divide+mod+switch):
  if (hue < 510) { // Red to Green-1
    b = 0;
    if (hue < 255) { //   Red to Yellow-1
      r = 255;
      g = hue;       //     g = 0 to 254
    } else {         //   Yellow to Green-1
      r = 510 - hue; //     r = 255 to 1
      g = 255;
    }
  } else if (hue < 1020) { // Green to Blue-1
    r = 0;
    if (hue < 765) { //   Green to Cyan-1
      g = 255;
      b = hue - 510;  //     b = 0 to 254
    } else {          //   Cyan to Blue-1
      g = 1020 - hue; //     g = 255 to 1
      b = 255;
    }
  } else if (hue < 1530) { // Blue to Red-1
    g = 0;
    if (hue < 1275) { //   Blue to Magenta-1
      r = hue - 1020; //     r = 0 to 254
      b = 255;
    } else { //   Magenta to Red-1
      r = 255;
      b = 1530 - hue; //     b = 255 to 1
    }
  } else { // Last 0.5 Red (quicker than % operator)
    r = 255;
    g = b = 0;
  }

  // Apply saturation and value to R,G,B, pack into 32-bit result:
  uint32_t v1 = 1 + val;  // 1 to 256; allows >>8 instead of /255
  uint16_t s1 = 1 + sat;  // 1 to 256; same reason
  uint8_t s2 = 255 - sat; // 255 to 0
  return ((((((r * s1) >> 8) + s2) * v1) & 0xff00) << 8) |
         (((((g * s1) >> 8) + s2) * v1) & 0xff00) |
         (((((b * s1) >> 8) + s2) * v1) >> 8);
}

/*!
  @brief   Query the color of a previously-set pixel.
  @param   n  Index of pixel to read (0 = first).
  @return  'Packed' 32-bit RGB or WRGB value. Most significant byte is white
           (for RGBW pixels) or 0 (for RGB pixels), next is red, then green,
           and least significant byte is blue.
  @note    If the strip brightness has been changed from the default value
           of 255, the color read from a pixel may not exactly match what
           was previously written with one of the setPixelColor() functions.
           This gets more pronounced at lower brightness levels.
*/
uint32_t AdafruitMyPixel::getPixelColor(uint16_t n) const
{
	if (n >= numLEDs)
		return 0; // Out of bounds, return no color.

	uint8_t *p = &pixels[n * 2];

	uint8_t r = p[0] & 0xF8;
	uint8_t g = ((p[0] & 0x03) << 6) | ((p[1]& 0xE0) >> 2);
	uint8_t b = p[1] << 3;
	
	if (brightness) {
		r /= brightness;
		g /= brightness;
		b /= brightness;
	}
	return ((uint32_t)r << 16)
		| ((uint32_t)g << 8)
		| (uint32_t)b;
}

/*!
  @brief   Adjust output brightness. Does not immediately affect what's
           currently displayed on the LEDs. The next call to show() will
           refresh the LEDs at this level.
  @param   b  Brightness setting, 0=minimum (off), 255=brightest.
  @note    This was intended for one-time use in one's setup() function,
           not as an animation effect in itself. Because of the way this
           library "pre-multiplies" LED colors in RAM, changing the
           brightness is often a "lossy" operation -- what you write to
           pixels isn't necessary the same as what you'll read back.
           Repeated brightness changes using this function exacerbate the
           problem. Smart programs therefore treat the strip as a
           write-only resource, maintaining their own state to render each
           frame of an animation, not relying on read-modify-write.
*/
void AdafruitMyPixel::setBrightness(uint8_t b)
{
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB. 'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  if (newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM,
    // This process is potentially "lossy," especially when increasing
    // brightness. The tight timing in the WS2811/WS2812 code means there
    // aren't enough free cycles to perform this scaling on the fly as data
    // is issued. So we make a pass through the existing color data in RAM
    // and scale it (subsequent graphics commands also work at this
    // brightness level). If there's a significant step up in brightness,
    // the limited number of steps (quantization) in the old data will be
    // quite visible in the re-scaled version. For a non-destructive
    // change, you'll need to re-render the full strip data. C'est la vie.
    // TODO: Format umgestellt, Code ist anzupassen
//    uint8_t c, *ptr = pixels,
//               oldBrightness = brightness - 1; // De-wrap old brightness value
//    uint16_t scale;
//    if (oldBrightness == 0)
//      scale = 0; // Avoid /0
//    else if (b == 255)
//      scale = 65535 / oldBrightness;
//    else
//      scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
//    
//    for (uint16_t i = 0; i < numBytes; i++) {
//      c = *ptr;
//      *ptr++ = (c * scale) >> 8;
//    }
    brightness = newBrightness;
  }
}

/*!
  @brief   Retrieve the last-set brightness value for the strip.
  @return  Brightness value: 0 = minimum (off), 255 = maximum.
*/
uint8_t AdafruitMyPixel::getBrightness(void) const
{
	return brightness - 1;
}

/*!
  @brief   Fill the whole NeoPixel strip with 0 / black / off.
*/
void AdafruitMyPixel::clear(void)
{ 
	//memset(pixels, 0, numBytes);
	for (uint16_t i = 0; i < numBytes; i++)
		pixels[i] = 0;
}

// A 32-bit variant of gamma8() that applies the same function
// to all components of a packed RGB or WRGB value.
uint32_t AdafruitMyPixel::gamma32(uint32_t x)
{
  uint8_t *y = (uint8_t *)&x;
  // All four bytes of a 32-bit value are filtered even if RGB (not WRGB),
  // to avoid a bunch of shifting and masking that would be necessary for
  // properly handling different endianisms (and each byte is a fairly
  // trivial operation, so it might not even be wasting cycles vs a check
  // and branch for the RGB case). In theory this might cause trouble *if*
  // someone's storing information in the unused most significant byte
  // of an RGB value, but this seems exceedingly rare and if it's
  // encountered in reality they can mask values going in or coming out.
  for (uint8_t i = 0; i < 4; i++)
    y[i] = gamma8(y[i]);
  return x; // Packed 32-bit return
}

/*!
  @brief   Fill NeoPixel strip with one or more cycles of hues.
           Everyone loves the rainbow swirl so much, now it's canon!
  @param   first_hue   Hue of first pixel, 0-65535, representing one full
                       cycle of the color wheel. Each subsequent pixel will
                       be offset to complete one or more cycles over the
                       length of the strip.
  @param   reps        Number of cycles of the color wheel over the length
                       of the strip. Default is 1. Negative values can be
                       used to reverse the hue order.
  @param   saturation  Saturation (optional), 0-255 = gray to pure hue,
                       default = 255.
  @param   brightness  Brightness/value (optional), 0-255 = off to max,
                       default = 255. This is distinct and in combination
                       with any configured global strip brightness.
  @param   gammify     If true (default), apply gamma correction to colors
                       for better appearance.
*/
void AdafruitMyPixel::rainbow(uint16_t first_hue, int8_t reps,
  uint8_t saturation, uint8_t brightness, bool gammify) {
  for (uint16_t i=0; i<numLEDs; i++) {
    uint16_t hue = first_hue + (i * reps * 65536) / numLEDs;
    uint32_t color = ColorHSV(hue, saturation, brightness);
    if (gammify) color = gamma32(color);
    setPixelColor(i, color);
  }
}

/*!
  @brief  Convert pixel color order from string (e.g. "BGR") to NeoPixel
          color order constant (e.g. NEO_BGR). This may be helpful for code
          that initializes from text configuration rather than compile-time
          constants.
  @param   v  Input string. Should be reasonably sanitized (a 3- or 4-
              character NUL-terminated string) or undefined behavior may
              result (output is still a valid NeoPixel order constant, but
              might not present as expected). Garbage in, garbage out.
  @return  One of the NeoPixel color order constants (e.g. NEO_BGR).
           NEO_KHZ400 or NEO_KHZ800 bits are not included, nor needed (all
           NeoPixels actually support 800 KHz it's been found, and this is
           the default state if no KHZ bits set).
  @note    This function is declared static in the class so it can be called
           without a NeoPixel object (since it's not likely been declared
           in the code yet). Use AdafruitMyPixel::str2order().
*/
neoPixelType AdafruitMyPixel::str2order(const char *v)
{
  int8_t r = 0, g = 0, b = 0, w = -1;
  if (v) {
    char c;
    for (uint8_t i=0; ((c = v[i])); i++) {
      if (c == 'r' || c == 'R') r = i;
      else if (c == 'g' || c == 'G') g = i;
      else if (c == 'b' || c == 'B') b = i;
      else if (c == 'w' || c == 'W') w = i;
    }
    r &= 3;
  }
  if (w < 0) w = r; // If 'w' not specified, duplicate r bits
  return (w << 6) | (r << 4) | ((g & 3) << 2) | (b & 3);
}
 
