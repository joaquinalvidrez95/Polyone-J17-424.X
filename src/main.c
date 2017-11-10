/* 
 * File:   main.c
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 7, 2017, 10:58 AM
 */

#include <16F886.h>

#FUSES HS, PUT, NOLVP, PROTECT,NOMCLR, BROWNOUT, NODEBUG     
//#device *=16
#use delay(clock=20M)
#use standard_io(a)
#use standard_io(c)
#use standard_io(b)
#use rtos(timer=0)
#byte WPUB = 0x95

#include "polyonedisplay.h"

// Timeouts
#define TIMEOUT_MENU_BUTTON_MILISECONDS 2000
#define TIMEOUT_RESET_TIMER_MILISECONDS 3000
#define TIMEOUT_HYPHENS_MILISECONDS     2000

//#define DELAY_INCREASE_NUMBER_MILISECONDS 100
#define DELAY_INCREASE_NUMBER_MILISECONDS 200

// Buttons
#define  PIN_BUTTON_START   PIN_B1
#define  PIN_BUTTON_MENU    PIN_B0
#define  PIN_BUZZER         PIN_C5
#define PIN_LED PIN_B3

typedef enum {
    EEPROM_CURRENT_STATE = 0,
    EEPROM_PREVIOUS_STATE,
    EEPROM_FORMAT,
    EEPROM_FIRST_NUMBER,
    EEPROM_SECOND_NUMBER,
    EEPROM_RTC_HOURS,
    EEPROM_RTC_MINUTES,
    EEPROM_RTC_SECONDS,
    EEPROM_BRIGHTNESS,
    EEPROM_TYPE_OF_COUNT,
    EEPROM_NUMBER_OF_DAYS
} EEPROM_ADDRESS;

// -------------------------FUNCTION PROTOTYPE----------------------------------
void setupHardware(void);
void turnOnBuzzer(void);
void blinkDisplay(void);
// -------------------------RTOS TASKS------------------------------------------
#task(rate=50ms, max=1ms)
void Task_checkIfStartStopResetButtonIsHeld(void);

#task(rate=50ms, max=1ms)
void Task_checkIfMenuButtonIsHeld(void);

#task(rate=10ms, max=5ms)
void Task_runStateMachine(void);

#task(rate=50ms, max=1ms)
void Task_blinkDisplay(void);

// ----------------------------GLOBAL VARIABLES---------------------------------
BOOLEAN showDisplayCompletely = TRUE;
ButtonState buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
ButtonState buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
PolyoneDisplay myPolyoneDisplay;

BOOLEAN startStopButtonState = TRUE;
BOOLEAN menuButtonState = TRUE;
PolyoneDisplayState nextStateAfterWaitingForButtonBeingReleased;
int numberOfMenuButtonHasBeenReleased = 0;

void main(void) {
    setupHardware();
    myPolyoneDisplay.currentState = STATE_INIT;
    rtos_run();
}

void x(void) {
    switch (myPolyoneDisplay.currentState) {
        case STATE_INIT:
            myPolyoneDisplay = PolyoneDisplay_new(
                    EEPROM_CURRENT_STATE,
                    EEPROM_PREVIOUS_STATE,
                    EEPROM_FORMAT,
                    EEPROM_FIRST_NUMBER,
                    EEPROM_SECOND_NUMBER,
                    EEPROM_RTC_HOURS,
                    EEPROM_RTC_MINUTES,
                    EEPROM_RTC_SECONDS,
                    EEPROM_BRIGHTNESS,
                    EEPROM_TYPE_OF_COUNT);
            PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
            break;

        case STATE_IDLE:
            if (input(PIN_BUTTON_START) && (!startStopButtonState)) {
                PolyoneDisplay_resume(&myPolyoneDisplay);
                PolyoneDisplay_saveState(&myPolyoneDisplay);
                PolyoneDisplay_updateRtc(&myPolyoneDisplay);
                blinkDisplay();
                PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
            }
            if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
                buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_RESETTING);
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_BRIGHTNESS);
            }
            break;

        case STATE_COUNTING_DOWN:
            PolyoneDisplay_updateTimer(&myPolyoneDisplay);
            PolyoneDisplay_showCount(&myPolyoneDisplay, TRUE);
            if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
                buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_RESETTING);
            }
            if (input(PIN_BUTTON_START) && (!startStopButtonState)) {
                PolyoneDisplay_stop(&myPolyoneDisplay);
                PolyoneDisplay_saveRtcCurrentTime(&myPolyoneDisplay);
                PolyoneDisplay_saveState(&myPolyoneDisplay);
                blinkDisplay();
                PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
            }
            if (PolyoneDisplay_isTimerDone(&myPolyoneDisplay)) {
                turnOnBuzzer();
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_COUNTING_UP);
                PolyoneDisplay_saveState(&myPolyoneDisplay);
                myPolyoneDisplay.typeOfCount = COUNTUP;
                PolyoneDisplay_saveTypeOfCount(&myPolyoneDisplay);
                Time_clearRtcTime();
                setDate(1, 1, 1, 1);
                PolyoneDisplay_updateTimer(&myPolyoneDisplay);
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_BRIGHTNESS);
            }
            break;

        case STATE_COUNTING_UP:
            if (!PolyoneDisplay_isCountUpDone(&myPolyoneDisplay)) {
                PolyoneDisplay_updateTimer(&myPolyoneDisplay);
                PolyoneDisplay_showCount(&myPolyoneDisplay, TRUE);
                if (input(PIN_BUTTON_START) && (!startStopButtonState)) {
                    PolyoneDisplay_stop(&myPolyoneDisplay);
                    PolyoneDisplay_saveRtcCurrentTime(&myPolyoneDisplay);
                    PolyoneDisplay_saveState(&myPolyoneDisplay);
                    blinkDisplay();
                    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
                }
            }
            if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
                buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_RESETTING);
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_BRIGHTNESS);
            }           
            break;

        case STATE_RESETTING:
            myPolyoneDisplay.typeOfCount = COUNTDOWN;
            PolyoneDisplay_saveTypeOfCount(&myPolyoneDisplay);
            Time_clearRtcTime();
            setDate(1, 1, 1, 1);
            SeventSegmentDisplay_showHyphens();
            delay_ms(TIMEOUT_HYPHENS_MILISECONDS);
            PolyoneDisplay_setState(&myPolyoneDisplay, STATE_WAITING_FOR_BUTTON_BEING_RELEASED);
            nextStateAfterWaitingForButtonBeingReleased = STATE_READY;
            Time_clearRtcTime();
            setDate(1, 1, 1, 1);
            PolyoneDisplay_updateTimer(&myPolyoneDisplay);
            PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
            break;

        case STATE_WAITING_FOR_BUTTON_BEING_RELEASED:
            if (input(PIN_BUTTON_START) && input(PIN_BUTTON_MENU)) {
                PolyoneDisplay_setState(&myPolyoneDisplay, nextStateAfterWaitingForButtonBeingReleased);
            }
            break;
    }
}

void Task_runStateMachine(void) {
    x();
    switch (myPolyoneDisplay.currentState) {
        case STATE_READY:
            PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
            if (input(PIN_BUTTON_START) && (!startStopButtonState)) {
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_COUNTING_DOWN);
                PolyoneDisplay_saveState(&myPolyoneDisplay);
                blinkDisplay();
                Time_clearRtcTime();
                setDate(1, 1, 1, 1);
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_FIRST_NUMBER);
            }
            break;

        case STATE_SETTING_FIRST_NUMBER:
            if (showDisplayCompletely) {
                PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
            } else {
                PolyoneDisplay_showSecondNumber(&myPolyoneDisplay);
            }
            if (!input(PIN_BUTTON_START)) {
                while (!input(PIN_BUTTON_START)) {
                    PolyoneDisplay_increaseFirstNumber(&myPolyoneDisplay);
                    PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
                    delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
                }
            }
            if (!menuButtonState && input(PIN_BUTTON_MENU)) {
                if (myPolyoneDisplay.previousState == STATE_READY) {
                    numberOfMenuButtonHasBeenReleased++;
                } else {
                    PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_SECOND_NUMBER);
                }
            }
            if (numberOfMenuButtonHasBeenReleased >= 2) {
                numberOfMenuButtonHasBeenReleased = 0;
                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_SECOND_NUMBER);
            }

            break;
        case STATE_SETTING_SECOND_NUMBER:
            if (showDisplayCompletely) {
                PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
            } else {
                PolyoneDisplay_showFirstNumber(&myPolyoneDisplay);
            }
            if (!input(PIN_BUTTON_START)) {
                while (!input(PIN_BUTTON_START)) {
                    PolyoneDisplay_increaseSecondNumber(&myPolyoneDisplay);
                    PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
                    delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
                }
            }
            if (!menuButtonState && input(PIN_BUTTON_MENU)) {
                if (PolyoneDisplay_isAlarmOkay(&myPolyoneDisplay)) {
                    PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_FORMAT);
                } else {

                    PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_FIRST_NUMBER);
                }
            }
            break;

        case STATE_SETTING_FORMAT:
            PolyoneDisplay_showFormat(&myPolyoneDisplay);
            if (input(PIN_BUTTON_START) && (!startStopButtonState)) {
                PolyoneDisplay_swapFormat(&myPolyoneDisplay);
            }
            if (!menuButtonState && input(PIN_BUTTON_MENU)) {
                PolyoneDisplay_saveAlarm(&myPolyoneDisplay);
                PolyoneDisplay_saveFormat(&myPolyoneDisplay);
                setDate(1, 1, 1, 1);
                Time_clearRtcTime();
                PolyoneDisplay_updateTimer(&myPolyoneDisplay);

                PolyoneDisplay_setState(&myPolyoneDisplay, STATE_SETTING_BRIGHTNESS);
            }
            break;

        case STATE_SETTING_BRIGHTNESS:
            if (showDisplayCompletely) {
                PolyoneDisplay_showBrightness(&myPolyoneDisplay);
            } else {
                PolyoneDisplay_hideBrightness();
            }
            if (!input(PIN_BUTTON_START)) {
                while (!input(PIN_BUTTON_START)) {
                    PolyoneDisplay_increaseBrightness(&myPolyoneDisplay);
                    PolyoneDisplay_showBrightness(&myPolyoneDisplay);
                    delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
                }
            }

            if (!menuButtonState && input(PIN_BUTTON_MENU)) {
                if (myPolyoneDisplay.previousState == STATE_SETTING_FORMAT) {
                    PolyoneDisplay_saveBrightness(&myPolyoneDisplay);
                    PolyoneDisplay_setState(&myPolyoneDisplay, STATE_READY);
                } else {
                    numberOfMenuButtonHasBeenReleased++;
                }
            }
            if (numberOfMenuButtonHasBeenReleased >= 2) {
                numberOfMenuButtonHasBeenReleased = 0;
                PolyoneDisplay_setState(&myPolyoneDisplay, myPolyoneDisplay.previousState);
                PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
                PolyoneDisplay_saveBrightness(&myPolyoneDisplay);
            }
            break;
    }

    startStopButtonState = input(PIN_BUTTON_START);
    menuButtonState = input(PIN_BUTTON_MENU);
    rtos_yield();
}

void Task_checkIfStartStopResetButtonIsHeld(void) {
    static int nextUpButtonCounter = 0;
    if (!input(PIN_BUTTON_START)) {
        nextUpButtonCounter++;
    } else {
        nextUpButtonCounter = 0;
        buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
    }
    if (nextUpButtonCounter >= (TIMEOUT_RESET_TIMER_MILISECONDS / 50 / 3)) {
        buttonStateStartStopReset = BUTTON_STATE_HELD;
        nextUpButtonCounter = 0;
    }
    rtos_yield();
}

void Task_checkIfMenuButtonIsHeld(void) {
    static int menuButtonCounter = 0;

    if (!input(PIN_BUTTON_MENU)) {
        menuButtonCounter++;
    } else {
        menuButtonCounter = 0;
        buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
    }
    if (menuButtonCounter >= (TIMEOUT_MENU_BUTTON_MILISECONDS / 50 / 3)) {
        buttonStateMenu = BUTTON_STATE_HELD;
        menuButtonCounter = 0;
    }
    rtos_yield();
}

void Task_blinkDisplay(void) {
    showDisplayCompletely = ~showDisplayCompletely;
    rtos_yield();
}

void setupHardware(void) {
    setup_timer_2(T2_DIV_BY_16, 255, 1);
    setup_ccp1(CCP_PWM | CCP_PWM_L_H);

    delay_ms(500);
    port_b_pullups(0xFF);
    WPUB = 0xFF;
}

void turnOnBuzzer(void) {
    output_high(PIN_BUZZER);
    delay_ms(200);
    output_low(PIN_BUZZER);
    delay_ms(100);
    output_high(PIN_BUZZER);
    delay_ms(200);
    output_low(PIN_BUZZER);
}

void blinkDisplay(void) {
    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    delay_ms(200);
    SevenSegmentDisplay_clearDisplay();
    delay_ms(200);
    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    delay_ms(200);
    SevenSegmentDisplay_clearDisplay();
}
