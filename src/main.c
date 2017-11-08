/* 
 * File:   main.c
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 7, 2017, 10:58 AM
 */

#include <16F886.h>

#FUSES HS, PUT, NOLVP, PROTECT,NOMCLR, BROWNOUT, NODEBUG     
#device *=16
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

#define DELAY_INCREASE_NUMBER_MILISECONDS 300

// Buttons
#define  BUTTON_START_STOP_RESET    PIN_B1
#define  BUTTON_MENU                PIN_B0

typedef enum {
    EEPROM_CURRENT_STATE = 0,
    EEPROM_PREVIOUS_STATE,
    EEPROM_FORMAT,
    EEPROM_FIRST_NUMBER,
    EEPROM_SECOND_NUMBER,
    EEPROM_RTC_HOURS,
    EEPROM_RTC_MINUTES,
    EEPROM_RTC_SECONDS,
} EEPROM_ADDRESS;

typedef void (*StateMachineFunction)(void);


// -------------------------FUNCTION PROTOTYPE----------------------------------
void setupHardware(void);
void StateMachine_idle(void);
void StateMachine_countingUp(void);
void StateMachine_countingDown(void);
void StateMachine_init(void);
void StateMachine_resetting(void);
void StateMachine_ready(void);
void StateMachine_settingFirstNumber(void);
void StateMachine_settingSecondNumber(void);
void StateMachine_settingFormat(void);
void StateMachine_waitingForButtonBeingReleased(void);
// -------------------------RTOS TASKS------------------------------------------
#task(rate=50ms, max=1ms)
void Task_checkIfStartStopResetButtonIsHeld(void);

#task(rate=50ms, max=1ms)
void Task_checkIfMenuButtonIsHeld(void);

#task(rate=10ms, max=5ms)
void Task_runStateMachine(void);

#task(rate=200ms, max=1ms)
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

void (*functionPtr)(void);

void main(void) {
    setupHardware();

    myPolyoneDisplay.currentState = STATE_INIT;
    //    functionPtr = StateMachine_init;

    rtos_run();
}

void x(void) {
    switch (myPolyoneDisplay.currentState) {
        case STATE_INIT:
            StateMachine_init();
            break;

        case STATE_IDLE:
            StateMachine_idle();
            break;

        case STATE_COUNTING_DOWN:
            StateMachine_countingDown();
            break;

        case STATE_COUNTING_UP:

            break;

        case STATE_RESETTING:
            StateMachine_resetting();
            break;

        case STATE_WAITING_FOR_BUTTON_BEING_RELEASED:
            StateMachine_waitingForButtonBeingReleased();
            break;
    }
}

void Task_runStateMachine(void) {
    x();
    switch (myPolyoneDisplay.currentState) {

        case STATE_READY:
            StateMachine_ready();
            break;

        case STATE_SETTING_FIRST_NUMBER:
            StateMachine_settingFirstNumber();

            break;
        case STATE_SETTING_SECOND_NUMBER:
            StateMachine_settingSecondNumber();
            break;

        case STATE_SETTING_FORMAT:
            StateMachine_settingFormat();
            break;
    }

    startStopButtonState = input(BUTTON_START_STOP_RESET);
    menuButtonState = input(BUTTON_MENU);
    rtos_yield();
}

void Task_checkIfStartStopResetButtonIsHeld(void) {
    static int nextUpButtonCounter = 0;
    if (!input(BUTTON_START_STOP_RESET)) {
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

    if (!input(BUTTON_MENU)) {
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

void StateMachine_init(void) {
    output_high(PIN_B3);
    myPolyoneDisplay = PolyoneDisplay_new(
            EEPROM_CURRENT_STATE,
            EEPROM_PREVIOUS_STATE,
            EEPROM_FORMAT,
            EEPROM_FIRST_NUMBER,
            EEPROM_SECOND_NUMBER,
            EEPROM_RTC_HOURS,
            EEPROM_RTC_MINUTES,
            EEPROM_RTC_SECONDS
            );
    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    
}

void StateMachine_idle(void) {

    if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
        PolyoneDisplay_resume(&myPolyoneDisplay);
        PolyoneDisplay_saveState(&myPolyoneDisplay);
        PolyoneDisplay_updateRtc(&myPolyoneDisplay);

    }
    if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
        buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
        myPolyoneDisplay.currentState = STATE_RESETTING;
    }
}

void StateMachine_countingUp(void) {

}

void StateMachine_countingDown(void) {
    PolyoneDisplay_updateTimer(&myPolyoneDisplay);
    PolyoneDisplay_showCount(&myPolyoneDisplay, TRUE);

    if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
        buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
        myPolyoneDisplay.currentState = STATE_RESETTING;
    }
    if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
        PolyoneDisplay_stop(&myPolyoneDisplay);
        PolyoneDisplay_saveRtcCurrentTime(&myPolyoneDisplay);
        PolyoneDisplay_saveState(&myPolyoneDisplay);
        PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    }
    if (PolyoneDisplay_isTimerDone(&myPolyoneDisplay)) {

        myPolyoneDisplay.currentState = STATE_COUNTING_UP;
    }
}

void StateMachine_resetting(void) {
    Time_clearRtcTime();
    SeventSegmentDisplay_showHyphens();
    delay_ms(TIMEOUT_HYPHENS_MILISECONDS);
    myPolyoneDisplay.currentState = STATE_WAITING_FOR_BUTTON_BEING_RELEASED;
    nextStateAfterWaitingForButtonBeingReleased = STATE_READY;
    Time_clearRtcTime();
    PolyoneDisplay_updateTimer(&myPolyoneDisplay);
    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    rtos_enable(Task_checkIfMenuButtonIsHeld);
}

void StateMachine_ready(void) {
    PolyoneDisplay_showCount(&myPolyoneDisplay, FALSE);
    if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
        PolyoneDisplay_setState(&myPolyoneDisplay, STATE_COUNTING_DOWN);
        PolyoneDisplay_saveState(&myPolyoneDisplay);
        Time_clearRtcTime();
    }
    if (buttonStateMenu == BUTTON_STATE_HELD) {
        buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
        myPolyoneDisplay.currentState = STATE_SETTING_FIRST_NUMBER;
        rtos_enable(Task_blinkDisplay);
        rtos_disable(Task_checkIfMenuButtonIsHeld);
    }
}

void StateMachine_settingFirstNumber(void) {
    if (showDisplayCompletely) {
        PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
    } else {
        PolyoneDisplay_showSecondNumber(&myPolyoneDisplay);
    }
    if (!input(BUTTON_START_STOP_RESET)) {
        while (!input(BUTTON_START_STOP_RESET)) {
            PolyoneDisplay_increaseFirstNumber(&myPolyoneDisplay);
            PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
            delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
        }
    }

    if (!menuButtonState && input(BUTTON_MENU)) {
        numberOfMenuButtonHasBeenReleased++;
    }
    if (numberOfMenuButtonHasBeenReleased >= 2) {
        numberOfMenuButtonHasBeenReleased = 0;
        myPolyoneDisplay.currentState = STATE_SETTING_SECOND_NUMBER;
    }
}

void StateMachine_settingSecondNumber(void) {
    if (showDisplayCompletely) {
        PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
    } else {
        PolyoneDisplay_showFirstNumber(&myPolyoneDisplay);
    }
    if (!input(BUTTON_START_STOP_RESET)) {
        while (!input(BUTTON_START_STOP_RESET)) {
            PolyoneDisplay_increaseSecondNumber(&myPolyoneDisplay);
            PolyoneDisplay_showLimitTime(&myPolyoneDisplay);
            delay_ms(DELAY_INCREASE_NUMBER_MILISECONDS);
        }
    }
    if (!menuButtonState && input(BUTTON_MENU)) {
        myPolyoneDisplay.currentState = STATE_SETTING_FORMAT;
    }
}

void StateMachine_settingFormat(void) {
    PolyoneDisplay_showFormat(&myPolyoneDisplay);
    if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
        PolyoneDisplay_swapFormat(&myPolyoneDisplay);
    }
    if (!menuButtonState && input(BUTTON_MENU)) {
        myPolyoneDisplay.currentState = STATE_READY;
    }
}

void StateMachine_waitingForButtonBeingReleased(void) {
    if (input(BUTTON_START_STOP_RESET) && input(BUTTON_MENU)) {
        myPolyoneDisplay.currentState = nextStateAfterWaitingForButtonBeingReleased;
    }
}

