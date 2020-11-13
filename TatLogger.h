#ifndef __TATLOGGER_H__
#define __TATLOGGER_H__

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <stdio.h>
#include <TatSensor.h>
#include <TatSensorDateTime.h>
#include <TatBTParser.h>
#include <TatUtils.h>

class TatLogger
{
public:
    TatLogger(int p_nbrOfDevicesToLog);
    // TODO: make destructor maybe
    void begin(bool p_forceAuth=false);
    void begin(const char* p_ssid, const char* p_password, bool p_startEepromAndSaveCredentials=true);
    void beginBLE(void);
    void login(bool p_forceAuth=false);
    void login(const String& p_username, const String& p_password);
    void smartLog(TatSensor p_sensorArray[]);
    void streamBLE(TatSensor p_sensorArray[]);
    String getHTTPResponse(void);
    String getDatetime();
    void tester(void);
    enum Membership {FREE=0, ADVANCED=1, PRO=2}; // 4/min, 15/min, 60/min

private:
    void authenticate(const Credentials& p_credentials);
    void authenticate(const String& p_username, const String& p_password);
    void extractCredentialsFromEeprom(void);
    void saveCredentialsToEeprom(const Credentials& p_credentials);
    bool readyToLog(TatSensor p_sensorArray[]);
    void makeJsonBody(TatSensor p_sensorArray[]);
    void makeJsonStreamable(TatSensor p_sensorArray[]);
    void logDataToServer(TatSensor p_sensorArray[]);
    void getRequest(const String& p_route, int p_headerLength, char *p_headerKeys[], char *p_headerValues[]);
    void postRequest(const String& p_route, int p_headerLength, char *p_headerKeys[], char *p_headerValues[], const String& p_payload);
    void eepromWriteString(int p_addr, const String&) const;
    String eepromReadString(int p_addr) const;
    void setMaxLogRateFromMembershipRequest();
    void requestDevicesCatalog(void);


    String m_servername = "https://www.teamatsensors.com/";
    int m_httpResponse;
    String m_httpPayload;
    String m_token;
    Membership m_membership;
    int m_maxLogRate;
    TatSensorDateTime m_datetime;
    int m_nbrOfDevicesToLog;
    String m_serializedData;
    int m_dataPerDevice[10];
    TatBTParser m_bleParser;
    bool m_connectedToWifi;
    Credentials m_eepromSavedCredentials;
    bool m_isAuthenticated;

    const bool m_sendBLE = true;

    

    static const int m_MAX_DATA_TO_LOG = 100;
};

#endif //__TATLOGGER_H__
