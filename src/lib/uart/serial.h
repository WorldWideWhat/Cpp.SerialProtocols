/**
 * @file serial.h
 * @author Lars Hederidder (lh@ctcoin.com)
 * @brief Serial interface for Linux and Windows
 * @version 0.1
 * @date 2021-09-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#ifdef _WIN32
// Windows specific includes.
#include <windows.h>
#define B115200 115200
#define B9600 9600

#else
/// Linux specific includes.
#include <termios.h>

#endif

#include <inttypes.h>



class Serial {
    public:
    Serial();
    ~Serial();

    /**
     * @brief Connect to serial device
     * 
     * @param devname Device name (ie COM1 or /dev/ttyUSB0)
     * @param baudrate Serial baudrate
     * @return Success
     */
    int connect(const char* devname, const int baudrate=B115200);

    /**
     * @brief Disconnect the serial interface
     * 
     * @return Success
     */
    int disconnect();

    /**
     * @brief Set the rts pin
     * 
     * @param state Pin state
     * @return Success
     */
    int set_rts(bool state);
    
    /**
     * @brief Set the dtr pin
     * 
     * @param state Pin state
     * @return Success
     */
    int set_dtr(bool state);
    protected:

    /**
     * @brief Receive data from the device
     * 
     * @param buffer Pointer to input buffer
     * @param len Number of bytes to read
     * @param offset Offset in pointer
     * @return Number of bytes received
     */
    int receive(uint8_t * buffer, int len, int offset=0);

    /**
     * @brief Transmit data to the device
     * 
     * @param buffer Pointer to output buffer
     * @param len Number of bytes to transmit
     * @param offset Offset in the pointer
     * @return Number of bytes written
     */
    int transmit(uint8_t* buffer, int len, int offset=0);

#ifdef _WIN32
    /**
     * @brief Sleep usleep isn't a part the the windows standard lib.
     * 
     * @param usec Number of micro seconds to sleep
     */
    void usleep(__int64 usec) 
    { 
        HANDLE timer; 
        LARGE_INTEGER ft; 

        ft.QuadPart = -(100*usec)/2; // Convert to 100 nanosecond interval, negative value indicates relative time

        timer = CreateWaitableTimer(NULL, TRUE, NULL); 
        SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
        WaitForSingleObject(timer, INFINITE); 
        CloseHandle(timer); 
    }
#endif    
    private:


#ifdef _WIN32
    HANDLE _fd;
    COMSTAT _status;
    DWORD _errors;
#else
    int _fd;
#endif


};

#endif //_SERIAL_H_