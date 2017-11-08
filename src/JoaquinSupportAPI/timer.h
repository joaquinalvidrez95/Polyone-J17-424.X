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
    int hoursUpperBound;
    int minutesUpperBound;
    int secondsUpperBound;
    char addressIsCounting;
    char addressAlarmSeconds;
    char addressAlarmMinutes;
    char addressAlarmHours;
    char addressRtcHours;
    char addressRtcMinutes;
    char addressRtcSeconds;
} Timer;

Timer Timer_newHoursMinutes(int maximumHours, int maximumMinutes, __EEADDRESS__ alarmHourAddress,
        __EEADDRESS__ alarmMinutesAddress, __EEADDRESS__ rtcHour,
        __EEADDRESS__ rtcMinutes, __EEADDRESS__ rtcSeconds) {
    Timer newTimer;

    newTimer.limitTime.hour = read_eeprom(alarmHourAddress) % (maximumHours + 1);
    newTimer.limitTime.minute = read_eeprom(alarmMinutesAddress) % (maximumMinutes + 1);
    newTimer.limitTime.second = 0;

    newTimer.hoursUpperBound = maximumHours;
    newTimer.minutesUpperBound = maximumMinutes;
    newTimer.secondsUpperBound = 59;

    newTimer.addressAlarmHours = alarmHourAddress;
    newTimer.addressAlarmMinutes = alarmMinutesAddress;
    newTimer.addressRtcHours = rtcHour;
    newTimer.addressRtcMinutes = rtcMinutes;
    newTimer.addressRtcSeconds = rtcSeconds;

    return newTimer;
}

Timer Timer_newMinutesSeconds(int maximumMinutes,
        __EEADDRESS__ alarmMinutesAddress, __EEADDRESS__ addressAlarmSeconds,
        __EEADDRESS__ rtcHour, __EEADDRESS__ rtcMinutes, __EEADDRESS__ rtcSeconds) {
    Timer newTimer;

    newTimer.hoursUpperBound = 0;
    newTimer.minutesUpperBound = maximumMinutes;
    newTimer.secondsUpperBound = 59;

    newTimer.limitTime.hour = 0;
    newTimer.limitTime.minute = read_eeprom(alarmMinutesAddress) % (maximumMinutes + 1);
    newTimer.limitTime.second =
            read_eeprom(addressAlarmSeconds) % (newTimer.secondsUpperBound + 1);

    newTimer.addressAlarmMinutes = alarmMinutesAddress;
    newTimer.addressAlarmSeconds = addressAlarmSeconds;
    newTimer.addressRtcHours = rtcHour;
    newTimer.addressRtcMinutes = rtcMinutes;
    newTimer.addressRtcSeconds = rtcSeconds;

    return newTimer;
}

Timer Timer_newHoursMinutes(int maximumHours, int maximumMinutes, __EEADDRESS__ alarmHourAddress,
        __EEADDRESS__ alarmMinutesAddress, __EEADDRESS__ isCounting,
        __EEADDRESS__ rtcHour, __EEADDRESS__ rtcMinutes, __EEADDRESS__ rtcSeconds) {
    Timer newTimer;

    newTimer = Timer_newHoursMinutes(
            maximumHours,
            maximumMinutes,
            alarmHourAddress,
            alarmMinutesAddress,
            rtcHour,
            rtcMinutes,
            rtcSeconds);

    newTimer.addressIsCounting = isCounting;
    return newTimer;
}

Timer Timer_newMinutesSeconds(int maximumMinutes,
        __EEADDRESS__ alarmMinutesAddress, __EEADDRESS__ addressAlarmSeconds,
        __EEADDRESS__ isCounting, __EEADDRESS__ rtcHour,
        __EEADDRESS__ rtcMinutes, __EEADDRESS__ rtcSeconds) {
    Timer newTimer;

    newTimer = Timer_newMinutesSeconds(
            maximumMinutes,
            alarmMinutesAddress,
            addressAlarmSeconds,
            rtcHour,
            rtcMinutes,
            rtcSeconds);

    newTimer.addressIsCounting = isCounting;

    return newTimer;
}

void Timer_updateRtc(Timer *timerPtr) {
    Time_setClockTime(&timerPtr->currentTime);
}

void Timer_updateCountdownTime(Timer *timerPtr) {

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
            read_eeprom(timerPtr->addressRtcHours) % (timerPtr->hoursUpperBound + 1);
    timerPtr->currentTime.minute =
            read_eeprom(timerPtr->addressRtcMinutes) % (timerPtr->minutesUpperBound + 1);
    timerPtr->currentTime.second =
            read_eeprom(timerPtr->addressRtcSeconds) % (timerPtr->secondsUpperBound + 1);
    Timer_updateCountdownTime(timerPtr);
}

BOOLEAN Timer_wasCounting(Timer *timerPtr) {
    return (read_eeprom(timerPtr->addressIsCounting) % 2);
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
            timerPtr->limitTime.hour == timerPtr->hoursUpperBound ?
            0 : timerPtr->limitTime.hour + 1;
}

void Timer_decreaseTimerHours(Timer *timerPtr) {
    timerPtr->limitTime.hour =
            timerPtr->limitTime.hour == 0 ?
            timerPtr->hoursUpperBound : timerPtr->limitTime.hour - 1;
}

void Timer_increaseTimerMinutes(Timer *timerPtr) {
    timerPtr->limitTime.minute =
            timerPtr->limitTime.minute == timerPtr->minutesUpperBound ?
            0 : timerPtr->limitTime.minute + 1;
}

void Timer_decreaseTimerMinutes(Timer *timerPtr) {
    timerPtr->limitTime.minute =
            timerPtr->limitTime.minute == 0 ?
            timerPtr->minutesUpperBound : timerPtr->limitTime.minute - 1;
}

void Timer_increaseTimerSeconds(Timer *timerPtr) {
    timerPtr->limitTime.second =
            timerPtr->limitTime.second == timerPtr->secondsUpperBound ?
            0 : timerPtr->limitTime.second + 1;
}

void Timer_decreaseTimerSeconds(Timer *timerPtr) {
    timerPtr->limitTime.second =
            timerPtr->limitTime.second == 0 ?
            timerPtr->secondsUpperBound : timerPtr->limitTime.second - 1;
}

void Timer_saveStateCounting(Timer *timerPtr, BOOLEAN isCounting) {
    write_eeprom(timerPtr->addressIsCounting, isCounting);
}

void Timer_saveRtcCurrentTime(Timer *timerPtr) {
    write_eeprom(timerPtr->addressRtcHours, timerPtr->currentTime.hour);
    write_eeprom(timerPtr->addressRtcMinutes, timerPtr->currentTime.minute);
    write_eeprom(timerPtr->addressRtcSeconds, timerPtr->currentTime.second);
}

void Timer_saveLimitTime(Timer *timerPtr) {
    write_eeprom(timerPtr->addressAlarmHours, timerPtr->limitTime.hour);
    write_eeprom(timerPtr->addressAlarmMinutes, timerPtr->limitTime.minute);
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
        blink = timerPtr->currentTime.second % 2;
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

void Timer_hideHoursAndShowMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[0] = SevenSegmentDisplay_characters[displayableTime.minute[0]];
    numbersToSend[1] = SevenSegmentDisplay_characters[displayableTime.minute[1]] |
            SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];

    ShiftRegister_sendData(numbersToSend, getArraySize(numbersToSend));

}

void Timer_hideMinutesAndShowSecondsOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[0] = SevenSegmentDisplay_characters[displayableTime.second[0]];
    numbersToSend[1] = SevenSegmentDisplay_characters[displayableTime.second[1]] |
            SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];

    ShiftRegister_sendData(numbersToSend, getArraySize(numbersToSend));
}

void Timer_showHoursAndMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[0] = displayableTime.minute[0];
    numbersToSend[1] = displayableTime.minute[1];
    numbersToSend[2] = displayableTime.hour[0];
    numbersToSend[3] = displayableTime.hour[1];

    SevenSegmentDisplay_showArrayOfNumbers(
            numbersToSend,
            getArraySize(numbersToSend),
            TRUE);
}

void Timer_showMinutesAndSecondsOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[0] = displayableTime.second[0];
    numbersToSend[1] = displayableTime.second[1];
    numbersToSend[2] = displayableTime.minute[0];
    numbersToSend[3] = displayableTime.minute[1];

    SevenSegmentDisplay_showArrayOfNumbers(
            numbersToSend,
            getArraySize(numbersToSend),
            TRUE);
}

void Timer_hideMinutesAndShowHoursOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[1] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    numbersToSend[2] = SevenSegmentDisplay_characters[displayableTime.hour[0]];
    numbersToSend[3] = SevenSegmentDisplay_characters[displayableTime.hour[1]];

    ShiftRegister_sendData(numbersToSend, getArraySize(numbersToSend));
}

void Timer_hideSecondsAndShowMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->limitTime, FALSE);

    numbersToSend[1] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    numbersToSend[2] = SevenSegmentDisplay_characters[displayableTime.minute[0]];
    numbersToSend[3] = SevenSegmentDisplay_characters[displayableTime.minute[1]];


    ShiftRegister_sendData(numbersToSend, getArraySize(numbersToSend));
}

#endif	/* TIMER_H */

