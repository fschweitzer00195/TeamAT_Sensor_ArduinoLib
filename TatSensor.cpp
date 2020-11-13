//
// Created by teamat on 2020-06-17.
//

#include "TatSensor.h"


TatSensor::TatSensor(int p_deviceID) : m_deviceID(p_deviceID), m_dataCursor(0)
{
}

void TatSensor::saveData(double p_data, const String& p_timestamp)
{
    m_data[m_dataCursor] = p_data;
    m_timestamp[m_dataCursor] = p_timestamp;
    m_dataCursor ++;
    if (m_dataCursor >= DATA_MAXLEN)
    {
        Serial.println("cursor >>");
        int j = 0;
        for (int i = DATA_MAXLEN/2; i < DATA_MAXLEN; i++)
        {
            m_data[j] = m_data[i];
            m_data[i] = 0;
            m_timestamp[j] = m_timestamp[i];
            m_timestamp[i] = "";
            j ++;
        }
        m_dataCursor = j;
        Serial.println("cursor = j");
    }
}

int TatSensor::getDataCursor(void) const
{
    return m_dataCursor;
}

void TatSensor::setDataCursor(int p_dataCursor)
{
    m_dataCursor = p_dataCursor;
}

int TatSensor::getDeviceID(void) const
{
    return m_deviceID;
}

double TatSensor::getData(int p_index) const
{
    return m_data[p_index];
}

const String& TatSensor::getTimestamp(int p_index) const
{
    return m_timestamp[p_index];
}
