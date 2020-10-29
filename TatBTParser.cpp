#include <TatBTParser.h>

TatBTParser::TatBTParser(void) : 
    m_server(NULL), m_characteristic(), 
    m_txValue(0), m_count(0),
    m_credentials(),
    m_deviceConnected(false), m_messageReceived(false)
{
    // m_deviceConnected = false;
    // m_messageReceived = false;
}

void TatBTParser::begin(void)
{
    // Create the BLE Device
    BLEDevice::init("ESP32");
    // Create server
    m_server = BLEDevice::createServer();
    m_server->setCallbacks(new MyServerCallbacks(&m_deviceConnected));
    // Create services
    BLEService *m_service = m_server->createService(SERVICE_UUID);

    // Create characteristics
    m_characteristic = m_service->createCharacteristic(
                      CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    m_characteristic->addDescriptor(new BLE2902());
    m_characteristic -> setCallbacks(new MyCallbacks(&m_messageReceived, &m_credentials));  // manque peut etre des parentheses
    // start the service
    m_service->start();
    // start advertising
    m_server->getAdvertising()->start();
    Serial.println("Waiting for client connection");  
}

void TatBTParser::tester(void)
{
    if (m_deviceConnected) 
    {
        delay(500);
        m_count ++;
        if (m_messageReceived)
        {
            // txValue = count;
            char txString[] = "Credential Received";
            // dtostrf(txValue, 1, 2, txString);
            m_characteristic -> setValue(txString);
            m_characteristic -> notify();
            Serial.println("sent value: " + String(txString));
            m_messageReceived = false;
        }
    }
}