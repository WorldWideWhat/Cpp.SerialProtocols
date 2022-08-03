/**
 * @file cctalk.cpp
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief CCTalk protocol interface
 * @version 0.1
 * @date 2021-09-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "cctalk.h"
#include <iostream>
#include <unistd.h>
#include <inttypes.h>
CCTalk::CCTalk(const uint8_t id) : _id(id){ }

CCTalk::~CCTalk(){
    disconnect();
}

uint8_t CCTalk::calcCrc(const CCTalkPackage &package){
    uint16_t n_crc = package.senderID + package.length + package.receiverID + package.header;
    for(uint8_t b_index = 0; b_index < package.length; b_index++){
        n_crc += package.data[b_index];
    }
    n_crc = 256 - ( n_crc^256 );
    return (uint8_t) n_crc;
}

int CCTalk::transmitPackage(CCTalkPackage &package){

    uint8_t* n_bffr = package.toBytearray();
    int n_size = sizeof(n_bffr) + 1;
    int n_written = transmit(n_bffr, n_size);

    if(n_size != n_written) return -1;
    return 0;
}

int CCTalk::receivePackage(CCTalkPackage &package){
    
    uint8_t n_head_bffr[4];
    int n_read = receive(n_head_bffr, 4);
    if(n_read != 4) return -1;
    package.receiverID = n_head_bffr[0];
    package.length = n_head_bffr[1];
    package.senderID = n_head_bffr[2];
    package.header = n_head_bffr[3];


    if(package.length > 0) {
        package.data = new uint8_t[package.length];
        n_read = receive(package.data, package.length);
        if(n_read != package.length) return -1;
    }
    
    n_read = receive(&package.crc, 1);
    if(n_read != 1) return -1;

    uint8_t n_crc = calcCrc(package);
    if(n_crc != package.crc) return -1;
    return 0;
}

int CCTalk::transmitPackageWithReply(CCTalkPackage &transmit, CCTalkPackage &reply){

    if(transmitPackage(transmit) != 0) return -1;
    usleep(100);
    if(receivePackage(reply) != 0) return -1;
    return 0;
}

int CCTalk::getEventStack(const uint8_t receiverID, EventStack &eventStack){
    CCTalkPackage n_sendPack;
    CCTalkPackage n_recvPack;

    static bool n_firstEvent = true;

    n_sendPack.senderID = _id;
    n_sendPack.length = 0;
    n_sendPack.receiverID = receiverID;
    n_sendPack.header = (uint8_t)Header::ReadBuffCreditOrErr;
    n_sendPack.crc = calcCrc(n_sendPack);
    
    int n_res = transmitPackageWithReply(n_sendPack, n_recvPack);
    if(n_res != 0)
        return -1;
    

    if(n_recvPack.header != (uint8_t)Header::ReturnMessage)
        return -1;
    

    if(n_recvPack.length == 0) 
        return -1;        

    uint8_t n_eventID = n_recvPack.data[0];
    int diff = 0;
    if(n_eventID != eventStack.lastEventId) {
        
        if(eventStack.lastEventId == 0) {
            eventStack.events.clear();
            n_firstEvent = !n_firstEvent;
        } else {
            if(!n_firstEvent) {
                if(eventStack.lastEventId > n_eventID)
                    diff = (255 - eventStack.lastEventId) + n_eventID;
                else
                    diff = n_eventID - eventStack.lastEventId;
                
                if(diff > ((n_recvPack.length - 1) / 2)) {
                    diff = (n_recvPack.length - 1) / 2;
                }

                int n_dataPos = 1;
                for(int n_eventIndex = 0; n_eventIndex < diff; n_eventIndex++){
                    CCT_Event n_event(n_recvPack.data[n_dataPos], n_recvPack.data[n_dataPos+1]);
                    n_dataPos+=2;
                    eventStack.events.push_back(n_event);
                }
            } else {
                n_firstEvent = false;            
            }
        }

        eventStack.lastEventId = n_eventID;
    }
    return diff;
}
