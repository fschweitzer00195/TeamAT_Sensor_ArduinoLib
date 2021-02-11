#include <TatBTParser.h>

TatBTParser::TatBTParser(void) : 
    m_server(NULL), m_credentialsCharacteristic(), 
    m_txValue(0), m_count(0),
    m_credentials(),
    m_deviceConnected(false), m_messageReceived(false),
    m_isON(false)
{
    // m_deviceConnected = false;
    // m_messageReceived = false;
}

void TatBTParser::begin(bool p_streamMode)
{
    // Create the BLE Device
    BLEDevice::init("TeamAT-Sensor");
    // Create server
    m_server = BLEDevice::createServer();
    m_server->setCallbacks(new MyServerCallbacks(&m_deviceConnected));
    // Create services
    Serial.print("SERVICE_UUID");
    Serial.println(SERVICE_UUID);

    BLEService *m_service = m_server->createService(SERVICE_UUID);
    if (!p_streamMode)
    {
        // Create characteristics
        m_credentialsCharacteristic = m_service->createCharacteristic(
                        CHARACTERISTIC_UUID_RX,
                        BLECharacteristic::PROPERTY_READ   |
                        BLECharacteristic::PROPERTY_WRITE  |
                        BLECharacteristic::PROPERTY_NOTIFY
                        );
        m_credentialsCharacteristic -> addDescriptor(new BLE2902());
        m_credentialsCharacteristic -> setCallbacks(new CredentialsCallbacks(&m_messageReceived, &m_credentials));  // manque peut etre des parentheses
    }
    else
    {
        m_dataCharacteristic = m_service->createCharacteristic(
                      DATA_CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
        m_dataCharacteristic->addDescriptor(new BLE2902());
        // m_credentialsCharacteristic -> setCallbacks(new CredentialsCallbacks(&m_messageReceived));
        Serial.println("ble stream mode");
    }
    
    // start the service
    m_service->start();
    // start advertising
    m_server->getAdvertising()->start();
    m_isON = true;
    Serial.println("Waiting for client connection");  
}

void TatBTParser::waitForCredentials(void)
{
    delay(500);
    if (m_deviceConnected && m_messageReceived)
    {
        char txString[] = "Credential Received";
        m_credentialsCharacteristic -> setValue(txString);
        m_credentialsCharacteristic -> notify();
        Serial.println("sent value: " + String(txString));
        m_messageReceived = false;
    }
}

void TatBTParser::stream(const String& p_serializedData)
{
    if (m_deviceConnected) 
    {
        m_dataCharacteristic -> setValue(p_serializedData.c_str());
        m_dataCharacteristic -> notify();
        Serial.println("sent value: " + p_serializedData);
        m_messageReceived = false;
    }
}

void TatBTParser::end(void)
{
    BLEDevice::deinit(true);
    m_deviceConnected = false;
    m_isON = false;
}

void TatBTParser::resume(void) // not working, not supposesd to be used
{
    // Create the BLE Device
    BLEDevice::init("TeamAT-Sensor");
    // Create server
    m_server = BLEDevice::createServer();
    m_server->setCallbacks(new MyServerCallbacks(&m_deviceConnected));
    // Create services
    Serial.print("SERVICE_UUID");
    Serial.println(SERVICE_UUID);

    BLEService *m_service = m_server->createService(SERVICE_UUID);

    // Create characteristics
    m_dataCharacteristic = m_service->createCharacteristic(
                      DATA_CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    m_dataCharacteristic->addDescriptor(new BLE2902());

    // start the service
    m_service->start();
    // start advertising
    m_server->getAdvertising()->start();
    m_isON = true;
    Serial.println("Waiting for client connection");
}

void TatBTParser::tester(void)
{
    if (m_deviceConnected) 
    {
        // delay(500);
        m_count ++;
        String txValue = "Sent #" + String(m_count);
        // char txString[] = txValue.c_str();
        // dtostrf(txValue, 1, 2, txString);
        m_dataCharacteristic -> setValue(txValue.c_str());
        m_dataCharacteristic -> notify();
        Serial.println("sent value: " + txValue);
        m_messageReceived = false;
    }
}