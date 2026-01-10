/*
 * MatrixOS.cpp
 *
 * Created: 30.03.2025 18:42:24
 * Author : Iggy
 */ 

#include <avr/io.h>
#include <stdlib.h>	// Header für Zufall
#include "config.h"

bool autoplay = true;			// 0 выкл / 1 вкл автоматическую смену режимов (откл. можно со смартфона)

// ******************************** ДЛЯ РАЗРАБОТЧИКОВ ********************************
#define DEBUG 0



#if (MCU_TYPE == 1)
#define FASTLED_INTERRUPT_RETRY_COUNT 0
#define FASTLED_ALLOW_INTERRUPTS 0
#include <ESP8266WiFi.h>
#endif


const byte MAX_DIM = max(WIDTH, HEIGHT);
byte buttons = 4;		// 0 - верх, 1 - право, 2 - низ, 3 - лево, 4 - не нажата
int globalBrightness = BRIGHTNESS;
//byte globalSpeed = 200;
uint32_t globalColor = 0x00ff00;   // цвет при запуске зелёный
byte breathBrightness;
bool loadingFlag = true;
byte frameNum;
//int gameSpeed = DEMO_GAME_SPEED;
//bool gameDemo = true;
bool idleState = true;		// флаг холостого режима работы
//bool BTcontrol = false;		// флаг контроля с блютус. Если false - управление с кнопок
int8_t thisMode = 12;
bool controlFlag = false;
//bool gamemodeFlag = false;
//bool mazeMode = false;
int effects_speed = D_EFFECT_SPEED;
int8_t hrs = 10, mins = 25, secs;
bool dotFlag;
byte modeCode;				// 0 бегущая, 1 часы, 2 игры, 3 нойс маднесс и далее, 21 гифка или картинка
bool fullTextFlag = false;
bool clockSet = false;

#if (USE_FONTS == 1)
#include "fonts.h"
#endif

unsigned long autoplayTime = ((long)AUTOPLAY_PERIOD * 1000);
unsigned long autoplayTimer = 0;

// Timer für Grafikeffekte
timerMinim effectTimer(D_EFFECT_SPEED);
//timerMinim gameTimer(DEMO_GAME_SPEED);
// Timer für die Textgeschwindigkeit
timerMinim scrollTimer(D_TEXT_SPEED);
// Zeitgeber für den Wechsel zurück zur Automatik
timerMinim idleTimer((long)IDLE_TIME * 1000);
// Zeitgeber für den Übergang zwischen den Effekten
timerMinim changeTimer(70);
#if (USE_CLOCK == 1)
// Timer für die Uhr
timerMinim halfsecTimer(500);
#endif

//microLED<NUM_LEDS, SIGNAL_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_HIGH>
//FastLED	fastLED(WIDTH, HEIGHT, ZIGZAG, RIGHT_TOP, DIR_DOWN);
FastLED	fastLED(WIDTH, HEIGHT, ZIGZAG, LEFT_BOTTOM, DIR_UP);

int main(void)
{
	// Initialisierung
	DDRB = DDRC = 0;
	srandom(PORTB * PORTC);
	
	setupMillis();
	fastLED.setBrightness(BRIGHTNESS);
	fastLED.fillGradient(0, NUM_LEDS, mRed, mBlue);
	fastLED.show();
	_delay_ms(500);

    // die OS-Methode
    while (1) 
    {
		customRoutine();
    }
}

