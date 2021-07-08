#include "KomootBLEConnect.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>

namespace Komoot
{
    boolean BLEHandler::doConnect = false;
    boolean BLEHandler::connected = false;
    boolean BLEHandler::doScan = false;

    uint8_t BLEHandler::lastIcon = -1;
    uint32_t BLEHandler::lastLength = -1;
    String BLEHandler::lastStreet;

    BLERemoteCharacteristic *BLEHandler::pRemoteCharacteristic;
    BLEAdvertisedDevice *BLEHandler::myDevice;

    BLEHandler::komoot_nav_notify_callback BLEHandler::myNotifyCallback = nullptr;
    boolean BLEHandler::callbackRegistered = false;

    void BLEHandler::Begin(String name)
    {
        lastStreet.reserve(100);
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
        pBLEScan->start(5, false);
    }

    void BLEHandler::Loop()
    {

        if (doConnect == true)
        {
            if (ConnectToServer())
            {
                Serial.println("We are now connected to the BLE Server.");
            }
            else
            {
                Serial.println("We have failed to connect to the server; there is nothin more we will do.");
            }

            doConnect = false;
        }

        // If we are connected to a peer BLE Server, update the characteristic each time we are reached
        // with the current time since boot.
        if (doScan && connected == false)
        {
            Serial.println("Scanning!");
            BLEDevice::getScan()->start(60); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
        }
    }

    bool BLEHandler::ConnectToServer()
    {
        Serial.print("Forming a connection to ");
        Serial.println(myDevice->getAddress().toString().c_str());
        BLEDevice::setMTU(127);
        BLEClient *pClient = BLEDevice::createClient();

        Serial.println(" - Created client");
        Serial.print("MTU: ");
        Serial.println(pClient->getMTU());

        pClient->setClientCallbacks(new MyClientCallback());

        // Connect to the remove BLE Server.
        pClient->connect(myDevice); // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
        Serial.println(" - Connected to server");
        //pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
        //pClient->getMTU
        // Obtain a reference to the service we are after in the remote BLE server.
        BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
        //pClient->getMTU
        if (pRemoteService == nullptr)
        {
            Serial.print("Failed to find our service UUID: ");
            Serial.println(serviceUUID.toString().c_str());
            pClient->disconnect();
            return false;
        }
        Serial.println(" - Found our service");

        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
        if (pRemoteCharacteristic == nullptr)
        {
            Serial.print("Failed to find our characteristic UUID: ");
            Serial.println(charUUID.toString().c_str());
            pClient->disconnect();
            return false;
        }
        Serial.println(" - Found our characteristic");

        if (pRemoteCharacteristic->canNotify())
            pRemoteCharacteristic->registerForNotify(NotifyCallback);

        connected = true;
        return true;
    }

    void BLEHandler::RegisterCallback(komoot_nav_notify_callback callBackFkt)
    {
        if (callBackFkt != nullptr)
        {
            myNotifyCallback = callBackFkt;
            callbackRegistered = true;
        }
        else
        {
            Serial.println("Callback cant be a nullptr!");
        }
    }
    void BLEHandler::NotifyCallback(
        BLERemoteCharacteristic *pBLERemoteCharacteristic,
        uint8_t *pData,
        size_t length,
        bool isNotify)
    {
        //Serial.print("Notify callback for characteristic ");
        //Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
        //Serial.print(" of data length ");
        //Serial.println(length);
        //Serial.flush();

        /*
        for (int i = 0; i < length; i++)
        {
            Serial.flush();
            printf("\nvalue %d: %#X", i, pData[i]);
        }
        */

        //https://www.binaryconvert.com/result_unsigned_int.html?hexadecimal=00000014

        union StreckeUnion
        {
            uint32_t real;
            char raw[4];
        } strecke;

        Serial.flush();
        strecke.raw[0] = pData[5]; //For-loop possible, but would only save one line! And its not as good readable
        strecke.raw[1] = pData[6];
        strecke.raw[2] = pData[7];
        strecke.raw[3] = pData[8];

        int expectedStreetStringLength = length - 9;

        //Serial.println("Expected length");
        //Serial.println(expectedStreetStringLength);
        char *subbuff;
        subbuff = (char *)malloc(expectedStreetStringLength);
        //char subbuff[expectedStreetStringLength];

        int subCnt = 0;
        for (int i = 9; i < length; i++)
        {
            subbuff[subCnt] = pData[i];
            subCnt++;
        }
        subbuff[subCnt] = '\0';

        //Serial.print("Icon: ");
        //Serial.println(pData[4]);
        lastIcon = pData[4];

        //Serial.print("Strasse: ");
        //Serial.println(subbuff);
        lastStreet = subbuff;

        //Serial.print("Laenge: ");
        //Serial.print(strecke.real);
        //Serial.println("m");
        //Serial.println("-------");
        lastLength = strecke.real;

        if (callbackRegistered == true)
        {
            //Serial.println("Notify!");
            myNotifyCallback(lastIcon, lastLength, lastStreet);
        }
        else
        {
            Serial.print("Callback already registered");
        }
    }

    uint8_t BLEHandler::GetIcon()
    {
        return lastIcon;
    }

    uint32_t BLEHandler::GetLength()
    {
        return lastLength;
    }

    String BLEHandler::GetStreet()
    {
        return lastStreet;
    }
}
