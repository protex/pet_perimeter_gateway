/*
 * messageUtilities.c
 *
 *  Created on: Feb 9, 2020
 *      Author: petermaggio
 */

#include "messageUtilities.h"

float hexToDecimal(char hex[9]) {
  uint32_t num;
  float f;
  sscanf(hex, "%x", &num);  // assuming you checked input
  f = *((float*)&num);
  return f;
}
// https://stackoverflow.com/questions/21323099/convert-a-hexadecimal-to-a-float-and-viceversa-in-c

uint16_t packMessageToUint32(char *data, uint32_t *out) {
  struct coordinate *outgoing = (struct coordinate*)out;
  uint16_t longLatCount = 0;
  uint16_t coordinateCount = 0;
  if (strncmp(data, "{\"message\":\"", 12) == 0) {
    char *runner = data + 12; // Jump to first longitude coordinate
    while (strncmp(runner, "\"}", 2) != 0) {
      if (longLatCount % 2 == 0 && strncmp(runner + 8, ",", 1) == 0) {
        runner[8] = '\0'; // terminate coordinate
        outgoing[coordinateCount].longitude = hexToDecimal(runner);
        runner += 9; // Move to latitude 7 chars + 1 comma + 1 (get to start)
        ++longLatCount;
      } else if (longLatCount % 2 == 1 && strncmp(runner + 8, ";", 1) == 0) {
        runner[8] = '\0';
        outgoing[coordinateCount].latitude = hexToDecimal(runner);
        runner += 9;
        ++longLatCount;
        ++coordinateCount;
      } else {
        break;
      }
    }
  }
  return coordinateCount;
}




