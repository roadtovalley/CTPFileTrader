#ifndef __CONFIG_SETTINGS_H__ 
#define __CONFIG_SETTINGS_H__
#include <string>
class ConfigParser;

struct ConfigSettings
{
public:
    ConfigSettings();
    ~ConfigSettings();

    char zqmd_Address[40];
    char zqtd_Address[40];
    char zqUser[32];
    char zqPwd[32];

    char zqOrdrDir[500];
    char logFile[500];

    bool LoadSettings(const std::string& strConfigFile);
    std::string ToString();

private:
    ConfigParser* m_pConfigParser;
    void RestoreStatus();
};
#endif