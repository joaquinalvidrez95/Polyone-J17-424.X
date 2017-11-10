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
    FORMAT_HOURS_MINUTES = 0,
    FORMAT_MINUTES_SECONDS
} PolyoneDisplayFormat;

typedef enum {
    COUNTUP = 0,
    COUNTDOWN,
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
    STATE_SETTING_BRIGHTNESS,
    STATE_WAITING_FOR_BUTTON_BEING_RELEASED,
    NUMBER_OF_STATES
} PolyoneDisplayState;

typedef struct {
    Timer timer;
    PolyoneDisplayFormat format;
    TypeOfCount typeOfCount;
    PolyoneDisplayState currentState;
    PolyoneDisplayState previousState;
    int brightness;
    int numberOfDays;
    char addressCurrentState;
    char addressPreviousState;
    char addressFormat;
    char addressBrightness;
    char addressTypeOfCount;
    char addressNumberOfDays;
} PolyoneDisplay;

const int brightnessLevels[10] = {10, 34, 58, 82, 106, 130, 154, 178, 202, 255};

void PolyoneDisplay_updateRtc(PolyoneDisplay *polyoneDisplayPtr) {
    Time time;
    time = polyoneDisplayPtr->timer.currentTime;
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        time.hour %= 24;
        Time_setClockTime(&time);
        setDate(1, (time.hour / 4) + 1, 1, 1);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        time.hour = time.minute / 60;
        time.minute %= 60;
        Time_setClockTime(&time);
    }
}

void PolyoneDisplay_updateTimer(PolyoneDisplay *polyoneDisplayPtr) {
    polyoneDisplayPtr->timer.currentTime = Time_getCurrentTime();
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        int dayOfMonth;
        int month;
        int year;
        int dayOfWeek;
        DS3231_get_Date(dayOfMonth, month, year, dayOfWeek);
        polyoneDisplayPtr->timer.currentTime.hour += ((dayOfMonth - 1) % 5) * 24;
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        polyoneDisplayPtr->timer.currentTime.minute += (polyoneDisplayPtr->timer.currentTime.hour % 2) * 60;
    }

    Timer_updateCountdownTime(&polyoneDisplayPtr->timer);
}

PolyoneDisplay PolyoneDisplay_new(char addressCurrentState,
        char addressPreviousState, char addressFormat,
        char addressFirstNumberAlarm,
        char addressSecondNumberAlarm, char addressRtcHours,
        char addressRtcMinutes, char addressRtcSeconds,
        char addressBrightness, char addressTypeOfCount, __EEADDRESS__ addressNumberOfDays) {
    PolyoneDisplay polyoneDisplay;

    polyoneDisplay.addressCurrentState = addressCurrentState;
    polyoneDisplay.addressPreviousState = addressPreviousState;
    polyoneDisplay.addressFormat = addressFormat;
    polyoneDisplay.addressBrightness = addressBrightness;
    polyoneDisplay.addressTypeOfCount = addressTypeOfCount;
    polyoneDisplay.addressNumberOfDays = addressNumberOfDays;

    polyoneDisplay.format = read_eeprom(addressFormat) % 2;
    polyoneDisplay.currentState = read_eeprom(addressCurrentState) % 3;
    polyoneDisplay.previousState = read_eeprom(addressPreviousState) % 3;
    polyoneDisplay.brightness = read_eeprom(addressBrightness) % 10;
    polyoneDisplay.typeOfCount = read_eeprom(addressTypeOfCount) % 2;
    polyoneDisplay.numberOfDays = read_eeprom(addressNumberOfDays) % 5;

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
        Timer_updateTimerFromEeprom(&polyoneDisplay.timer);
        PolyoneDisplay_updateRtc(&polyoneDisplay);
        Timer_updateCountdownTime(&polyoneDisplay);
    } else {
        PolyoneDisplay_updateTimer(&polyoneDisplay);
    }

    return polyoneDisplay;
}

void PolyoneDisplay_showCount(PolyoneDisplay *polyoneDisplayPtr, BOOLEAN withBlink) {
    if (polyoneDisplayPtr->typeOfCount == COUNTUP) {
        if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
            Time_showHoursMinutesRtc(withBlink);
        } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
            Time_showMinutesSecondsRtc();
        }
    } else if (polyoneDisplayPtr->typeOfCount == COUNTDOWN) {
        if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
            Timer_showHoursAndMinutesOfCountdownTime(&polyoneDisplayPtr->timer, withBlink);
        } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
            Timer_showMinutesAndSecondsOfCountdownTime(&polyoneDisplayPtr->timer);
        }
    }
}

void PolyoneDisplay_showLimitTime(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_showHoursAndMinutesOfLimitTime(&polyoneDisplayPtr->timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_showMinutesAndSecondsOfLimitTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_hideBrightness(void) {
    int numbersToSend[4] = {0};

    numbersToSend[2] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_B];
    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
}

void PolyoneDisplay_showBrightness(PolyoneDisplay *polyoneDisplayPtr) {
    int numbersToSend[4] = {0};

    numbersToSend[0] = SevenSegmentDisplay_characters[(polyoneDisplayPtr->brightness + 1) % 10];
    numbersToSend[1] = SevenSegmentDisplay_characters[(polyoneDisplayPtr->brightness + 1) / 10] |
            SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    numbersToSend[2] = SevenSegmentDisplay_characters[INDEX_LOWERCASE_B];


    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
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

    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
}

void PolyoneDisplay_increaseBrightness(PolyoneDisplay *polyoneDisplayPtr) {
    polyoneDisplayPtr->brightness = (polyoneDisplayPtr->brightness + 1) % 10;
    set_pwm1_duty(brightnessLevels[polyoneDisplayPtr->brightness]);
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
        polyoneDisplayPtr->timer.alarmTime.hour = polyoneDisplayPtr->timer.alarmTime.minute;
        polyoneDisplayPtr->timer.alarmTime.minute = polyoneDisplayPtr->timer.alarmTime.second;
        polyoneDisplayPtr->timer.alarmTime.second = 0;
        polyoneDisplayPtr->timer.hoursUpperBound = FIRST_NUMBER_UPPER_BOUND;
        polyoneDisplayPtr->timer.minutesUpperBound = SECOND_NUMBER_UPPER_BOUND;

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        setDate(1, 1, 1, 1);
        polyoneDisplayPtr->timer.alarmTime.second = polyoneDisplayPtr->timer.alarmTime.minute;
        polyoneDisplayPtr->timer.alarmTime.minute = polyoneDisplayPtr->timer.alarmTime.hour;
        polyoneDisplayPtr->timer.alarmTime.hour = 0;
        polyoneDisplayPtr->timer.hoursUpperBound = 0;
        polyoneDisplayPtr->timer.minutesUpperBound = FIRST_NUMBER_UPPER_BOUND;

    }
}

BOOLEAN PolyoneDisplay_isAlarmOkay(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        return (polyoneDisplayPtr->timer.alarmTime.hour != 0) || (polyoneDisplayPtr->timer.alarmTime.minute != 0);

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        return (polyoneDisplayPtr->timer.alarmTime.minute != 0) || (polyoneDisplayPtr->timer.alarmTime.second != 0);
    }
}

void PolyoneDisplay_saveRtcCurrentTime(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_saveRtcCurrentTime(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_saveTypeOfCount(PolyoneDisplay *polyoneDisplayPtr) {
    write_eeprom(polyoneDisplayPtr->addressTypeOfCount, polyoneDisplayPtr->typeOfCount);
}

void PolyoneDisplay_saveBrightness(PolyoneDisplay *polyoneDisplayPtr) {
    write_eeprom(polyoneDisplayPtr->addressBrightness, polyoneDisplayPtr->brightness);
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

void PolyoneDisplay_saveFormat(PolyoneDisplay *polyoneDisplayPtr) {
    write_eeprom(polyoneDisplayPtr->addressFormat, polyoneDisplayPtr->format);
}

void PolyoneDisplay_saveAlarm(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_saveAlarmHoursMinutes(&polyoneDisplayPtr.timer);
    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_saveAlarmMinutesSeconds(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_resume(PolyoneDisplay *polyoneDisplayPtr) {
    polyoneDisplayPtr->previousState = polyoneDisplayPtr->currentState;
    if (polyoneDisplayPtr->typeOfCount == COUNTUP) {
        polyoneDisplayPtr->currentState = STATE_COUNTING_UP;
    } else if (polyoneDisplayPtr->typeOfCount == COUNTDOWN) {
        polyoneDisplayPtr->currentState = STATE_COUNTING_DOWN;
    }
}

BOOLEAN PolyoneDisplay_isTimerDone(PolyoneDisplay *polyoneDisplayPtr) {
    return Timer_isTimerFinished(&polyoneDisplayPtr->timer);
}
#endif	/* POLYONEDISPLAY_H */

