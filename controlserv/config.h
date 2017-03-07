#include "tinystr.h"
#include "tinyxml.h"

 
#define  XML_MERGETIME  "mergetime"
#define  XML_UPLOADTIME  "uploadtime"





class CConfig
{
public:
	CConfig(char* szFullFileName);
	~CConfig();
	bool Load();
	bool Save();
	const char* GetValueString(const char* szTag);
	int GetValueInt(const char* szTag);
	long GetValueLong(const char* szTag);
	bool SetValueString(const char* szTag, const char* szValue);
	bool SetValueInt(const char* szTag, int iValue);
	bool SetValueLong(const char* szTag, long iValue);
	
private:
	TiXmlDocument* c_myDoc;
	
	TiXmlElement* m_mergetime;
	TiXmlElement* m_uploadtime;

};

