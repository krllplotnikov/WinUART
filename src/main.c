#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "WinUART.h"

#define PORT_NAME "\\\\.\\COM11"
#define BAUDRATE  115200

WinUART_st winUART = {0};
WinUART_Descriptor_st winUART_Descriptor = {
		.baudrate = BAUDRATE,
		.portName = PORT_NAME
};

int main() {
    uint8_t buffer[16] = {0};

	if(WinUART_Open(&winUART, &winUART_Descriptor) != WIN_UART_OK){
		printf("WinUART_Open error\n");
		return 1;
	}
	printf("WinUART_Open\n");

	WinUART_Transmit(&winUART, buffer, sizeof(buffer));
}

//
//void WinUART_Receive_Callback(&winUART, uint8_t* data, size_t size){
//	printf("Data received\n");
//	printf("Size: %d\n", size);
//	printf("Data:\n");
//	for(size_t i = 0; i < size; i++){
//		printf("0x%02X ", data[i]);
//	}
//	printf("\n");
//}
