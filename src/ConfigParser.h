#ifndef __CONFIG_PARSER_H__
#define __CONFIG_PARSER_H__


#include <vector>
#include <string>

class ConfigParser
{
public:
    ConfigParser();
    ~ConfigParser();

    bool Load(const std::string& strFile);//如果没有效行，或则文件打开失败，则返回false
    
    std::vector<std::string>& GetLines(); //获得加载的所有行

    int GetFileSize();//获得原始文件大小


    //返回下一个搜索位置，如果为-1，表示本轮未找到
    //对于key会重复许多次的配置，nOffset千万别重复设置为0
    int GetConfigString(std::string& strValue, const std::string& key, int nOffset=0) const ;
private:
    std::vector<std::string> m_vecStrLines;//读入所有的非空、非注释行，并且将行内的注释移除后存入

    std::string m_strFile;//文件名
    int m_nFileSize;//文件大小
    int m_cComment; //注释字符：#

};

class LineParser
{
public:
    LineParser(const std::string& strLine, int nSplitter=',');
    ~LineParser();
    const std::string& operator[](int i);
    int size();

private:
    std::vector<std::string>m_vecTokens;
};


#endif