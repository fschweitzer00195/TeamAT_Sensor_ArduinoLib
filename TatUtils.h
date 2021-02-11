/**
 * @file TatUtils.h
 * @author Frederic Schweitzer
 * @brief utilities classes and functions
 * @version 0.1
 * 
 */
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
        bool areValid(void)
        {
            if (m_username.length() > 0 
                && m_accountPassword.length() > 0 
                && m_ssid.length() > 0
                && m_wifiPassword.length() > 0)
            {
                return true;
            }
            return false;
        }
        String m_username;
        String m_accountPassword;
        String m_ssid;
        String m_wifiPassword;
};

#endif //__TATUTILS_H__