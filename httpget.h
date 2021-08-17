#ifndef HTTPGET_H
#define HTTPGET_H
#include <stdio.h>
#include <string.h>
#include <ti/display/Display.h>
#include <ti/net/http/httpclient.h>
#include <ti/drivers/UART.h>
#include "ti_drivers_config.h"

#define HOSTNAME              "http://192.168.9.1:5000"
#define USER_AGENT            "HTTPClient (ARM; TI)"
#define CONTENT_ENCODING      "application/json"


void printError(char *errString, int code);
HTTPClient_Handle createHTTPHandle(char* userAgent);
void connect(HTTPClient_Handle clientHandle, char hostName[]);
void *httpTask(void *arg0);
uint16_t readResponse(HTTPClient_Handle httpClient, char *data);
uint16_t packMessageToUint32(char *data, uint32_t *out);
float hexToDecimal(char hex[9]);
void HTTPrequest(HTTPClient_Handle httpClient, const char *method, const char *uri,
    const char *body, uint32_t bodyLen);
void HTTPpost(HTTPClient_Handle httpClient, char uri[],
    const char *body, uint32_t bodyLen);
void HTTPget(HTTPClient_Handle httpClient, const char *uri);

#endif
