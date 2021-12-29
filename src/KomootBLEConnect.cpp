//
// Created by mime on 27.12.21.
//

#include "KomootBLEConnect.h"

BLEUUID serviceUUID("71c1e128-d92f-4fa8-a2b2-0f171db3436c");
BLEUUID charUUID("503DD605-9BCB-4F6E-B235-270A57483026");

uint8_t Komoot::bLEState;
BLEClient *Komoot::pClient;
BLERemoteService *Komoot::pRemoteService;
BLEAdvertisedDevice *Komoot::myDevice;
KomootPayloadContainer Komoot::dataSet;
BLERemoteCharacteristic *Komoot::pRemoteCharacteristic;

void Komoot::loop() {
    if (bLEState == IS_SCANNING)
    {
        Serial.println("Scanning!");
        BLEDevice::getScan()->start(5, false);
        BLEDevice::getScan()->stop();
    } else if(bLEState == CONNECT_TO_DEVICE) {
        Serial.print("Forming a connection to ");
        Serial.println(myDevice->getAddress().toString().c_str());
        pClient = BLEDevice::createClient();
        if(!pClient->connect(myDevice)){
            Serial.println("Cant connect to device, scanning again");
            bLEState = IS_SCANNING;
        } else {
            Serial.println("Connection done");
            bLEState = CONNECT_TO_SERVICE;
        }
    } else if(bLEState == CONNECT_TO_SERVICE){
        Serial.print("Trying to connect to service");
        pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr){
            Serial.print("Failed to find our service UUID. Going back for scanning");
            pClient->disconnect();
            bLEState = IS_SCANNING;
        } else {
            Serial.println("Found our service");
            bLEState = CONNECT_TO_CHARACTERISTIC;
        }
    } else if(bLEState == CONNECT_TO_CHARACTERISTIC){
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr){
            Serial.print("Failed to find our characteristic UUID. Going back to scanning");
            pClient->disconnect();
            bLEState = IS_SCANNING;
        } else {
            Serial.println("Connected to service");
            bLEState = REGISTER_FOR_NOTIFICATION;
        }
    } else if(bLEState == REGISTER_FOR_NOTIFICATION){
        if(pRemoteCharacteristic->canNotify()){
            Serial.println("Registering for data notification");
            pRemoteCharacteristic->registerForNotify(Komoot::notifyCallback);
            bLEState = CONNECTION_DONE;
        } else {
            Serial.println("Remote characteristic cant notify. Going back to scan");
            bLEState = IS_SCANNING;
        }
    } else if(bLEState == CONNECTION_DONE){
        if(!pClient->isConnected()){
            //pClient->disconnect();
            Serial.println("Client is not connected anymore, going back for scanning");
            bLEState = IS_SCANNING;
        }
    }
}

class Komoot::MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) override
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
        {
            Serial.println("Found possile candidate");
            Komoot::myDevice = new BLEAdvertisedDevice(advertisedDevice);
            Komoot::bLEState = CONNECT_TO_DEVICE;
        }
    }
};

void Komoot::begin(const String& name) {
    bLEState = IS_SCANNING;
    dataSet.street = "WAIT";

    BLEDevice::init(name.c_str());
    BLEDevice::setMTU(127);
    BLEDevice::startAdvertising();
    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();

    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);

    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);

}

char subBuffer[22];
void Komoot::notifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic, uint8_t *pData, size_t length,
                            bool isNotify) {
    if(pBLERemoteCharacteristic->canRead()){
        //Serial.println("Test stuff");
        //uint8_t testStuff = pBLERemoteCharacteristic->readUInt8();
        //Serial.println(testStuff, HEX);


    } else {
        Serial.println("Cannot read");
    }
    Serial.println("Got notified!");
    dataSet.length.raw[0] = pData[5];
    dataSet.length.raw[1] = pData[6];
    dataSet.length.raw[2] = pData[7];
    dataSet.length.raw[3] = pData[8];

    int expectedStreetStringLength = length - 9;
    Serial.println("Expected Street Length");
    Serial.println(expectedStreetStringLength);

    int subCnt = 0;
    for (int i = 9; i < length; i++)
    {
        subBuffer[subCnt] = pData[i];
        subCnt++;
    }
    subBuffer[subCnt] = '\0';
    dataSet.street = subBuffer;

    dataSet.icon = pData[4];
}

int Komoot::getRSSI() {
    if(isConnected()){
        return pClient->getRssi();
    } else {
        return -1111;
    }
}

const KomootPayloadContainer &Komoot::getDataSet() {
    return dataSet;
}

boolean Komoot::isConnected() {
    if(pClient != nullptr && bLEState == CONNECTION_DONE){
        if(pClient->isConnected()){
            return true;
        }
    }
    return false;
}

uint8_t Komoot::getBleState() {
    return bLEState;
}
