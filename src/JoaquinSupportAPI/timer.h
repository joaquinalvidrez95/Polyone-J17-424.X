/* 
 * File:   timer.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on 29 de septiembre de 2017, 04:59 PM
 */

#ifndef TIMER_H
#define	TIMER_H

#include "tiempo.h"
#include "sevensegmentdisplay.h"

typedef struct {
    Time countdownTime;
    Time currentTime;
    Time limitTime;
    Time secondaryLimitTime;
    int allowedMaximumHours;
    int allowedMaximumMinutes;
    int allowedMaximumSeconds;
    __EEADDRESS__ isCounting;
    __EEADDRESS__ alarmSeconds;
    __EEADDRESS__ alarmMinutes;
    __EEADDRESS__ alarmHours;
    __EEADDRESS__ rtcHours;
    __EEADDRESS__ rtcMinutes;
    __EEADDRESS__ rtcSeconds;
} Timer;

Timer createTimer(Time *limitTimePtr, Time *secondaryLimitTimePtr,
        int maximumMinutes, int maximumSeconds) {
    Timer timer;
    timer.limitTime = *limitTimePtr;
    timer.secondaryLimitTime = *secondaryLimitTimePtr;
    timer.allowedMaximumMinutes = maximumMinutes;
    timer.allowedMaximumSeconds = maximumSeconds;

    return timer;
}

Timer Timer_new(int maximumHours, int maximumMinutes, __EEADDRESS__ alarmHourAddress,
        __EEADDRESS__ alarmMinutesAddress, __EEADDRESS__ isCounting,
        __EEADDRESS__ rtcHour, __EEADDRESS__ rtcMinutes, __EEADDRESS__ rtcSeconds) {
    Timer newTimer;
 
    newTimer.limitTime.hour = read_eeprom(alarmHourAddress) % (maximumHours + 1);
    newTimer.limitTime.minute = read_eeprom(alarmMinutesAddress) % (maximumMinutes + 1);
    newTimer.limitTime.second = 0;

    newTimer.allowedMaximumHours = maximumHours;
    newTimer.allowedMaximumMinutes = maximumMinutes;
    newTimer.allowedMaximumSeconds = 59;

    newTimer.isCounting = isCounting;
    newTimer.alarmHours = alarmHourAddress;
    newTimer.alarmMinutes = alarmMinutesAddress;
    newTimer.rtcHours = rtcHour;
    newTimer.rtcMinutes = rtcMinutes;
    newTimer.rtcSeconds = rtcSeconds;

    return newTimer;
}

void Timer_updateRtc(Timer *timerPtr) {
    setClockTime(&timerPtr->currentTime);
}

static void Timer_updateCountdownTime(Timer *timerPtr) {

    signed int32 currentTimeInSeconds;
    signed int32 limitTimeInSeconds;
    signed int32 countdownTimeInSeconds;
    currentTimeInSeconds = Time_changeTimeToSeconds(&timerPtr->currentTime);
    limitTimeInSeconds = Time_changeTimeToSeconds(&timerPtr->limitTime);
    countdownTimeInSeconds = limitTimeInSeconds - currentTimeInSeconds;

    countdownTimeInSeconds = countdownTimeInSeconds < 0 ? 0 : countdownTimeInSeconds;

    int32 hour = (countdownTimeInSeconds / 3600);
    int32 minute = (countdownTimeInSeconds % 3600) / 60;
    timerPtr->countdownTime.second = (countdownTimeInSeconds % 3600) % 60;

    //    if ((minute + 60) <= 99 && (hour > 0)) {
    //        hour -= 1;
    //        minute += 60;
    //    }
    timerPtr->countdownTime.hour = hour;
    timerPtr->countdownTime.minute = minute;

}

void Timer_updateTimerFromEeprom(Timer *timerPtr) {

    timerPtr->currentTime.hour =
            read_eeprom(timerPtr->rtcHours) % (timerPtr->allowedMaximumHours + 1);
    timerPtr->currentTime.minute =
            read_eeprom(timerPtr->rtcMinutes) % (timerPtr->allowedMaximumMinutes + 1);
    timerPtr->currentTime.second =
            read_eeprom(timerPtr->rtcSeconds) % (timerPtr->allowedMaximumSeconds + 1);
    Timer_updateCountdownTime(timerPtr);
}

BOOLEAN Timer_wasCounting(Timer *timerPtr) {
    return (read_eeprom(timerPtr->isCounting) % 2);
}

void Timer_updateTimer(Timer *timerPtr) {
    timerPtr->currentTime = Time_getCurrentTime();
    Timer_updateCountdownTime(timerPtr);
}

BOOLEAN Timer_hasTimerChanged(Timer *timerPtr) {
    Time currentTime;
    currentTime = Time_getCurrentTime();

    return (timerPtr->currentTime.second != currentTime.second);
}

void Timer_increaseTimerHours(Timer *timerPtr) {
    timerPtr->limitTime.hour =
            timerPtr->limitTime.hour == timerPtr->allowedMaximumHours ?
            0 : timerPtr->limitTime.hour + 1;
}

void Timer_decreaseTimerHours(Timer *timerPtr) {
    timerPtr->limitTime.hour =
            timerPtr->limitTime.hour == 0 ?
            timerPtr->allowedMaximumHours : timerPtr->limitTime.hour - 1;
}

void Timer_increaseTimerMinutes(Timer *timerPtr) {
    timerPtr->limitTime.minute =
            timerPtr->limitTime.minute == timerPtr->allowedMaximumMinutes ?
            0 : timerPtr->limitTime.minute + 1;
}

void Timer_decreaseTimerMinutes(Timer *timerPtr) {
    timerPtr->limitTime.minute =
            timerPtr->limitTime.minute == 0 ?
            timerPtr->allowedMaximumMinutes : timerPtr->limitTime.minute - 1;
}

void Timer_increaseTimerSeconds(Timer *timerPtr) {
    timerPtr->limitTime.second =
            timerPtr->limitTime.second == timerPtr->allowedMaximumSeconds ?
            0 : timerPtr->limitTime.second + 1;
}

void Timer_decreaseTimerSeconds(Timer *timerPtr) {
    timerPtr->limitTime.second =
            timerPtr->limitTime.second == 0 ?
            timerPtr->allowedMaximumSeconds : timerPtr->limitTime.second - 1;
}

void Timer_saveStateCounting(Timer *timerPtr, BOOLEAN isCounting) {
    write_eeprom(timerPtr->isCounting, isCounting);
}

void Timer_saveRtcCurrentTime(Timer *timerPtr) {
    write_eeprom(timerPtr->rtcHours, timerPtr->currentTime.hour);
    write_eeprom(timerPtr->rtcMinutes, timerPtr->currentTime.minute);
    write_eeprom(timerPtr->rtcSeconds, timerPtr->currentTime.second);
}

void Timer_saveLimitTime(Timer *timerPtr) {
    write_eeprom(timerPtr->alarmHours, timerPtr->limitTime.hour);
    write_eeprom(timerPtr->alarmMinutes, timerPtr->limitTime.minute);
}

BOOLEAN Timer_isTimerFinished(Timer *timerPtr) {
    return Time_isTimeZero(&timerPtr->countdownTime);
}

BOOLEAN Timer_isLimitTimeZero(Timer *timerPtr) {
    return (timerPtr->limitTime.hour == 0)&& (timerPtr->limitTime.minute == 0);
}

void Timer_showHoursAndMinutesOfCountdownTime(Timer *timerPtr, BOOLEAN blink) {
    int timeToSend[4] = {0};
    TimeInDigits timeInDigits;

    timeInDigits = Time_getTimeInDigits(&timerPtr->countdownTime, FALSE);
    timeToSend[0] = timeInDigits.minute[0];
    timeToSend[1] = timeInDigits.minute[1];
    timeToSend[2] = timeInDigits.hour[0];
    timeToSend[3] = timeInDigits.hour[1];

    if (blink) {
        if ((timerPtr->currentTime.second % 2)) {
            blink = TRUE;
        }
    } else {
        blink = TRUE;
    }

    SevenSegmentDisplay_showArrayOfNumbers(
            timeToSend,
            getArraySize(timeToSend),
            blink
            );
}

void Timer_showMinutesAndSecondsOfCountdownTime(Timer *timerPtr) {
    int timeToSend[4] = {0};
    TimeInDigits timeInDigits;

    timeInDigits = Time_getTimeInDigits(&timerPtr->countdownTime, FALSE);
    timeToSend[0] = timeInDigits.second[0];
    timeToSend[1] = timeInDigits.second[1];
    timeToSend[2] = timeInDigits.minute[0];
    timeToSend[3] = timeInDigits.minute[1];  

    SevenSegmentDisplay_showArrayOfNumbers(
            timeToSend,
            getArraySize(timeToSend),
            TRUE
            );
}

//void Timer_hideHoursAndShowMinutesOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    TimeInDigits displayableTime;
//
//    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime);
//
//    timeToSend[0] = displayableTime.minute[0];
//    timeToSend[1] = displayableTime.minute[1];
//    timeToSend[1] |= 0b00001000;
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}
//
//void Timer_showHoursAndMinutesOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    TimeInDigits displayableTime;
//
//    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime);
//
//    timeToSend[0] = displayableTime.minute[0];
//    timeToSend[1] = displayableTime.minute[1];
//    timeToSend[1] |= 0b00001000;
//    timeToSend[2] = displayableTime.hour[0];
//    timeToSend[3] = displayableTime.hour[1];
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}
//
//void Timer_hideMinutesAndShowHoursOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    TimeInDigits displayableTime;
//
//    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime);
//
//    timeToSend[1] |= 0b00001000;
//    timeToSend[2] = displayableTime.hour[0];
//    timeToSend[3] = displayableTime.hour[1];
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}

#endif	/* TIMER_H */

