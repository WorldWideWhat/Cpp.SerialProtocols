/**
 * @file stmboot.h
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief STM Bootloader handler / interface
 * @version 0.1
 * @date 2021-09-02
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef _STMBOOT_H_
#define _STMBOOT_H_

#include "../uart/serial.h"
#include <fstream>
#include <string>
#include <functional>

class STMBoot : public Serial {
    public:
    /** @brief Target devices */
    enum class Target {
        STM32_NATIVE    = 0x7F /* STM32 native bootloader inititalize command */

    };

    /** @brief STM Bootloader commands + custom commands */
    enum class Commands {
        GET             = 0x00,   /*!< Get version, and list of supported commands */
        GET_PROT        = 0x01,   /*!< Get version, and read protection status */
        GET_ID          = 0x02,   /*!< Get chip ID */
        READ            = 0x11,   /*!< Rread memory (max 256 bytes) */
        GO              = 0x21,   /*!< Jump to address in memory. This function is currently not available */
        WRITE           = 0x31,   /*!< Write data to memeory (max 256 bytes) */
        ERASE           = 0x43,   /*!< Erase memeory page wise (currently just erases all memory area) */
        EXT_ERASE       = 0x44,   /*!< Erase memory */
        WR_PROTECT      = 0x63,   /*!< Enable write protection */
        WR_UNPROTECT    = 0x73,   /*!< Disable write protection */
        RD_PROTECT      = 0x82,   /*!< Enable read protection */
        RD_UNPROTECT    = 0x92,   /*!< Disable read protection */
    };

    /** @brief Protocol reponses */
    enum class Response {
        ACK     = 0x79,
        NACK    = 0x1F
    };

    /** @brief File signatures */
    enum class Signature : uint32_t {
        
        SmartControllerFW       = 0x4354636F, // Smart controller signature
        SmartControllerSettings = 0x4354637F, // Smart controller settings signature
        SmartSensorFW           = 0xF000F85F, // Smart sensor signature
        SmartPackage            = 0xFEFEFEFE, // Smart package signature
        UNKNOWN                 = 0xFFFFFFFF // Unknown signature
    };

    /** @brief Binary file information header (binary packed) */
    struct Header
    {
        public:
        Signature signature;
        uint32_t date;
        uint32_t time;
        uint16_t build;
        uint8_t minor;
        uint8_t major;
        
    } __attribute__((packed));

    public:
    STMBoot();
    ~STMBoot();

    /**
     * @brief Initialize bootloader with target identifier.
     * 
     * @param target Target identifier
     * @return Success
     */
    int init(Target target);

    /**
     * @brief Set the Binary File to transfere
     * 
     * @param filepath Path to file
     * @return Success
     */
    int setBinaryFile(const std::string filepath);

    /**
     * @brief Get the Header from file
     * 
     * @param header Reference to header object
     * @return Success
     */
    int getHeader(Header &header);

    /**
     * @brief Program target device
     * 
     * @param verbose Verbose output to terminal
     * @return Success
     */
    int programTarget(bool verbose=false);    

    /**
     * @brief Set the Progress Callback object
     * 
     * @param callback Callback function
     */
    void setProgressCallback(std::function<void(uint32_t, uint32_t)> callback);

    private:
    /**
     * @brief Check if file has a CRC 
     * 
     * @return Result
     */
    int hasCrc();
    
    /**
     * @brief Check if file is of a known type
     * 
     * @return Result
     */
    int isKnownType();

    /**
     * @brief Load file content
     * 
     * @return Success
     */
    int loadFileContent();

    /**
     * @brief Write data to target
     * 
     * @param addr Write address
     * @param buffer Pointer to buffer
     * @param offset Offset in buffer
     * @param len Number of bytes to write
     * @return Success
     */
    int writeMemory(uint32_t addr, uint8_t *buffer, uint32_t offset, uint32_t len);

    /**
     * @brief Calculate LRC value
     * 
     * @param bffr Data buffer
     * @param offset Offset in buffer
     * @param len Number of bytes to calculate from
     * @param invert Invert result
     * @return Calculated value
     */
    uint8_t calcLrc(uint8_t* bffr, int offset=0, int len=1, bool invert=true);

    int get();

    int erase(uint8_t pageNo);
    int extendedErase();

    int write_addr(uint32_t address, uint8_t *buffer, int offset, int length);

    int read_addr(uint32_t address, uint8_t *buffer, int offset, int length);

    int go(uint32_t address);

    int reboot();

    private:
    std::string _bin_file_path;
    Header _header;
    uint8_t *_filecontent;
    long _content_size;
    std::function<void(uint32_t, uint32_t)> _progressCallback;
};

#endif //_STMBOOT_H_
