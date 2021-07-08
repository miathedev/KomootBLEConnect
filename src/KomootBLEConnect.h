#include <Arduino.h>
#include "BLEDevice.h"
namespace Komoot
{
    // The remote service we wish to connect to.
    static BLEUUID serviceUUID("71c1e128-d92f-4fa8-a2b2-0f171db3436c");
    // The characteristic of the remote service we are interested in.
    static BLEUUID charUUID("503DD605-9BCB-4F6E-B235-270A57483026");

    class BLEHandler
    {
        class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
        {
            void onResult(BLEAdvertisedDevice advertisedDevice)
            {
                Serial.print("BLE Advertised Device found: ");
                Serial.println(advertisedDevice.toString().c_str());

                if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID))
                {

                    BLEDevice::getScan()->stop();
                    myDevice = new BLEAdvertisedDevice(advertisedDevice);
                    doConnect = true;
                    doScan = true;
                }
            }
        };

        class MyClientCallback : public BLEClientCallbacks
        {
            void onConnect(BLEClient *pclient)
            {
                Serial.println("onConnect Callback");
            }

            void onDisconnect(BLEClient *pclient)
            {
                connected = false;
                Serial.println("onDisconnect Callback");
            }
        };

    public:
        static void Begin(String name);
        static void Loop();
        static uint8_t GetIcon();
        static uint32_t GetLength();
        static String GetStreet();

        typedef std::function<void(uint8_t icon, uint32_t length, String street)> komoot_nav_notify_callback;
        static void RegisterCallback(komoot_nav_notify_callback callBackFkt);
        

    private:
        static void NotifyCallback(BLERemoteCharacteristic *pBLERemoteCharacteristic,
                                   uint8_t *pData,
                                   size_t length,
                                   bool isNotify);

        static BLERemoteCharacteristic *pRemoteCharacteristic;
        static BLEAdvertisedDevice *myDevice;

        static bool ConnectToServer();
        static boolean doConnect;
        static boolean connected;
        static boolean doScan;
        static uint8_t lastIcon;
        static uint32_t lastLength;
        static String lastStreet;
        static komoot_nav_notify_callback myNotifyCallback;
        static boolean callbackRegistered;
    };
}