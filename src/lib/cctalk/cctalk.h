/**
 * @file cctalk.h
 * @author Lars Hederidder (coder@worldwidewhat.dk)
 * @brief CCTalk protocol interface
 * @version 0.1
 * @date 2021-09-03
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef _CCTALK_H_
#define _CCTALK_H_
#include "../uart/serial.h"
#include <vector>
#include "cctalkpackage.h"

class CCTalk : public Serial {

    public:

    /** @brief CCTalk headers/commands */
    enum class Header{
        FactorySetupTest                = 255,
        SimplePoll                      = 254, /*!< Core command */
        AddressPoll                     = 253,
        AddressClash                    = 252,
        AddressChange                   = 251,
        AddressRandom                   = 250,
        RequestPollPriority             = 249,
        RequestStatus                   = 248,
        RequestVariableSet              = 247,
        RequestManufactId               = 246, /*!< Core command */
        RequestEquiptCatId              = 245, /*!< Core command */
        RequestProductCode              = 244, /*!< Core command */
        RequestDatabaseVer              = 243,
        RequestSerialNo                 = 242,
        RequestSoftwareVer              = 241,
        TestSolenoids                   = 240,
        OperateMotors                   = 239,
        TestOutputLines                 = 238,
        ReadInputLines                  = 237,
        ReadOptoStates                  = 236,
        ReadLastCreditOrErr             = 235, // Obsolete
        IssueGuardCode                  = 234, // Obsolete
        LatchOutputLines                = 233,
        PerformSelfCheck                = 232,
        ModifyInhibitStatus             = 231,
        RequestInhibitStatus            = 230,
        ReadBuffCreditOrErr             = 229,
        ModifyMasterInhibit             = 228,
        RequestMasterInhibit            = 227,
        RequestInsertionCounter         = 226,
        RequestAcceptCounter            = 225,
        DispenseCoins                   = 224, // Obsolete
        DispenseChange                  = 223, // Obsolete
        ModifySorterOverride            = 222,
        RequestSorterOverride           = 221,
        OneShotCredit                   = 220, // Obsolete
        EnterNewPinNo                   = 219,
        EnterPinNo                      = 218,
        RequestPayoutStatus             = 217,
        RequestDataStorageAvail         = 216,
        ReadDataBlock                   = 215,
        WriteDataBlock                  = 214,
        RequestOptionFlags              = 213,
        RequestCoinPosition             = 212,
        PowerManagementControl          = 211,
        ModifySorterPaths               = 210,
        RequestSorterPaths              = 209,
        ModifyPayoutAbsCount            = 208,
        RequestPayoutAbsCount           = 207,
        EmptyPayout                     = 206,
        RequestAuditInfoBlock           = 205,
        MeterControl                    = 204,
        DisplayControl                  = 203,
        TeachModeControl                = 202,
        RequestTeachStatus              = 201,
        UploadCoinData                  = 200, // Obsolete
        ConfigToEeprom                  = 199,
        CountersToEeprom                = 198,
        CalcRomCrc                      = 197,
        RequestCreationDate             = 196,
        RequestLastModifyDate           = 195,
        RequestRejectCounter            = 194,
        RequestFraudCounter             = 193,
        RequestBuildCode                = 192, // Core
        KeypadControl                   = 191,
        //RequestPayoutStatus           = 190, // Obsolete
        ModifyDefaultSortPath           = 189, // Coin
        RequestDefaultSortPath          = 188, // Coin
        ModifyPayoutCapacity            = 187, // Payout
        RequestPayoutCapacity           = 186, // Payout
        ModifyCoinId                    = 185, // Coin
        RequestCoinId                   = 184, // Coin
        UploadWindowData                = 183, // Coin
        DownloadCalibationInfo          = 182, // Coin
        ModifySecuritySetting           = 181, // Coin, Bill
        RequestSecuritySetting          = 180, // Coin, Bill
        ModifyBankSelect                = 179, // Coin, Bill
        RequestBankSelect               = 178, // Coin, Bill
        HandheldFunction                = 177, // Coin
        RequestAlarmCounter             = 176, // Coin
        ModifyPayoutFloat               = 175, // Payout
        RequestPayoutFloat              = 174, // Payout
        RequestThremistorRead           = 173, // Coin
        EmergencyStop                   = 172, // Payout
        RequestHopperCoin               = 171, // Payout
        RequestBaseYear                 = 170, // Coin, Bill
        RequestAddressMode              = 169, // Coin, Payout, Bill
        RequestHopperDispenseCount      = 168, // Payout
        DispenseHopperCoins             = 167, // Payout
        RequestHopperStatus             = 166, // Payout
        ModifyVariableSet               = 165, // Payout
        EnableHopper                    = 164, // Payout
        TestHopper                      = 163, // Payout
        ModifyInhibitOverrideReg        = 162, // Coin
        PumpRNG                         = 161, // Payout
        RequestCipherKey                = 160, // Payout

        /* 136 - 159 PURE BILL                        */

        SetAcceptLimit                  = 135, // Coin
        DispenseHopperValue             = 134, // Payout
        RequestHopperPollingValue       = 133, // Payout
        EmergencyStopValue              = 132, // Payout
        RequestHopperCoinValue          = 131, // Payout
        RequestIndexHopperDispCount     = 130, // Payout

        RequestMoneyIn                  = 128, // Escrow/Changer
        RequestMoneyOut                 = 127, // Escrow/Changer
        ClearMoneyCounter               = 126, // Escrow/Changer
        PayMoneyOut                     = 125, // Escrow/Changer
        VerifyMoneyOut                  = 124, // Escrow/Changer
        RequestActivityReqister         = 123, // Escrow/Changer
        RequestErrorStatus              = 122, // Escrow/Changer
        PurgeHopper                     = 121, // Escrow/Changer
        ModifyHopperBalance             = 120, // Escrow/Changer
        RequestHopperBalance            = 119, // Escrow/Changer
        ModifyCashboxValue              = 118, // Escrow/Changer
        RequestCashboxValue             = 117, // Escrow/Changer
        ModifyRealtimeClock             = 116, // Escrow/Changer
        RequestRealtimeClock            = 115, // Escrow/Changer

        RequestUsbID                    = 114, // Core+
        SwitchBaudRate                  = 113, // Core+

        ReadEncryptedEvents             = 112, // Coin, Bill
        RequestEncryptedSupport         = 111, // Core
        SwitchEncryptionKey             = 110, // Core+
        RequestEncryptedHopperStatus    = 109, // Payout
        RequestEncryptedMonetaryID      = 108, // Coin
        OperateEscrow                   = 107, // Escrow/Changer
        RequestEscrowStatus             = 106, // Escrow/Changer
        DataStream                      = 105, // Core+
        RequestServiceStatus            = 104, // Escrow/Changer
        ExpansionHeader4                = 103,
        ExpansionHeader3                = 102,
        ExpansionHeader2                = 101,
        ExpansionHeader1                = 100,

        /* 20 - 99 Application specific */

        BUSYmessage                     = 6, // Core+
        NAKmessage                      = 5, // Core+
        RequestCommsRevision            = 4, // Core+
        ClearCommsStatusVariables       = 3, // Coin, Payout, Bill
        RequestCommsStatusVariables     = 2, // Coin, Payout, Bill
        ResetDevice                     = 1, // Core+
        ReturnMessage                   = 0     
    };

    /**
     * @brief CCTalk Event object from ReadBuffCreditOrErr list 
     */
    class CCT_Event {
        public:
        CCT_Event(uint8_t eventType=0, uint8_t eventValue=0) : event_Type(eventType), event_Value(eventValue){}
        uint8_t event_Type; // If 0 then event_Value is an error code, otherwise event_Type is a coin ID number
        uint8_t event_Value; // Error code or coin output number
    };

    /** @brief CCTalk event stack object for storing data from ReadBuffCreditOrErr */
    class EventStack{
        public:
        EventStack() : lastEventId(0){}
        uint8_t lastEventId;    // Last marked event id (if 0 then the havent had any errors)
        std::vector<CCT_Event> events;  // Event object list
    };


    public:
    /**
     * @brief Construct a new CCTalk object
     * 
     * @param id CCTalk ID for this object
     */
    CCTalk(const uint8_t id);
    ~CCTalk();

    /**
     * @brief Get the Event Stack object from device
     * 
     * @param receiverID Id the device that should respond transmission
     * @param eventStack Refrence to the eventStack object
     * @return Result 
     */
    int getEventStack(const uint8_t receiverID, EventStack &eventStack);

    /**
     * @brief Transmit CCTalk message
     * 
     * @param package Message to transmit
     * @return Result
     */
    int transmitPackage(CCTalkPackage &package);

    /**
     * @brief Receive a message
     * 
     * @param package Reference to message object to place received data in
     * @return Result
     */
    int receivePackage(CCTalkPackage &package);

    /**
     * @brief Transmit message and receive reply
     * 
     * @param transmit Message object to transmit
     * @param reply Reference to message object to place received data in
     * @return Result
     */
    int transmitPackageWithReply(CCTalkPackage &transmit, CCTalkPackage &reply);

    /**
     * @brief Calculate CRC value
     * 
     * @param package Package object to calculate CRC value from
     * @return Calculated value
     */
    uint8_t calcCrc(const CCTalkPackage &package);

    private:
    const uint8_t _id;

    protected:
};

#endif //_CCTALK_H_