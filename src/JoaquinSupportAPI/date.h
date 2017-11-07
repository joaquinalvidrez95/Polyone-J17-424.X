#ifndef DATE_H
#define   DATE_H

#include "ds3231.h"
#include "tiempo.h"
#include "matrixdotdisplay.h"

typedef enum {
    DAY_SUNDAY = 1,
    DAY_MONDAY,
    DAY_TUESDAY,
    DAY_WEDNESDAY,
    DAY_THURSDAY,
    DAY_FRIDAY,
    DAY_SATURDAY,
} Days;

typedef enum {
    MONTH_JANUARY = 1,
    MONTH_FEBRUARY,
    MONTH_MARCH,
    MONTH_APRIL,
    MONTH_MAY,
    MONTH_JUNE,
    MONTH_JULY,
    MONTH_AUGUST,
    MONTH_SEPTEMBER,
    MONTH_OCTOBER,
    MONTH_NOVEMBER,
    MONTH_DECEMBER
} Months;

typedef struct {
    int dayOfMonth;
    int year;
    int dayOfWeek;
    int month;
} Date;

typedef struct {
    int dayOfMonth[2];
    int year[2];
    int month[3];
} DateInDigitsAlphaMonth;

const int daysOfEachMonth[12] = {
    31,
    28,
    31,
    30,
    31,
    30,
    31,
    31,
    30,
    31,
    30,
    31
};

const int ALPHA_MONTH[12][3] = {
    {MATRIXDOTDISPLAY_LETTER_E, MATRIXDOTDISPLAY_LETTER_N, MATRIXDOTDISPLAY_LETTER_E},
    {MATRIXDOTDISPLAY_LETTER_F, MATRIXDOTDISPLAY_LETTER_E, MATRIXDOTDISPLAY_LETTER_B},
    {MATRIXDOTDISPLAY_LETTER_M, MATRIXDOTDISPLAY_LETTER_A, MATRIXDOTDISPLAY_LETTER_R},
    {MATRIXDOTDISPLAY_LETTER_A, MATRIXDOTDISPLAY_LETTER_B, MATRIXDOTDISPLAY_LETTER_R},
    {MATRIXDOTDISPLAY_LETTER_M, MATRIXDOTDISPLAY_LETTER_A, MATRIXDOTDISPLAY_LETTER_Y},
    {MATRIXDOTDISPLAY_LETTER_J, MATRIXDOTDISPLAY_LETTER_U, MATRIXDOTDISPLAY_LETTER_N},
    {MATRIXDOTDISPLAY_LETTER_J, MATRIXDOTDISPLAY_LETTER_U, MATRIXDOTDISPLAY_LETTER_L},
    {MATRIXDOTDISPLAY_LETTER_A, MATRIXDOTDISPLAY_LETTER_G, MATRIXDOTDISPLAY_LETTER_O},
    {MATRIXDOTDISPLAY_LETTER_S, MATRIXDOTDISPLAY_LETTER_E, MATRIXDOTDISPLAY_LETTER_P},
    {MATRIXDOTDISPLAY_LETTER_O, MATRIXDOTDISPLAY_LETTER_C, MATRIXDOTDISPLAY_LETTER_T},
    {MATRIXDOTDISPLAY_LETTER_N, MATRIXDOTDISPLAY_LETTER_O, MATRIXDOTDISPLAY_LETTER_V},
    {MATRIXDOTDISPLAY_LETTER_D, MATRIXDOTDISPLAY_LETTER_I, MATRIXDOTDISPLAY_LETTER_C}
};

DateInDigitsAlphaMonth Date_getDateInDigitsAlphaMonth(Date *datePtr) {
    DateInDigitsAlphaMonth newDate;
    Array_splitNumberIntoDigits(
            datePtr->dayOfMonth,
            newDate.dayOfMonth,
            getArraySize(newDate.dayOfMonth));
    Array_splitNumberIntoDigits(
            datePtr->year,
            newDate.year,
            getArraySize(newDate.year));
    newDate.month[2] = ALPHA_MONTH[datePtr->month - 1][0];
    newDate.month[1] = ALPHA_MONTH[datePtr->month - 1][1];
    newDate.month[0] = ALPHA_MONTH[datePtr->month - 1][2];

    return newDate;
}

BOOLEAN Date_isLeapYear(Date *datePtr) {
    return (datePtr->year % 4) == 0;
}

void Date_increaseYear(Date *datePtr) {
    datePtr->year = datePtr->year >= 99 ? 0 : datePtr->year + 1;
}

void Date_increaseMonth(Date *datePtr) {
    datePtr->month = datePtr->month >= 12 ? 1 : datePtr->month + 1;
}

void Date_increaseDayOfMonth(Date *datePtr) {
    if (datePtr->month == MONTH_FEBRUARY && Date_isLeapYear(datePtr)) {
        datePtr->dayOfMonth =
                (datePtr->dayOfMonth >= 29) ?
                1 : (datePtr->dayOfMonth + 1);
    } else {
        datePtr->dayOfMonth =
                (datePtr->dayOfMonth >= daysOfEachMonth[datePtr->month - 1]) ?
                1 : (datePtr->dayOfMonth + 1);
    }

}

Date Date_getCurrentDate() {
    int dayOfMonth;
    int month;
    int year;
    int dayOfWeek;
    Date currentDate;

    DS3231_get_Date(dayOfMonth, month, year, dayOfWeek);

    currentDate.dayOfMonth = dayOfMonth;
    currentDate.month = month;
    currentDate.year = year;
    currentDate.dayOfWeek = dayOfWeek;

    return currentDate;
}

void Date_updateRealDate(Date *datePtr) {
    setDate(
            datePtr->dayOfWeek,
            datePtr->dayOfMonth,
            datePtr->month,
            datePtr->year);
}

BOOLEAN Date_isSummerDayLightSavingTime(Date *datePtr, Time *timePtr) {
    return (datePtr->dayOfMonth <= 7) &&
            (datePtr->dayOfWeek == DAY_SUNDAY)&&
            (datePtr->month == MONTH_APRIL) &&
            Time_isTimeToStartDayLightSavingTime(timePtr);
}

BOOLEAN Date_isWinterDayLightSavingTime(Date *datePtr, Time *timePtr) {
    return (datePtr->dayOfMonth >= 25) &&
            (datePtr->dayOfWeek == DAY_SUNDAY)&&
            (datePtr->month == MONTH_OCTOBER) &&
            Time_isTimeToStartDayLightSavingTime(timePtr);
}

void Date_updateTimeIfDayLightSavingTime(Date *datePtr, Time *timePtr) {
    if (Date_isSummerDayLightSavingTime(datePtr, timePtr)) {
        Time_fixTimeToSummerDayLightSavingTime(timePtr);
    }
    if (Date_isWinterDayLightSavingTime(datePtr, timePtr)) {      
        Time_fixTimeToWinterDayLightSavingTime(timePtr);
    }
}

int Date_getDayOfWeek(int16 year, int16 month, int16 dayOfMonth) {
    signed int16 dayOfWeek;
    signed int16 c, y, m;
    signed int16 cc, yy;
    signed int16 signedDayOfMonth;

    signedDayOfMonth = dayOfMonth;
    cc = (signed int16)year / 100;
    yy = (signed int16)year - (((signed int16)year / 100)*100);

    c = (cc / 4) - 2 * cc - 1;
    //    c = (cc / 4) - 2 * cc;
    y = 5 * yy / 4;
    m = 26 * (month + 1) / 10;

    dayOfWeek = (c + y + m + signedDayOfMonth) % 7;

    return (int) dayOfWeek + 1;
}

void Date_fixDayOfWeek(Date *datePtr) {
    datePtr->dayOfWeek = Date_getDayOfWeek(
            datePtr->year,
            datePtr->month,
            datePtr->dayOfMonth);
}
#endif   
