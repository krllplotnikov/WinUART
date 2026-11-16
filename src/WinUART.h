#ifndef WINUART_H_
#define WINUART_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum WinUART_Error WinUART_Error_et;

typedef struct WinUART WinUART_st;
typedef struct WinUART_Descriptor WinUART_Descriptor_st;

typedef void (*WinUART_Receive_Callback)(WinUART_st winUART, uint8_t* data, size_t size);

enum WinUART_Error{
	WIN_UART_OK,
	WIN_UART_PORT_OPEN_ERROR,
	WIN_UART_PORT_GET_STATE_ERROR,
	WIN_UART_PORT_SET_STATE_ERROR,
	WIN_UART_PORT_SET_TIMEOUTS_ERROR,
	WIN_UART_TRANSMIT_ERROR,
};

struct WinUART_Descriptor{
	char* portName;
	uint32_t baudrate;
	WinUART_Receive_Callback receiveCallback;
};

struct WinUART{
	HANDLE hSerial;
	WinUART_Descriptor_st descriptor;
};

WinUART_Error_et WinUART_Open(WinUART_st* winUART, WinUART_Descriptor_st* descriptor);
WinUART_Error_et WinUART_Transmit(WinUART_st* winUART, uint8_t* data, size_t size);
WinUART_Error_et WinUART_Close(WinUART_st* winUART);

#endif
