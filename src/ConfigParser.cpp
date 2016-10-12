#include <fstream>
#include "ConfigParser.h"
#include "PathHelper.h"

ConfigParser::ConfigParser()
{
    m_cComment = '#';
}

ConfigParser::~ConfigParser()
{

}

bool ConfigParser::Load(const std::string& strFile)
{
    //clear the last context
    if (m_vecStrLines.size())
    {
        m_vecStrLines.clear();
    }
    if (m_strFile.size())
    {
        m_strFile.clear();
    }
    m_nFileSize = 0;


    m_strFile = strFile;
    PathHelper::trim(m_strFile);
    if (!m_strFile.size())
    {
        return false;
    }

    PathHelper::NormalizePath(m_strFile, false);

    FILE* fp = fopen(m_strFile.c_str(), "rb");
    if (!fp)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    m_nFileSize = ftell(fp);
    fclose(fp);

    std::string strLine;
    std::ifstream file(m_strFile, std::ios_base::in);
    
    if (!file.is_open())
    {
        return false;
    }

    while (std::getline(file, strLine))
    {
        PathHelper::ltrim(strLine);
        if (!strLine.size())
        {
            continue;
        }

        //remove the # lines
        int nCommentPos = strLine.find_first_of(m_cComment);
        if (nCommentPos!=std::string::npos)
        {
            strLine.erase(nCommentPos);
        }

        PathHelper::trim(strLine);

        if (strLine.size())
        {
            m_vecStrLines.push_back(strLine);
        }
    }

    if (m_vecStrLines.size())
    {
        return true;
    }
    else
    {
        return false;
    }
}


std::vector<std::string>& ConfigParser::GetLines()
{
    return m_vecStrLines;
}

int ConfigParser::GetFileSize()
{
    return m_nFileSize;
}

//-1 means nOffset及其以后的下标不包含key=字样的配置
//返回下一个起始的搜索的位置
int ConfigParser::GetConfigString(std::string& strValue, const std::string& key, int nSeachStart/*=0*/) const //default seperate char is '='
{
    if (strValue.size())
    {
        strValue.clear();
    }
    if (nSeachStart<0 &&  nSeachStart >= m_vecStrLines.size())
    {
        return -1;
    }

    /////////
    int nFoundNext = -1;

    for (int i=nSeachStart; i<m_vecStrLines.size(); i++)
    {
        const std::string& strLine = m_vecStrLines[i];
        if (PathHelper::StartsWith(strLine, key+"="))
        {
            nFoundNext = i+1;
            strValue = m_vecStrLines[i].substr(key.size()+1);
            break;
        }
    }

    return nFoundNext;
}

LineParser::LineParser(const std::string& strLine, int nSplitter/*=','*/)
{
    std::string strLineTmp = strLine;

#if 0
    {
        char* pTemp = (char*)strLineTmp.c_str();
        char* pContext = NULL;

        char* pToken = strtok_s(pTemp, ",", &pContext);

        if (pToken)
        {
            m_vecTokens.push_back(pToken);
            while( pToken = strtok_s(NULL, ",", &pContext) )
            {
                m_vecTokens.push_back(PathHelper::trim(std::string(pToken)));
            }
        }
    }
#else
    std::string strToken;
    bool bMeetQuote = false;
    for (int i=0; i<strLine.size(); i++)
    {
        char ch = strLine.at(i);

        if (ch == '"')
        {
            if (!bMeetQuote )
            {
                bMeetQuote = true;
            }
            else
            {
                //m_vecTokens.push_back(PathHelper::trim(strToken));
                //strToken.clear();
                bMeetQuote = false;
            }
        }
        else if(ch == nSplitter)
        {
            if (bMeetQuote)
            {
                strToken.push_back(ch);
            }
            else
            {
                m_vecTokens.push_back(PathHelper::trim(strToken));
                strToken.clear();
            }
        }
        else
        {
            strToken.push_back(ch);
        }

        if(i == strLine.size()-1)
        {
            PathHelper::trim(strToken);
            if (strToken.size())
            {
                m_vecTokens.push_back(strToken);
            }
            strToken.clear();
        }
    }
#endif
}

int LineParser::size()
{
    return m_vecTokens.size();
}

LineParser::~LineParser()
{
}

const std::string& LineParser::operator[](int i)
{
    return m_vecTokens[i];
}