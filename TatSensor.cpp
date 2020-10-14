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
