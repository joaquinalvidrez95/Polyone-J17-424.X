/* 
 * File:   time.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on 29 de septiembre de 2017, 03:54 PM
 */

#ifndef TIME_H
#define	TIME_H

#include "ds3231.h"
#include "array.h"

typedef enum {
    FORMAT_24 = 0,
    FORMAT_12
} Format;

typedef struct {
    int hour;
    int minute;
    int second;
    Format format;
    __EEADDRESS__ formatAddress;
} Time;

typedef struct {
    int hour[2];
    int minute[2];
    int second[2];
} TimeInDigits;

Time createTime(int hour, int minute, int second) {
    Time time;

    time.hour = hour;
    time.minute = minute;
    time.second = second;
    return time;
}

Time Time_getCurrentTime() {
    Time currentTime;
    int currentHour = 0;
    int currentMinute = 0;
    int currentSecond = 0;

    ds3231_get_Time(currentHour, currentMinute, currentSecond);
    currentTime = createTime(currentHour, currentMinute, currentSecond);

    return currentTime;
}

Time Time_new(__EEADDRESS__ formatAddress) {
    Time newTime;
    newTime = Time_getCurrentTime();
    newTime.formatAddress = formatAddress;
    newTime.format = read_eeprom(formatAddress) % 2;

    return newTime;
}

void Time_updateTime(Time *timePtr) {
    Time currentTime;
    currentTime = Time_getCurrentTime();
    timePtr->hour = currentTime.hour;
    timePtr->minute = currentTime.minute;
    timePtr->second = currentTime.second;
}

void Time_saveFormat(Time *timePtr) {
    write_eeprom(timePtr->formatAddress, timePtr->format);
}

void Time_changeTimeFormat(Time *timePtr) {
    if (timePtr->format == FORMAT_12) {
        timePtr->format = FORMAT_24;
    } else if (timePtr->format == FORMAT_24) {
        timePtr->format = FORMAT_12;
    }
}

TimeInDigits Time_getTimeInDigits(Time *timePtr, BOOLEAN formatted) {
    TimeInDigits newTimeInDigits;
    int hour = timePtr->hour;

    if (formatted) {
        if (timePtr->format == FORMAT_12) {
            if (timePtr->hour == 0) {
                hour = 12;
            } else if (timePtr->hour > 12) {
                hour = timePtr->hour - 12;
            }
        }
    }

    Array_splitNumberIntoDigits(
            hour,
            newTimeInDigits.hour,
            getArraySize(newTimeInDigits.hour)
            );
    Array_splitNumberIntoDigits(
            timePtr->minute,
            newTimeInDigits.minute,
            getArraySize(newTimeInDigits.minute)
            );

    Array_splitNumberIntoDigits(
            timePtr->second,
            newTimeInDigits.second,
            getArraySize(newTimeInDigits.second)
            );

    return newTimeInDigits;
}

BOOLEAN Time_isTimeEqual(Time *timePtr1, Time *timePtr2) {
    return (
            timePtr1->hour == timePtr2->hour)&&(
            timePtr1->minute == timePtr2->minute)&&(
            timePtr1->second == timePtr2->second);
}

BOOLEAN Time_isANewDay(Time *timePtr) {
    return (timePtr->hour == 0)&&(timePtr->minute == 0);
}

BOOLEAN Time_isTimeZero(Time *timePtr) {
    return (timePtr->hour == 0)&&(timePtr->minute == 0)&&(timePtr->second == 0);
}

Time getInitialTime() {
    return createTime(0, 0, 0);
}

Time getCountdownTime(Time *limitTimePtr, Time *currentTimePtr) {
    Time time;
    signed int16 hoursInSeconds;
    signed int16 minutesInSeconds;
    signed int16 seconds;

    hoursInSeconds = ((signed int16)limitTimePtr->hour
            - (signed int16)currentTimePtr->hour)*3600;
    minutesInSeconds = ((signed int16)limitTimePtr->minute
            - (signed int16)currentTimePtr->minute)*60;
    seconds = (signed int16)(limitTimePtr->second)
            -(signed int16)(currentTimePtr->second);

    seconds += hoursInSeconds + minutesInSeconds;
    seconds = seconds < 0 ? 0 : seconds;

    time.hour = seconds / 3600;
    time.minute = (seconds % 3600) / 60;
    time.second = (seconds % 3600) % 60;

    return time;
}

void Time_increaseHours(Time *timePtr, int maximumHours) {
    timePtr->hour = timePtr->hour == maximumHours ? 0 : timePtr->hour + 1;
}

void Time_increaseMinutes(Time *timePtr, int maximumMinutes) {
    timePtr->minute = timePtr->minute == maximumMinutes ? 0 : timePtr->minute + 1;
}

void Time_increaseSeconds(Time *timePtr, int maximumSeconds) {
    timePtr->second = timePtr->second == maximumSeconds ? 0 : timePtr->second + 1;
}

void Time_resetSeconds(Time *timePtr) {
    timePtr->second = 0;
}
////////

void setClockTime(Time *timePtr) {
    setTime(
            timePtr->hour,
            timePtr->minute,
            timePtr->second
            );
}

int32 Time_changeTimeToSeconds(Time *timePtr) {
    int32 hoursInSeconds;
    int32 minutesInSeconds;
    int32 seconds;

    hoursInSeconds = (int32) timePtr->hour * 3600;
    minutesInSeconds = (int32) timePtr->minute * 60;
    seconds = hoursInSeconds + minutesInSeconds + timePtr->second;

    return seconds;
}

void Time_clearRtcTime() {
    setTime(0, 0, 0);
}

void Time_saveRtcCurrentTime(int hourAddress,
        int minuteAddress, int secondAddress) {
    Time currentTime;

    currentTime = Time_getCurrentTime();
    write_eeprom(hourAddress, currentTime.hour);
    write_eeprom(minuteAddress, currentTime.minute);
    write_eeprom(secondAddress, currentTime.second);
}

void saveLimitTime(Time *limitTimePtr, int limitHourAddress,
        int limitMinuteAddress, int limitSecondAddress) {

    write_eeprom(limitHourAddress, limitTimePtr->hour);
    write_eeprom(limitMinuteAddress, limitTimePtr->minute);
    write_eeprom(limitSecondAddress, limitTimePtr->second);
}

int getYear(void) {
    int date, month, year, dow;
    DS3231_get_Date(date, month, year, dow);

    return year;
}

void Time_fixTimeToSummerDayLightSavingTime(Time *timePtr) {
    timePtr->hour = 3;
    setClockTime(timePtr);
}

void Time_fixTimeToWinterDayLightSavingTime(Time *timePtr) {
    timePtr->hour = 1;
    setClockTime(timePtr);
}

BOOLEAN Time_isTimeToStartDayLightSavingTime(Time *timePtr) {
    return (timePtr->hour == 2)&&
            (timePtr->minute == 0)&&
            (timePtr->second == 0);
}

//TimeInDigits Time_getTimeInDigits(Time *timePtr) {
//    TimeInDigits displayableTime;
//    
//    Array_splitNumberIntoDigits(timePtr->hour, displayableTime.hour,
//            getArraySize(displayableTime.hour));
//    
//    Array_splitNumberIntoDigits(timePtr->minute, displayableTime.minute,
//            getArraySize(displayableTime.hour));
//  
//    Array_splitNumberIntoDigits(timePtr->second, displayableTime.second,
//            getArraySize(displayableTime.hour));
//  
//    return displayableTime;
//}


#endif	/* TIME_H */

