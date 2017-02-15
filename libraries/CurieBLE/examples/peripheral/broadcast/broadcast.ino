#include <CurieBLE.h>

short value = 0;

BLEService service = BLEService("EEE0");
BLEShortCharacteristic characteristic = BLEShortCharacteristic("EEE1", BLERead | BLENotify | BLEBroadcast);

void setup() {
  Serial.begin(9600);

  BLE.setLocalName("Broadcast");
  BLE.setAdvertisedServiceUuid(service.uuid());

  BLE.addService(service);
  service.addCharacteristic(characteristic);

  characteristic.setValue(value);

  BLE.begin();
  characteristic.broadcast();

  Serial.println(F("BLE Broadcast Test Sketch"));
}

void loop() {
    BLE.poll();
    characteristic.setValue(value);    
    delay(1000);
    value++;
}


