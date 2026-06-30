#include "WinUART.h"

HANDLE hSerial;
DWORD dwThreadId;

DWORD WINAPI WinUART_Receive_Thread(LPVOID lpParam) {
	while(1){
	    char readBuffer[1024] = {0};
	    DWORD bytesRead;
		BOOL readSuccess = ReadFile(hSerial, readBuffer, sizeof(readBuffer) - 1, &bytesRead, NULL);
		if (readSuccess && bytesRead > 0) {
			//printf("Read %lu bytes: %s\n", bytesRead, readBuffer);
			WinUART_Receive_Callback((uint8_t*)readBuffer, bytesRead);
		}
		else{
			//printf("No readSuccess\n");
		}
	}
}

WinUART_Error_et WinUART_Open(const char* portName, uint32_t baudrate){
    hSerial = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        //printf("Can not open port %s\n", portName);
        return WIN_UART_PORT_OPEN_ERROR;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(hSerial, &dcbSerialParams)) {
        //printf("Error get port state\n");
        CloseHandle(hSerial);
        return WIN_UART_PORT_GET_STATE_ERROR;
    }

    dcbSerialParams.BaudRate = baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(hSerial, &dcbSerialParams)) {
        //printf("Error set port state\n");
        CloseHandle(hSerial);
        return WIN_UART_PORT_SET_STATE_ERROR;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(hSerial, &timeouts)) {
        //printf("Error set timeouts\n");
        CloseHandle(hSerial);
        return WIN_UART_PORT_SET_TIMEOUTS_ERROR;
    }

    //printf("Port %s open success\n", portName);

    CreateThread(NULL, 0, WinUART_Receive_Thread, NULL, 0, &dwThreadId);

    return WIN_UART_OK;
}

WinUART_Error_et WinUART_Transmit(uint8_t* data, size_t size){
    DWORD bytesWritten;
	BOOL writeSuccess = WriteFile(hSerial, data, size, &bytesWritten, NULL);
	if (writeSuccess) {
		//printf("Write %lu bytes\n", bytesWritten);
		return WIN_UART_OK;
	}
	return WIN_UART_TRANSMIT_ERROR;
}

WinUART_Error_et WinUART_Close(){
	CloseHandle(hSerial);
	return WIN_UART_OK;
}

__attribute__((weak)) void WinUART_Receive_Callback(uint8_t* data, size_t size){}
