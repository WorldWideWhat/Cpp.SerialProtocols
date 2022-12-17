/**
 * @file serial.cpp
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief Serial interface for Linux and Windows
 * @version 0.1
 * @date 2021-09-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
// Windows specific includes
#include <windows.h>
#include <conio.h>

#else
// Linux specific includes.
#include <termios.h>
#include <sys/ioctl.h>
#endif

#include <fcntl.h>
#ifndef _WIN32
#include <unistd.h>
#endif  
#include <iostream>
#include "serial.h"

Serial::Serial(): _fd(0){}
Serial::~Serial(){}

int Serial::connect(const char *devname, const int baudrate) {

#ifdef _WIN32
    char *n_port = (char *)malloc(strlen(devname) + 8);
    strcpy(n_port, "\\\\.\\");
    strcat(n_port, devname);

    _fd = CreateFile((LPCSTR)n_port,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);
    if(_fd == INVALID_HANDLE_VALUE){
        if(GetLastError() != ERROR_FILE_NOT_FOUND){
            perror("PORT ERROR");
            return -1;
        } else {
            perror("Serial open ERROR");
            return -1;
        }
    }

	COMMTIMEOUTS n_commTimeouts;
	n_commTimeouts.ReadIntervalTimeout          = 5; 
	n_commTimeouts.ReadTotalTimeoutMultiplier   = 0;
	n_commTimeouts.ReadTotalTimeoutConstant		= 0;
	n_commTimeouts.WriteTotalTimeoutMultiplier	= 0;
	n_commTimeouts.WriteTotalTimeoutConstant	= 0;

    if(!SetCommTimeouts(_fd, &n_commTimeouts)){
        perror("[SERIAL] SetCommTimeouts() failed");
        return -1;
    }

    DCB n_dcbSerialParameters;
	if(!GetCommState(_fd, &n_dcbSerialParameters)) {
		perror("unable to fetch serial parameters");
		return -1;
	} else {
		n_dcbSerialParameters.BaudRate = baudrate;
		n_dcbSerialParameters.ByteSize = 8;
		n_dcbSerialParameters.StopBits = ONESTOPBIT;
		n_dcbSerialParameters.Parity = NOPARITY;
		n_dcbSerialParameters.fDtrControl = DTR_CONTROL_DISABLE;

		if(!SetCommState(_fd, &n_dcbSerialParameters)) {
			perror("Unable to set serial parameters");
			return -1;	
		} else {
			PurgeComm(_fd, PURGE_RXCLEAR | PURGE_TXCLEAR);
			Sleep(100);
		}
	}

#else
    struct termios new_tio;
    
    _fd = open(devname, O_RDWR | O_NOCTTY);
    if(_fd < 0) return -1;

    bzero (&new_tio, sizeof(new_tio));

    new_tio.c_cflag = baudrate | CS8 | CLOCAL | CREAD;
    new_tio.c_iflag = IGNPAR;
    new_tio.c_oflag = 0;
    new_tio.c_lflag = 0;
    new_tio.c_cc[VMIN] = 0;
    new_tio.c_cc[VTIME] = 5;
    tcflush(_fd, TCIFLUSH);
    tcflush(_fd, TCOFLUSH);
    tcsetattr (_fd, TCSANOW, &new_tio);
#endif
    return 0;
}


int Serial::disconnect(){
#ifdef _WIN32    
    CloseHandle(_fd);
#else
    tcflush(_fd, TCIFLUSH);
    tcflush(_fd, TCOFLUSH);
    close(_fd);
#endif
    
    return 0;
}

int Serial::set_rts(bool state){
#ifdef _WIN32
    return EscapeCommFunction(_fd, state ? SETRTS : CLRRTS) ? 0 : -1;
#else
    int n_rtsFlag = TIOCM_RTS;
    return ioctl(_fd, state ? TIOCMBIS : TIOCMBIC, &n_rtsFlag);
#endif    
}

int Serial::set_dtr(bool state){
#ifdef _WIN32    
    return EscapeCommFunction(_fd, state ? SETDTR : CLRDTR) ? 0 : -1;
#else
    int n_dtrFlag = TIOCM_DTR;
    return ioctl(_fd, state ? TIOCMBIS : TIOCMBIC, &n_dtrFlag);
#endif    
}

int Serial::receive(uint8_t * buffer, int len, int offset){
    buffer+=offset;
#ifdef _WIN32    
    DWORD n_bytesread = 0;
    unsigned int n_toRead;
    ClearCommError(_fd, (LPDWORD)&_errors, (LPCOMSTAT)&_status);
	if(_status.cbInQue > 0) {
		n_toRead = (_status.cbInQue > (DWORD)len) ? (DWORD)len : _status.cbInQue;
	}

    if(!ReadFile(_fd, buffer, n_toRead, &n_bytesread, NULL))
        n_bytesread = 0;

#else
    ssize_t n_bytesread = 0;
    n_bytesread = read(_fd, buffer, len);
#endif    

    return (int)n_bytesread;
}

int Serial::transmit(uint8_t * buffer, int len, int offset){
    
    buffer+=offset;
#ifdef _WIN32
    DWORD n_byteswritten = 0;
    if(!WriteFile(_fd, (void*)buffer, len, &n_byteswritten, 0)){
        ClearCommError(_fd, (LPDWORD)&_errors, (LPCOMSTAT)&_status);
        return 0;
    }
#else    
    
    ssize_t n_byteswritten = write(_fd, buffer, len);
    tcflush(_fd, TCOFLUSH);
#endif    
    return (int)n_byteswritten;
}

