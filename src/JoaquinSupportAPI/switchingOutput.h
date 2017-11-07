/* 
 * File:   switchingOutput.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on 3 de octubre de 2017, 02:23 PM
 */

#ifndef SWITCHINGOUTPUT_H
#define	SWITCHINGOUTPUT_H

typedef enum {
    STATE_OUTPUT_A_ON_B_OFF = 0,
    STATE_OUTPUT_A_OFF_B_ON,
    STATE_OUTPUTS_OFF,
    NUMBER_OF_STATES
} SwitchingOutputsState;

typedef struct {
    SwitchingOutputsState currentState;
    SwitchingOutputsState previousState;
    int16 outputAPin;
    int16 outputBPin;
    __EEADDRESS__ currentStateAddress;
    __EEADDRESS__ previousStateAddress;
} SwitchingOutputs;

SwitchingOutputs SwitchingOutputs_new(int16 outputAPin, int16 outputBPin) {
    SwitchingOutputs newSwitchingOutputs;
    newSwitchingOutputs.outputAPin = outputAPin;
    newSwitchingOutputs.outputBPin = outputBPin;
    newSwitchingOutputs.currentState = STATE_OUTPUT_A_ON_B_OFF;
    newSwitchingOutputs.previousState = STATE_OUTPUTS_OFF;

    return newSwitchingOutputs;
}

SwitchingOutputs SwitchingOutputs_new(int16 outputAPin, int16 outputBPin,
        __EEADDRESS__ currentStateAddress, __EEADDRESS__ previousStateAddress) {
    SwitchingOutputs newSwitchingOutputs;
    newSwitchingOutputs.outputAPin = outputAPin;
    newSwitchingOutputs.outputBPin = outputBPin;
    newSwitchingOutputs.currentState = read_eeprom(currentStateAddress) % NUMBER_OF_STATES;
    newSwitchingOutputs.previousState = read_eeprom(previousStateAddress) % NUMBER_OF_STATES;
    newSwitchingOutputs.currentStateAddress = currentStateAddress;
    newSwitchingOutputs.previousStateAddress = previousStateAddress;

    return newSwitchingOutputs;
}

void SwitchingOutputs_run(SwitchingOutputs *switchingOutputsPtr) {
    switch (switchingOutputsPtr->currentState) {
        case STATE_OUTPUT_A_OFF_B_ON:
            output_low(switchingOutputsPtr->outputAPin);
            output_high(switchingOutputsPtr->outputBPin);
            break;

        case STATE_OUTPUT_A_ON_B_OFF:
            output_high(switchingOutputsPtr->outputAPin);
            output_low(switchingOutputsPtr->outputBPin);
            break;

        case STATE_OUTPUTS_OFF:
            output_low(switchingOutputsPtr->outputAPin);
            output_low(switchingOutputsPtr->outputBPin);
            break;

    }

}

void SwitchingOutputs_toggle(SwitchingOutputs *switchingOutputsPtr) {
    switchingOutputsPtr->previousState = switchingOutputsPtr->currentState;
    if (switchingOutputsPtr->currentState == STATE_OUTPUT_A_OFF_B_ON) {
        switchingOutputsPtr->currentState = STATE_OUTPUT_A_ON_B_OFF;
    } else if (switchingOutputsPtr->currentState == STATE_OUTPUT_A_ON_B_OFF) {
        switchingOutputsPtr->currentState = STATE_OUTPUT_A_OFF_B_ON;
    }
    SwitchingOutputs_run(switchingOutputsPtr);
}

void SwitchingOutputs_stop(SwitchingOutputs *switchingOutputsPtr) {
    switchingOutputsPtr->previousState = switchingOutputsPtr->currentState;
    switchingOutputsPtr->currentState = STATE_OUTPUTS_OFF;
    SwitchingOutputs_run(switchingOutputsPtr);
}

void SwitchingOutputs_resume(SwitchingOutputs *switchingOutputsPtr) {
    SwitchingOutputsState newState;
    newState = switchingOutputsPtr->currentState;
    switchingOutputsPtr->currentState = switchingOutputsPtr->previousState;
    switchingOutputsPtr->previousState = newState;
    SwitchingOutputs_run(switchingOutputsPtr);

}

void SwitchingOutputs_restart(SwitchingOutputs *switchingOutputsPtr) {
    switchingOutputsPtr->previousState = switchingOutputsPtr->currentState;
    switchingOutputsPtr->currentState = STATE_OUTPUT_A_ON_B_OFF;
    SwitchingOutputs_run(switchingOutputsPtr);
}

void SwitchingOutputs_saveState(SwitchingOutputs *switchingOutputsPtr) {
    write_eeprom(switchingOutputsPtr->currentStateAddress, switchingOutputsPtr->currentState);
    write_eeprom(switchingOutputsPtr->previousStateAddress, switchingOutputsPtr->previousState);
}
#endif	/* SWITCHINGOUTPUT_H */

