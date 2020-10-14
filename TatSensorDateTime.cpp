 //
// Created by teamat on 2020-06-19.
//

#include "TatSensorDateTime.h"


TatSensorDateTime::TatSensorDateTime() : m_tics{2020, 2, 28, 23, 59, 50, 15, 0, 0}
{
    Serial.println("datetime created");
}

void TatSensorDateTime::start()
{
    Serial.println("datetime start");
    const char* ntpServer = "pool.ntp.org";
    const long  gmtOffset_sec = 0; //-5 * 3600;
    const int   daylightOffset_sec = 0; //3600;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
    m_tics[0] = timeinfo.tm_year + 1900;
    m_tics[1] = timeinfo.tm_mon + 1;
    m_tics[2] = timeinfo.tm_mday;
    m_tics[3] =  timeinfo.tm_hour;
    m_tics[4] = timeinfo.tm_min;
    m_tics[5] = timeinfo.tm_sec;
    m_tics[6] = 0;

//    // synchrnize
//    getLocalTime(&timeinfo);
//    m_tics[5] = timeinfo.tm_sec;
//    int i = 0;
//    while(timeinfo.tm_sec == m_tics[5])
//    {
//        i++;
//        getLocalTime(&timeinfo);
//        delay(5);
//    }
//    m_tics[5] = timeinfo.tm_sec;
//
   // Serial.print(m_tics[0]);
   // Serial.print(" - ");
   // Serial.print(m_tics[1]);
   // Serial.print(" - ");
   // Serial.print(m_tics[2]);
   // Serial.print(" - ");
   // Serial.print(m_tics[3]);
   // Serial.print(" - ");
   // Serial.print(m_tics[4]);
   // Serial.print(" - ");
   // Serial.print(m_tics[5]);
   // Serial.print(" - ");
   // Serial.println(m_tics[6]);



    m_clocker.attach_ms(10, TatSensorDateTime::doClock, m_tics);
//    Serial.print("waited");
//    Serial.println(i);
}

void TatSensorDateTime::reset()
{
//    m_tics = 0;
}

void TatSensorDateTime::doClock(int* p_tics)
{
    // p_tics = [y, m, d, h, m, s, cents, chronoEnabeled, chrono]
    int day_by_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if( (p_tics[0] - 2020) % 4 == 0)
    {
        day_by_month[1] = 29;
    }
    (p_tics[6]) ++;
    if (p_tics[6] >= 100) { // cents
        p_tics[6] = 0;
        (p_tics[5]) ++;
        if (p_tics[7])
           (p_tics[8]) ++;
    }
    if (p_tics[5] >= 60) { // secs
     p_tics[5] = 0;
     (p_tics[4]) ++;
    }
    if (p_tics[4] >= 60) { // minutes
        p_tics[4] = 0;
        (p_tics[3]) ++;
    }
    if (p_tics[3] >= 24) { // hours
        p_tics[3] = 0;
        (p_tics[2]) ++;
    }
    if (p_tics[2] > day_by_month[ p_tics[1] - 1 ]) { // month days
        p_tics[2] = 0;
        (p_tics[1]) ++;
    }
    if (p_tics[1] > 12) { // month
        p_tics[1] = 0;
        (p_tics[0]) ++;
    }
 }

 int* TatSensorDateTime::getDateTime()
 {
    return m_tics;
 }

 void TatSensorDateTime::startChrono(void)
 {
     m_tics[7] = 1;
     m_tics[8] = 0;
 }

 void TatSensorDateTime::stopChrono(void)
 {
     m_tics[7] = 0;
     m_tics[8] = 0;
 }

 long int TatSensorDateTime::getChrono(void) const
 {
     return m_tics[8];
 }
