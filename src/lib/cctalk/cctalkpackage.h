/**
 * @file cctalkpackage.h
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief CCTalk message/package object class definition
 * @version 0.1
 * @date 2021-09-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _CCTALK_PACKAGE_H_
#define _CCTALK_PACKAGE_H_
#include <inttypes.h>
#include <stdlib.h>
#include <iostream>

/**
 * @brief ccTalk package format
  */
class CCTalkPackage {
    public:

    /**
     * @brief Construct a new CCTalkPackage object
     */
    CCTalkPackage(): data(nullptr), _buffer(nullptr){}

    /**
     * @brief Destroy the CCTalkPackage object
     */
    ~CCTalkPackage(){
        if(data != nullptr) delete [] data;
        if(_buffer != nullptr) delete [] _buffer;
    }

    /**
     * @brief Convert package object to a byte array
     * 
     * @param buffer - data buffer
     * @return int  - size of the buffer
     */
    uint8_t* toBytearray(){
        int pos = 0;

        if(_buffer != nullptr) delete [] _buffer;
        _buffer = new uint8_t[length + 5];

        _buffer[pos++]=receiverID;
        _buffer[pos++]=length;
        _buffer[pos++]=senderID;
        _buffer[pos++]=header;
        for(uint8_t index = 0; index < length; index++)
            _buffer[pos++] = data[index];
        _buffer[pos++] = crc;
        return _buffer;
    }

    int getMessageSize(){
        return length + 5;
    }

    public:
    /** @brief Package receiver ID */
    uint8_t receiverID;
    /** @brief length of the data container */
    uint8_t length;
    /** @brief Package sender ID */
    uint8_t senderID;
    /** @brief Package header / command */
    uint8_t header;
    /** @brief Data container */
    uint8_t *data;
    /** @brief Package crc/lrc value */
    uint8_t crc;

    private:
    uint8_t *_buffer;
};

#endif //_CCTALK_PACKAGE_H_