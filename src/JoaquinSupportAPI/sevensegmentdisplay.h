/* 
 * File:   sevensegmentdisplay.h
 * Author: DISENO4
 *
 * Created on 2 de octubre de 2017, 09:12 AM
 */

#ifndef SEVENSEGMENTDISPLAY_H
#define   SEVENSEGMENTDISPLAY_H

#include "array.h"
#include "shiftregister.h"

typedef enum {
    INDEX_SEVEN_SEGMENT_HYPHEN = 10,
    INDEX_SEVEN_SEGMENT_DOT,
    INDEX_CAPS_A,
    INDEX_CAPS_C,
    INDEX_CAPS_E,
    INDEX_CAPS_F,
    INDEX_CAPS_H,
    INDEX_LOWERCASE_B,
    INDEX_LOWERCASE_C,
    INDEX_LOWERCASE_D,
    INDEX_LOWERCASE_N,
    INDEX_LOWERCASE_U,
} SEVEN_SEGMENT_DISPLAY_INDEX;

const int SevenSegmentDisplay_characters[] = {
    0b11110110, // 0   
    0b01100000, // 1
    0b11010101, // 2
    0b11110001, // 3
    0b01100011, // 4
    0b10110011, // 5
    0b10110111, // 6
    0b11100000, // 7
    0b11110111, // 8
    0b11110011, // 9
    0b00000001, // -
    0b00001000, // .   
    0b11100111, // A 
    0b10010110, // C
    0b10010111, // E
    0b10000111, // F
    0b01100111, //H    
    0b00110111, //b    
    0b10010110, //c    
    0b01110101, //d    
    0b00100101, //n    
    0b01110110, //u     
    0b00010111,
    0b00000100,
    0b01100100,
    0b11000011, //� 
    0b00110101,
};

void SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(int arrayOfNumbers[], int arraySize) {
    int i = 0;
    for (i = 0; i < arraySize; i++) {
        arrayOfNumbers[i] = SevenSegmentDisplay_characters[arrayOfNumbers[i]];
    }
}

//DisplayableTime getDisplayableTimeFromTime(Time *timePtr) {
//    DisplayableTime displayableTime;
//    
//    Array_splitNumberIntoDigits(timePtr->hour, displayableTime.hour,
//            getArraySize(displayableTime.hour));
//    changeNumbersIntoDisplayableNumbers(displayableTime.hour,
//            getArraySize(displayableTime.hour));
//
//    Array_splitNumberIntoDigits(timePtr->minute, displayableTime.minute,
//            getArraySize(displayableTime.hour));
//    changeNumbersIntoDisplayableNumbers(displayableTime.minute,
//            getArraySize(displayableTime.minute));
//
//    Array_splitNumberIntoDigits(timePtr->second, displayableTime.second,
//            getArraySize(displayableTime.hour));
//    changeNumbersIntoDisplayableNumbers(displayableTime.second,
//            getArraySize(displayableTime.second));
//
//    return displayableTime;
//}
//
//void SevenSegmentDisplay_showHoursAndMinutesOfCountdownTime(Timer *timerPtr, BOOLEAN blink) {
//    int timeToSend[4] = {0};
//    DisplayableTime displayableTime;
//
//    displayableTime = getDisplayableTimeFromTime(&timerPtr->countdownTime);
//
//    timeToSend[0] = displayableTime.minute[0];
//    timeToSend[1] = displayableTime.minute[1];
//    if (blink) {
//        if ((timerPtr->currentTime.second % 2)) {
//            timeToSend[1] |= 0b00001000;
//        }
//    } else {
//        timeToSend[1] |= 0b00001000;
//    }
//    timeToSend[2] = displayableTime.hour[0];
//    timeToSend[3] = displayableTime.hour[1];
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}
//
//void SevenSegmentDisplay_hideHoursAndShowMinutesOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    DisplayableTime displayableTime;
//
//    displayableTime = getDisplayableTimeFromTime(&timerPtr->limitTime);
//
//    timeToSend[0] = displayableTime.minute[0];
//    timeToSend[1] = displayableTime.minute[1];
//    timeToSend[1] |= 0b00001000;
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}
//
//void SevenSegmentDisplay_showHoursAndMinutesOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    DisplayableTime displayableTime;
//
//    displayableTime = getDisplayableTimeFromTime(&timerPtr->limitTime);
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
//void SevenSegmentDisplay_hideMinutesAndShowHoursOfLimitTime(Timer *timerPtr) {
//    int timeToSend[4] = {0};
//    DisplayableTime displayableTime;
//
//    displayableTime = getDisplayableTimeFromTime(&timerPtr->limitTime);
//
//    timeToSend[1] |= 0b00001000;
//    timeToSend[2] = displayableTime.hour[0];
//    timeToSend[3] = displayableTime.hour[1];
//
//    ShiftRegister_sendData(timeToSend, getArraySize(timeToSend));
//}

void SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(int arrayOfNumbers[], int arraySize) {
    int i = 0;
    for (i = 0; i < arraySize; i++) {
        arrayOfNumbers[i] = SevenSegmentDisplay_characters[arrayOfNumbers[i]];
        arrayOfNumbers[i] |= SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_DOT];
    }
}

void SevenSegmentDisplay_showArrayOfNumbers(int arrayOfNumbers[], int arraySize, BOOLEAN withDot) {
    if (withDot) {
        SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(arrayOfNumbers, arraySize);
    } else {
        SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(arrayOfNumbers, arraySize);
    }

    ShiftRegister_sendData(arrayOfNumbers, arraySize);
}

void SeventSegmentDisplay_showHyphens(void) {
    int arrayToSend[4] = {0};

    arrayToSend[0] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_HYPHEN];
    arrayToSend[1] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_HYPHEN] | 0b00001000;
    arrayToSend[2] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_HYPHEN];
    arrayToSend[3] = SevenSegmentDisplay_characters[INDEX_SEVEN_SEGMENT_HYPHEN];
    ShiftRegister_sendData(arrayToSend, getArraySize(arrayToSend));
}

void SevenSegmentDisplay_clearDisplay(void) {
    int charactersArray[4] = {0};
    ShiftRegister_sendData(charactersArray, getArraySize(charactersArray));
}

void SevenSegmentDisplay_clearDisplayTwoLines(void) {
    int charactersArray[8] = {0};

    ShiftRegister_sendData(charactersArray, getArraySize(charactersArray));
    ShiftRegister_sendData(charactersArray, getArraySize(charactersArray));
}

void SevenSegmentDisplay_showPin(int16 pin) {
    int arrayToSend[4] = {0};

    Array_splitNumberIntoDigits(pin, arrayToSend, getArraySize(arrayToSend));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(arrayToSend, getArraySize(arrayToSend));

    ShiftRegister_sendData(arrayToSend, getArraySize(arrayToSend));
}

void SevenSegmentDisplay_showPinTwoLines(int16 pin) {
    int pinDigits[4] = {0};
    int arrayToSend[8] = {0};

    Array_splitNumberIntoDigits(pin, pinDigits, getArraySize(pinDigits));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(pinDigits, getArraySize(pinDigits));

    arrayToSend[4] = pinDigits[0];
    arrayToSend[5] = pinDigits[1];
    arrayToSend[6] = pinDigits[2];
    arrayToSend[7] = pinDigits[3];
    ShiftRegister_sendData(arrayToSend, getArraySize(arrayToSend));
}
#endif   /* SEVENSEGMENTDISPLAY_H */

