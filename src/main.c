/* 
 * File:   main.c
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 7, 2017, 10:58 AM
 */

#include <16F886.h>

#FUSES HS, PUT, NOLVP, PROTECT,NOMCLR, BROWNOUT, NOLVP 
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

// Buttons
#define  BUTTON_START_STOP_RESET    PIN_B1
#define  BUTTON_MENU                PIN_B0

// -------------------------FUNCTION PROTOTYPE----------------------------------
void setupHardware(void);
// -------------------------RTOS TASKS------------------------------------------
#task(rate=50ms, max=1ms)
void Task_checkIfStartStopResetButtonIsHeld(void);

#task(rate=50ms, max=1ms)
void Task_checkIfMenuButtonIsHeld(void);

#task(rate=10ms, max=5ms)
void Task_runStateMachine(void);

#task(rate=100ms, max=1ms)
void Task_blinkDisplay(void);

// ----------------------------GLOBAL VARIABLES---------------------------------
BOOLEAN showDisplayCompletely = TRUE;
ButtonState buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
ButtonState buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
PolyoneDisplay myPolyoneDisplay;

void main(void) {
    setupHardware();
    Time_clearRtcTime();
    myPolyoneDisplay.currentState = STATE_INIT;
    rtos_run();
}

void Task_runStateMachine(void) {
    static BOOLEAN startStopButtonState = TRUE;
    static BOOLEAN menuButtonState = TRUE;
    static PolyoneDisplayState nextStateAfterWaitingForButtonBeingReleased;

    switch (myPolyoneDisplay.currentState) {
        case STATE_INIT:
            myPolyoneDisplay = PolyoneDisplay_new();
            break;

        case STATE_IDLE:
            if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
                myPolyoneDisplay.currentState = myPolyoneDisplay.previousState;
//                Timer_saveStateCounting(&myTimer);
                PolyoneDisplay_updateRtc(&myPolyoneDisplay);
              
            }
            if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
                buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
                myPolyoneDisplay.currentState = STATE_RESETTING;
            }
            break;

        case STATE_COUNTING_DOWN:
            PolyoneDisplay_updateTimer(&myPolyoneDisplay);
            PolyoneDisplay_showCount(&myPolyoneDisplay);

            if (buttonStateStartStopReset == BUTTON_STATE_HELD) {
                buttonStateStartStopReset = BUTTON_STATE_NOT_PUSHED;
                myPolyoneDisplay.currentState = STATE_RESETTING;
            }
            if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
                myPolyoneDisplay.currentState = STATE_IDLE;
                //
                //                Timer_saveRtcCurrentTime(&myTimer);
                //                Timer_saveStateAsWasNotCounting(&myTimer);
                //                SevenSegmentDisplay_showHoursAndMinutesOfCountdownTime(&myTimer, FALSE);
            }
            if (PolyoneDisplay_isTimerDone(&myPolyoneDisplay)) {

                myPolyoneDisplay.currentState = STATE_COUNTING_UP;
            }
            break;

        case STATE_COUNTING_UP:

            break;

        case STATE_RESETTING:
            Time_clearRtcTime();
            SeventSegmentDisplay_showHyphens();
            delay_ms(TIMEOUT_HYPHENS_MILISECONDS);
            myPolyoneDisplay.currentState = STATE_WAITING_FOR_BUTTON_BEING_RELEASED;
            nextStateAfterWaitingForButtonBeingReleased = STATE_READY;
            Time_clearRtcTime();
            PolyoneDisplay_updateTimer(&myPolyoneDisplay);
            PolyoneDisplay_showCount(&myPolyoneDisplay);
            rtos_enable(Task_checkIfMenuButtonIsHeld);
            break;

        case STATE_WAITING_FOR_BUTTON_BEING_RELEASED:
            if (input(BUTTON_START_STOP_RESET) && input(BUTTON_MENU)) {
                myPolyoneDisplay.currentState = nextStateAfterWaitingForButtonBeingReleased;
            }
            break;

        case STATE_READY:
            PolyoneDisplay_showCount(&myPolyoneDisplay);
            if (input(BUTTON_START_STOP_RESET) && (!startStopButtonState)) {
                myPolyoneDisplay.currentState = STATE_COUNTING_DOWN;
                //                Timer_saveStateAsWasCounting(&myTimer);
                Time_clearRtcTime();
            }
            if (buttonStateMenu == BUTTON_STATE_HELD) {
                buttonStateMenu = BUTTON_STATE_NOT_PUSHED;
                myPolyoneDisplay.currentState = STATE_SETTING_ALARM_HOUR;
                rtos_enable(Task_blinkDisplay);
                rtos_disable(Task_checkIfMenuButtonIsHeld);
            }
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

