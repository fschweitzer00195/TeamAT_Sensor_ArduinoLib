//
// Created by teamat on 2020-06-19.
//

#ifndef __TATSENSORDATETIME_H__
#define __TATSENSORDATETIME_H__

#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include "time.h"


class TatSensorDateTime
{
public:
    TatSensorDateTime();
    void start();
    void display();
    void reset(void);
    void startChrono();
    void stopChrono();
    long int getChrono() const;
    static void doClock(int *p_tics);
    int * getDateTime(void);

private:
    Ticker m_clocker;
    int m_tics[9];

};




#endif //__TATSENSORDATETIME_H__
