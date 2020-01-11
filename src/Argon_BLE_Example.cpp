/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#line 1 "/Users/markingle/Downloads/IoTCode/Argon_BLE_Example/src/Argon_BLE_Example.ino"
/*
 * Project Argon_BLE_Example
 * Description:
 * Author:
 * Date:
 */

#include "Particle.h"

// Debugging Setup Code
void onDataReceived_HistData(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);
void onDisconnect(const BlePeerDevice& peer, void* context);
void setup();
void loop();
int bin2hex(const byte *input, char *output, int len, int reverse);
#line 11 "/Users/markingle/Downloads/IoTCode/Argon_BLE_Example/src/Argon_BLE_Example.ino"
SerialLogHandler logHandler(LOG_LEVEL_TRACE);    //this means Log.info, Log.warn, Log.error will be sent over USB serial virtual com port

SYSTEM_MODE(SEMI_AUTOMATIC)     // for debugging. comment out normally. code will execute without attempting to connect to WiFi or cloud
#define led D7                  // blue LED pin on Boron board
#define White_LED D6
#define Button D5                
int loop_counter = 0;           //misc counter for debugging
byte data[10];
String LED_state;
//end Debugging Setup Code


// Bluetooth Setup Code

//UUID codes for the Services and Characteristics of the Xiaomi Flower Care Moisture Sensor 
//https://github.com/vrachieru/xiaomi-flower-care-api#protocol
const BleUuid serviceUuidRoot("0000FE95-0000-1000-8000-00805F9B34FB");  

//Variables for discovering and creating service objects
Vector<BleService>          ble_peer_all_services;
BleService                  ble_service;
char                        ble_service_uuid_string[40];
byte                        ble_service_value[250];
char                        ble_service_value_string[501];
  
Vector<BleCharacteristic>   ble_peer_all_chars;
BleCharacteristic           ble_char;
char                        ble_char_uuid_string[40];
byte                        ble_char_value[250];
char                        ble_char_value_string[501];
BlePeerDevice               peer;                                      

BleCharacteristic CIOT_RocksChar;
BleCharacteristic LED_StateChar;
BleCharacteristic ButtonChar;

const unsigned long SCAN_PERIOD_MS = 2000;   
unsigned long lastScan = 0;
const size_t SCAN_RESULT_MAX = 50;          
BleScanResult   scanResults[SCAN_RESULT_MAX];

int char_connections = 0;

void onDataReceived_BT(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context);


void onDataReceived_HistData(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
    for (size_t ii = 0; ii < len; ii++) {   
    }
}

void onDisconnect(const BlePeerDevice& peer, void* context){
    Log.info("Disconnect");
    char_connections = 0;
}

void setup() {
    //A callback function is required inorder to run code when data is recieved from a characteristic....note there is no callback needed for sending a value from the central
    CIOT_RocksChar.onDataReceived(onDataReceived_BT, NULL);
    
    pinMode(led, OUTPUT);
    pinMode(White_LED, OUTPUT);
    pinMode(Button, INPUT);
  
  Serial.begin(115200);

  BLE.onDisconnected(onDisconnect, NULL);

}


void loop() {

    if (!BLE.connected()){
        char_connections = 0;  //Set char_connections to zero until all services and characteritistics are discovered....see below
        Log.info("Scanning for peripherals....");                                            
        auto start_time = millis();                                       
        int scan_result_count = BLE.scan(scanResults, SCAN_RESULT_MAX);   //Scan is always used by the central
        auto finish_time = millis();  
        BLE.setScanTimeout(800);
        int total_time = finish_time - start_time;                          
        Log.info("Scan duration (ms): %i", total_time);    
        Log.info("BLE.scan found %d devices", scan_result_count);       

        for (int ii = 0; ii < scan_result_count; ii++) {
            
            BleUuid foundServiceUuid;

            //The BLE.scan provides the address, advertisingData, scanData, and rssi values for 
            size_t svcCount = scanResults[ii].advertisingData.serviceUUID(&foundServiceUuid, 1);

            Log.info("MAC: %02X:%02X:%02X:%02X:%02X:%02X | RSSI: %dBm Services: %d",
                    scanResults[ii].address[0], scanResults[ii].address[1], scanResults[ii].address[2],
                    scanResults[ii].address[3], scanResults[ii].address[4], scanResults[ii].address[5], scanResults[ii].rssi,
                    svcCount);

            String device_name = scanResults[ii].advertisingData.deviceName();
            Log.info(device_name);

            Log.info("Found %d services", svcCount);
            for (uint8_t i = 0; i < svcCount; i++) 
                {
                    Log.info("Service %d, UUID: %s", i,(const char*)foundServiceUuid.toString());     
                }
            if (svcCount > 0 && foundServiceUuid == "10AB"){
                peer = BLE.connect(scanResults[ii].address, true);
                if (peer.connected()){
                    BLE.stopScanning();
                    Log.info("Stopping BLE scan....");
                    peer.getCharacteristicByUUID(CIOT_RocksChar, BleUuid(0xFE95));
                    peer.getCharacteristicByUUID(LED_StateChar, BleUuid(0x1A01));
                    peer.getCharacteristicByUUID(ButtonChar, BleUuid(0x10EF));
                    digitalWrite(led, HIGH);
                } else {
                    Log.info("Connection Failed");
                }
                
            }
        }
    } else { //The code in this else section is just to show how you could get a list of all services or characteristics to find a specific one.  It only runs once.
            if (peer.connected() && char_connections == 0) {
                ble_peer_all_services = peer.discoverAllServices();
                if (ble_peer_all_services.size()) {
                    Log.info("Found %u Services", ble_peer_all_services.size());

                    //The code below is based on the follow forum topic - https://community.particle.io/t/after-successful-ble-connect-cannot-read-characteristic/52318/7
                    for (int i = 0; i < ble_peer_all_services.size(); i++){
                        bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[0], &ble_service_uuid_string[0],2,1);
                        Log.info("Service : 0x%s", ble_service_uuid_string);
                        if (ble_peer_all_services[i].UUID().type() == BleUuidType::LONG) {
                            bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[12], &ble_service_uuid_string[0], 4, 1);
                            ble_char_uuid_string[8] = '-';
                            bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[10], &ble_service_uuid_string[9], 2, 1);
                            ble_char_uuid_string[13] = '-';
                            bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[8], &ble_service_uuid_string[14], 2, 1);
                            ble_char_uuid_string[18] = '-';
                            bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[6], &ble_service_uuid_string[19], 2, 1);
                            ble_char_uuid_string[23] = '-';
                            bin2hex(&ble_peer_all_services[i].UUID().rawBytes()[0], &ble_service_uuid_string[24], 6, 1);
                            Log.info("     LONG  %s", ble_char_uuid_string);
                        }
                    }
                }
                ble_peer_all_chars = peer.discoverAllCharacteristics();
                if (ble_peer_all_chars.size()) {
                    Log.info("Found %u charactericsitics", ble_peer_all_chars.size());
                    for (int i = 0; i < ble_peer_all_chars.size(); i++){
                        bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[0], &ble_char_uuid_string[0],2,1);
                        Log.info("Characterisitic : 0x%s", ble_char_uuid_string);
                    // Display header for long characteristics
                        if (ble_peer_all_chars[i].UUID().type() == BleUuidType::LONG) {
                            bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[12], &ble_char_uuid_string[0], 4, 1);
                            ble_char_uuid_string[8] = '-';
                            bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[10], &ble_char_uuid_string[9], 2, 1);
                            ble_char_uuid_string[13] = '-';
                            bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[8], &ble_char_uuid_string[14], 2, 1);
                            ble_char_uuid_string[18] = '-';
                            bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[6], &ble_char_uuid_string[19], 2, 1);
                            ble_char_uuid_string[23] = '-';
                            bin2hex(&ble_peer_all_chars[i].UUID().rawBytes()[0], &ble_char_uuid_string[24], 6, 1);
                            Log.info("     LONG  %s", ble_char_uuid_string);
                        }
                    }
                }
                // We only want to run the discover code once
                char_connections = 1;
            }
        }
    if (peer.connected()) {
        LED_StateChar.getValue(LED_state);
        if (LED_state == "1"){
            digitalWrite(White_LED, HIGH);
        } else {
            digitalWrite(White_LED, LOW);
        }
        //LED_State.setValue(Switch_state);
        CIOT_RocksChar.getValue(data, 10);
        if (digitalRead(Button) == 0){ 
            Log.info("CIOT ROCKS!!!!");
        } else {
            Log.info("...and so do %s", data);
        }
        
    } else {
        Log.info("Peripheral is NOT connected");
    }
    //delay(3000);
}



// BIN2HEX - Used to convert binary UUID into hex....very handy!

int bin2hex(const byte *input, char *output, int len, int reverse) {
    
    // Check if we have data to convert
	if (input == NULL || output == NULL || len == 0) return 0;

    // Step through each byte
	for (int i = 0; i < len; i++) {
	    if (reverse == 1) {
		    output[i * 2]     = "0123456789ABCDEF"[input[len - (i + 1)] >> 4];
		    output[i * 2 + 1] = "0123456789ABCDEF"[input[len - (i + 1)] & 0x0F];
	    } else {
		    output[i * 2]     = "0123456789ABCDEF"[input[i] >> 4];
		    output[i * 2 + 1] = "0123456789ABCDEF"[input[i] & 0x0F];
	    }
	}
	output[len * 2] = '\0';

    // Return
	return 1;
}

void onDataReceived_BT(const uint8_t* data, size_t len, const BlePeerDevice& peer, void* context) {
    Log.info("Hello the data changed");
    for (size_t ii = 0; ii < len; ii++) {   
        
        uint8_t flags = data[0];

        uint16_t plant_data;
        if (flags & 0x01) {
            memcpy(&plant_data, &data[1], sizeof(int16_t));
        } else {
            plant_data = data[1];
        }
        Log.info("Plant data: %u", plant_data);
    }
}