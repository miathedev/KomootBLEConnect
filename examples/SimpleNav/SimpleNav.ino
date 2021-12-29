#include <Arduino.h>
#include <KomootBLEConnect.h>
#include "BLEDevice.h"

Komoot komoot;

void setup() {
    Serial.begin(115200);
    Serial.println("Komoot Simple Example");

    komoot.begin("BLEDEV");
}

void loop() {
    komoot.loop();
    if(komoot.isConnected()){
        KomootPayloadContainer dataSet = komoot.getDataSet();
        Serial.println(dataSet.street);
    }
}

