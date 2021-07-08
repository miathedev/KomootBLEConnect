#include "Arduino.h"
#include "KomootBLEConnect.h"

//When data is received from Komoot, this callback is triggered. Start your processing here
void NavCallback(uint8_t icon, uint32_t length, String street)
{
  Serial.printf("icon: %d, length %d, street: %s\n", icon,length,street);
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting Arduino BLE Client application...");
  Komoot::BLEHandler::Begin("ESP32 Komoot");
  Komoot::BLEHandler::RegisterCallback(NavCallback);
}


void loop()
{
  Komoot::BLEHandler::Loop(); //Scans for BLE devices that serve the BLEConnect Service and connects to them IF paired
  delay(1000);
}
