#ifndef WINUART_H_
#define WINUART_H_

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum{
	WIN_UART_OK,
	WIN_UART_PORT_OPEN_ERROR,
	WIN_UART_PORT_GET_STATE_ERROR,
	WIN_UART_PORT_SET_STATE_ERROR,
	WIN_UART_PORT_SET_TIMEOUTS_ERROR,
	WIN_UART_TRANSMIT_ERROR,
}WinUART_Error_et;

WinUART_Error_et WinUART_Open(const char* portName, uint32_t baudrate);
WinUART_Error_et WinUART_Transmit(uint8_t* data, size_t size);
WinUART_Error_et WinUART_Close();

void WinUART_Receive_Callback(uint8_t* data, size_t size);

#endif
