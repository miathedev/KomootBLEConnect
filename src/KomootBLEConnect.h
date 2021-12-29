//
// Created by mime on 27.12.21.
//

#ifndef ESP32_KOMOOT_H
#define ESP32_KOMOOT_H
#include "Arduino.h"
#include "BLEDevice.h"

struct KomootPayloadContainer {
    union Length
    {
        uint32_t real;
        char raw[4];
    } length;
    String street;
    uint8_t icon;
};

class Komoot {
public:
    void loop();
    static void begin(const String& name);
    int getRSSI();
    boolean isConnected();

    enum BLEStates {
        IS_SCANNING,
        CONNECT_TO_DEVICE,
        CONNECT_TO_SERVICE,
        CONNECT_TO_CHARACTERISTIC,
        REGISTER_FOR_NOTIFICATION,
        CONNECTION_DONE
    };

private:
    static void notifyCallback(
            BLERemoteCharacteristic *pBLERemoteCharacteristic,
            uint8_t *pData,
            size_t length,
            bool isNotify);
    static BLEAdvertisedDevice *myDevice;

    static uint8_t bLEState;
    static KomootPayloadContainer dataSet;
public:
    static uint8_t getBleState();

private:
    static BLEClient *pClient;
    static BLERemoteService *pRemoteService;
    static BLERemoteCharacteristic *pRemoteCharacteristic;
    class MyAdvertisedDeviceCallbacks;

public:
    static const KomootPayloadContainer &getDataSet();
};


#endif //ESP32_KOMOOT_H
