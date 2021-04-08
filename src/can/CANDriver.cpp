//
// Created by Markus on 31.03.21.
//

#include "can/CANDriver.h"

#include <utility>
#include <string>

CANDriver::CANDriver(std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onInitRecvCallback,
                     std::function<void(uint8_t &, uint32_t &, uint8_t *, uint32_t &, uint64_t &)> onRecvCallback,
                     std::function<void(std::string)> onErrorCallback,
                     kvBusParamsTq arbitrationParams,
                     kvBusParamsTq dataParams)
                     : onRecvCallback(std::move(onInitRecvCallback)),
                     seqRecvCallback(std::move(onRecvCallback)),
                     onErrorCallback(std::move(onErrorCallback)),
                     arbitrationParams(arbitrationParams), dataParams(dataParams)
{
    canStatus stat;
    for (size_t i = 0; i < CAN_CHANNELS; i++)
    {
        stat = InitializeCANChannel(i);
        if (stat < 0) {
            std::ostringstream stringStream;
            stringStream << "CANDriver - Constructor: CAN-Channel " << i << ": " << CANError(stat);
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
    //TODO: MP mutual exclusion needed if implemented this way
    onRecvCallback = seqRecvCallback;
}

void CANDriver::SendCANMessage(uint32_t canChannelID, uint32_t canID, uint8_t *payload, uint32_t payloadLength)
{
    // Flags mean that the message is a FD message (FDF, BRS) and that an extended id is used (EXT)
    canStatus stat = canWrite(canHandles[canChannelID], canID, (void *) payload, payloadLength, canFDMSG_FDF | canFDMSG_BRS | canMSG_EXT);

    if(stat < 0) {
        throw std::runtime_error(CANError(stat));
    }
}

std::map<std::string, bool> CANDriver::GetCANStatusReadable(uint32_t canBusChannelID)
{
    std::map<std::string, bool> canState;
    uint64_t flags;
    canStatus stat = canReadStatus(canHandles[canBusChannelID], &flags);

    if(stat == canOK) {
        std::vector<std::string> names = {"ERROR_PASSIVE", "BUS_OFF", "ERROR_WARNING", "ERROR_ACTIVE",
                             "TX_PENDING", "RX_PENDING", "RESERVED_1", "TXERR", "RXERR", "HW_OVERRUN",
                             "SW_OVERRUN", "OVERRUN"};

        for(uint8_t i = 0; i < names.size(); i++){
            canState[names[i]] = (bool)(stat & (1 << i));
        }
    }

    return canState;
}

//TODO: MP maybe create 2 callbacks one for until init done and one for afterwards to eliminate bus channel id search
// if not needed
void CANDriver::OnCANCallback(int handle, void *context, unsigned int event)
{
    canStatus stat;
    int64_t id;
    uint32_t dlc, flags;
    uint8_t data[64];
    uint64_t timestamp;
    
    // As the callback only gets entered when the receive queue was empty, empty it in here
    do {
        stat = canRead(handle, &id, data, &dlc, &flags, &timestamp);

        if (id < 0)
        {
            Debug::error("CANDriver - OnCANCallback: id negative");
            return;
        }
        uint8_t canBusChannelID = -1;
        for (int i = 0; i < CAN_CHANNELS; i++)
        {
            if (handle == canHandles[i])
            {
                canBusChannelID = i;
            }
        }
        if (canBusChannelID == (uint8_t)-1)
        {
            Debug::error("CANDriver - OnCANCallback: can handle not found");
            return;
        }

        switch(event) {
            case canNOTIFY_ERROR:
                onErrorCallback(CANError(stat));
            break;
            case canNOTIFY_RX:
                onRecvCallback(canBusChannelID, (uint32_t &) id, data, dlc, timestamp);
            break;
            default:
                //TODO: MP since this thread is managed by the canlib, should we really throw exceptions?
                throw std::runtime_error("Callback got called with neither ERROR nor RX, gigantic UFF");
            break;
        }

    } while(stat == canOK);

    // stat is either canERR_NOMSG or any different error code
    if(stat != canERR_NOMSG) {
        throw std::runtime_error(CANError(stat));
    }
}


std::string CANDriver::CANError(canStatus status) {
    char msg[64];
    canGetErrorText(status, msg, sizeof msg);
    return msg;
}

canStatus CANDriver::InitializeCANChannel(uint32_t canBusChannelID) {
    canStatus stat;
    canInitializeLibrary();
    // TODO: Might want to remove canOPEN_ACCEPT_VIRTUAL later (DB)
    canHandles[canBusChannelID] = canOpenChannel(canBusChannelID, canOPEN_EXCLUSIVE | canOPEN_CAN_FD | canOPEN_ACCEPT_LARGE_DLC | canOPEN_ACCEPT_VIRTUAL);
    if(canHandles[canBusChannelID] < 0){
        return stat;
    }

    stat = canSetBusParamsFdTq(canHandles[canBusChannelID], arbitrationParams, dataParams);
    if(stat < 0) {
        return stat;
    }

    stat = canSetBusOutputControl(canHandles[canBusChannelID], canDRIVER_NORMAL);
    if(stat < 0) {
        return stat;
    }

    stat = canBusOn(canHandles[canBusChannelID]);
    if(stat < 0) {
        return stat;
    }

    // Register callback for receiving a msg when the rcv buffer has been empty or when an error frame got received
    stat = kvSetNotifyCallback(canHandles[canBusChannelID], (kvCallback_t) &CANDriver::OnCANCallback, NULL, canNOTIFY_RX | canNOTIFY_ERROR);
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
