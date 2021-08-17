/*
 * Copyright (c) 2015-2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;

/*
 *  ======== printError ========
 */
void printError(char *errString, int code)
{
  Display_printf(display, 0, 0, "Error! code = %d, desc = %s\n", code,
          errString);
  while(1);
}

HTTPClient_Handle createHTTPHandle(char* userAgent) {
  int16_t statusCode = 0;
  int16_t ret = 0;

  HTTPClient_Handle httpClientHandle = HTTPClient_create(&statusCode,0);
  if (statusCode < 0)
  {
    printError("httpTask: creation of http client handle failed", statusCode);
  }

  ret = HTTPClient_setHeader(httpClientHandle,
          HTTPClient_HFIELD_REQ_USER_AGENT, USER_AGENT,
          strlen(USER_AGENT) + 1, HTTPClient_HFIELD_PERSISTENT);
  if (ret < 0) {
    printError("httpTask: setting request header failed", ret);
  }

  ret = HTTPClient_setHeader(httpClientHandle,
          HTTPClient_HFIELD_REQ_CONTENT_TYPE, CONTENT_ENCODING,
          strlen(CONTENT_ENCODING) + 1, HTTPClient_HFIELD_PERSISTENT);
  if (ret < 0) {
    printError("httpTask: setting request header failed", ret);
  }

  return httpClientHandle;
}

void connect(HTTPClient_Handle clientHandle, char hostName[]) {
  int16_t ret = 0;
  Display_printf(display, 0, 0, "Connecting to %s\n", hostName);
  ret = HTTPClient_connect(clientHandle, hostName, 0, 0);
  if (ret < 0) {
    printError("httpTask: connect failed", ret);
  }
}

void HTTPget(HTTPClient_Handle httpClient, const char *uri) {
  HTTPrequest(httpClient, HTTP_METHOD_GET, uri, NULL, 0);
}

void HTTPpost(HTTPClient_Handle httpClient, char uri[],
    const char *body, uint32_t bodyLen) {
  HTTPrequest(httpClient, HTTP_METHOD_POST, uri, body, bodyLen);
}

void HTTPrequest(HTTPClient_Handle httpClient, const char *method, const char *uri,
    const char *body, uint32_t bodyLen) {
  int16_t ret = 0;
  Display_printf(display, 0, 0, "Sending %s request to '%s'\n", method, uri);
  ret = HTTPClient_sendRequest(httpClient, method,
      uri, body, bodyLen, 0);
  if (ret < 0) {
    Display_printf(display, 0, 0, "Error! %s send failed. code = %d", method, ret);
  }
  if (ret != HTTP_SC_OK) {
    Display_printf(display, 0, 0, "Error! %s cannot get status. code = %d", method, ret);
  }

  Display_printf(display, 0, 0, "%s Response Status Code: %d\n", method, ret);
}

//
uint16_t readResponse(HTTPClient_Handle httpClient, char *data) {
  bool moreDataFlag = false;
  int16_t len = 0;
  int16_t ret = 0;
  do {
    ret = HTTPClient_readResponseBody(httpClient, data, sizeof(char)*DATA_SIZE,
            &moreDataFlag);
    if (ret < 0) {
      printError("httpTask: response body processing failed", ret);
    }
    Display_printf(display, 0, 0, "%.*s \r\n",ret ,data);
    len += ret;
  } while (moreDataFlag);
  data[len] = NULL;

  Display_printf(display, 0, 0, "Received %d bytes of payload\n", len);

  return len;

}

int unpackToJSON(uint32_t *packedCoordinates, uint16_t packedCoordinatesNum, char *unpacked) {
  const char header[] = "{\"message\":\"";
  const char footer[] = "\"}";
  char *body = unpacked;
  char *runner = body;
  struct packedCoordinate *outgoing = (struct packedCoordinate*)packedCoordinates;
  
  // Append header
  strncpy(body, header, 12);
  runner += 12;

  int i = 0;
  while (i < packedCoordinatesNum && i < ( DATA_SIZE / 18 )) {
    sprintf(runner, "%08x", outgoing[i].longitude);
    runner[8] = ',';
    runner += 9;
    sprintf(runner, "%08x", outgoing[i].latitude);
    runner[8] = ';';
    runner += 9;
    ++i;
  }

  strncpy(runner, footer, 3);

  int bytes = (int) runner - (int) body + 2;
  return bytes;

}

// UART Vars
UART_Handle uart;
UART_Params uartParams;

/*
 *  ======== httpTask ========
 *  Makes a HTTP GET request
 */
void *httpTask(void *arg0)
{
  Display_printf(display, 0, 0, "Initializing UART\n");
  /* Create a UART with data processing off. */
  UART_init();
  UART_Params_init(&uartParams);
  uartParams.writeDataMode = UART_DATA_BINARY;
  uartParams.writeMode = UART_MODE_BLOCKING;
  uartParams.readDataMode = UART_DATA_BINARY;
  uartParams.readReturnMode = UART_RETURN_FULL;
  uartParams.readMode = UART_MODE_BLOCKING;
  uartParams.readEcho = UART_ECHO_OFF;
  uartParams.baudRate = 115200;

  uart = UART_open(CONFIG_UART_2, &uartParams);
  if (uart == NULL)
  {
      // UART_open() failed
      while (1); //hang if UART is not initialized properly
  }
  
  Display_printf(display, 0, 0, "Intializing HTTP session\n");
  int16_t ret = 0;
  char data[DATA_SIZE];
  uint32_t UART_BUF[MAX_COORDINATES];
  HTTPClient_Handle httpClientHandle;

  fdOpenSession(TaskSelf());

  httpClientHandle = createHTTPHandle(USER_AGENT);

  connect(httpClientHandle, HOSTNAME);

  char stopMsg[] = "STOP";
  while(1) {
    Display_printf(display, 0, 0, "Reading in from UART\n");
    // Read "in" through UART
    uint32_t uart_input;
    uint8_t i;
    uint32_t bytesInFifo = 0;
    // Wait here until there's something in the buffer
    while (bytesInFifo == 0) {
      UART_control(uart, UART_CMD_GETRXCOUNT, &bytesInFifo);
    }
    for(i = 0; i < MAX_COORDINATES*2; i++) {
      UART_read(uart, &uart_input, 4);
      if(memcmp(&uart_input, stopMsg, sizeof(stopMsg) - 1) == 0) {
        break;
      }
      if (uart_input == 0)
        break;
      UART_BUF[i] = uart_input;
      uart_input = 0;
    }
    
    Display_printf(display, 0, 0, "Sending POST\n");
    // Sending http post
    int bodyLen = unpackToJSON(UART_BUF, i/2, data);
    Display_printf(display, 0, 0, "%s", data);
    HTTPpost(httpClientHandle, "/location/gateway/1", data, bodyLen);
    
    Display_printf(display, 0, 0, "Reading GET\n");
    // Reading http get response
    HTTPget(httpClientHandle, "/perimeter/gateway/1/1");
    uint16_t bytesRead = readResponse(httpClientHandle, data);
    uint16_t coordinatesPacked = packMessageToUint32(data, UART_BUF);
    
    
    /*
    Display_printf(display, 0, 0, "Sending out through UART\n");
    // Send "out" through UART
    memcpy(UART_BUF + coordinatesPacked*2, stopMsg, sizeof(stopMsg));
    UART_write(uart, UART_BUF, coordinatesPacked*2 + sizof(stoopMsg));
    */
  }

  ret = HTTPClient_disconnect(httpClientHandle);
  if (ret < 0)
  {
    printError("httpTask: disconnect failed", ret);
  }

  HTTPClient_destroy(httpClientHandle);

  fdCloseSession(TaskSelf());

  return (NULL);
}
