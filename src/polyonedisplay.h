/* 
 * File:   polyonedisplay.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on November 7, 2017, 11:36 AM
 */

#ifndef POLYONEDISPLAY_H
#define	POLYONEDISPLAY_H

#include "timer.h"
#include "buttons.h"
#include "sevensegmentdisplay.h"

// Upper bounds
#define FIRST_NUMBER_UPPER_BOUND    99
#define SECOND_NUMBER_UPPER_BOUND   59

typedef enum {
    FORMAT_MINUTES_SECONDS = 0,
    FORMAT_HOURS_MINUTES
} PolyoneDisplayFormat;

typedef enum {
    COUNTDOWN,
    COUNTUP
} TypeOfCount;

typedef enum {
    STATE_IDLE = 0,
    STATE_COUNTING_UP,
    STATE_COUNTING_DOWN,
    STATE_INIT,
    STATE_RESETTING,
    STATE_READY,
    STATE_SETTING_FIRST_NUMBER,
    STATE_SETTING_SECOND_NUMBER,
    STATE_SETTING_FORMAT,
    STATE_WAITING_FOR_BUTTON_BEING_RELEASED,
    NUMBER_OF_STATES
} PolyoneDisplayState;

typedef struct {
    Timer timer;
    PolyoneDisplayFormat format;
    TypeOfCount typeOfCount;
    PolyoneDisplayState currentState;
    PolyoneDisplayState previousState;
    char addressCurrentState;
    char addressPreviousState;
    char addressFormat;
} PolyoneDisplay;

void PolyoneDisplay_updateRtc(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_updateRtc(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_updateTimer(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_updateTimer(&polyoneDisplayPtr->timer);
}

PolyoneDisplay PolyoneDisplay_new(__EEADDRESS__ addressCurrentState,
        __EEADDRESS__ addressPreviousState, __EEADDRESS__ addressFormat,
        __EEADDRESS__ addressFirstNumberAlarm,
        __EEADDRESS__ addressSecondNumberAlarm, __EEADDRESS__ addressRtcHours,
        __EEADDRESS__ addressRtcMinutes, __EEADDRESS__ addressRtcSeconds) {
    PolyoneDisplay polyoneDisplay;

    polyoneDisplay.addressCurrentState = addressCurrentState;
    polyoneDisplay.addressPreviousState = addressPreviousState;
    polyoneDisplay.addressFormat = addressFormat;

    polyoneDisplay.format = read_eeprom(addressFormat) % 2;
    polyoneDisplay.currentState = read_eeprom(addressCurrentState) % 3;
    polyoneDisplay.previousState = read_eeprom(addressPreviousState) % 3;

    if ((polyoneDisplay.previousState == STATE_IDLE)
            && (polyoneDisplay.currentState == STATE_IDLE)) {
        polyoneDisplay.previousState = STATE_COUNTING_DOWN;
    }

    switch (polyoneDisplay.format) {
        case FORMAT_HOURS_MINUTES:
            polyoneDisplay.timer = Timer_newHoursMinutes(
                    FIRST_NUMBER_UPPER_BOUND,
                    SECOND_NUMBER_UPPER_BOUND,
                    addressFirstNumberAlarm,
                    addressSecondNumberAlarm,
                    addressRtcHours,
                    addressRtcMinutes,
                    addressRtcSeconds
                    );
            break;
        case FORMAT_MINUTES_SECONDS:
            polyoneDisplay.timer = Timer_newMinutesSeconds(
                    FIRST_NUMBER_UPPER_BOUND,
                    addressFirstNumberAlarm,
                    addressSecondNumberAlarm,
                    addressRtcHours,
                    addressRtcMinutes,
                    addressRtcSeconds
                    );

            break;
    }

    if (polyoneDisplay.currentState == STATE_IDLE) {
        PolyoneDisplay_updateTimer(&polyoneDisplay);
        PolyoneDisplay_updateRtc(&polyoneDisplay);
    } else {
        PolyoneDisplay_updateTimer(&polyoneDisplay);
    }

    return polyoneDisplay;
}

void PolyoneDisplay_showCount(PolyoneDisplay *polyoneDisplayPtr, BOOLEAN withBlink) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_showHoursAndMinutesOfCountdownTime(&polyoneDisplayPtr->timer, withBlink);

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {

        Timer_showMinutesAndSecondsOfCountdownTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_showLimitTime(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_showHoursAndMinutesOfLimitTime(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_showMinutesAndSecondsOfLimitTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_showFirstNumber(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_hideMinutesAndShowHoursOfLimitTime(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_hideSecondsAndShowMinutesOfLimitTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_showSecondNumber(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_hideHoursAndShowMinutesOfLimitTime(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_hideMinutesAndShowSecondsOfLimitTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_showFormat(PolyoneDisplay *polyoneDisplayPtr) {
    int numbersToSend[4] = {0};

    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {

        numbersToSend[0] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_N];
        numbersToSend[1] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_N] |
                SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
        numbersToSend[2] = SevenSegmentDisplay_characters[INDEX_CAPS_H];
        numbersToSend[3] = SevenSegmentDisplay_characters[INDEX_CAPS_H];

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        numbersToSend[0] = SevenSegmentDisplay_characters[5];
        numbersToSend[1] = SevenSegmentDisplay_characters[5] |
                SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
        numbersToSend[2] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_N];
        numbersToSend[3] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_N];
    }

    ShiftRegister_sendData(numbersToSend, getArraySize(numbersToSend));
}

void PolyoneDisplay_increaseFirstNumber(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_increaseTimerHours(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_increaseTimerMinutes(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_increaseSecondNumber(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_increaseTimerMinutes(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_increaseTimerSeconds(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_swapFormat(PolyoneDisplay *polyoneDisplayPtr) {
    polyoneDisplayPtr->format = !polyoneDisplayPtr->format;

    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        polyoneDisplayPtr->timer.limitTime.hour = polyoneDisplayPtr->timer.limitTime.minute;
        polyoneDisplayPtr->timer.limitTime.minute = polyoneDisplayPtr->timer.limitTime.second;
        polyoneDisplayPtr->timer.limitTime.second = 0;
        polyoneDisplayPtr->timer.hoursUpperBound = FIRST_NUMBER_UPPER_BOUND;
        polyoneDisplayPtr->timer.minutesUpperBound = SECOND_NUMBER_UPPER_BOUND;

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        polyoneDisplayPtr->timer.limitTime.second = polyoneDisplayPtr->timer.limitTime.minute;
        polyoneDisplayPtr->timer.limitTime.minute = polyoneDisplayPtr->timer.limitTime.hour;
        polyoneDisplayPtr->timer.limitTime.hour = 0;
        polyoneDisplayPtr->timer.hoursUpperBound = 0;
        polyoneDisplayPtr->timer.minutesUpperBound = FIRST_NUMBER_UPPER_BOUND;

    }
}

BOOLEAN PolyoneDisplay_isAlarmOkay(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        return (polyoneDisplayPtr->timer.limitTime.hour != 0) || (polyoneDisplayPtr->timer.limitTime.minute != 0);

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        return (polyoneDisplayPtr->timer.limitTime.minute != 0) || (polyoneDisplayPtr->timer.limitTime.second != 0);
    }
}

void PolyoneDisplay_saveRtcCurrentTime(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_saveRtcCurrentTime(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_setState(PolyoneDisplay *polyoneDisplayPtr, PolyoneDisplayState polyoneDisplayState) {

    polyoneDisplayPtr->previousState = polyoneDisplayPtr->currentState;
    polyoneDisplayPtr->currentState = polyoneDisplayState;
}

void PolyoneDisplay_stop(PolyoneDisplay *polyoneDisplayPtr) {

    polyoneDisplayPtr->previousState = polyoneDisplayPtr->currentState;
    polyoneDisplayPtr->currentState = STATE_IDLE;
}

void PolyoneDisplay_saveState(PolyoneDisplay *polyoneDisplayPtr) {

    write_eeprom(polyoneDisplayPtr->addressCurrentState, polyoneDisplayPtr->currentState);
    write_eeprom(polyoneDisplayPtr->addressPreviousState, polyoneDisplayPtr->previousState);
}

void PolyoneDisplay_resume(PolyoneDisplay *polyoneDisplayPtr) {

    PolyoneDisplayState currentState;
    currentState = polyoneDisplayPtr->currentState;
    polyoneDisplayPtr->currentState = polyoneDisplayPtr->previousState;
    polyoneDisplayPtr->previousState = currentState;
}

BOOLEAN PolyoneDisplay_isTimerDone(PolyoneDisplay *polyoneDisplayPtr) {
    return Timer_isTimerFinished(&polyoneDisplayPtr->timer);
}
#endif	/* POLYONEDISPLAY_H */

