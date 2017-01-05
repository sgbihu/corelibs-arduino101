
#include "CurieBLE.h"

// LED pin
#define LED_PIN   13

// create service
BLEService          bigtrunkService("19b10100e8f2537e4f6cd104768a1214");
BLECharacteristic   bigtrunkCharacteristic("19b10101e8f2537e4f6cd104768a1214", BLERead | BLEWrite | BLENotify, 20);
unsigned char trunk_data[1024];

void init_trunk_data()
{
    static unsigned char offset = 0;
    for (int i = 0; i < (int)sizeof(trunk_data); i++)
    {
        trunk_data[i] = (unsigned char)(i + offset);
    }
    offset++;
}

void send_trunk_data(unsigned char *data, int length)
{
    int i = 0;
    unsigned char send_length = 16;
    unsigned char send_buffer[20];
    
    Serial.print(F("Total send length: "));
    Serial.println(length);
    while (i < length)
    {
        // Send 16byte per cycle
        // 
        int send_index = i / 16;
        int reserve_len = length - i;
        if (reserve_len > 16)
        {
            send_length = 16;
        }
        else
        {
            send_length = reserve_len;
        }
        send_buffer[0] = (unsigned char) send_index & 0xFF;
        send_buffer[1] = (unsigned char) (send_index >> 8) & 0xFF;
        send_buffer[2] =  send_length;
        
        memcpy(&send_buffer[3], &data[i], send_length);
        send_length += 3;// Increase index bytes and length
        bigtrunkCharacteristic.writeValue(send_buffer, send_length);
        
        Serial.print(F("Currnt send index: "));
        Serial.println(i);
        i += 16;
        delay(40);// TODO: need to twist
    }
    Serial.println(F("Send Complete"));
}

void setup() {
    Serial.begin(9600);
    Serial.println("test---");
    
    // set LED pin to output mode
    pinMode(LED_PIN, OUTPUT);

    // begin initialization
    BLE.begin();
    Serial.println(BLE.address());
    
    // set advertised local name and service UUID
    BLE.setLocalName("BigTrunk");
    
    // add service and characteristic
    BLE.addService(bigtrunkService);
    bigtrunkService.addCharacteristic(bigtrunkCharacteristic);
    
    init_trunk_data();
    
    BLE.advertise();
}

void loop() {
    BLEDevice central = BLE.central();
    bool temp = central;
    if (temp)
    {
        // central connected to peripheral
        Serial.print(F("Connected to central: "));
        Serial.println(central.address());

        while (central.connected()) 
        {
            // central still connected to peripheral
            if (bigtrunkCharacteristic.subscribed()) 
            {
                send_trunk_data(trunk_data, sizeof(trunk_data));
                delay(5000);
            }
        }

        // central disconnected
        Serial.print(F("Disconnected from central--: "));
        Serial.println(central.address());
    }
}
