/* 
 * File:   ascii.h
 * Author: Diseno4
 *
 * Created on October 24, 2017, 4:13 PM
 */

#ifndef ASCII_H
#define	ASCII_H

#define ASCII_OFFSET 48

void ASCII_convertToAsciiArray(char charArray[], int arraySize) {
    int i = 0;
    for (i = 0; i < arraySize; i++) {
        charArray[i] += ASCII_OFFSET;
    }
}

#endif	/* ASCII_H */

