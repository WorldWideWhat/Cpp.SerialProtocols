/**
 * @file stmboot.cpp
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief STM Bootloader handler / interface
 * @version 0.1
 * @date 2021-09-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#include "stmboot.h"

STMBoot::STMBoot(){
    _filecontent = nullptr;
}

STMBoot::~STMBoot(){
    if(_filecontent != nullptr) delete [] _filecontent;
    disconnect();
}

int STMBoot::init(Target target){
    if(_bin_file_path.empty()) return -1;
    int n_res = loadFileContent();
    if(n_res != 0) return -1;
    set_rts(false);
    set_dtr(false);
    
//    uint8_t n_reboot[] = {0x02, 0x03, 0x00, 0x00, 0x01, 0x01, 0x10, 0x23, 0x03};
//    transmit(n_reboot, 9, 0);
//    readEnd();
    time_t n_start;
    time_t now;
    time(&n_start);
    time(&now);

    int n_return = -1;

    n_res = 0;
    uint8_t n_resp = 0;

    uint8_t n_cmd = (uint8_t)target;
    while(true){
        n_res = transmit(&n_cmd, 1, 0);
        if(n_res != 1) break;
        usleep(10);
        n_res = receive(&n_resp, 1, 0);
        if(n_res == 1){
            if(n_resp == (uint8_t)Response::ACK){
                n_return = 0;
                break;
            }
        }
        time(&now);
        if(difftime(now, n_start) > 10.0) break;
        usleep(100);
    }
    return n_return;
}

int STMBoot::setBinaryFile(const std::string filepath){
    _bin_file_path = filepath;
    _content_size = 0;
    if(_filecontent != nullptr) delete [] _filecontent;
    _filecontent = nullptr;

    if(isKnownType() != 0){
        _bin_file_path = "";
        return -1;
    }
    return 0;
}

int STMBoot::programTarget(bool verbose){
    int n_res = 0;

    n_res = extendedErase();
    const uint32_t address = 0x08000000;
    if(n_res != 0) {
        if(verbose) {

            if(n_res == -2){
                std::printf("Erase timeout\n");
            } else {
                std::printf("Error while erasing %d\n", n_res);
            }
        }
        return -1;
    }

    if(verbose) std::printf("Target erased\n");

    if(writeMemory(address, _filecontent, 0, _content_size) != 0){
        if(verbose) std::printf("Error while programming device\n");
        return -1;
    }
    if(verbose && _progressCallback) std::printf("\n");
    if(verbose) std::printf("Rebooting device\n");

    return reboot();
    //return 0;
}

void STMBoot::setProgressCallback(std::function<void(uint32_t, uint32_t)> callback){
    _progressCallback = callback;
}

int STMBoot::getHeader(Header &header) {
    if(_bin_file_path.empty()) return -1;
    int n_hasCrc = hasCrc();
    if(n_hasCrc < 0) return -1;

    const int n_HeaderLength = 16;
    int n_file = open(_bin_file_path.c_str(), O_RDONLY);

    if(n_file < 0) return -2;

    long n_size = lseek(n_file, 0, SEEK_END);
    if(n_size <= n_HeaderLength){
        close(n_file);
        return -3;
    }

    long n_startPos = n_size - n_HeaderLength;
    if(n_hasCrc) n_startPos -= 4;

    lseek(n_file, n_startPos, SEEK_SET);
    uint8_t n_bffr[n_HeaderLength];

    read(n_file, n_bffr, n_HeaderLength);
    close(n_file);

    header = *(Header*)n_bffr;
    return 0;
}

int STMBoot::hasCrc(){
    if(_bin_file_path.empty()) return -1;

    int n_file = open(_bin_file_path.c_str(), O_RDONLY);

    if(n_file < 0) return -1;

    long n_size = lseek(n_file, 0, SEEK_END);

    lseek(n_file, n_size - 2, SEEK_SET);
    uint8_t n_bffr[2];
    read(n_file, n_bffr, 2);
    close(n_file);


    return ((n_bffr[0] & n_bffr[1]) == 0xFF) ? 1 : 0;
}

int STMBoot::loadFileContent(){
    if(_bin_file_path.empty()) return -1;

    int n_file = open(_bin_file_path.c_str(), O_RDONLY);

    if(n_file < 0) return -1;
    _content_size = lseek(n_file, 0, SEEK_END);
    
    lseek(n_file, 0, SEEK_SET);

    if(_filecontent != nullptr) delete [] _filecontent;
    _filecontent = new uint8_t[_content_size];

    read(n_file, _filecontent, _content_size);
    close(n_file);

    return 0;
}

int STMBoot::isKnownType(){
    int n_res = getHeader(_header);

    if(n_res != 0) return -1;
    switch (_header.signature)
    {
    case Signature::SmartControllerFW:
    case Signature::SmartControllerSettings:
    case Signature::SmartSensorFW:
        return 0;
    default:
        return -1;
    } 
}

int STMBoot::writeMemory(uint32_t addr, uint8_t *buffer, uint32_t offset, uint32_t len){
    int n_len = len;
    int n_offset = offset;
    uint32_t n_addr = addr;
    
    int n_size = 0;

    int n_res = 0;
    
    while(n_len > 0) {
        if(n_len > 256)
            n_size = 256;
        else
            n_size = n_len;


        n_res = write_addr(n_addr, buffer, n_offset, n_size);
        if( n_res != 0) {
            std::printf("Error: %d\n", n_res);
            return -1;
        }


        n_addr += n_size;    
        n_offset += n_size;
        n_len -= n_size;
        if(_progressCallback) _progressCallback(len, (len - n_len));    
    }
    return 0;
}


uint8_t STMBoot::calcLrc(uint8_t* bffr, int offset, int len, bool invert){
    uint8_t n_lrc = 0xFF;

    if(len == 1) {
        n_lrc = bffr[offset];
    } else {
        for(int n_pos = offset; n_pos < (offset + len); n_pos++) {
            n_lrc ^= bffr[n_pos];
        }
    }
    return invert ? (uint8_t)~n_lrc : n_lrc;
}

int STMBoot::get(){
    return 0;
}

int STMBoot::erase(uint8_t pageNo){
    return 0;
}

int STMBoot::extendedErase(){
    usleep(10000);
    uint8_t n_tx[2];
    uint8_t n_rx;

    int n_res = 0;
    int n_return = -1;
    n_tx[0] = (uint8_t)Commands::EXT_ERASE;
    n_tx[1] = calcLrc(n_tx, 0, 1);

    n_res = transmit(n_tx, 2, 0);
    if(n_res != 2) return -1;
    usleep(10000);
    n_res = receive(&n_rx, 1, 0);

    if(n_res != 1 || n_rx != (uint8_t)Response::ACK) return -1;
    usleep(10000);
    time_t n_start;
    time_t n_now;
    time(&n_start);
    time(&n_now);

    uint8_t n_tx2[3];
    n_tx2[0] = n_tx2[1] = 0xFF;
    n_tx2[2] = 0xFF;
    n_res = transmit(n_tx2, 3, 0);

    if(n_res != 3) return -1;

    n_rx = 0;
    while (true)
    {
        n_res = receive(&n_rx, 1, 0);
        if(n_res == 1){
            if(n_rx == (uint8_t)Response::ACK) {
                n_return = 0;
            }
            break;
        }

        time(&n_now);
        if(difftime(n_now, n_start) > 40.0) {
            n_return = -2;
            break;
        }
        usleep(100);
    }
    return n_return;
}

int STMBoot::write_addr(uint32_t address, uint8_t *buffer, int offset, int length){
    uint8_t n_rx = 0;
    uint8_t n_tx[9];
    int n_res = 0;

    // Set write command
    n_tx[0] = (uint8_t)Commands::WRITE;
    n_tx[1] = calcLrc(n_tx);

    // Set address
    n_tx[2] = (uint8_t)((address >> 24) & 0xff);
    n_tx[3] = (uint8_t)((address >> 16) & 0xff);
    n_tx[4] = (uint8_t)((address >> 8) & 0xff);
    n_tx[5] = (uint8_t)(address & 0xff);
    n_tx[6] = calcLrc(n_tx, 2, 4);

    // Set length
    n_tx[7] = (uint8_t)(length - 1);
    n_tx[8] = (uint8_t)(calcLrc(buffer, offset, length) ^ n_tx[7]);

    // Transmit initial command
    n_res = transmit(n_tx, 2, 0);
    if(n_res != 2) return -18;
    usleep(500);
    n_res = receive(&n_rx, 1, 0);
    if(n_res != 1 || n_rx != (uint8_t)Response::ACK) return -1;
        
    // Transmit address
    n_res = transmit(n_tx, 5, 2);
    if(n_res != 5) return -19;
    usleep(500);
    n_res = receive(&n_rx, 1, 0);
    if(n_res != 1 || n_rx != (uint8_t)Response::ACK) return -2;


    // Transmit data
    n_res = transmit(&n_tx[7], 1, 0);
    if(n_res != 1) return -20;

    n_res = transmit(buffer, length, offset);
    if(n_res != length) return -21;

    n_res = transmit(&n_tx[8], 1, 0);
    if(n_res != 1) return -22;
    usleep(500);
    n_res = receive(&n_rx, 1, 0);

    if(n_res != 1 || n_rx != (uint8_t)Response::ACK) return -3;
    return 0;
}

int STMBoot::read_addr(uint32_t address, uint8_t *buffer, int offset, int length){
    return 0;
}

int STMBoot::go(uint32_t address){
    return 0;
}

int STMBoot::reboot(){
    uint8_t n_rx;
    uint8_t n_tx[2];
    n_tx[0] = (uint8_t)Commands::REBOOT;
    n_tx[1] = calcLrc(&n_tx[0], 0, 1);

    int n_res = transmit(n_tx, 2, 0);
    if(n_res != 2) return -1;
    usleep(100);
    n_res = receive(&n_rx, 1, 0);
    if(n_res != 1 || n_rx != (uint8_t)Response::ACK) return -1;

    return 0;
}
