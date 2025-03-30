/*
 * main.c
 *
 * Created: 5/7/2024 5:47:34 PM
 * Author: Iggy
 */ 

//#include <ctype.h>
//#include <stdint.h>
#include <stdio.h>

//#include <xc.h>
#include <avr/io.h>
#include <util/delay.h>
//#include <Wire.h>
//#include <math.h>
//#include "AdafruitMyPixel.h"
//#include "color.h"

// Ein Byte pro Farbe global im Projekt setzen und Bibliothek laden
#include "microLED/microLED.h"


#define SIGNAL_PIN   6	// Signalpin für die NeoPixels
#define LED       PINB5	// LED auf dem Board: DP 13
//#define ZIGZAG    1	// Wechselnde Richtung der LED-Streifen
#define DELAYVAL  50	// Periodendauer (in milliseconds)

// How many NeoPixels are attached to the Arduino?
const int ZEILEN = 30;
const int SPALTEN = 10;
const int NUMPIXELS = ZEILEN * SPALTEN; // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter.
//AdafruitMyPixel strip(NUMPIXELS, SIGNAL_PIN);
microLED<NUMPIXELS, SIGNAL_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_HIGH> 
	strip(SPALTEN, ZEILEN, ZIGZAG, RIGHT_TOP, DIR_DOWN);

//void setLed(uint8_t zeile, uint8_t spalte, const RGB_Color& color)
//{
//#if ZIGZAG
	//uint16_t ledpos = ZEILEN * spalte + (spalte % 2 ? (ZEILEN - 1 - zeile) : zeile);
//#else
	//uint16_t ledpos = ZEILEN * spalte + zeile;
//#endif
	//strip.setPixelColor(ledpos, strip.Color(color.red, color.green, color.blue));
//}

void blinken()
{
	PORTB |= (1 << LED);	// LED an
	_delay_ms(30.0);
	PORTB &= ~(1 << LED);	// LED aus
}

int main(void)
{
	int lauf = 0;
	mData farbe;
	
	//  Serial.begin(19200);
	//pinMode(LED, OUTPUT);
	//digitalWrite(LED, HIGH);
	DDRB |= (1 << LED);
	PORTB |= (1 << LED);	// LED an
	//if (strip.getPixels())
		//_delay_ms(1000);
	//else
	_delay_ms(1000);
	//digitalWrite(LED, LOW);
	PORTB &= ~(1 << LED);	// LED aus
	//printf("Anzahl initialisierter LEDs: %i\n", strip.numPixels());
	
	//randomSeed(analogRead(0
	//USART_init();
	//Serial.begin();
	_delay_ms(1000);
	blinken();		// erstes Blinken

	// INITIALIZE NeoPixel strip object (REQUIRED)
	strip.setBrightness(50);
	strip.fillGradient(0, 29, mBlack, mBlue);
	strip.fillGradient(30, 59, mGreen, mGray);
	strip.fillGradient(60, 239, mYellow, mOrange);
	strip.fillGradient(240, 299, mRed, mSilver);
	//strip.begin();
	// strip.clear(); // Set all pixel colors to 'off'
	strip.show();
	
	_delay_ms(1000);
	blinken();		// zweites Blinken
	
	// Zurücksetzen
	strip.clear();
	farbe = strip.get(lauf);
	int len = 8;
	
    while (true)
    {
        strip.set(lauf, mBlack);
		lauf = (lauf + 1) % NUMPIXELS;
        strip.fillGradient(lauf, lauf + len/2, mBlack, mRed);
		strip.fillGradient(lauf + len/2, lauf + len, mRed, mBlack);

		//strip.set(lauf, farbe);
		//farbe = strip.get(lauf);
		        
        // Anzeigen und pausieren für nächsten Lauf
        //Serial.println("Zeige Matrix an");
        strip.show();   // Send the updated pixel colors to the hardware.
        _delay_ms(DELAYVAL); // Pause before next pass through loop
        
		blinken();
    }
}