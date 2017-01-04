
#include "CurieBLE.h"
#include <errno.h>
// LED pin
#define LED_PIN   13
BLEService          ledService("19b10100e8f2537e4f6cd104768a1214");

BLECharacteristic   switchCharacteristic("19b10101e8f2537e4f6cd104768a1214", BLERead | BLEWrite | BLENotify, 1);

BLEDescriptor       switchDescriptor("2901", "switch");

void setup() {
    Serial.begin(115200);
    
    // set LED pin to output mode
    pinMode(LED_PIN, OUTPUT);

    // begin initialization
    BLE.begin();
    Serial.println(BLE.address());
    ledService.addCharacteristic(switchCharacteristic);
    switchCharacteristic.addDescriptor(switchDescriptor);
    BLE.addService(ledService);
	
    BLE.scanForName("LED");
}


void loop() {
    BLEDevice peripheral = BLE.available();
    if (peripheral) 
    {
        Serial.println(peripheral.address());
		
        BLE.stopScan();
		
		if (peripheral.connect())
		{
			Serial.print("Connected: ");
			Serial.println(peripheral.address());
			while (peripheral.connected())
			{
				delay (1000);
			}
		}
		else
		{
			Serial.println("Failed to connect!");
		}
        delay (4000);
        BLE.scanForName("LED");
    }
}


