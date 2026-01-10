/*
 * config.h
 *
 * Created: 30.03.2025 19:30:34
 *  Author: Iggy
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#include <color_utility.h>
#include <microLED.h>
#include "timerMinim.h"

#define AUTOPLAY_PERIOD 150	// время между авто сменой режимов (секунды)
#define IDLE_TIME 10		// время бездействия кнопок или Bluetooth (в секундах) после которого запускается автосмена режимов и демо в играх

// ************************ МАТРИЦА *************************
// если прошивка не лезет в Arduino NANO - отключай режимы! Строка 60 и ниже
#define BRIGHTNESS		50	// стандартная маскимальная яркость (0-255)
#define CURRENT_LIMIT 2000	// лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

#define WIDTH 20			// ширина матрицы
#define HEIGHT 15			// высота матрицы
#define SEGMENTS 1			// диодов в одном "пикселе" (для создания матрицы из кусков ленты)
#define NUM_LEDS WIDTH * HEIGHT * SEGMENTS

#define MCU_TYPE 0			// микроконтроллер: 
//								0 - AVR (Arduino NANO/MEGA/UNO)
//								1 - ESP8266 (NodeMCU, Wemos D1)
//								2 - STM32 (Blue Pill)

// ****************** ПИНЫ ПОДКЛЮЧЕНИЯ *******************
// Arduino (Nano, Mega)
#if (MCU_TYPE == 0)
#define LED_PIN 6           // пин ленты
#define BUTT_UP 3           // кнопка вверх
#define BUTT_DOWN 5         // кнопка вниз
#define BUTT_LEFT 2         // кнопка влево
#define BUTT_RIGHT 4        // кнопка вправо
#define BUTT_SET 7          // кнопка выбор/игра

// пины подписаны согласно pinout платы, а не надписям на пинах!
// esp8266 - плату выбирал Wemos D1 R1
#elif (MCU_TYPE == 1)
#define LED_PIN 2           // пин ленты
#define BUTT_UP 14          // кнопка вверх
#define BUTT_DOWN 13        // кнопка вниз
#define BUTT_LEFT 0         // кнопка влево
#define BUTT_RIGHT 12       // кнопка вправо
#define BUTT_SET 15         // кнопка выбор/игра

// STM32 (BluePill) - плату выбирал STM32F103C
#elif (MCU_TYPE == 2)
#define LED_PIN PB12         // пин ленты
#define BUTT_UP PA1          // кнопка вверх
#define BUTT_DOWN PA3        // кнопка вниз
#define BUTT_LEFT PA0        // кнопка влево
#define BUTT_RIGHT PA2       // кнопка вправо
#define BUTT_SET PA4         // кнопка выбор/игра
#endif

#define RUNNING_STRING 0
#define CLOCK_MODE 1
#define GAME_MODE 2
#define MADNESS_NOISE 3
#define CLOUD_NOISE 4
#define LAVA_NOISE 5
#define PLASMA_NOISE 6
#define RAINBOW_NOISE 7
#define RAINBOWSTRIPE_NOISE 8
#define ZEBRA_NOISE 9
#define FOREST_NOISE 10
#define OCEAN_NOISE 11
#define SNOW_ROUTINE 12
#define SPARKLES_ROUTINE 13
#define MATRIX_ROUTINE 14
#define STARFALL_ROUTINE 15
#define BALL_ROUTINE 16
#define BALLS_ROUTINE 17
#define RAINBOW_ROUTINE 18
#define RAINBOWDIAGONAL_ROUTINE 19
#define FIRE_ROUTINE 20
#define IMAGE_MODE 21

// ******************** ЭФФЕКТЫ И РЕЖИМЫ ********************
#define D_TEXT_SPEED 150		// скорость бегущего текста по умолчанию (мс)
#define D_EFFECT_SPEED 200		// скорость эффектов по умолчанию (мс)
#define D_GAME_SPEED 250		// скорость игр по умолчанию (мс)
#define D_GIF_SPEED 80			// скорость гифок (мс)
#define DEMO_GAME_SPEED 60		// скорость игр в демо режиме (мс)

// о поддерживаемых цветах читай тут https://alexgyver.ru/gyvermatrixos-guide/
#define GLOBAL_COLOR_1 getHEX(COLORS::mGreen)    // основной цвет №1 для игр
#define GLOBAL_COLOR_2 getHEX(COLORS::mOrange)   // основной цвет №2 для игр

#define SCORE_SIZE 0			// размер символов счёта в игре. 0 - маленький для 8х8 (шрифт 3х5), 1 - большой (шрифт 5х7)
#define FONT_TYPE 1				// (0 / 1) два вида маленького шрифта в выводе игрового счёта

// ************** ОТКЛЮЧЕНИЕ КОМПОНЕНТОВ СИСТЕМЫ (для экономии памяти) *************
// внимание! отключение модуля НЕ УБИРАЕТ его эффекты из списка воспроизведения!
// Это нужно сделать вручную во вкладке custom, удалив ненужные функции

#define USE_BUTTONS 0			// использовать физические кнопки управления играми (0 нет, 1 да)
#define BT_MODE 0				// использовать блютус (0 нет, 1 да)
#define USE_NOISE_EFFECTS 0		// крутые полноэкранные эффекты (0 нет, 1 да) СИЛЬНО ЖРУТ ПАМЯТЬ!!!
#define USE_FONTS 1				// использовать буквы (бегущая строка) (0 нет, 1 да)
#define USE_CLOCK 0				// использовать часы (0 нет, 1 да)
#define OVERLAY_CLOCK 1			// часы на фоне всех эффектов и игр. Жрёт SRAM память!
#define USE_FIRE 1

// игры
#define USE_SNAKE 0				// игра змейка (0 нет, 1 да)
#define USE_TETRIS 0			// игра тетрис (0 нет, 1 да)
#define USE_MAZE 0				// игра лабиринт (0 нет, 1 да)
#define USE_RUNNER 0			// игра бегалка-прыгалка (0 нет, 1 да)
#define USE_FLAPPY 0			// игра flappy bird
#define USE_ARKAN 0				// игра арканоид

// utility.cpp
void displayScore(byte score);
void drawPixelXY(int8_t x, int8_t y, uint32_t color);
uint32_t expandColor(uint16_t color);
uint32_t gammaCorrection(uint32_t color);
uint32_t getPixColor(int thisSegm);
uint32_t getPixColorXY(int8_t x, int8_t y);
uint16_t getPixelNumber(int8_t x, int8_t y);
void drawDigit3x5(byte digit, byte X, byte Y, uint32_t color);
void drawDigit5x7(byte digit, byte X, byte Y, uint32_t color);

// noise_effects.cpp
#if (USE_NOISE_EFFECTS == 1)
void madnessNoise();       // ??????? ??? ?? ??? ???????
void cloudNoise();         // ??????
void lavaNoise();          // ????
void plasmaNoise();        // ??????
void rainbowNoise();       // ???????? ????????
void rainbowStripeNoise(); // ????????? ???????? ????????
void zebraNoise();         // ?????
void forestNoise();        // ??????? ???
void oceanNoise();
#endif

// effects.cpp
void snowRoutine();
void sparklesRoutine();
void matrixRoutine();
void starfallRoutine();
void ballRoutine();
void ballsRoutine();
void rainbowRoutine();
void rainbowDiagonalRoutine();
void fireRoutine();

// custom.cpp
void btnsModeChange();
void customRoutine();

// runningText.cpp
void fillString(const char* text, uint32_t color);

#if (USE_CLOCK == 1)
// clock.cpp
bool overlayAllowed();
extern timerMinim halfsecTimer;

#endif

#if (USE_FONTS == 1)
extern const uint8_t fontHEX[][5] PROGMEM;
#endif

typedef microLED<NUM_LEDS, LED_PIN, MLED_NO_CLOCK, LED_WS2818, ORDER_GRB, CLI_HIGH> FastLED;

// globale Variablen deklarieren
extern const byte MAX_DIM;
extern FastLED fastLED;		// (SPALTEN, ZEILEN, ZIGZAG, RIGHT_TOP, DIR_DOWN);
extern bool loadingFlag;
//extern bool gamemodeFlag;
//extern bool gameDemo;
extern bool idleState;
//extern bool BTcontrol;
extern bool fullTextFlag;
extern int globalBrightness;
extern byte breathBrightness;
//extern int gameSpeed;
extern byte modeCode;
extern int8_t hrs, mins, secs;
extern uint32_t globalColor;

extern bool autoplay;			// 0 выкл / 1 вкл автоматическую смену режимов (откл. можно со смартфона)
extern uint32_t autoplayTime;
extern uint32_t autoplayTimer;
extern timerMinim effectTimer;
//extern timerMinim gameTimer;
extern timerMinim scrollTimer;
extern timerMinim idleTimer;
extern timerMinim changeTimer;

#endif /* CONFIG_H_ */