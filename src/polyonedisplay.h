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

typedef enum {
    FORMAT_MINUTES_SECONDS,
    FORMAT_HOURS_MINUTES
} PolyoneDisplayFormat;

typedef enum {
    COUNTDOWN,
    COUNTUP
} TypeOfCount;

typedef enum {
    STATE_INIT,
    STATE_IDLE,
    STATE_COUNTING_UP,
    STATE_COUNTING_DOWN,
    STATE_RESETTING,
    STATE_READY,
    STATE_OVERFLOWED,
    STATE_SETTING_ALARM_MINUTES,
    STATE_SETTING_ALARM_HOUR,
    STATE_WAITING_FOR_BUTTON_BEING_RELEASED
} PolyoneDisplayState;

typedef struct {
    Timer timer;
    PolyoneDisplayFormat format;
    BOOLEAN isCounting;
    TypeOfCount typeOfCount;
    PolyoneDisplayState currentState;
    PolyoneDisplayState previousState;
} PolyoneDisplay;

PolyoneDisplay PolyoneDisplay_new() {
    PolyoneDisplay polyoneDisplay;
    polyoneDisplay.currentState = STATE_COUNTING_DOWN;
    polyoneDisplay.timer = Timer_new(5, 12, 0, 1, 2, 3, 4, 5);
    return polyoneDisplay;
}

void PolyoneDisplay_updateTimer(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_updateTimer(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_showCount(PolyoneDisplay *polyoneDisplayPtr) {
    if (polyoneDisplayPtr->format == FORMAT_HOURS_MINUTES) {
        Timer_showHoursAndMinutesOfCountdownTime(&polyoneDisplayPtr->timer, TRUE);

    } else if (polyoneDisplayPtr->format == FORMAT_MINUTES_SECONDS) {
        Timer_showMinutesAndSecondsOfCountdownTime(&polyoneDisplayPtr->timer);
    }
}

void PolyoneDisplay_saveRtcCurrentTime(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_saveRtcCurrentTime(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_updateRtc(PolyoneDisplay *polyoneDisplayPtr) {
    Timer_updateRtc(&polyoneDisplayPtr->timer);
}

void PolyoneDisplay_saveStateCounting(PolyoneDisplay *polyoneDisplayPtr, BOOLEAN isCounting) {
    Timer_saveStateCounting(&polyoneDisplayPtr->timer, isCounting);
}

BOOLEAN PolyoneDisplay_isTimerDone(PolyoneDisplay *polyoneDisplayPtr) {
    return Timer_isTimerFinished(&polyoneDisplayPtr->timer);
}
#endif	/* POLYONEDISPLAY_H */

