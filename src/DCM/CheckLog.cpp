#include "CheckLog.h"
#include <fstream>
using namespace std;

CCheckLog::CCheckLog()
{
}

CCheckLog::~CCheckLog()
{
	SaveLog();
}

void CCheckLog::SetLogFile(const std::string& strLogFile)
{
	m_strFileName = strLogFile;
}

int CCheckLog::SetTestItem(const char* lpszItemName)
{
	if (nullptr == lpszItemName)
	{
		return -1;
	}
	m_strCheckItemName = "-------------------------";
	m_strCheckItemName += lpszItemName;
	m_strCheckItemName += "-------------------------";
	fstream FileLog(m_strFileName.c_str(), ios::app | ios::out);
	FileLog << endl << m_strCheckItemName.c_str() << endl;
	m_setTotalFailController.clear();
	m_setCurFailController.clear();
	for (auto& Log : m_avecControllerLog)
	{
		Log.clear();
	}
	return 0;
}

void CCheckLog::SetCheckController(const std::set<BYTE>& setController)
{
	m_setController = setController;
}

void CCheckLog::GetFailController(std::set<BYTE>& setFailController)
{
	setFailController = m_setTotalFailController;
}

int CCheckLog::SetSubItem(const char* lpszSubItem)
{
	if (nullptr == lpszSubItem)
	{
		return -1;
	}
	if (0 != m_strSubItem.size())
	{
		SaveLog();
	}
	m_strSubItem = "----------";
	m_strSubItem += lpszSubItem;
	m_strSubItem += "----------";

	fstream LogFile(m_strFileName.c_str(), ios::out | ios::app);

	LogFile << endl;
	LogFile << m_strSubItem.c_str() << endl;
	LogFile.close();

	return 0;
}

int CCheckLog::SetCheckDataName(const char* lpszCheckDataName)
{
	if (nullptr == lpszCheckDataName)
	{
		return -1;
	}
	AddDataName();
	m_strDataName = lpszCheckDataName;
	return 0;
}

int CCheckLog::AddFailData(BYTE byControllerIndex, const char* lpszFormat, ...)
{
	if (m_setController.end() == m_setController.find(byControllerIndex))
	{
		return -1;
	}
	m_setCurFailController.insert(byControllerIndex);
	m_setTotalFailController.insert(byControllerIndex);
	char lpszMsg[128] = { 0 };
	va_list ap;
	va_start(ap, lpszFormat);
	vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, ap);
	va_end(ap);
	m_avecControllerLog[byControllerIndex + 1].push_back(lpszMsg);
	return 0;
}

void CCheckLog::SaveLog()
{
	if (0 == m_strDataName.size())
	{
		return;
	}
	AddDataName();

	int anMaxColumn[DCM_MAX_CONTROLLERS_PRE_BOARD + 1] = { 10, 15, 15, 15, 15 };
	
	int nLogIndex = 0;
	for (auto& Controlller : m_avecControllerLog)
	{
		for (auto& Log : Controlller)
		{
			if (Log.size() > anMaxColumn[nLogIndex] - 5)
			{
				anMaxColumn[nLogIndex] = Log.size() + 5;
			}
		}
		++nLogIndex;
	}
	string strLog;
	auto Format = [&](int nSize, BYTE byAlignType, const std::string& strData)
	{
		strLog.clear();
		int nAddSize = nSize - strData.size();
		if (0 >= nAddSize)
		{
			strLog = strData;
			return strLog.c_str();
		}
		strLog.clear();
		switch (byAlignType)
		{
		case 0:
			///<Left alignment
			strLog += strData;
			strLog.append(nAddSize, ' ');
			break;
		case 1:
			///<Middle alignment
			strLog.append(nAddSize / 2, ' ');
			strLog += strData;
			strLog.append((nAddSize + 1) / 2, ' ');
			break;
		case 2:
		default:
			///<Right alignment
			strLog.append(' ', nAddSize);
			strLog = strData;
			break;
		}
		return strLog.c_str();
	};
	vector<string>* pvecLog = &m_avecControllerLog[0];
	string* pstrLog = nullptr;
	int nLineCount = pvecLog->size();
	for (int nLineIndex = 0; nLineIndex < nLineCount;++nLineIndex)
	{
		pstrLog = &pvecLog->at(nLineIndex);
		*pstrLog = Format(anMaxColumn[0], 0, *pstrLog);
		for (BYTE byControllerIndex = 1; byControllerIndex <= DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			if (m_setController.end() == m_setController.find(byControllerIndex - 1))
			{
				continue;
			}
			*pstrLog += Format(anMaxColumn[byControllerIndex], 1, m_avecControllerLog[byControllerIndex][nLineIndex]);
		}
	}
	fstream LogFile(m_strFileName.c_str(), ios::out | ios::app);
	
	LogFile << Format(anMaxColumn[0], 0, "Memory");
	for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
	{
		if (m_setController.end() == m_setController.find(byControllerIndex))
		{
			continue;
		}
		char lpszData[32] = { 0 };
		sprintf_s(lpszData, sizeof(lpszData), "Controller %d", byControllerIndex);
		LogFile << Format(anMaxColumn[byControllerIndex + 1], 1, lpszData);
	}
	LogFile << endl;

	for (auto& CheckLog : *pvecLog)
	{
		LogFile << CheckLog << endl;
	}

	if (0 != m_setCurFailController.size())
	{
		///<Current item is fail
		LogFile << Format(20, 0, "----Check result");
		LogFile << Format(10, 2, "FAIL");
		LogFile << endl;
	}
	LogFile.close();
	m_strDataName.clear();
	m_strSubItem.clear();
	m_setCurFailController.clear();
}

void CCheckLog::AddDataName()
{
	if (0 == m_strDataName.size())
	{
		return;
	}
	m_avecControllerLog->push_back(m_strDataName);
	int nPassLogCount = m_avecControllerLog[0].size();
	int nMaxLogCount = 0;
	for (auto& Controlller : m_avecControllerLog)
	{
		if (Controlller.size() > nMaxLogCount)
		{
			nMaxLogCount = Controlller.size();
		}
	}
	vector<string>* pvecLog = &m_avecControllerLog[0];
	int nAddLogCount = nMaxLogCount - pvecLog->size();
	for (int nIndex = 0; nIndex < nAddLogCount;++nIndex)
	{
		pvecLog->push_back(m_strDataName);
	}

	for (BYTE byControllerIndex = 1; byControllerIndex <= DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
	{
		if (m_setController.end() == m_setController.find(byControllerIndex - 1))
		{
			continue;
		}
		pvecLog = &m_avecControllerLog[byControllerIndex];
		int nLogLineCount = pvecLog->size();
		
		for (int nLineIndex = nLogLineCount; nLineIndex < nMaxLogCount;++nLineIndex)
		{
			pvecLog->push_back("PASS");
		}
	}
}
