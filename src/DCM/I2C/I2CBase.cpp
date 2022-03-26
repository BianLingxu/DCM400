#include "I2CBase.h"
#include "I2CRAM.h"
#include "I2CLineInfo.h"
using namespace std;
//#define PLAINTEXT_KEY 1
#ifdef RECORD_TIME
#include "..\Timer.h"
#endif // RECORD_TIME

CI2CBase::CI2CBase(BOOL bRead, const CI2CSite& I2CSite, CDriverAlarm* pAlarm, CI2CBoardManage* pBoardMange, CI2CRAM* pRAMMange)
	: m_Data(pAlarm)
{
	m_bRead = bRead;
	m_pSite = &I2CSite;
	m_byRegisterByteCount = 1;
	m_bySlaveAddress = 0;
	m_uRegisterAddress = 0;
	m_uDataByteCount = 0;
	m_pAlarm = pAlarm;
	m_pBoardManage = pBoardMange;
	m_pRAMManage = pRAMMange;
}

CI2CBase::~CI2CBase()
{
	auto iterI2CLine = m_mapI2C.begin();
	while (m_mapI2C.end() != iterI2CLine)
	{
		if (nullptr != iterI2CLine->second)
		{
			delete iterI2CLine->second;
			iterI2CLine->second = nullptr;
		}
		++iterI2CLine;
	}
	m_mapI2C.clear();
}

void CI2CBase::SetStopStatus(BOOL bHighImpedance)
{
	for (auto& pI2CLine : m_mapI2C)
	{
		if (nullptr != pI2CLine.second)
		{
			pI2CLine.second->SetStopStatus(bHighImpedance);
		}
	}
}

int CI2CBase::GetNACKIndex(USHORT usSiteNo)
{
	if (m_pSite->GetSiteCount() <= usSiteNo)
	{
		///<The site is over range
		return -1;
	}
	auto iterI2C = m_mapI2C.find(m_strLatestKey);
	if (m_mapI2C.end() == iterI2C)
	{
		///<Not ran before
		return -2;
	}

	int nRetVal = m_Data.GetNACK(usSiteNo);
	if (0 > nRetVal)
	{
		///<Not gotten NACK before
		nRetVal = iterI2C->second->GetNACK(m_Data, usSiteNo);
	}
	
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Site is over range
			nRetVal = -1;
			break;
		case -2:
			///<The site is invalid
			nRetVal = -3;
			break;
		case -3:
			///<The board of the site is not existed
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CI2CBase::SetREGByte(BYTE byByteCount)
{
	if (4 < byByteCount)
	{
		return -1;
	}
	if (m_byRegisterByteCount != byByteCount)
	{
		///<The register byte count of the latest I2C is not equal to current
		m_strLatestKey.clear();
	}
	m_byRegisterByteCount = byByteCount;
	return 0;
}

void CI2CBase::SetAddress(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataCount)
{
	m_bySlaveAddress = bySlaveAddress;
	m_uRegisterAddress = uRegisterAddress;
	m_uDataByteCount = uDataCount;
	m_Data.Reset();
	m_Data.SetDataByteCount(m_uDataByteCount);
}

int CI2CBase::SetSiteData(USHORT usSiteNo, const BYTE* pbyData)
{
	m_strLatestKey.clear();
	if (m_pSite->GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
		m_pAlarm->SetAlarmMsg("The site number(%d) is over range([%d,%d]).", usSiteNo, 0, m_pSite->GetSiteCount() - 1);
		return -1;
	}
	if (0 == m_uDataByteCount)
	{
		///<Not set data byte count before
		return -2;
	}
	if (nullptr == pbyData)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetAlarmMsg("The point to write data of SITE_%d is nullptr.", usSiteNo + 1);
		return -3;
	}

	int nRetVal = m_Data.SetSiteData(usSiteNo, pbyData, m_uDataByteCount);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not set the data byte count
			nRetVal = -2;
			break;
		case-2:
			///<The data byte count is not equal the count set before, not will happen
			break;
		case -3:
			///<The site number is not existed
			nRetVal = -4;
			break;
		case -4:
			///<Allocate memory fail
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
			m_pAlarm->SetAlarmMsg("Allocate memory to save %d byte data fail", m_uDataByteCount);
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CI2CBase::GetSiteData(USHORT usSiteNo, UINT uDataByteIndex)
{
	if (m_pSite->GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
		m_pAlarm->SetAlarmMsg("The site number(%d) is over range([0, %d]).", usSiteNo, 0, m_pSite->GetSiteCount() - 1);
		return -1;
	}
	if (!m_pSite->IsSiteValid(usSiteNo))
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_INVALID);
		m_pAlarm->SetAlarmMsg("The SITE_%d is invalid.", usSiteNo + 1);
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		return -2;
	}
	if(uDataByteIndex >= m_uDataByteCount)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		m_pAlarm->SetAlarmMsg("The data index(%d) is over range([0, %d]).", usSiteNo, 0, m_uDataByteCount - 1);
		return -3;
	}
	int nRetVal = m_Data.GetSiteData(usSiteNo, uDataByteIndex);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The data of the site is not existed
			m_pAlarm->SetAlarmMsg("The data of SITE_%d is not existed.", usSiteNo + 1);
			nRetVal = -4;
			break;
		case -2:
			///<The data byte index is over range, not happen, judge before
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

UINT CI2CBase::GetDataByteCount()
{
	return m_Data.GetDataByteCount();
}

int CI2CBase::Run()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CBaseRun");
	CTimer::Instance()->Start("GetKey");
#endif // RECORD_TIME

	string strKey;
	GetKey(strKey);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetOperation");
#endif // RECORD_TIME


	int nRetVal = 0;
	auto iterLine = m_mapI2C.find(strKey);
	if (m_mapI2C.end() == iterLine)
	{
		nRetVal = NewOperation(strKey);
		if (0 != nRetVal)
		{
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
			CTimer::Instance()->Stop();
#endif // RECORD_TIME

			return nRetVal;
		}
		iterLine = m_mapI2C.find(strKey);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("LoadVector");
#endif // RECORD_TIME

	if (!iterLine->second->IsLoadVector())
	{
		nRetVal = iterLine->second->LoadVector();
		if (0 != nRetVal)
		{
			///<No enough line
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
			CTimer::Instance()->Stop();
#endif // RECORD_TIME

			return iterLine->second->GetPatternCount();
		}
		CI2CLineInfo::Instance()->RecordLine(*iterLine, m_bRead);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("ResetData");
#endif // RECORD_TIME

	m_Data.Reset();


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Run");
#endif // RECORD_TIME

	m_strLatestKey = strKey;
	iterLine->second->Run();
	if (m_bRead)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("GetData");
#endif // RECORD_TIME

		iterLine->second->GetData(m_Data);
	}


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

	return nRetVal;
}

void CI2CBase::Reset()
{
	for (auto& I2C : m_mapI2C)
	{
		if (nullptr != I2C.second)
		{
			delete I2C.second;
			I2C.second = nullptr;
		}
	}
	m_strLatestKey.clear();
	m_Data.Reset();
	m_mapI2C.clear();
}

int CI2CBase::GetLatestMemoryInfo(UINT& uStartLine, UINT& uLineCount, BOOL& bWithDRAM, UINT& uLineCountBeforeOut, UINT& uDRAMStartLine, UINT& uDRAMLineCount)
{
	auto iterLine = m_mapI2C.find(m_strLatestKey);
	if (m_mapI2C.end() == iterLine)
	{
		return -1;
	}
	bWithDRAM = FALSE;
	iterLine->second->GetLineCount(TRUE, &uStartLine, &uLineCount);
	iterLine->second->GetLineCount(FALSE, &uDRAMStartLine, &uDRAMLineCount);
	uLineCountBeforeOut = 0;
	if (0 != uDRAMLineCount)
	{
		bWithDRAM = TRUE;
		uLineCountBeforeOut = iterLine->second->GetLineCountBeforeSwitch();
	}
	
	return 0;
}

void CI2CBase::RecordLine()
{
	if (0 == m_mapI2C.size())
	{
		return;
	}
	for (auto& I2CLine : m_mapI2C)
	{
		CI2CLineInfo::Instance()->RecordLine(I2CLine, m_bRead);
	}
}

int CI2CBase::FreeI2C(std::string& strKey)
{
	auto iterI2C = m_mapI2C.find(strKey);
	if (m_mapI2C.end() == iterI2C)
	{
		return -1;
	}
	if (m_strLatestKey == strKey)
	{
		m_strLatestKey.clear();
	}
	UINT uBRAMStartLine = 0;
	UINT uBRAMLineCount = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;
	iterI2C->second->GetExclusiveLineCount(&uBRAMLineCount, &uDRAMLineCount);
	iterI2C->second->GetStartLine(&uBRAMStartLine, &uDRAMStartLine);
	UINT uFreeLineCount = m_pRAMManage->FreeLine(uBRAMStartLine, uBRAMLineCount, uDRAMStartLine, uDRAMLineCount);

	delete iterI2C->second;
	iterI2C->second = nullptr;
	m_mapI2C.erase(iterI2C);

	return 0;
}

inline void CI2CBase::GetKey(std::string& strKey)
{
	strKey.clear();
	char lpszMsg[128] = { 0 };
	char lpszFormat[64] = { 0 };
	sprintf_s(lpszFormat, sizeof(lpszFormat), "Slave:%%02X;REG:%%d|%%0%dX;", m_byRegisterByteCount * 2);
	sprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, m_bySlaveAddress, m_byRegisterByteCount, m_uRegisterAddress);

	string strTempKey;
#ifdef PLAINTEXT_KEY
	strKey = lpszMsg;
#else
	strTempKey = lpszMsg;
#endif // PLAINTEXT_KEY

	if (m_bRead)
	{
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Data:%d", m_uDataByteCount);
		strTempKey += lpszMsg;
	}
	else
	{
#ifdef PLAINTEXT_KEY
		m_Data.GetDataKey(strTempKey);
		strTempKey = "Data:" + strTempKey;
#else
		strTempKey += "Data:";
#endif // PLAINTEXT_KEY
	}

#ifdef PLAINTEXT_KEY
	strKey += strTempKey;
#else
	m_Data.SetMD5Data((const unsigned char*)strTempKey.c_str(), strTempKey.size());
	m_Data.GetMD5(strKey);
#endif // PLAINTEXT_KEY

}

inline int CI2CBase::NewOperation(std::string& strKey)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("NewOperation");
	CTimer::Instance()->Start("Check Existed");
#endif // RECORD_TIME

	if (m_mapI2C.end() != m_mapI2C.find(strKey))
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
		return 0;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("New I2CLine");
#endif // RECORD_TIME

	CI2CLine* pI2CLine = new CI2CLine(m_bRead, *m_pSite, m_pAlarm, m_pBoardManage, m_pRAMManage);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetParam");
#endif // RECORD_TIME
	pI2CLine->SetRegisterByte(m_byRegisterByteCount);
	pI2CLine->SetData(m_Data);
	pI2CLine->SetAddress(m_bySlaveAddress, m_uRegisterAddress, m_uDataByteCount);
	m_mapI2C.insert(make_pair(strKey, pI2CLine));

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("LoadVector");
#endif // RECORD_TIME

	int nRetVal = pI2CLine->LoadVector();
	if (0 != nRetVal)
	{
		nRetVal = pI2CLine->GetPatternCount();
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
	return nRetVal;
}
