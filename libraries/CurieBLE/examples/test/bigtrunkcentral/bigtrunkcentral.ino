
#include "CurieBLE.h"

// LED pin
#define LED_PIN   13

unsigned char trunk_data[1024];
#define SHIFT 3
#define MASK 0x07
#define N 64
#define BITSPERWORD 8

#define bit_set(a, i) \
{\
  a[i>>SHIFT] |= (1<<(i & MASK));\
}

#define bit_clr(a, i)\
{\
  a[i>>SHIFT] &= ~(1<<(i & MASK));\
}

void bigtrunkCharacteristicEventHandler(BLEDevice bledev, BLECharacteristic characteristic)
{
    const unsigned char *datas = characteristic.value();
    const int data_length = characteristic.valueLength();
    int index = 0;
    int copy_length = 0;
    static unsigned char bitmap[N/BITSPERWORD];
    if (data_length < 3)
    {
        // The index has 2 Bytes and 1 Byte for length
        return;
    }
    index = datas[0] | (datas[1] << 8);
    
    // Only for test to get the lost packet
    bit_set(bitmap, index);
    
    copy_length = datas[2];
    if (copy_length + 3 != data_length)
    {
        return;
    }
    index *= 16;
    if (index >= (int)sizeof(trunk_data))
    {
        return;
    }
    memcpy(&trunk_data[index], &datas[3], copy_length);
    
    // Only for test to get the lost packet
    if (index + copy_length == sizeof(trunk_data))
    {
        Serial.println("Completed");
        Serial.print("Index bit map:");
        for (int j = 0; j < sizeof(bitmap); j++)
        {
            Serial.print(bitmap[j], HEX);
        }
        Serial.println("");
        memset(bitmap, 0, sizeof(bitmap));
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("test---");
    
    // set LED pin to output mode
    pinMode(LED_PIN, OUTPUT);

    // begin initialization
    BLE.begin();
    Serial.println(BLE.address());
    
    BLE.scanForName("BigTrunk");
}

void controlLed(BLEDevice &peripheral)
{
    // connect to the peripheral
    Serial.print("Connecting ... ");
    Serial.println(peripheral.address());

    if (peripheral.connect())
    {
        Serial.print("Connected: ");
        Serial.println(peripheral.address());
    }
    else
    {
        Serial.println("Failed to connect22!");
        return;
    }
    
    peripheral.discoverAttributes();
    
    BLECharacteristic bigtrunkCharacteristic = peripheral.characteristic("19b10101-e8f2-537e-4f6c-d104768a1214");
    
    if (!bigtrunkCharacteristic)
    {
        peripheral.disconnect();
        Serial.println("Peripheral does not have big trunk characteristic!");
        delay(5000);
        return;
    }
    
    bigtrunkCharacteristic.subscribe();
      
    bigtrunkCharacteristic.setEventHandler(BLEValueUpdated, bigtrunkCharacteristicEventHandler);

    while (peripheral.connected())
    {
    }
    Serial.print("Disconnected");
    Serial.println(peripheral.address());
}

void loop() {
    BLEDevice peripheral = BLE.available();
    if (peripheral) 
    {
        Serial.println(peripheral.address());
        BLE.stopScan();
        delay (1000);
        // central connected to peripheral
        controlLed(peripheral);
        delay (4000);
        BLE.scanForName("BigTrunk");
    }
}


