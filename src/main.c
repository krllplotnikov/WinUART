#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "WinUART.h"

#define PORT_NAME "COM9"
#define BAUDRATE  115200

int main() {
    uint8_t buffer[16];

	if(WinUART_Open(PORT_NAME, BAUDRATE) != WIN_UART_OK){
		printf("WinUART_Open error\n");
		return 1;
	}
	WinUART_Transmit(buffer, sizeof(buffer));

	while(1){

	}
}

void WinUART_Receive_Callback(uint8_t* data, size_t size){
	printf("Data received\n");
	printf("Size: %d\n", size);
	printf("Data:\n");
	for(size_t i = 0; i < size; i++){
		printf("0x%02X ", data[i]);
	}
	printf("\n");
}
