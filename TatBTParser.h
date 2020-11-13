#ifndef __TATBTPARSER_H__
#define __TATBTPARSER_H__

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <TatUtils.h>

// #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID_RX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
// #define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SERVICE_UUID        "c1fbb8e2-1d1a-11eb-adc1-0242ac120002"
#define CHARACTERISTIC_UUID_RX "b562e545-371a-4981-aa33-26b8ecc3c788"
#define CHARACTERISTIC_UUID_TX "b562e545-371a-4981-aa33-26b8ecc3c788"
#define DATA_CHARACTERISTIC_UUID_TX "a6463c38-1ed9-11eb-adc1-0242ac120002"


class TatBTParser
{
public:
    TatBTParser();
    void begin(bool p_streamMode=false);
    void waitForCredentials(void);
    void stream(const String& p_serializedData);
    void end(void);
    void resume(void);
    void tester(void);

    bool m_isON;
    bool m_deviceConnected;
    bool m_messageReceived;
    Credentials m_credentials;

private:
    BLEServer* m_server;
    BLECharacteristic* m_credentialsCharacteristic;
    BLECharacteristic* m_dataCharacteristic;
    int m_txValue;
    int m_count;
    
};


class MyServerCallbacks: public BLEServerCallbacks 
{
    public: 
        MyServerCallbacks(bool* p_deviceConnected): 
            BLEServerCallbacks(), i_deviceConnected(p_deviceConnected)
        {}
        bool* i_deviceConnected;
    void onConnect(BLEServer* pServer) {
        *i_deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
        *i_deviceConnected = false;
    }
};

class CredentialsCallbacks: public BLECharacteristicCallbacks 
{
    public:
        CredentialsCallbacks(bool* p_messageReceived, Credentials* p_credentials):
                BLECharacteristicCallbacks(), i_messageReceived(p_messageReceived), i_credentials(p_credentials)
        {}
        bool* i_messageReceived;
        Credentials* i_credentials;
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        std::string rxValue = pCharacteristic->getValue();
        char buffer[rxValue.length()];

        if (rxValue.length() > 0)
        {
            Serial.println("====START RECIEVED===");
            Serial.print("RECIEVED VALUE: ");

            for (int i = 0; i < rxValue.length(); i++)
            {
                Serial.print(rxValue[i]);
                buffer[i] = rxValue[i];
            }
            Serial.println("");
            Serial.println("=======END RECIEVED======");
            Serial.println("=======Printing conversion======");
            Serial.print("buffer");
            Serial.println(buffer);
            parseJSONCredentials(buffer);
            Serial.println(i_credentials->m_username);
            Serial.println(i_credentials->m_accountPassword);
            Serial.println(i_credentials->m_ssid);
            Serial.println(i_credentials->m_wifiPassword);
            *i_messageReceived = true;
            Serial.println("=======END printing======");
                    
        }
    }
    
    void parseJSONCredentials(char p_object[])
    {
        Serial.print("p_object");
        Serial.println(p_object);
        const size_t capacity = 2 * JSON_OBJECT_SIZE(4);
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, p_object);
        const char* username = doc["username"];
        const char* accountPassword = doc["accountPassword"];
        const char* ssid = doc["ssid"];
        const char* wifiPassword = doc["wifiPassword"];
        i_credentials->m_username = String(username); 
        i_credentials->m_accountPassword = String(accountPassword); 
        i_credentials->m_ssid = String(ssid); 
        i_credentials->m_wifiPassword = String(wifiPassword);
    }
};

// class DataStreamCallbacks: public BLECharacteristicCallbacks 
// {
//     public:

//     private:

// };

#endif //__TATBTPARSER_H__