#include "WinUART.h"

WinUART_Error_et WinUART_Open(WinUART_st* winUART, WinUART_Descriptor_st* descriptor){
	winUART->descriptor.portName = descriptor->portName;
	winUART->descriptor.baudrate = descriptor->baudrate;

	winUART->hSerial = CreateFile(winUART->descriptor.portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (winUART->hSerial == INVALID_HANDLE_VALUE) {
        //printf("Can not open port %s\n", portName);
        return WIN_UART_PORT_OPEN_ERROR;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(winUART->hSerial, &dcbSerialParams)) {
        //printf("Error get port state\n");
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_GET_STATE_ERROR;
    }

    dcbSerialParams.BaudRate = winUART->descriptor.baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(winUART->hSerial, &dcbSerialParams)) {
        //printf("Error set port state\n");
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_SET_STATE_ERROR;
    }

    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(winUART->hSerial, &timeouts)) {
        //printf("Error set timeouts\n");
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_SET_TIMEOUTS_ERROR;
    }

    //printf("Port %s open success\n", portName);

    return WIN_UART_OK;
}

WinUART_Error_et WinUART_Transmit(WinUART_st* winUART, uint8_t* data, size_t size){
    DWORD bytesWritten;
	BOOL writeSuccess = WriteFile(winUART->hSerial, data, size, &bytesWritten, NULL);
	if (writeSuccess) {
		//printf("Write %lu bytes\n", bytesWritten);
		return WIN_UART_OK;
	}
	return WIN_UART_TRANSMIT_ERROR;
}

WinUART_Error_et WinUART_Close(WinUART_st* winUART){
	CloseHandle(winUART->hSerial);
	return WIN_UART_OK;
}
