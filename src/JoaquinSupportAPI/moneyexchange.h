/* 
 * File:   moneyexchange.h
 * Author: Joaquín Alvidrez
 *
 * Created on 5 de octubre de 2017, 10:44 AM
 */

#ifndef MONEYEXCHANGE_H
#define	MONEYEXCHANGE_H

#include "shiftregister.h"
#include "sevensegmentdisplay.h"
#include "array.h"
#include "ascii.h"

typedef enum {
    EFFECT_STATIC = 0,
    EFFECT_BLINKING,
    EFFECT_SWITCHING,
    NUMBER_OF_EFFECTS
} Effect;

typedef enum {
    TYPE_NONE = 0,
    ONE_LINE,
    TWO_LINES
} TypeOfDisplay;

typedef struct {
    TypeOfDisplay typeOfDisplay;
    int16 buy;
    int16 sell;
    BOOLEAN isBuyVisible;
    BOOLEAN isSellVisible;
    int speed;
    int brightness;
    BOOLEAN isEnabled;
    Effect currentEffect;
    __EEADDRESS__ buyAddress;
    __EEADDRESS__ sellAddress;
    __EEADDRESS__ isEnabledAddress;
    __EEADDRESS__ isBuyVisibleAddress;
    __EEADDRESS__ isSellVisibleAddress;
    __EEADDRESS__ speedAddress;
    __EEADDRESS__ effectAddress;
    __EEADDRESS__ brightnessAddress;
} MoneyExchange;

typedef struct {
    int buyDigits[4];
    int sellDigits[4];
} MoneyExchangeInDigits;

typedef enum {
    MESSAGE_POWER = 'A',
    MESSAGE_EXCHANGE,
    MESSAGE_ONE_LINE_BUY_VISIBILITY,
    MESSAGE_ONE_LINE_SELL_VISIBILITY,
    MESSAGE_ONE_LINE_EFFECT,
    MESSAGE_ONE_LINE_SPEED,
    MESSAGE_ONE_LINE_BRIGHTNESS,
    MESSAGE_TWO_LINES_BUY_VISIBILITY,
    MESSAGE_TWO_LINES_SELL_VISIBILITY,
    MESSAGE_TWO_LINES_EFFECT,
    MESSAGE_TWO_LINES_SPEED,
    MESSAGE_TWO_LINES_BRIGHTNESS,
    MESSAGE_REQUEST_FOR_PARAMETERS,
    MESSAGE_RESPONSE_FOR_PARAMETERS
} Message;

typedef enum {
    STATE_INIT,
    STATE_IDLE,
    STATE_SHOWING,
} MoneyExchangeState;

typedef enum {
    PARAMETER_INDEX_DURATION = 1,
    PARAMETER_INDEX_SPEED,
    PARAMETER_INDEX_EFFECT,
} ParameterIndex;

#define LETTER_C_INDEX  3
#define LETTER_V_INDEX  2
#define LETTER_DECIMAL_POINT_INDEX  1

const int brightness[10] = {10, 34, 58, 82, 106, 130, 154, 178, 202, 255};

MoneyExchange MoneyExchange_newOneLine(TypeOfDisplay typeOfDisplay, __EEADDRESS__ buyAddress,
        __EEADDRESS__ sellAddress, __EEADDRESS__ isEnabledAddress, __EEADDRESS__ isBuyVisible, __EEADDRESS__ isSellVisible,
        __EEADDRESS__ speedAddress, __EEADDRESS__ effectAddress,
        __EEADDRESS__ brightnessAddress) {
    MoneyExchange newMoneyExchange;
    const int16 MAX_VALUE = 10000;

    newMoneyExchange.buyAddress = buyAddress;
    newMoneyExchange.sellAddress = sellAddress;
    newMoneyExchange.isEnabledAddress = isEnabledAddress;
    newMoneyExchange.speedAddress = speedAddress;
    newMoneyExchange.isBuyVisibleAddress = isBuyVisible;
    newMoneyExchange.isSellVisibleAddress = isSellVisible;
    newMoneyExchange.effectAddress = effectAddress;
    newMoneyExchange.brightnessAddress = brightnessAddress;

    newMoneyExchange.buy = read_int16_eeprom(buyAddress) % MAX_VALUE;
    newMoneyExchange.sell = read_int16_eeprom(sellAddress) % MAX_VALUE;
    newMoneyExchange.speed = read_eeprom(speedAddress) % 11;
    newMoneyExchange.isEnabled = read_eeprom(isEnabledAddress) % 2;
    newMoneyExchange.isBuyVisible = read_eeprom(isBuyVisible) % 2;
    newMoneyExchange.isSellVisible = read_eeprom(isSellVisible) % 2;
    newMoneyExchange.currentEffect = read_eeprom(effectAddress) % NUMBER_OF_EFFECTS;
    if (newMoneyExchange.isBuyVisible && newMoneyExchange.isSellVisible) {
        newMoneyExchange.currentEffect = EFFECT_SWITCHING;
    }
    newMoneyExchange.brightness = read_eeprom(brightnessAddress) % 10;

    set_pwm1_duty(brightness[newMoneyExchange.brightness]);
    newMoneyExchange.typeOfDisplay = typeOfDisplay;
    return newMoneyExchange;
}

MoneyExchange MoneyExchange_new(TypeOfDisplay typeOfDisplay, __EEADDRESS__ buyAddress,
        __EEADDRESS__ sellAddress, __EEADDRESS__ isEnabledAddress, __EEADDRESS__ isBuyVisible, __EEADDRESS__ isSellVisible,
        __EEADDRESS__ speedAddress, __EEADDRESS__ effectAddress,
        __EEADDRESS__ brightnessAddress) {
    MoneyExchange newMoneyExchange;
    const int16 MAX_VALUE = 10000;

    newMoneyExchange.buyAddress = buyAddress;
    newMoneyExchange.sellAddress = sellAddress;
    newMoneyExchange.isEnabledAddress = isEnabledAddress;
    newMoneyExchange.speedAddress = speedAddress;
    newMoneyExchange.isBuyVisibleAddress = isBuyVisible;
    newMoneyExchange.isSellVisibleAddress = isSellVisible;
    newMoneyExchange.effectAddress = effectAddress;
    newMoneyExchange.brightnessAddress = brightnessAddress;

    newMoneyExchange.buy = read_int16_eeprom(buyAddress) % MAX_VALUE;
    newMoneyExchange.sell = read_int16_eeprom(sellAddress) % MAX_VALUE;
    newMoneyExchange.speed = read_eeprom(speedAddress) % 11;
    newMoneyExchange.isEnabled = read_eeprom(isEnabledAddress) % 2;
    newMoneyExchange.isBuyVisible = read_eeprom(isBuyVisible) % 2;
    newMoneyExchange.isSellVisible = read_eeprom(isSellVisible) % 2;
    newMoneyExchange.currentEffect = read_eeprom(effectAddress) % NUMBER_OF_EFFECTS;
    newMoneyExchange.brightness = read_eeprom(brightnessAddress) % 10;

    set_pwm1_duty(brightness[newMoneyExchange.brightness]);
    newMoneyExchange.typeOfDisplay = typeOfDisplay;
    return newMoneyExchange;
}


//void MoneyExchange_saveDuration(MoneyExchange * moneyExchangePtr) {
//    write_eeprom(moneyExchangePtr->levelOfDurationAddress, moneyExchangePtr->levelOfDuration);
//}

MoneyExchangeInDigits MoneyExchange_getMoneyExchangeInDigits(MoneyExchange * moneyExchangePtr) {
    MoneyExchangeInDigits moneyExchangeInDigits;

    Array_splitNumberIntoDigits(
            moneyExchangePtr->buy,
            moneyExchangeInDigits.buyDigits,
            getArraySize(moneyExchangeInDigits.buyDigits));

    Array_splitNumberIntoDigits(
            moneyExchangePtr->sell,
            moneyExchangeInDigits.sellDigits,
            getArraySize(moneyExchangeInDigits.sellDigits));

    return moneyExchangeInDigits;
}

void MoneyExchange_saveEffect(MoneyExchange * moneyExchangePtr) {
    write_eeprom(moneyExchangePtr->effectAddress, moneyExchangePtr->currentEffect);
}

void MoneyExchange_saveSpeed(MoneyExchange * moneyExchangePtr) {
    write_eeprom(moneyExchangePtr->speedAddress, moneyExchangePtr->speed);
}

void MoneyExchange_saveBrightness(MoneyExchange * moneyExchangePtr) {
    write_eeprom(moneyExchangePtr->brightnessAddress, moneyExchangePtr->brightness);
}

//void MoneyExchange_updateLevelOfDuration(MoneyExchange *moneyExchangePtr, char string[]) {
//    moneyExchangePtr->levelOfDuration = (string[PARAMETER_INDEX_DURATION] - 48) + 1;
//}

void MoneyExchange_setSpeed(MoneyExchange *moneyExchangePtr, char string[]) {
    moneyExchangePtr->speed = ((string[1] - 48)) % 10;
}

void MoneyExchange_setBrightness(MoneyExchange *moneyExchangePtr, char string[]) {
    moneyExchangePtr->brightness = ((string[1] - 48)) % 10;

    set_pwm1_duty(brightness[moneyExchangePtr->brightness]);
}

void MoneyExchange_setEffect(MoneyExchange *moneyExchangePtr, char string[]) {
    moneyExchangePtr->currentEffect = (string[1] - 48) % NUMBER_OF_EFFECTS;
    MoneyExchange_saveEffect(moneyExchangePtr);
}

void MoneyExchange_updateEffect(MoneyExchange *moneyExchangePtr, char string[]) {
    moneyExchangePtr->currentEffect = string[PARAMETER_INDEX_EFFECT] - 48;
}

void MoneyExchange_updateSpeed(MoneyExchange *moneyExchangePtr, char string[]) {
    moneyExchangePtr->speed = (string[PARAMETER_INDEX_SPEED] - 48);
}

//void MoneyExchange_updateParameters(MoneyExchange *moneyExchangePtr, char string[]) {
//    MoneyExchange_updateLevelOfDuration(moneyExchangePtr, string);
//    MoneyExchange_updateEffect(moneyExchangePtr, string);
//    MoneyExchange_updateSpeed(moneyExchangePtr, string);
//    MoneyExchange_saveDuration(moneyExchangePtr);
//    MoneyExchange_saveEffect(moneyExchangePtr);
//    MoneyExchange_saveSpeed(moneyExchangePtr);
//}

void MoneyExchange_saveState(MoneyExchange *moneyExchangePtr, BOOLEAN isShowing) {
    write_eeprom(moneyExchangePtr->isEnabledAddress, isShowing);
}

void MoneyExchange_saveBuyVisibility(MoneyExchange *moneyExchangePtr) {
    write_eeprom(moneyExchangePtr->isBuyVisibleAddress, moneyExchangePtr->isBuyVisible);
}

void MoneyExchange_saveSellVisibility(MoneyExchange *moneyExchangePtr) {
    write_eeprom(moneyExchangePtr->isSellVisibleAddress, moneyExchangePtr->isSellVisible);
}

void MoneyExchange_setEnabled(MoneyExchange *moneyExchangePtr, BOOLEAN isEnabled) {
    moneyExchangePtr->isEnabled = isEnabled;
    MoneyExchange_saveState(moneyExchangePtr, isEnabled);
}

void MoneyExchange_setBuyVisible(MoneyExchange *moneyExchangePtr, BOOLEAN isVisible) {
    moneyExchangePtr->isBuyVisible = isVisible % 2;
}

void MoneyExchange_setSellVisible(MoneyExchange *moneyExchangePtr, BOOLEAN isVisible) {
    moneyExchangePtr->isSellVisible = isVisible % 2;
}

void MoneyExchange_swapVisibility(MoneyExchange *moneyExchangePtr) {
    moneyExchangePtr->isBuyVisible = ~moneyExchangePtr->isBuyVisible;
    moneyExchangePtr->isSellVisible = ~moneyExchangePtr->isSellVisible;
}

void MoneyExchange_setDisplayVisible(MoneyExchange *moneyExchangePtr,
        BOOLEAN isVisible) {
    MoneyExchange_setBuyVisible(moneyExchangePtr, isVisible);
    MoneyExchange_setSellVisible(moneyExchangePtr, isVisible);
}

BOOLEAN MoneyExchange_isEnabled(MoneyExchange * moneyExchangePtr) {
    return moneyExchangePtr->isEnabled;
}

void MoneyExchange_saveBuy(MoneyExchange * moneyExchangePtr) {
    write_int16_eeprom(moneyExchangePtr->buyAddress, moneyExchangePtr->buy);
}

void MoneyExchange_saveSell(MoneyExchange * moneyExchangePtr) {
    write_int16_eeprom(moneyExchangePtr->sellAddress, moneyExchangePtr->sell);
}

void MoneyExchange_showBuy(MoneyExchange * moneyExchangePtr) {
    int numberToSend[4] = {0};

    Array_splitNumberIntoDigits(
            moneyExchangePtr->buy,
            numberToSend,
            getArraySize(numberToSend));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(
            numberToSend,
            getArraySize(numberToSend));

    if (moneyExchangePtr->buy / 1000 == 0) {
        numberToSend[3] = 0;
    }
    numberToSend[LETTER_DECIMAL_POINT_INDEX] |= 0b00001000;
    numberToSend[LETTER_C_INDEX] |= 0b00001000; // C

    ShiftRegister_sendData(numberToSend, getArraySize(numberToSend));
}

void MoneyExchange_showSell(MoneyExchange * moneyExchangePtr) {
    int numberToSend[4] = {0};

    Array_splitNumberIntoDigits(
            moneyExchangePtr->sell,
            numberToSend,
            getArraySize(numberToSend));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbers(
            numberToSend,
            getArraySize(numberToSend));

    if (moneyExchangePtr->sell / 1000 == 0) {
        numberToSend[3] = 0;
    }

    numberToSend[LETTER_DECIMAL_POINT_INDEX] |= SevenSegmentDisplay_DOT;
    numberToSend[LETTER_V_INDEX] |= SevenSegmentDisplay_DOT; // V
    ShiftRegister_sendData(numberToSend, getArraySize(numberToSend));
}

void MoneyExchange_showMoneyExchange(MoneyExchange * moneyExchangePtr) {
    int numberToSend[8] = {0};
    int buyDigits[4] = {0};
    int sellDigits[4] = {0};

    Array_splitNumberIntoDigits(
            moneyExchangePtr->buy,
            buyDigits,
            getArraySize(buyDigits));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(
            buyDigits,
            getArraySize(buyDigits));
    Array_splitNumberIntoDigits(
            moneyExchangePtr->sell,
            sellDigits,
            getArraySize(sellDigits));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(
            sellDigits,
            getArraySize(sellDigits));

    if (moneyExchangePtr->buy / 1000 == 0) {
        buyDigits[3] = 0;
    }
    if (moneyExchangePtr->sell / 1000 == 0) {
        sellDigits[3] = 0;
    }

    if (moneyExchangePtr->isBuyVisible) {
        numberToSend[4] = buyDigits[0];
        numberToSend[5] = buyDigits[1];
        numberToSend[6] = buyDigits[2];
        numberToSend[7] = buyDigits[3];
    }
    if (moneyExchangePtr->isSellVisible) {
        numberToSend[0] = sellDigits[0];
        numberToSend[1] = sellDigits[1];
        numberToSend[2] = sellDigits[2];
        numberToSend[3] = sellDigits[3];
    }
    ShiftRegister_sendData(numberToSend, getArraySize(numberToSend));
}

void MoneyExchange_updateBuy(MoneyExchange *moneyExchangePtr, char string[]) {
    int16 number = (int16) (string[7] - 48)*1000 +
            (int16) (string[8] - 48)*100 +
            (int16) (string[9] - 48)*10 + (int16) (string[10] - 48);
    moneyExchangePtr->buy = number;
}

void MoneyExchange_updateSell(MoneyExchange *moneyExchangePtr, char string[]) {
    int16 number = (int16) (string[6] - 48)*1000 +
            (int16) (string[7] - 48)*100 +
            (int16) (string[8] - 48)*10 + (int16) (string[9] - 48);
    moneyExchangePtr->sell = number;
}

void MoneyExchange_setBuyAndSell(MoneyExchange *moneyExchangePtr, char string[]) {
    int16 buy = (int16) (string[1] - 48)*1000 +
            (int16) (string[2] - 48)*100 +
            (int16) (string[3] - 48)*10 + (int16) (string[4] - 48);
    moneyExchangePtr->buy = buy;

    int16 sell = (int16) (string[5] - 48)*1000 +
            (int16) (string[6] - 48)*100 +
            (int16) (string[7] - 48)*10 + (int16) (string[8] - 48);
    moneyExchangePtr->sell = sell;

    MoneyExchange_saveBuy(moneyExchangePtr);
    MoneyExchange_saveSell(moneyExchangePtr);
}

void MoneyExchange_hideBuy(MoneyExchange * moneyExchangePtr) {
    int numberToSend[8] = {0};
    int sellDigits[4] = {0};

    Array_splitNumberIntoDigits(
            moneyExchangePtr->sell,
            sellDigits,
            getArraySize(sellDigits));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(
            sellDigits,
            getArraySize(sellDigits));

    if (moneyExchangePtr->sell / 1000 == 0) {
        sellDigits[3] = 0;
    }

    numberToSend[0] = sellDigits[0];
    numberToSend[1] = sellDigits[1];
    numberToSend[2] = sellDigits[2];
    numberToSend[3] = sellDigits[3];
    ShiftRegister_sendData(numberToSend, getArraySize(numberToSend));
}

void MoneyExchange_hideSell(MoneyExchange * moneyExchangePtr) {
    int numberToSend[8] = {0};
    int buyDigits[4] = {0};

    Array_splitNumberIntoDigits(
            moneyExchangePtr->buy,
            buyDigits,
            getArraySize(buyDigits));
    SevenSegmentDisplay_changeNumbersIntoDisplayableNumbersWithDot(
            buyDigits,
            getArraySize(buyDigits));

    if (moneyExchangePtr->buy / 1000 == 0) {
        buyDigits[3] = 0;
    }

    numberToSend[4] = buyDigits[0];
    numberToSend[5] = buyDigits[1];
    numberToSend[6] = buyDigits[2];
    numberToSend[7] = buyDigits[3];
    ShiftRegister_sendData(numberToSend, getArraySize(numberToSend));
}

BOOLEAN MoneyExchange_isBuy(char string[]) {

    return (
            string[0] == 'C' &&
            string[1] == 'O' &&
            string[2] == 'M' &&
            string[3] == 'P' &&
            string[4] == 'R' &&
            string[5] == 'A' &&
            string[6] == '=' &&
            string[11] != 'V'
            );
}

BOOLEAN MoneyExchange_isSell(char string[]) {

    return (
            string[0] == 'V' &&
            string[1] == 'E' &&
            string[2] == 'N' &&
            string[3] == 'T' &&
            string[4] == 'A' &&
            string[5] == '='
            );
}

BOOLEAN MoneyExchange_isBuyAndSell(char string[]) {
    return (string[0] == 'C' &&
            string[1] == 'O' &&
            string[2] == 'M' &&
            string[3] == 'P' &&
            string[4] == 'R' &&
            string[5] == 'A' &&
            string[6] == '=' &&
            string[11] == 'V' &&
            string[12] == 'E' &&
            string[13] == 'N' &&
            string[14] == 'T' &&
            string[15] == 'A' &&
            string[16] == '=');
}

BOOLEAN MoneyExchange_isLevelOfDuration(char string[]) {
    return (string[0] == 'T' &&
            string[1] == 'I' &&
            string[2] == 'E' &&
            string[3] == 'M' &&
            string[4] == 'P' &&
            string[5] == 'O' &&
            string[6] == '=');
}

BOOLEAN MoneyExchange_isOn(char string[]) {
    return (string[0] == 'O' &&
            string[1] == 'N'
            );
}

BOOLEAN MoneyExchange_isOff(char string[]) {
    return (string[0] == 'O' &&
            string[1] == 'F' &&
            string[2] == 'F'
            );
}

void MoneyExchange_setArrayToSendParameters(MoneyExchange *moneyExchangePtr, char messageToSend[]) {
    MoneyExchangeInDigits moneyExchangeInDigits;

    moneyExchangeInDigits = MoneyExchange_getMoneyExchangeInDigits(moneyExchangePtr);
    messageToSend[0] = MESSAGE_RESPONSE_FOR_PARAMETERS;
    messageToSend[1] = moneyExchangePtr->isEnabled;

    messageToSend[2] = moneyExchangeInDigits.buyDigits[3];
    messageToSend[3] = moneyExchangeInDigits.buyDigits[2];
    messageToSend[4] = moneyExchangeInDigits.buyDigits[1];
    messageToSend[5] = moneyExchangeInDigits.buyDigits[0];

    messageToSend[6] = moneyExchangeInDigits.sellDigits[3];
    messageToSend[7] = moneyExchangeInDigits.sellDigits[2];
    messageToSend[8] = moneyExchangeInDigits.sellDigits[1];
    messageToSend[9] = moneyExchangeInDigits.sellDigits[0];

    messageToSend[10] = moneyExchangePtr->typeOfDisplay;
    messageToSend[11] = moneyExchangePtr->isBuyVisible;
    messageToSend[12] = moneyExchangePtr->isSellVisible;
    messageToSend[13] = moneyExchangePtr->currentEffect;
    messageToSend[14] = moneyExchangePtr->speed;
    messageToSend[15] = moneyExchangePtr->brightness;
    messageToSend[16] = "\0";

    ASCII_convertToAsciiArray(messageToSend, 16);

}
#endif	/* MONEYEXCHANGE_H */

