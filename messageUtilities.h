/*
 * messageUtilities.h
 *
 *  Created on: Feb 9, 2020
 *      Author: petermaggio
 */

#ifndef MESSAGEUTILITIES_H_
#define MESSAGEUTILITIES_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ti/utils/json/json.h>
#define MAX_COORDINATES       100
#define DATA_SIZE             2048


float hexToDecimal(char hex[9]);
uint16_t packMessageToUint32(char *data, uint32_t *out);
int unpackToJSON(uint32_t *packedCoordinates, uint16_t packedCoordinatesNum, char *unpacked);
struct coordinate {
  float longitude;
  float latitude;
} __attribute__((packed));
struct packedCoordinate {
  uint32_t longitude;
  uint32_t latitude;
} __attribute__((packed));



#endif /* MESSAGEUTILITIES_H_ */
