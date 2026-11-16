#include "WinUART.h"

static DWORD WINAPI WinUART_RxThread(LPVOID param);
static WinUART_Error_et WinUART_StartReceive(WinUART_st* winUART);
static void WinUART_StopReceive(WinUART_st* winUART);

WinUART_Error_et WinUART_Open(WinUART_st* winUART, WinUART_Descriptor_st* descriptor){
	winUART->descriptor = *descriptor;
	winUART->hRxThread = NULL;
	winUART->hRxStopEvent = NULL;

	winUART->hSerial = CreateFile(winUART->descriptor.portName, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

    if (winUART->hSerial == INVALID_HANDLE_VALUE) {
        return WIN_UART_PORT_OPEN_ERROR;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    if (!GetCommState(winUART->hSerial, &dcbSerialParams)) {
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_GET_STATE_ERROR;
    }

    dcbSerialParams.BaudRate = winUART->descriptor.baudrate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    if (!SetCommState(winUART->hSerial, &dcbSerialParams)) {
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_SET_STATE_ERROR;
    }

    COMMTIMEOUTS timeouts = { 0 };
    if (winUART->descriptor.rxIdleTimeout == 0) {
        timeouts.ReadIntervalTimeout = MAXDWORD;
        timeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        timeouts.ReadTotalTimeoutConstant = 100;
    } else {
        timeouts.ReadIntervalTimeout = winUART->descriptor.rxIdleTimeout;
        timeouts.ReadTotalTimeoutMultiplier = 0;
        timeouts.ReadTotalTimeoutConstant = 0;
    }
    timeouts.WriteTotalTimeoutConstant = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(winUART->hSerial, &timeouts)) {
        CloseHandle(winUART->hSerial);
        return WIN_UART_PORT_SET_TIMEOUTS_ERROR;
    }

    PurgeComm(winUART->hSerial, PURGE_RXCLEAR | PURGE_TXCLEAR);

    if (winUART->descriptor.receiveCallback != NULL) {
    	WinUART_Error_et err = WinUART_StartReceive(winUART);
    	if (err != WIN_UART_OK) {
    		CloseHandle(winUART->hSerial);
    		return err;
    	}
    }

    return WIN_UART_OK;
}

WinUART_Error_et WinUART_Transmit(WinUART_st* winUART, uint8_t* data, size_t size){
	OVERLAPPED ov = { 0 };
	DWORD bytesWritten = 0;
	WinUART_Error_et result = WIN_UART_TRANSMIT_ERROR;

	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		return WIN_UART_TRANSMIT_ERROR;
	}

	if (!WriteFile(winUART->hSerial, data, (DWORD)size, &bytesWritten, &ov)) {
		if (GetLastError() != ERROR_IO_PENDING) {
			CloseHandle(ov.hEvent);
			return WIN_UART_TRANSMIT_ERROR;
		}
	}

	if (GetOverlappedResult(winUART->hSerial, &ov, &bytesWritten, TRUE) && bytesWritten == size) {
		result = WIN_UART_OK;
	}

	CloseHandle(ov.hEvent);
	return result;
}

WinUART_Error_et WinUART_Close(WinUART_st* winUART){
	WinUART_StopReceive(winUART);
	CloseHandle(winUART->hSerial);
	winUART->hSerial = INVALID_HANDLE_VALUE;
	return WIN_UART_OK;
}

static WinUART_Error_et WinUART_StartReceive(WinUART_st* winUART){
	winUART->hRxStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (winUART->hRxStopEvent == NULL) {
		return WIN_UART_RX_THREAD_ERROR;
	}

	winUART->hRxThread = CreateThread(NULL, 0, WinUART_RxThread, winUART, 0, &winUART->rxThreadId);
	if (winUART->hRxThread == NULL) {
		CloseHandle(winUART->hRxStopEvent);
		winUART->hRxStopEvent = NULL;
		return WIN_UART_RX_THREAD_ERROR;
	}

	return WIN_UART_OK;
}

static void WinUART_StopReceive(WinUART_st* winUART){
	if (winUART->hRxThread == NULL) {
		return;
	}

	SetEvent(winUART->hRxStopEvent);

	if (GetCurrentThreadId() != winUART->rxThreadId) {
		WaitForSingleObject(winUART->hRxThread, INFINITE);
	}

	CloseHandle(winUART->hRxThread);
	CloseHandle(winUART->hRxStopEvent);
	winUART->hRxThread = NULL;
	winUART->hRxStopEvent = NULL;
}

static DWORD WINAPI WinUART_RxThread(LPVOID param){
	WinUART_st* winUART = (WinUART_st*)param;
	HANDLE hSerial = winUART->hSerial;
	HANDLE hStop = winUART->hRxStopEvent;
	WinUART_Receive_Callback callback = winUART->descriptor.receiveCallback;
	uint8_t buffer[WIN_UART_RX_BUFFER_SIZE];
	OVERLAPPED ov = { 0 };

	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) {
		return 1;
	}

	HANDLE waitHandles[2] = { hStop, ov.hEvent };

	while (WaitForSingleObject(hStop, 0) != WAIT_OBJECT_0) {
		DWORD bytesRead = 0;
		ResetEvent(ov.hEvent);

		if (!ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, &ov)) {
			if (GetLastError() != ERROR_IO_PENDING) {
				break;
			}

			DWORD w = WaitForMultipleObjects(2, waitHandles, FALSE, INFINITE);
			if (w == WAIT_OBJECT_0) {
				CancelIo(hSerial);
				GetOverlappedResult(hSerial, &ov, &bytesRead, TRUE);
				break;
			}

			if (!GetOverlappedResult(hSerial, &ov, &bytesRead, FALSE)) {
				break;
			}
		}

		if (bytesRead > 0) {
			callback(winUART, buffer, bytesRead);
		}
	}

	CloseHandle(ov.hEvent);
	return 0;
}
