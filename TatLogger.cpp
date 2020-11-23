#include "TatLogger.h"

/*!
   \brief constructor
*/
TatLogger::TatLogger(int p_nbrOfDevicesToLog) :
 m_httpResponse(-1), m_httpPayload(""), m_token(""),
  m_membership(FREE), m_maxLogRate(0), m_nbrOfDevicesToLog(p_nbrOfDevicesToLog),
  m_serializedData(""), m_connectedToWifi(false), m_eepromSavedCredentials(), m_isAuthenticated(false)
{

}

void TatLogger::beginBLE(void)
{
    const bool bleStreamData = true;
    m_bleParser.begin(bleStreamData);
}

void TatLogger::begin(bool p_forceAuth)
{
    Serial.println("no wifi id hardcoded");
    delay(500);
    EEPROM.begin(200);
    // EEPROM.write(0, 0);
    // EEPROM.write(1, 0);
    // EEPROM.write(2, 0);
    // EEPROM.commit();
    bool credentialsAreInEeprom = EEPROM.read(0);
    if (credentialsAreInEeprom && !p_forceAuth)
    {
        extractCredentialsFromEeprom();
        Serial.println("Found credentials");
        const char* ssid = m_eepromSavedCredentials.m_ssid.c_str();
        const char* password = m_eepromSavedCredentials.m_wifiPassword.c_str();
        begin(ssid, password, false);
    }
    else
    {
        m_bleParser.begin();
        while (!m_bleParser.m_credentials.areValid())
        {
            Serial.println("waiting for credentials");
            m_bleParser.waitForCredentials();
        }
        const char* ssid = m_bleParser.m_credentials.m_ssid.c_str();
        const char* password = m_bleParser.m_credentials.m_wifiPassword.c_str();
        begin(ssid, password, false);
    }
}

/*!
   \brief begin WiFi connection and start clock
   \param p_ssid wifi network name
   \param p_password wifi password
*/
void TatLogger::begin(const char* p_ssid, const char* p_password, bool p_startEepromAndSaveCredentials)
{
    Serial.println("from sensor logger");
    WiFi.begin(p_ssid, p_password);
    unsigned long timer = millis();
    unsigned long timeMax = 100000;
    bool timeout = false;
    while (WiFi.status() != WL_CONNECTED && !timeout)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
        if (millis() - timer >= timeMax)
        {
            timeout = true;
        }
    }
    if (!timeout)
    {
        Serial.println("Connected to the WiFi network");
        m_connectedToWifi = true;
        if (p_startEepromAndSaveCredentials)
        {
            m_eepromSavedCredentials.m_ssid = p_ssid;
            m_eepromSavedCredentials.m_wifiPassword = p_password;
            delay(500);
            EEPROM.begin(200);
        }
        m_datetime.start();
        delay(500);
    }
    else 
    {
        Serial.println("Unable to connect to the WiFi network");
        // begin(true);
    }
    
}

void TatLogger::login(bool p_forceAuth)
{
    // m_bleParser.end();
    while (!m_connectedToWifi)
    {
        delay(500);
        Serial.println("No wifi connection. Perhaps no credentials received");
    }

    if (m_eepromSavedCredentials.areValid() && !p_forceAuth)
    {
        Serial.println("Found credentials in eeprom");
        authenticate(m_eepromSavedCredentials);
        if (m_isAuthenticated)
        {
            saveCredentialsToEeprom(m_eepromSavedCredentials);
            setMaxLogRateFromMembershipRequest();
            m_datetime.startChrono();
        }
    }
    else if (m_bleParser.m_credentials.areValid())
    {
        Serial.println("Received credentials via BLE");
        authenticate(m_bleParser.m_credentials);
        if (m_isAuthenticated)
        {
            saveCredentialsToEeprom(m_bleParser.m_credentials);
            setMaxLogRateFromMembershipRequest();
            m_datetime.startChrono();
        }
    }
    else
    {
        Serial.println("please call begin() before login()");
    }
}

/*!
   \brief Logs user in the TeamAt server
   \param p_username username
   \param p_password password
*/
void TatLogger::login(const String &p_username, const String &p_password)
{
    m_eepromSavedCredentials.m_username = p_username;
    m_eepromSavedCredentials.m_accountPassword = p_password;
    login();
}

void TatLogger::smartLog(TatSensor p_sensorArray[])
{

    if (readyToLog(p_sensorArray)) {
        makeJsonBody(p_sensorArray);
        logDataToServer(p_sensorArray);
        m_datetime.startChrono();
    }

}

void TatLogger::streamBLE(TatSensor p_sensorArray[])
{
    if (m_bleParser.m_deviceConnected && m_bleParser.m_isON)
    {
        makeJsonStreamable(p_sensorArray);
        m_bleParser.stream(m_serializedData);
    }
    
}

bool TatLogger::readyToLog(TatSensor p_sensorArray[])
{  
    if (m_datetime.getChrono() > 60/m_maxLogRate)
    {
        return true;
    }
    bool conclusion = false;
    int sumOfData = 0;
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        m_dataPerDevice[i] = p_sensorArray[i].getDataCursor();
        if (m_dataPerDevice[i] >= TatSensor::DATA_MAXLEN - 1)
        {
            conclusion = true;
        }
        sumOfData += m_dataPerDevice[i];
    }
    return conclusion;
}

void TatLogger::makeJsonStreamable(TatSensor p_sensorArray[])
{
    String buffer = "[";
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        buffer += "[";
        String deviceID = String(p_sensorArray[i].getDeviceID());
        int cursor = p_sensorArray[i].getDataCursor() - 1;
        String data = String(p_sensorArray[i].getData(cursor));
        buffer += deviceID + "," + data + "]";
        p_sensorArray[i].setDataCursor(0);
        if (i + 1 < m_nbrOfDevicesToLog) 
        {
            buffer += ",";
        }
    }
    buffer += "]";
    m_serializedData = buffer;
}

void TatLogger::makeJsonBody(TatSensor p_sensorArray[])
{
    size_t computableCapacity = JSON_ARRAY_SIZE(m_nbrOfDevicesToLog)
                            + m_nbrOfDevicesToLog*JSON_OBJECT_SIZE(3); // collection
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        computableCapacity += 2*JSON_ARRAY_SIZE(m_dataPerDevice[i]);// arr of data + arr of timestamps
    }
    const size_t capacity = 2*computableCapacity;
    DynamicJsonDocument doc(capacity);
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        JsonObject doc_0 = doc.createNestedObject();
        doc_0["device_id"] = p_sensorArray[i].getDeviceID();
        JsonArray doc_0_data = doc_0.createNestedArray("data");
        JsonArray doc_0_timestamps = doc_0.createNestedArray("timestamps");
        for (int j = 0; j < p_sensorArray[i].getDataCursor(); j++)
        {
            doc_0_data.add(p_sensorArray[i].getData(j));
            doc_0_timestamps.add(p_sensorArray[i].getTimestamp(j));
        }
    }
    serializeJson(doc, m_serializedData);
    // Serial.print("JSON body: ");
    // Serial.println(m_serializedData);

}

void TatLogger::logDataToServer(TatSensor p_sensorArray[])
{
    // make headers
    String tokenHeader = "Token " + m_token;
    char charTokenHeader[256];
    strcpy(charTokenHeader, tokenHeader.c_str());
    char *headerkeys[] = {"Content-Type", "Authorization"};
    char *headervalues[] = {"application/json", charTokenHeader};
    // request
    postRequest("api/datalogmany", 2, headerkeys, headervalues, m_serializedData);
    if (m_httpResponse == 401)
    {
        Serial.println("re-login");
        login();
        // make headers
        String rtokenHeader = "Token " + m_token;
        char rcharTokenHeader[256];
        strcpy(rcharTokenHeader, rtokenHeader.c_str());
        char *rheaderkeys[] = {"Content-Type", "Authorization"};
        char *rheadervalues[] = {"application/json", rcharTokenHeader};
        // request
        postRequest("api/datalogmany", 2, rheaderkeys, rheadervalues, m_serializedData);
    }
    // reset
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        p_sensorArray[i].setDataCursor(0);
    }
    m_serializedData = "";

}

void TatLogger::authenticate(const Credentials& p_credentials)
{
    bool isToken = (m_token.length() > 0);
    if (isToken) 
    {
        // request user with current token
        String tokenHeader = "Token " + m_token;
        char charTokenHeader[256];
        strcpy(charTokenHeader, tokenHeader.c_str());
        char *headerkeys[] = {"Content-Type", "Authorization"};
        char *headervalues[] = {"application/json", charTokenHeader};
        delay(500);
        getRequest("api/auth/user", 2, headerkeys, headervalues);
        delay(500);
        bool tokenIsValid = (m_httpResponse == 200);
        if (tokenIsValid)
        {
            // setMaxLogRateFromMembershipRequest();
            Serial.println("token valid");
            m_isAuthenticated = true;
        }
        else
        {
            Serial.println("token invalid");
            // ask for a new token
            authenticate(p_credentials.m_username, p_credentials.m_accountPassword);
            delay(500);
        }
        
    }
    else
    {
        Serial.println("token not found");
        // authenticate
        authenticate(p_credentials.m_username, p_credentials.m_accountPassword);
        delay(500);
    }
}

/*!
   \brief performs the authentication, posts login request to server
   \param p_username username
   \param p_password password
*/
void TatLogger::authenticate(const String &p_username, const String &p_password)
{
    String route = "api/auth/login";
    char *headerkeys[] = {"Content-Type"};
    char *headervalues[] = {"application/json"};
    String payload = "{\"username\": \"" + p_username + "\", \"password\": \"" + p_password + "\"}";
    postRequest(route, 1, headerkeys, headervalues, payload);
    if (m_httpResponse != 200)
    {
        m_isAuthenticated = false;
        Serial.println("Wrong credentials");
        return;
    }
    Serial.print("m_httpPayload:  ");
    Serial.println(m_httpPayload);
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 300;
    DynamicJsonDocument doc(capacity);
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, m_httpPayload.c_str());
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        m_isAuthenticated = false;
        return;
    }
    Serial.print("TOKEN: ");
    const char* token =  doc["token"];
    m_token = token;
    Serial.println(m_token);
    m_isAuthenticated = true;
}

void TatLogger::extractCredentialsFromEeprom(void)
{
    int cursor = 1;
    m_eepromSavedCredentials.m_username = eepromReadString(cursor);
    cursor += (EEPROM.read(cursor) + 1); // 1 + (4 + 1) = 6
    m_eepromSavedCredentials.m_accountPassword = eepromReadString(cursor);
    cursor += (EEPROM.read(cursor) + 1); // 6 + (6 + 1) = 13
    m_eepromSavedCredentials.m_ssid = eepromReadString(cursor);
    cursor += (EEPROM.read(cursor) + 1); // 13 + (6 + 1) = 20
    m_eepromSavedCredentials.m_wifiPassword = eepromReadString(cursor);
    cursor += (EEPROM.read(cursor) + 1); // 13 + (6 + 1) = 20
    m_token = eepromReadString(cursor);
}

void TatLogger::saveCredentialsToEeprom(const Credentials& p_credentials)
{
    int cursor = 0;
    eepromWriteString(cursor, "1"); // bool 
    cursor ++;
    eepromWriteString(cursor, p_credentials.m_username);
    cursor += (p_credentials.m_username.length() + 1);
    eepromWriteString(cursor, p_credentials.m_accountPassword);
    cursor += (p_credentials.m_accountPassword.length() + 1);
    eepromWriteString(cursor, p_credentials.m_ssid);
    cursor += (p_credentials.m_ssid.length() + 1);
    eepromWriteString(cursor, p_credentials.m_wifiPassword);
    cursor += (p_credentials.m_wifiPassword.length() + 1);
    eepromWriteString(cursor, m_token);
}

/*!
   \brief execute a http get request
   \param p_route API url
   \param p_headerLength expected length of the request's header
   \param p_headerKeys array of header's field names
   \param p_headerValues array of header's field contents
*/
void TatLogger::getRequest(const String &p_route, int p_headerLength, char *p_headerKeys[], char *p_headerValues[])
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (m_bleParser.m_isON)
        {
            Serial.println("m_bleParser.end();");
            m_bleParser.end();
        }
        HTTPClient http;
        String serverpath = m_servername + p_route;
        http.begin(serverpath.c_str());

        for (int i = 0; i < p_headerLength; i++)
        {
            http.addHeader(p_headerKeys[i], p_headerValues[i]);
        }

        // Send HTTP GET request
        int httpResponseCode = http.GET();

        if (httpResponseCode>0)
        {
            String payload = http.getString();
            m_httpPayload = payload;
        }
        m_httpResponse = httpResponseCode;
        // Free resources
        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
}

/*!
   \brief execute a http post request
   \param p_route API url
   \param p_headerLength expected length of the request's header
   \param p_headerKeys array of header's field names
   \param p_headerValues array of header's field contents
   \param p_payload body of the request
*/
void TatLogger::postRequest(const String &p_route, int p_headerLength, char *p_headerKeys[], char *p_headerValues[],
                              const String &p_payload)
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (m_bleParser.m_isON)
        {
            Serial.println("m_bleParser.end();");
            m_bleParser.end();
        }
        HTTPClient http;
        String serverpath = m_servername + p_route;
        http.begin(serverpath.c_str());

        for (int i = 0; i < p_headerLength; i++)
        {
            http.addHeader(p_headerKeys[i], p_headerValues[i]);
        }

        // Send HTTP POST request
        int httpResponseCode = http.POST(p_payload);
        if (httpResponseCode>0)
        {
            String payload = http.getString();
            m_httpPayload = payload;
        }
        m_httpResponse = httpResponseCode;
        // Free resources
        http.end();
    }
    else
    {
        Serial.println("WiFi Disconnected");
    }
}

/*!
   \brief reads a string saved in EEPROM.
   \param p_addr string beginning address. Content of this address must be the length of the following string
   \return currentRead the decoded string
*/
String TatLogger::eepromReadString(int p_addr) const
{
    int len = EEPROM.read(p_addr);
    String currentRead = "";
    for (int i = p_addr; i < len + p_addr; i++)
    {
        char tokenBit = EEPROM.read(i + 1);
        currentRead += tokenBit;
    }
    Serial.print("read EEPROM string :");
    Serial.println(currentRead);
    return currentRead;
}

/*!
   \brief writes a string in EEPROM with format [stringLength, p_value[0], ..., p_value[stringLength - 1]]
   \param p_addr string beginning address. Content of this address must be the length of the following string
   \param p_value string to save
   \return currentRead the decoded string
*/
void TatLogger::eepromWriteString(int p_addr, const String& p_value) const
{
    EEPROM.write(p_addr, p_value.length());
    for (int i = p_addr; i < p_value.length() + p_addr; i++)
    {
        EEPROM.write(i + 1, p_value.c_str()[i - p_addr]);
    }
    EEPROM.commit();
}

/*!
   \brief format datetime
   \return formatted datetime
*/
String TatLogger::getDatetime()
{
    int* datetime = m_datetime.getDateTime();
    char * sep = "-- ::.0";
    String datetimeFormat = "";
    for (int i = 0; i < 7; i++)
    {
        if (datetime[i] < 10)
        {
            datetimeFormat += "0" + String(datetime[i]) + sep[i];
        }
        else
        {
            datetimeFormat += String(datetime[i]) + sep[i];
        }
    }
    return  datetimeFormat + "Z";
}

/*!
   \brief request device catalog
*/
void TatLogger::requestDevicesCatalog()
{
    String route = "api/devicecatalog";
    char *headerkeys[] = {"Content-Type", "Authorization"};
    char *headervalues[] = {"application/json", "Token "};
    getRequest(route, 2, headerkeys, headervalues);
}

/*!

*/
void TatLogger::setMaxLogRateFromMembershipRequest(void)
{
    String tokenHeader = "Token " + m_token;
    char charTokenHeader[256];
    strcpy(charTokenHeader, tokenHeader.c_str());
    char *headerkeys[] = {"Content-Type", "Authorization"};
    char *headervalues[] = {"application/json", charTokenHeader};
    delay(500);
    getRequest("api/auth/user", 2, headerkeys, headervalues);
    delay(500);
    Serial.println(m_httpPayload);
    // parse payload
    const size_t capacity = JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 300;
    DynamicJsonDocument doc(capacity);
    DeserializationError error = deserializeJson(doc, m_httpPayload.c_str());
    const char* membership =  doc["membership"];
    String convertedMembership = membership;
    if (convertedMembership == "FREE")
        m_membership = FREE;
    else if (convertedMembership == "ADVANCED")
        m_membership = ADVANCED;
    else if (convertedMembership == "PRO")
        m_membership = PRO;
    m_maxLogRate = (int)doc["logs_per_minute"];
    // Serial.print("membreship ");
    // Serial.print(m_maxLogRate);
    // Serial.print(", ");
    // Serial.println(m_membership);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
}

/*!
   \brief format http response for display
   \return formatted http response
*/
String TatLogger::getHTTPResponse()
{
    return String(m_httpResponse) + String(": ") + m_httpPayload;
}

void TatLogger::tester(void)
{
    m_bleParser.tester();
}