/* 
 * File:   temperature.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on 17 de octubre de 2017, 10:57 AM
 */

#ifndef TEMPERATURE_H
#define	TEMPERATURE_H

#include "array.h"

typedef struct {
    int temperature[2];
    BOOLEAN isNegative;
} TemperatureInDigits;

typedef enum {
    SIGN_NEUTRAL = 0,
    SIGN_PLUS,
    SIGN_NEGATIVE,
} SIGN;

typedef struct {
    signed int value;
    signed int offset;
    signed int minimumTemperature;
    __EEADDRESS__ offsetTempetatureAddress;
    __EEADDRESS__ isOffsetNegativeAddress;
} Temperature;

Temperature Temperature_new(__EEADDRESS__ offsetTempetatureAddress,
        __EEADDRESS__ isOffsetNegativeAddress, signed int minimumTemperature) {
    Temperature newTemperature;

    newTemperature.offsetTempetatureAddress = offsetTempetatureAddress;
    newTemperature.isOffsetNegativeAddress = isOffsetNegativeAddress;

    newTemperature.offset = read_eeprom(offsetTempetatureAddress) % 10;

    if ((read_eeprom(isOffsetNegativeAddress) % 2) == TRUE) {
        newTemperature.offset *= -1;
    }
    newTemperature.minimumTemperature = minimumTemperature;

    return newTemperature;
}

void Temperature_saveTemperatureOffset(Temperature *temperaturePtr) {
    BOOLEAN isNegative;
    if (temperaturePtr->offset >= 0) {
        isNegative = FALSE;
    } else if (temperaturePtr->offset < 0) {
        isNegative = TRUE;
    }
    write_eeprom(
            temperaturePtr->offsetTempetatureAddress,
            abs(temperaturePtr->offset)
            );
    write_eeprom(
            temperaturePtr->isOffsetNegativeAddress,
            isNegative
            );
}

TemperatureInDigits Temperature_getTemperatureInDigits(int temperature) {
    TemperatureInDigits newTemperatureInDigits;
    Array_splitNumberIntoDigits(
            temperature,
            newTemperatureInDigits.temperature,
            2
            );

    return newTemperatureInDigits;
}

TemperatureInDigits Temperature_getTemperatureInDigits(Temperature *temperaturePtr) {
    TemperatureInDigits newTemperatureInDigits;
    signed int absoluteValue = temperaturePtr->value + temperaturePtr->offset;

    absoluteValue =
            absoluteValue <= temperaturePtr->minimumTemperature ?
            temperaturePtr->minimumTemperature : absoluteValue;

    Array_splitNumberIntoDigits(
            abs(absoluteValue),
            newTemperatureInDigits.temperature,
            2
            );
    newTemperatureInDigits.isNegative = absoluteValue < 0;
    return newTemperatureInDigits;
}

void Temperature_increaseTemperatureOffset(Temperature *temperaturePtr) {
    temperaturePtr->offset =
            temperaturePtr->offset >= 9 ? -9 : temperaturePtr->offset + 1;
}

void Temperature_setTemperature(Temperature *temperaturePtr, signed int value) {
    temperaturePtr->value = value;
}
#endif	/* TEMPERATURE_H */

