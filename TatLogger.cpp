#include "TatLogger.h"

/*!
   \brief constructor
*/
TatLogger::TatLogger(int p_nbrOfDevicesToLog) :
 m_httpResponse(-1), m_httpPayload(""), m_token(""),
  m_membership(FREE), m_maxLogRate(0), m_nbrOfDevicesToLog(p_nbrOfDevicesToLog),
  m_serializedData(""), m_connectedToWifi(false)
{

}

void TatLogger::begin(void)
{
    Serial.println("no wifi id hardcoded");
    m_bleParser.begin();
    while (!m_bleParser.m_credentials.areValid())
    {
        Serial.println("waiting for credentials");
        m_bleParser.waitForCredentials();
    }
    const char* ssid = m_bleParser.m_credentials.m_ssid.c_str();
    const char* password = m_bleParser.m_credentials.m_wifiPassword.c_str();
    
    begin(ssid, password);
}

/*!
   \brief begin WiFi connection and start clock
   \param p_ssid wifi network name
   \param p_password wifi password
*/
void TatLogger::begin(const char* p_ssid, const char* p_password)
{
    Serial.println("from sensor logger");
    WiFi.begin(p_ssid, p_password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the WiFi network");
    m_connectedToWifi = true;
    EEPROM.begin(200);
    m_datetime.start();
}

void TatLogger::login(void)
{
    m_bleParser.end();
    while (!m_connectedToWifi)
    {
        delay(500);
        Serial.println("No wifi connection. Perhaps no credentials received");
    }
    login(m_bleParser.m_credentials.m_username, m_bleParser.m_credentials.m_accountPassword);

}

/*!
   \brief Logs user in the TeamAt server
   \param p_username username
   \param p_password password
*/
void TatLogger::login(const String &p_username, const String &p_password)
{
    // Check if token in eeprom => addr 0 : bool, addr 1: length int, addr 2+ Token
    int isToken = EEPROM.read(0);
    if (isToken == 1)
    {
        String token = eepromReadString(1);
        // request user with current token
        String tokenHeader = "Token " + token;
        char charTokenHeader[256];
        strcpy(charTokenHeader, tokenHeader.c_str());
        char *headerkeys[] = {"Content-Type", "Authorization"};
        char *headervalues[] = {"application/json", charTokenHeader};
        getRequest("api/auth/user", 2, headerkeys, headervalues);
        bool tokenIsValid = (m_httpResponse == 200);
        if (tokenIsValid)
        {
            setMaxLogRateFromMembershipRequest();
            Serial.println("token valid");
            Serial.print("TOKEN: ");
            m_token = token;
            // Serial.println(m_token);
        }
        else
        {
            Serial.println("token invalid");
            // ask for a new token
            authenticate(p_username, p_password);
            EEPROM.write(0, 1);
            eepromWriteString(1, m_token);
        }
    }
    else
    {
        Serial.println("token not found");
        // authenticate
        authenticate(p_username, p_password);
        EEPROM.write(0, 1);
        eepromWriteString(1, m_token);
    }
    m_datetime.startChrono();
}

void TatLogger::smartLog(TatSensor p_sensorArray[])
{

    if (readyToLog(p_sensorArray)) {
        makeJsonBody(p_sensorArray);
        logDataToServer(p_sensorArray);
        m_datetime.startChrono();
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
    // reset
    for (int i = 0; i < m_nbrOfDevicesToLog; i++)
    {
        p_sensorArray[i].setDataCursor(0);
    }
    m_serializedData = "";

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
    Serial.print("payload:  ");
    Serial.println(payload);
    postRequest(route, 1, headerkeys, headervalues, payload);
    Serial.print("m_httpPayload:  ");
    Serial.println(m_httpPayload);
    const size_t capacity = JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 300;
    DynamicJsonDocument doc(capacity);
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, m_httpPayload.c_str());
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    Serial.print("TOKEN: ");
    const char* token =  doc["token"];
    m_token = token;
    Serial.println(m_token);
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
        HTTPClient http;
        String serverpath = m_servername + p_route;
        http.begin(serverpath.c_str());

        for (int i = 0; i < p_headerLength; i++)
        {
            http.addHeader(p_headerKeys[i], p_headerValues[i]);
        }

        // Send HTTP POST request
        int httpResponseCode = http.POST(p_payload);
        Serial.print("httpResponseCode:  ");
        Serial.println(httpResponseCode);
        if (httpResponseCode>0)
        {
            String payload = http.getString();
            Serial.print("HTTP+payload:  ");
            Serial.println(payload);
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
   \param p_addr string beginnig adress. Content of this adress must be the length of the following string
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
   \param p_addr string beginnig adress. Content of this adress must be the length of the following string
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
    return  datetimeFormat + "Z"; // TODO: do not hardcode
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