/*
  Новые функции в прошивке 1.2 описаны в README.MD
  Автор: Геннадий Дегтерёв, 2021
  gennadij@degterjow.de

  Скетч к проекту "Подсветка лестницы"
  Страница проекта (схемы, описания): https://alexgyver.ru/ledstairs/
  Исходники на GitHub: https://github.com/AlexGyver/LEDstairs
  Нравится, как написан код? Поддержи автора! https://alexgyver.ru/support_alex/
  Автор: AlexGyver Technologies, 2019
  https://AlexGyver.ru/
*/

#include "config.h"



// описание всех ступенек с возможностью подсветки ЛЮБЫХ ступенек в ночном режиме
Step steps[STEP_AMOUNT] = {
	{ 10, 0b1001001001 },		// первая ступенька 7 чипов, 0b1001001 - каждый третий чип активен в ночном режиме
	{ 10, 0b0000000000 },		// вторая ступенька 7 чипов, 0b0000000000 - не активен в ночном режиме
	{ 10, 0b0000000000 },		// 3
	{ 10, 0b0000000000 },		// 4
	{ 10, 0b0000000000 },		// 5
	{ 10, 0b1001001001 },		// 6
	{ 10, 0b0000000000 },		// 7
	{ 10, 0b0000000000 },		// 8
	{ 10, 0b1001001001 },		// 9
	{ 10, 0b0000000000 },		// 10
	{ 10, 0b0000000000 },		// 11
	{ 10, 0b0000000000 },		// 12
	{ 10, 0b0000000000 },		// 13
	{ 10, 0b1001001001 }		// 14
	//{ 16, 0b0000000000000000 },   // 15
};

//#define ORDER_BGR       // порядок цветов ORDER_GRB / ORDER_RGB / ORDER_BRG
//#define COLOR_DEBTH 2   // цветовая глубина: 1, 2, 3 (в байтах)

// для разработчиков
//#include <microLED.h>
//#include <FastLED.h> // ФЛ для функции Noise

#if (BUTTON == 1)
#include <GyverButton.h>
#endif

int railingSegmentLength = RAILING_LED_AMOUNT / STEP_AMOUNT;   // количество чипов WS2811/WS2812 на сегмент ленты перил

// объект лента (НЕ МАТРИЦА) из-за разного количества диодов на ступеньку!
//MColor stripLEDs[STRIP_LED_AMOUNT];  // буфер ленты ступенек
microLED<STRIP_LED_AMOUNT, STRIP_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_GRB, CLI_HIGH> strip;

#if (RAILING == 1)
// объект лента
//MColor railingLEDs[RAILING_LED_AMOUNT];  // буфер ленты перил
microLED<RAILING_LED_AMOUNT, RAILING_PIN, MLED_NO_CLOCK, LED_WS2811, ORDER_GRB, CLI_HIGH> railing;
#endif

uint16_t effSpeed;
int8_t effectDirection;
byte curBright = CUSTOM_BRIGHT;
Effect curEffect = START_EFFECT;
byte effectCounter;
uint32_t timeoutCounter;
bool systemIdleState;
bool systemOffState;
int steps_start[STEP_AMOUNT];

struct PirSensor {
  int8_t effectDirection;
  int8_t pin;
  bool lastState;
};

PirSensor startPirSensor = { 1, SENSOR_START, false};
PirSensor endPirSensor = { -1, SENSOR_END, false};

CRGB firePalette[16] = {
	getFireColor(0 * 16),
	getFireColor(1 * 16),
	getFireColor(2 * 16),
	getFireColor(3 * 16),
	getFireColor(4 * 16),
	getFireColor(5 * 16),
	getFireColor(6 * 16),
	getFireColor(7 * 16),
	getFireColor(8 * 16),
	getFireColor(9 * 16),
	getFireColor(10 * 16),
	getFireColor(11 * 16),
	getFireColor(12 * 16),
	getFireColor(13 * 16),
	getFireColor(14 * 16),
	getFireColor(15 * 16)
};

int8_t minStepLength = steps[0].led_amount;

#if (BUTTON == 1)
GButton button(BUTTON_PIN);
#endif

void setup()
{
	//Serial.begin(9600);
	setBrightness(curBright);    // яркость (0-255)
	clear();
	show();  
  
#if (BUTTON == 1)
	button.setType(HIGH_PULL);
	button.setDirection(NORM_OPEN);
	button.setDebounce(100);     // настройка антидребезга (по умолчанию 80 мс)
	button.setTimeout(700);      // настройка таймаута на удержание (по умолчанию 500 мс)
	button.setClickTimeout(600); // настройка таймаута между кликами (по умолчанию 300 мс)
#endif

  // определяем минимальную ширину ступеньки для корректной работы эффекта огня
	steps_start[0] = 0;
	FOR_i(1, STEP_AMOUNT)
	{
		if (steps[i].led_amount < minStepLength) {
			minStepLength = steps[i].led_amount;
		}
		steps_start[i] = steps_start[i-1] + steps[i-1].led_amount; // вычисляем стартовые позиции каждой ступеньки
	}
	delay(100);
	clear();
	show();
}

void handleButton()
{
#if (BUTTON == 1)
	button.tick();
	if (button.isClick() || button.isHolded())
	{
		curEffect = ++effectCounter % EFFECTS_AMOUNT;
	}
#endif
}

void handlePhotoResistor()
{
#if (AUTO_BRIGHT == 1)
	EVERY_MS(3000) {            // каждые 3 сек
		int photo = analogRead(PHOTO_PIN);
		//Serial.print("Photo resistor ");
		//Serial.println(photo);
		systemOffState = photo > NIGHT_PHOTO_MAX;
		curBright = systemOffState ? 0 : map(photo, 30, 800, 10, 200);
		setBrightness(curBright);
	}
#endif
}

void handleNightLight()
{
  EVERY_MS(60000) {
    nightLight();
  }
}

void nightLight()
{
	if (systemOffState) {
		//Serial.println("System OFF ");
		clear();
		show();
		return;
	}
	animatedSwitchOff(NIGHT_LIGHT_BRIGHT);
	clear();
	FOR_i(0, STEP_AMOUNT)
	{
		// циклически сдвигаем маску, чтобы диоды не выгорали
		if (steps[i].night_mode_bitmask) {
			steps[i].night_mode_bitmask = (uint16_t) steps[i].night_mode_bitmask >> 1 | steps[i].night_mode_bitmask << 15;
			fillStepWithBitMask(i, NIGHT_LIGHT_COLOR, steps[i].night_mode_bitmask);
		}
	}
	animatedSwitchOn(NIGHT_LIGHT_BRIGHT);
}

void handleTimeout()
{
	if (millis() - timeoutCounter >= (TIMEOUT * 1000L)) {
		systemIdleState = true;
		if (effectDirection == 1) {
			stepFader(0, 1);
		} else {
			stepFader(1, 1);
		}
		nightLight();
	}
}

void handlePirSensor(PirSensor *sensor)
{
	if (systemOffState) return;

	int newState = digitalRead(sensor->pin);
	if (newState && !sensor->lastState) {
		//Serial.print("PIR sensor ");
		//Serial.println(sensor->pin);
		timeoutCounter = millis(); // при срабатывании датчика устанавливаем заново timeout
		if (systemIdleState) {
			effectDirection = sensor->effectDirection;
			if (ROTATE_EFFECTS) {
				curEffect = Effect(++effectCounter % (uint8_t)EFFECTS_AMOUNT);
			}
			stepFader(effectDirection == 1 ? 0 : 1,  0);
			systemIdleState = false;
		}
	}
	sensor->lastState = newState;
}

// крутилка эффектов в режиме активной работы
void effectFlow()
{
	static unsigned long tmr;
	if (millis() - tmr >= effSpeed) {
		tmr = millis();
		switch (curEffect) {
			case COLOR: staticColor(effectDirection, 0, STEP_AMOUNT); break;
			case RAINBOW: rainbowStripes(-effectDirection, 0, STEP_AMOUNT); break; // rainbowStripes - приёмный
			case FIRE: fireStairs(effectDirection, 0, 0); break;
			case EFFECTS_AMOUNT: break;	// Dummy
		}
		show();
	}
}


void loop()
{
	handleButton();
	handlePirSensor(&startPirSensor);
	handlePirSensor(&endPirSensor);
	if (systemIdleState || systemOffState) {
		handlePhotoResistor();
		handleNightLight();
		delay(50);
	} else {
		effectFlow();
		handleTimeout();
	}
}