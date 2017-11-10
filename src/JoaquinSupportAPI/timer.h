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
    Time alarmTime;
    char addressIsCounting;
    char addressAlarmSeconds;
    char addressAlarmMinutes;
    char addressAlarmHours;
    char addressRtcHours;
    char addressRtcMinutes;
    char addressRtcSeconds;
    int hoursUpperBound;
    int minutesUpperBound;
    int secondsUpperBound;
} Timer;

void Timer_updateCountdownTime(Timer *timerPtr) {
    signed int32 currentTimeInSeconds;
    signed int32 limitTimeInSeconds;
    signed int32 countdownTimeInSeconds;
    currentTimeInSeconds = Time_changeTimeToSeconds(&timerPtr->currentTime);
    limitTimeInSeconds = Time_changeTimeToSeconds(&timerPtr->alarmTime);
    countdownTimeInSeconds = limitTimeInSeconds - currentTimeInSeconds;

    countdownTimeInSeconds = countdownTimeInSeconds < 0 ? 0 : countdownTimeInSeconds;

    int32 hour = (countdownTimeInSeconds / 3600);
    int32 minute = (countdownTimeInSeconds % 3600) / 60;
    timerPtr->countdownTime.second = (countdownTimeInSeconds % 3600) % 60;

    timerPtr->countdownTime.hour = hour;
    timerPtr->countdownTime.minute = minute;
}

Timer Timer_newHoursMinutes(int hoursUpperBound, int minutesUpperBound, __EEADDRESS__ addressAlarmHour,
        __EEADDRESS__ addressAlarmMinutes, __EEADDRESS__ addressRtcHour,
        __EEADDRESS__ addressRtcMinutes, __EEADDRESS__ addressRtcSeconds) {
    Timer newTimer;

    newTimer.addressAlarmHours = addressAlarmHour;
    newTimer.addressAlarmMinutes = addressAlarmMinutes;
    newTimer.addressRtcHours = addressRtcHour;
    newTimer.addressRtcMinutes = addressRtcMinutes;
    newTimer.addressRtcSeconds = addressRtcSeconds;

    newTimer.alarmTime.hour = read_eeprom(addressAlarmHour) % (hoursUpperBound + 1);
    newTimer.alarmTime.minute = read_eeprom(addressAlarmMinutes) % (minutesUpperBound + 1);
    newTimer.alarmTime.second = 0;

    newTimer.hoursUpperBound = hoursUpperBound;
    newTimer.minutesUpperBound = minutesUpperBound;
    newTimer.secondsUpperBound = 59;

//    newTimer.currentTime.hour = read_eeprom(addressRtcHour) % 100;
//    newTimer.currentTime.minute = read_eeprom(addressRtcMinutes) % (minutesUpperBound + 1);
//    newTimer.currentTime.second = read_eeprom(addressRtcSeconds) % 60;

    return newTimer;
}

Timer Timer_newMinutesSeconds(int minutesUpperBound,
        __EEADDRESS__ addressAlarmMinutes, __EEADDRESS__ addressAlarmSeconds,
        __EEADDRESS__ addressRtcHour, __EEADDRESS__ addressRtcMinutes, __EEADDRESS__ addressRtcSeconds) {
    Timer newTimer;

    newTimer.addressAlarmMinutes = addressAlarmMinutes;
    newTimer.addressAlarmSeconds = addressAlarmSeconds;
    newTimer.addressRtcHours = addressRtcHour;
    newTimer.addressRtcMinutes = addressRtcMinutes;
    newTimer.addressRtcSeconds = addressRtcSeconds;

    newTimer.hoursUpperBound = 0;
    newTimer.minutesUpperBound = minutesUpperBound;
    newTimer.secondsUpperBound = 59;

    newTimer.alarmTime.hour = 0;
    newTimer.alarmTime.minute = read_eeprom(addressAlarmMinutes) % (minutesUpperBound + 1);
    newTimer.alarmTime.second =
            read_eeprom(addressAlarmSeconds) % (newTimer.secondsUpperBound + 1);


//    newTimer.currentTime.hour = read_eeprom(addressRtcHour) % 2;
//    newTimer.currentTime.minute = read_eeprom(addressRtcMinutes) % (minutesUpperBound + 1);
//    newTimer.currentTime.minute = (newTimer.currentTime.minute + newTimer.currentTime.hour * 60) % 100;
//    newTimer.currentTime.second = read_eeprom(addressRtcSeconds) % 60;

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

void Timer_setRtc(Timer *timerPtr) {
    Time_setClockTime(&timerPtr->currentTime);
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
    timerPtr->alarmTime.hour =
            timerPtr->alarmTime.hour == timerPtr->hoursUpperBound ?
            0 : timerPtr->alarmTime.hour + 1;
}

void Timer_decreaseTimerHours(Timer *timerPtr) {
    timerPtr->alarmTime.hour =
            timerPtr->alarmTime.hour == 0 ?
            timerPtr->hoursUpperBound : timerPtr->alarmTime.hour - 1;
}

void Timer_increaseTimerMinutes(Timer *timerPtr) {
    timerPtr->alarmTime.minute =
            timerPtr->alarmTime.minute == timerPtr->minutesUpperBound ?
            0 : timerPtr->alarmTime.minute + 1;
}

void Timer_decreaseTimerMinutes(Timer *timerPtr) {
    timerPtr->alarmTime.minute =
            timerPtr->alarmTime.minute == 0 ?
            timerPtr->minutesUpperBound : timerPtr->alarmTime.minute - 1;
}

void Timer_increaseTimerSeconds(Timer *timerPtr) {
    timerPtr->alarmTime.second =
            timerPtr->alarmTime.second == timerPtr->secondsUpperBound ?
            0 : timerPtr->alarmTime.second + 1;
}

void Timer_decreaseTimerSeconds(Timer *timerPtr) {
    timerPtr->alarmTime.second =
            timerPtr->alarmTime.second == 0 ?
            timerPtr->secondsUpperBound : timerPtr->alarmTime.second - 1;
}

void Timer_saveStateCounting(Timer *timerPtr, BOOLEAN isCounting) {
    write_eeprom(timerPtr->addressIsCounting, isCounting);
}

void Timer_saveRtcCurrentTime(Timer *timerPtr) {
    write_eeprom(timerPtr->addressRtcHours, timerPtr->currentTime.hour);
    write_eeprom(timerPtr->addressRtcMinutes, timerPtr->currentTime.minute);
    write_eeprom(timerPtr->addressRtcSeconds, timerPtr->currentTime.second);
}

void Timer_saveAlarm(Timer *timerPtr) {
    write_eeprom(timerPtr->addressAlarmHours, timerPtr->alarmTime.hour);
    write_eeprom(timerPtr->addressAlarmMinutes, timerPtr->alarmTime.minute);
    write_eeprom(timerPtr->addressAlarmSeconds, timerPtr->alarmTime.second);
}
void Timer_saveAlarmHoursMinutes(Timer *timerPtr) {
    write_eeprom(timerPtr->addressAlarmHours, timerPtr->alarmTime.hour);
    write_eeprom(timerPtr->addressAlarmMinutes, timerPtr->alarmTime.minute);    
}

void Timer_saveAlarmMinutesSeconds(Timer *timerPtr) {  
    write_eeprom(timerPtr->addressAlarmMinutes, timerPtr->alarmTime.minute);
    write_eeprom(timerPtr->addressAlarmSeconds, timerPtr->alarmTime.second);
}

BOOLEAN Timer_isTimerFinished(Timer *timerPtr) {
    return Time_isTimeZero(&timerPtr->countdownTime);
}

BOOLEAN Timer_isLimitTimeZero(Timer *timerPtr) {
    return (timerPtr->alarmTime.hour == 0)&& (timerPtr->alarmTime.minute == 0);
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
            Array_getArraySize(timeToSend),
            blink
            );
}

void Timer_showMinutesAndSecondsOfCountdownTime(Timer *timerPtr) {
    int timeToSend[4] = {0};
    TimeInDigits timeInDigits;
    Time time;

    time = timerPtr->countdownTime;
    time.minute = (time.minute + time.hour * 60) % 100;
    timeInDigits = Time_getTimeInDigits(&time, FALSE);
    //    timeInDigits = Time_getTimeInDigits(&timerPtr->countdownTime, FALSE);
    timeToSend[0] = timeInDigits.second[0];
    timeToSend[1] = timeInDigits.second[1];
    timeToSend[2] = timeInDigits.minute[0];
    timeToSend[3] = timeInDigits.minute[1];

    SevenSegmentDisplay_showArrayOfNumbers(
            timeToSend,
            Array_getArraySize(timeToSend),
            TRUE
            );
}

void Timer_hideHoursAndShowMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[0] = SevenSegmentDisplay_characters[displayableTime.minute[0]];
    numbersToSend[1] = SevenSegmentDisplay_characters[displayableTime.minute[1]] |
            SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];

    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));

}

void Timer_hideMinutesAndShowSecondsOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[0] = SevenSegmentDisplay_characters[displayableTime.second[0]];
    numbersToSend[1] = SevenSegmentDisplay_characters[displayableTime.second[1]] |
            SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];

    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
}

void Timer_showHoursAndMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[0] = displayableTime.minute[0];
    numbersToSend[1] = displayableTime.minute[1];
    numbersToSend[2] = displayableTime.hour[0];
    numbersToSend[3] = displayableTime.hour[1];

    SevenSegmentDisplay_showArrayOfNumbers(
            numbersToSend,
            Array_getArraySize(numbersToSend),
            TRUE);
}

void Timer_showMinutesAndSecondsOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[0] = displayableTime.second[0];
    numbersToSend[1] = displayableTime.second[1];
    numbersToSend[2] = displayableTime.minute[0];
    numbersToSend[3] = displayableTime.minute[1];

    SevenSegmentDisplay_showArrayOfNumbers(
            numbersToSend,
            Array_getArraySize(numbersToSend),
            TRUE);
}

void Timer_hideMinutesAndShowHoursOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[1] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    numbersToSend[2] = SevenSegmentDisplay_characters[displayableTime.hour[0]];
    numbersToSend[3] = SevenSegmentDisplay_characters[displayableTime.hour[1]];

    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
}

void Timer_hideSecondsAndShowMinutesOfLimitTime(Timer *timerPtr) {
    int numbersToSend[4] = {0};
    TimeInDigits displayableTime;

    displayableTime = Time_getTimeInDigits(&timerPtr->alarmTime, FALSE);

    numbersToSend[1] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    numbersToSend[2] = SevenSegmentDisplay_characters[displayableTime.minute[0]];
    numbersToSend[3] = SevenSegmentDisplay_characters[displayableTime.minute[1]];


    ShiftRegister_sendData(numbersToSend, Array_getArraySize(numbersToSend));
}

#endif	/* TIMER_H */

