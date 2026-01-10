/*
 * config.h
 *
 * Created: 21.04.2025 16:26:05
 *  Author: Iggy
 */ 
#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>
#include <myarduino.h>
#include <microLED.h>

// --------------------------------------------------------
// Definitionen/Einstellungen
// --------------------------------------------------------
#define STRIP_LED_AMOUNT 140	// количество чипов WS2811/WS2812 на всех ступеньках. Для WS2811 кол-во чипов = кол-во светодиодов / 3
#define STEP_AMOUNT 14			// количество ступенек


#define AUTO_BRIGHT 0			// автояркость вкл(1)/выкл(0) (с фоторезистором)
#define CUSTOM_BRIGHT 100		// ручная яркость

#define FADR_SPEED 300			// скорость переключения с одной ступеньки на другую, меньше - быстрее
#define START_EFFECT RAINBOW	// режим при старте COLOR, RAINBOW, FIRE
#define ROTATE_EFFECTS 1		// вкл(1)/выкл(0) - автосмена эффектов
#define TIMEOUT 15				// секунд, таймаут выключения ступенек после срабатывания одного из датчиков движения

#define NIGHT_LIGHT_COLOR COLORS::mWhite  // по умолчанию белый
#define NIGHT_LIGHT_BRIGHT 50	// 0 - 255 яркость ночной подсветки
#define NIGHT_PHOTO_MAX 500		// максимальное значение фоторезистора для отключения подсветки, при освещении выше этого подсветка полностью отключается

#define RAILING 0		// вкл(1)/выкл(0) - подсветка перил
#define RAILING_LED_AMOUNT 75    // количество чипов WS2811/WS2812 на ленте перил

#define BUTTON  0      // вкл(1)/выкл(0) - сенсорная кнопка переключения эффектов

// пины
// если перепутаны сенсоры - можно поменять их местами в коде! Вот тут
#define SENSOR_START 3	// пин датчика движения
#define SENSOR_END	2	// пин датчика движения
#define STRIP_PIN	6	// пин ленты ступенек
#define RAILING_PIN	8	// пин ленты перил
#define PHOTO_PIN	A0	// пин фоторезистора
#define BUTTON_PIN	9	// пин сенсорной кнопки переключения эффектов


// ==== удобные макросы ====
#define FOR_i(from, to) for(int i = (from); i < (to); i++)
#define FOR_j(from, to) for(int j = (from); j < (to); j++)
#define FOR_k(from, to) for(int k = (from); k < (to); k++)
#define EVERY_MS(x) \
	static uint32_t tmr;\
	bool flag = millis() - tmr >= (x);\
	if (flag) tmr = millis();\
	if (flag)
//===========================

// Deklarationen
typedef enum {NOBLEND=0, LINEARBLEND=1} TBlendType;
typedef enum {COLOR, RAINBOW, FIRE, EFFECTS_AMOUNT} Effect;
	
struct Step {
	int8_t led_amount;
	uint16_t night_mode_bitmask;
};

// Variablen
extern uint16_t effSpeed;
extern int8_t effectDirection;
extern byte curBright;
extern Effect curEffect;
extern byte effectCounter;
extern uint32_t timeoutCounter;
extern bool systemIdleState;
extern bool systemOffState;
extern int8_t minStepLength;
extern int steps_start[STEP_AMOUNT];
extern Step steps[STEP_AMOUNT];

extern microLED<STRIP_LED_AMOUNT, STRIP_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_GRB, CLI_HIGH> strip;
#if (RAILING == 1)
extern microLED<RAILING_LED_AMOUNT, RAILING_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_GRB, CLI_HIGH> railing;
#endif


// Methoden
CRGB getFireColor(int val);
void stepFader(bool dir, bool state);
void fireStairs(int8_t dir, byte from, byte to);

void staticColor(int8_t dir, byte from, byte to);
void rainbowStripes(int8_t dir, byte from, byte to);
void fillStep(int8_t num, MColor color);
void fillStepWithBitMask(int8_t num, MColor color, uint32_t bitMask);
void animatedSwitchOff(int bright);
void animatedSwitchOn(int bright);
void setBrightness(int brightness);

void nightLight();
void handleTimeout();
void effectFlow();

void show();
void clear();

void setup();
void loop();

#endif /* CONFIG_H_ */