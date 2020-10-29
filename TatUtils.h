#ifndef __TATUTILS_H__
#define __TATUTILS_H__


#include <Arduino.h>

class Credentials
{
    public:
        Credentials(const String& p_username, const String& p_accountPassword,const String& p_ssid, const String& p_wifiPassword) :
            m_username(p_username), m_accountPassword(p_accountPassword), m_ssid(p_ssid), m_wifiPassword(p_wifiPassword) {}
        Credentials() :
            m_username(""), m_accountPassword(""), m_ssid(""), m_wifiPassword("") {}
        String m_username;
        String m_accountPassword;
        String m_ssid;
        String m_wifiPassword;
};

#endif //__TATUTILS_H__