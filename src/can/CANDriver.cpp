//
// Created by Markus on 31.03.21.
//

#include "can/CANDriver.h"

#include <utility>
#include <string>

canStatus InitializeCANChannel(uint32_t canChannelID);
char* CANError(canStatus status);

CANDriver::CANDriver(std::function<void(uint32_t, uint8_t *, uint32_t, uint64_t)> onInitRecvCallback,
                     std::function<void(uint32_t, uint8_t *, uint32_t, uint64_t)> onRecvCallback, std::function<void(char *)> onErrorCallback)
                     : onRecvCallback(std::move(onRecvCallback)), seqRecvCallback(std::move(onRecvCallback)), onErrorCallback(std::move(onErrorCallback))
{
    canStatus stat;
    for (size_t i = 0; i < CAN_CHANNELS; i++)
    {
        if((stat = InitializeCANChannel(i)) < 0) { 
            std::ostringstream stringStream;
            stringStream << "CAN-Channel " << i << ": " << CANError(stat);
            throw std::runtime_error(stringStream.str());
        }
    }
    
}

CANDriver::~CANDriver()
{
    // Empty transfer queues (not strictly necessary but recommended by Kvaser)
    for (size_t i = 0; i < CAN_CHANNELS; i++)
    {
        (void) canWriteSync(canHandles[i], 5);
        (void) canBusOff(canHandles[i]);
        (void) canClose(canHandles[i]);
    }
    canUnloadLibrary();
}

void CANDriver::InitDone(void)
{
    onRecvCallback = seqRecvCallback;
}

void CANDriver::SendCANMessage(uint32_t canChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{
    // Flags mean that the message is a FD message (FDF, BRS) and that an extended id is used (EXT)
    canStatus stat = canWrite(canHandles[canChannelID], canID, (void *) payload, payload_length, canFDMSG_FDF | canFDMSG_BRS | canMSG_EXT);

    if(stat < 0) {
        throw std::runtime_error(CANError(stat));
    }
}

std::map<std::string, bool> CANDriver::GetCANStatusReadable(uint32_t canChannelID)
{
    std::map<std::string, bool> CANState;
    uint64_t flags;
    canStatus stat = canReadStatus(canHandles[canChannelID], &flags);

    if(stat == canOK) {
        uint8_t length = 8;
        std::string names[length] = {"ERROR_PASSIVE", "BUS_OFF", "ERROR_WARNING", "ERROR_ACTIVE",
                             "TX_PENDING", "RX_PENDING", "RESERVED_1", "TXERR", "RXERR", "HW_OVERRUN",
                             "SW_OVERRUN", "OVERRUN"};

        for(uint8_t i = 0; i < length; i++){
            CANState.insert(names[i], (bool)(state & (1 << i)));
        }
    }

    return CANState;
}

void CANDriver::OnCANCallback(int handle, void *context, unsigned int event)
{
    canStatus stat;
    uint32_t id, dlc, flags;
    uint8_t data[64];
    uint64_t timestamp;
    
    // As the callback only gets entered when the receive queue was empty, empty it in here
    do {
        stat = canRead(hnd, &id, data, &dlc, &flags, &timestamp);

        switch(event) {
            case canNOTIFY_ERROR:
                onErrorCallback(CANError(stat));
            break;
            case canNOTIFY_RX:
                onRecvCallback(&id, data, &dlc, &timestamp);
            break;
            default:
                throw std::runtime_error("Callback got called with neither ERROR nor RX, gigantic UFF");
            break;
        }

    } while(stat == canOK);

    // stat is either canERR_NOMSG or any different error code
    if(stat != canERR_NOMSG) {
        throw std::runtime_error(CANError(stat));
    }
}


char* CANError(canStatus status); {
    char msg[64];
    canGetErrorText(status, msg, sizeof msg);
    return msg;
}

canStatus InitializeCANChannel(uint32_t canChannelID) {
    canStatus stat;
    canInitializeLibrary();
    // TODO: Might want to remove canOPEN_ACCEPT_VIRTUAL later (DB)
    canHandles[canChannelID] = canOpenChannel(channel, canOPEN_EXCLUSIVE | canOPEN_CAN_FD | canOPEN_ACCEPT_LARGE_DLC | canOPEN_ACCEPT_VIRTUAL);
    if(canHandles[canChannelID] < 0){
        return stat;
    }

    stat = canSetBusParamsFdTq(canHandles[canChannelID], canFdParams);
    if(stat < 0) {
        return stat;
    }

    stat = canSetBusOutputControl(canHandles[canChannelID], canDRIVER_NORMAL);
    if(stat < 0) {
        return stat;
    }

    stat = canBusOn(canHandles[canChannelID]);
    if(stat < 0) {
        return stat;
    }

    // Register callback for receiving a msg when the rcv buffer has been empty or when an error frame got received
    stat = kvSetNotifyCallback(canHandles[canChannelID], (kvCallback_t) &OnCANCallback, NULL, canNOTIFY_RX | canNOTIFY_ERROR);
    if(stat < 0) {
        return stat;
    }

    // Filter messages with direction=0
    /* stat = canAccept(canHandles[canChannelID], 0x1, canFILTER_SET_MASK_EXT);
    if(stat < 0) {
        throw new std::runtime_error(CANError(stat));
    }

    stat = canAccept(canHandles[canChannelID], 0x1, canFILTER_SET_CODE_EXT);
    if(stat < 0) {
        throw new std::runtime_error(CANError(stat));
    } */
}
