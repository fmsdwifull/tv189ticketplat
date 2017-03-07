//#include "depend.h"
#include "config.h"

#include <strstream>


CConfig::CConfig(char* szFullFileName)
{
	c_myDoc = new TiXmlDocument(szFullFileName);
}

CConfig::~CConfig()
{
	if (c_myDoc)
	{
		delete c_myDoc;
		c_myDoc = NULL;
	}
}

bool CConfig::Load()
{
	bool bRet = true;
	do
	{
		if (!c_myDoc->LoadFile())
		{
			bRet = false;
			break;
		}
		TiXmlHandle docHandle(c_myDoc);
		m_mergetime= docHandle.FirstChild("setting").FirstChild("mergetime").ToElement();
		if (m_mergetime== NULL)
		{
			bRet = false;
			break;
		}
		m_uploadtime= docHandle.FirstChild("setting").FirstChild("uploadtime").ToElement();
		if (m_uploadtime== NULL)
		{
			bRet = false;
			break;
		}
	} while(false);
	return bRet;
}

bool CConfig::Save()
{
	return c_myDoc->SaveFile();
}

const char* CConfig::GetValueString(const char* szTag)
{
	if (strcmp(szTag, XML_UPLOADTIME) == 0)
    {
          return m_uploadtime->GetText();
    }else{
         return NULL;
    }

}

int CConfig::GetValueInt(const char* szTag)
{

	if (strcmp(szTag, "mergetime") == 0)
	{
		return atoi(m_mergetime->GetText());
	}else if (strcmp(szTag, "uploadtime") == 0)
	{
		return atoi(m_uploadtime->GetText());
	}
	else
	{
		return 0;
	}

}

long CConfig::GetValueLong(const char* szTag)
{

}

bool CConfig::SetValueString(const char* szTag, const char* szValue)
{
	bool bRet = true;
	return bRet;
}

bool CConfig::SetValueInt(const char* szTag, int iValue)
{
	bool bRet = true;
	/*

	TiXmlElement* tmp;
	if (strcmp(szTag, "mediacuttime") == 0)
	{
		tmp = m_mediacuttime;
	}else if(strcmp(szTag, "once") == 0)
	{
		tmp = m_once;
	}
	else 
	{
		bRet = false;
	}
	if (bRet)
	{
		TiXmlNode *oldnode = tmp->FirstChild();
		std::strstream strtmp;
		strtmp << iValue;
		strtmp << '\0';
		TiXmlText newText(strtmp.str());
		tmp->ReplaceChild(oldnode, newText);
		if (!Save())
		{
			bRet = false;
		}
	}
*/
	return bRet;

}

bool CConfig::SetValueLong(const char* szTag, long iValue)
{
	bool bRet = true;

	return bRet;
}

