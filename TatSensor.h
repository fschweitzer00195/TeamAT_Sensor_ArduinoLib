//
// Created by teamat on 2020-06-17.
//

#ifndef __TATSENSOR_H__
#define __TATSENSOR_H__

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <stdio.h>


class TatSensor
{
public:
    TatSensor(int p_deviceID);
    void saveData(double p_data, const String& p_timestamp);
    int getDataCursor(void) const;
    void setDataCursor(int p_dataCursor);
    int getDeviceID(void) const;
    double getData(int p_index) const;
    const String& getTimestamp(int p_index) const;
    // int m_tester;
    static const int DATA_MAXLEN = 100;

private:
    void logSingleDevice();


    int m_deviceID;
    String m_datetime;
    double m_data[DATA_MAXLEN];
    String m_timestamp[DATA_MAXLEN];
    int m_dataCursor;



};


#endif //__TATSENSOR_H__
