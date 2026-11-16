#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "WinUART.h"

#define PORT_NAME "\\\\.\\COM23"
#define BAUDRATE  115200

void WinUART_Receive_Handler(WinUART_st* winUART, uint8_t* data, size_t size);

WinUART_st winUART = {0};
WinUART_Descriptor_st winUART_Descriptor = {
		.baudrate = BAUDRATE,
		.portName = PORT_NAME,
		.receiveCallback = WinUART_Receive_Handler,
		.rxIdleTimeout = 50
};

int main() {
    uint8_t buffer[16] = "Hello";

	if(WinUART_Open(&winUART, &winUART_Descriptor) != WIN_UART_OK){
		printf("WinUART_Open error\n");
		return 1;
	}
	printf("WinUART_Open\n");

	WinUART_Transmit(&winUART, buffer, sizeof(buffer));

	printf("Press Enter to exit\n");
	getchar();

	WinUART_Close(&winUART);
	return 0;
}

void WinUART_Receive_Handler(WinUART_st* winUART, uint8_t* data, size_t size){
	(void)winUART;
	printf("Data received, size: %u\n", (unsigned)size);
	for(size_t i = 0; i < size; i++){
		printf("0x%02X ", data[i]);
	}
	printf("\n");
}
