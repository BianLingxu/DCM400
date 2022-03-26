#include "I2CLine.h"
#include "HardwareInfo.h"
#include "I2CRAM.h"
#include "..\ChannelClassify.h"
#ifdef RECORD_TIME
#include "..\Timer.h"
#endif // RECORD_TIME
using namespace std;
CI2CLine::CI2CLine(BOOL bRead, const CI2CSite& I2CSite, CDriverAlarm* pAlarm, CI2CBoardManage* pBoardMange, CI2CRAM* pRAMMange)
	: m_pAlarm(pAlarm)
{
	m_pSite = &I2CSite;
	m_bRead = static_cast<bool>(bRead);
	m_bLoad = FALSE;
	m_byRegisterAddressByteCount = 1;
	m_bySlaveAddress = 0;
	m_uDataByteCount = 0;
	m_uRegisterAddress = 0;
	m_bWithDRAM = FALSE;
	m_uStartLine = 0;
	m_uStopLine = 0;
	m_uCurLine = 0;
	m_uDataBaseOffset = 0;
	m_uDRAMStartLine = 0;
	m_uDRAMLineCount = 0;
	m_uLineCountBeforeSwitch = 0;
	m_pBoardManage = pBoardMange;
	m_pRAMManage = pRAMMange;
}

void CI2CLine::SetStopStatus(BOOL bHighImpedance)
{
	if (m_bWithDRAM)
	{
		///<Use the commond line
		return;
	}
	///<Change the stop status in pattern
	ChangeStopStatus(m_uStopLine, bHighImpedance);
}

CI2CLine::~CI2CLine()
{
}

void CI2CLine::SetRegisterByte(BYTE byByteCount)
{
	m_byRegisterAddressByteCount = byByteCount;
}

BYTE CI2CLine::GetRegisterByte()
{
	return m_byRegisterAddressByteCount;
}

void CI2CLine::SetAddress(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount)
{
	m_bySlaveAddress = bySlaveAddress;
	m_uRegisterAddress = uRegisterAddress;
	UINT uAddValue = 0xFF;
	for (BYTE byIndex = 1; byIndex < m_byRegisterAddressByteCount;++byIndex)
	{
		uAddValue |= 0xFF << byIndex * 8;
	}
	m_uRegisterAddress &= uAddValue;
	m_uDataByteCount = uDataByteCount;
}

int CI2CLine::GetSlaveAddress()
{
	return m_bySlaveAddress;
}

UINT CI2CLine::GetRegisterAddress()
{
	return m_uRegisterAddress;
}

UINT CI2CLine::GetDataCount()
{
	return m_uDataByteCount;
}

void CI2CLine::SetData(const CI2CSiteData& SiteData)
{
	m_pSiteData = &SiteData;
}


int CI2CLine::GetSiteData(USHORT usSiteNo, UINT uDataByteIndex)
{
	if (m_pSite->GetSiteCount() <= usSiteNo)
	{
		return -1;
	}
	if (m_uDataByteCount <= uDataByteIndex)
	{
		return -2;
	}
	
	CHANNEL_INFO SCLChannel;
	CHANNEL_INFO SDAChannel;
	CHANNEL_INFO* pChannel = nullptr;
	m_pSite->GetSiteChannel(usSiteNo, SCLChannel, SDAChannel);
	for (BYTE byIndex = 0; byIndex < 2;++byIndex)
	{
		if (0 == byIndex)
		{
			pChannel = &SCLChannel;
		}
		else
		{
			pChannel = &SDAChannel;
		}
		CI2CBoard* pBoard = m_pBoardManage->GetBoard(pChannel->m_bySlotNo, m_pAlarm);
		if (nullptr == pBoard)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
			const char* lpszChannel[2] = { "SCL","SDA" };
			m_pAlarm->SetAlarmMsg("The %d channel(S%d_%d) of SITE_%d is not existed.", lpszChannel[byIndex], pChannel->m_bySlotNo, pChannel->m_usChannelID, usSiteNo + 1);
			return -3;
		}
	}

	int nRetVal = m_pSiteData->GetSiteData(usSiteNo, uDataByteIndex);
	if (0 > nRetVal)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		char lpszChannel[2][4] = { "SCL","SDA" };
		m_pAlarm->SetAlarmMsg("The SITE_%d hasn't read I2C before.",  usSiteNo + 1);
		nRetVal = -4;
	}
	return nRetVal;
}

void CI2CLine::GetData(CI2CSiteData& SiteData)
{
	SiteData.Reset();
	map<USHORT, CHANNEL_INFO> mapDataChannel;
	m_pSite->GetDataChannel(mapDataChannel, TRUE);
	CHANNEL_INFO* pChannelInfo = nullptr;
	for (auto& Site : mapDataChannel)
	{
		pChannelInfo = &Site.second;

		CI2CBoard* pBoard = m_pBoardManage->GetBoard(pChannelInfo->m_bySlotNo, m_pAlarm);
		if (nullptr == pBoard)
		{
			continue;
		}
		vector<int> vecFailLine;
		pBoard->GetResult(pChannelInfo->m_usChannel, vecFailLine);
		SetSiteData(SiteData, Site.first, vecFailLine);
	}
}

int CI2CLine::LoadVector()
{
	if (m_bLoad)
	{
		return 0;
	}
	int nRetVal = LoadPattern();
	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CI2CLine::Run()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CLineRun");
	CTimer::Instance()->Start("LoadVector");
#endif // RECORD_TIME

	int nRetVal = 0;
	if (!m_bLoad)
	{
		nRetVal = LoadPattern();
		if (0 != nRetVal)
		{
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
			CTimer::Instance()->Stop();
#endif // RECORD_TIME

			return -1;
		}
	}


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetRunParam");
#endif // RECORD_TIME

	CI2CBoardManage* pBoardManage =m_pBoardManage;
	pBoardManage->Run(m_uStartLine, m_uStopLine, m_bWithDRAM, m_uDRAMStartLine);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
	return 0;
}

int CI2CLine::GetLineCount(BOOL bBRAM, UINT* puStartLine, UINT* puLineCount)
{
	if (!m_bLoad)
	{
		return -1;
	}

	if (nullptr == puStartLine || nullptr == puLineCount)
	{
		return -2;
	}
	if (bBRAM)
	{
		*puStartLine = m_uStartLine;
		*puLineCount = m_uStopLine - m_uStartLine + 1;
	}
	else
	{
		if (m_bWithDRAM)
		{
			*puStartLine = m_uDRAMStartLine;
			*puLineCount = m_uDRAMLineCount;
		}
		else
		{
			*puStartLine = 0;
			*puLineCount = 0;
		}
	}
	return 0;
}

void CI2CLine::InitialCommonLine()
{
	UINT uKey = 0;
	vector<BYTE> vecUseBoard;
	m_pSite->GetUseBoard(vecUseBoard, FALSE);

	for (auto Board : vecUseBoard)
	{
		CI2CBoard* pBoard = m_pBoardManage->GetBoard(Board, m_pAlarm);
		if (nullptr == pBoard)
		{
			continue;
		}
		pBoard->SetPatternRAM(TRUE);
	}

	BYTE byOddHeadLineCount = 0;
	BYTE byOddEndLineCount = 0;
	BYTE byEvenHeadLineCount = 0;
	BYTE byEvenEndLineCount = 0;
	m_pRAMManage->GetCommonLineInfo(TRUE, &byOddHeadLineCount, &byOddEndLineCount);
	m_pRAMManage->GetCommonLineInfo(FALSE, &byEvenHeadLineCount, &byEvenEndLineCount);
	UINT uOddStartLine = 0;
	UINT uOddStopLine = 0;
	UINT uEvenStartLine = 0;
	UINT uEvenStopLine = 0;
	m_pRAMManage->GetCommonLine(&uOddStartLine, &uOddStopLine, &uEvenStartLine, &uEvenStopLine);

	UINT uStartLine = 0;
	BYTE byHeadLineCount = 0;
	BYTE byEndLineCount = 0;
	for (BYTE byIndex = 0; byIndex < 2; ++byIndex)
	{
		if (0 == byIndex)
		{
			uStartLine = uOddStartLine;
			byHeadLineCount = byOddHeadLineCount;
			byEndLineCount = byOddEndLineCount;
		}
		else
		{
			uStartLine = uEvenStartLine;
			byHeadLineCount = byEvenHeadLineCount;
			byEndLineCount = byEvenEndLineCount;
		}
		LoadCommonLine(uStartLine, byHeadLineCount, byEndLineCount);
	}
}

void CI2CLine::ChangeCommonLineStopStatus(BOOL bHighImpedance)
{
	UINT uOddStartLine = 0;
	UINT uOddStopLine = 0;
	UINT uEvenStartLine = 0;
	UINT uEvenStopLine = 0;
	m_pRAMManage->GetCommonLine(&uOddStartLine, &uOddStopLine, &uEvenStartLine, &uEvenStopLine);
	ChangeStopStatus(uOddStopLine, bHighImpedance);
	ChangeStopStatus(uEvenStopLine, bHighImpedance);
}

int CI2CLine::GetNACK(CI2CSiteData& SiteData, USHORT usSiteNo)
{
	int nRetVal = 0;
	if (m_bRead)
	{
		///<Have get NACK when gotten data
		return 0;
	}
	vector<int> vecFailLine;
	nRetVal = GetFailLineNo(usSiteNo, vecFailLine);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Site number is over range
			nRetVal = -1;
			break;
		case -2:
			///<The site is invalid
			nRetVal = -2;
			break;
		case -3:
			///<The board on the site is not valid
			nRetVal = -3;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	SetNACK(SiteData, usSiteNo, vecFailLine);
	return SiteData.GetNACK(usSiteNo);
}

BOOL CI2CLine::IsLoadVector()
{
	return m_bLoad;
}

BOOL CI2CLine::IsWithDRAM()
{
	return m_bWithDRAM;
}

UINT CI2CLine::GetExclusiveLineCount(UINT* puBRAMLineCount, UINT* puDRAMLineCount)
{
	UINT uLineCount = 0;
	if (m_bWithDRAM)
	{
		uLineCount = m_uDRAMLineCount;
		if (nullptr != puBRAMLineCount)
		{
			*puBRAMLineCount = 0;
		}
		if (nullptr != puDRAMLineCount)
		{
			*puDRAMLineCount = uLineCount;
		}
	}
	else
	{
		uLineCount = GetPatternCount();
		if (nullptr != puBRAMLineCount)
		{
			*puBRAMLineCount = uLineCount;
		}
		if (nullptr != puDRAMLineCount)
		{
			*puDRAMLineCount = 0;
		}
	}
	return uLineCount;
}

BOOL CI2CLine::GetStartLine(UINT* puBRAMStartLine, UINT* puDRAMStartLine)
{
	if (nullptr != puBRAMStartLine)
	{
		*puBRAMStartLine = m_uStartLine;
	}
	if (nullptr != puDRAMStartLine)
	{
		*puDRAMStartLine = m_uDRAMStartLine;
	}
	return m_bWithDRAM;
}

UINT CI2CLine::GetLineCountBeforeSwitch()
{
	return m_uLineCountBeforeSwitch;
}

inline void CI2CLine::LoadCommonLine(UINT uStartLine, BYTE byHeadLineCount, BYTE byEndLineCount)
{
	vector<char> vecSCL;
	vector<char> vecSDA;

	GetStartPattern(vecSCL, vecSDA);
	int nStartLineCount = vecSCL.size();

	m_uCurLine = uStartLine;
	for (BYTE byIndex = byHeadLineCount; byIndex < nStartLineCount;++byIndex)
	{
		vecSCL.pop_back();
		vecSDA.pop_back();
	}

	LoadPattern(vecSCL, vecSDA, TRUE);
	vecSCL.clear();
	vecSDA.clear();
	vector<char> vecFullSCL;
	vector<char> vecFullSDA;
	GetStopPattern(vecFullSCL, vecFullSDA);

	BYTE byStopLineCount = vecFullSCL.size();
	for (BYTE byIndex = byStopLineCount - byEndLineCount; byIndex < byStopLineCount; ++byIndex)
	{
		vecSCL.push_back(vecFullSCL[byIndex]);
		vecSDA.push_back(vecFullSDA[byIndex]);
	}
	LoadPattern(vecSCL, vecSDA, FALSE, TRUE);
	DownloadPattern();
}

UINT CI2CLine::GetPatternCount()
{
	UINT uLineCount = 3;
	uLineCount += 9;//Slave address + ACK;
	uLineCount += m_byRegisterAddressByteCount * 9; //Register address + ACK
	if (m_bRead)
	{
		uLineCount += 3;//Restart;
		uLineCount += 9;//Slave address + ACK
	}
	uLineCount += m_uDataByteCount * 9;//Data + ACK
	uLineCount += 4;//Stop
	return uLineCount;
}

int CI2CLine::LoadPattern()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("CI2CLine::LoadPattern");
	CTimer::Instance()->Start("AllocateLine");
#endif // RECORD_TIME

	int nRetVal = AllocateLine();
	if (0 > nRetVal)
	{
		//No line available for save pattern.
		return -1;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetUseBoard");
#endif // RECORD_TIME
	
	m_vecACKLineOffset.clear();

	UINT uKey = 0;
	CI2CBoard* pBoard = nullptr;
	vector<BYTE> vecUseBoard;
	m_pSite->GetUseBoard(vecUseBoard, FALSE);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetPatternRAM");
#endif // RECORD_TIME

	for (auto Board : vecUseBoard)
	{
		pBoard = m_pBoardManage->GetBoard(Board, m_pAlarm);
		if (nullptr == pBoard)
		{
			continue;
		}
		pBoard->SetPatternRAM(!m_bWithDRAM);
	}


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetPatternCount");
#endif // RECORD_TIME

	vector<char> vecSCLPattern;
	vector<char> vecSDAPattern;
	vector<char>* pvecSCLPattern = nullptr;
	vector<char>* pvecSDAPattern = nullptr;

	BYTE byHeadLineCount = 0;
	BYTE byEndLineCount = 0;

	UINT uLineCount = GetPatternCount();

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("GetStartPattern");
#endif // RECORD_TIME

	m_uCurLine = m_uStartLine;

	vector<char> vecStartSCL;
	vector<char> vecStartSDA;
	GetStartPattern(vecStartSCL, vecStartSDA);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Start With DRAM %d", m_bWithDRAM);
#endif // RECORD_TIME

	//Start
	UINT uOffset = m_uCurLine;
	if (m_bWithDRAM)
	{
		m_uCurLine = m_uDRAMStartLine;
		uOffset = m_uCurLine;
		BYTE byStartIndex = 0;
		if (0 == uLineCount % 2)
		{
			m_pRAMManage->GetCommonLineInfo(FALSE, &byHeadLineCount, &byEndLineCount);
		}
		else
		{
			m_pRAMManage->GetCommonLineInfo(TRUE, &byHeadLineCount, &byEndLineCount);
		}

		m_uLineCountBeforeSwitch = byHeadLineCount;
		BYTE byPatternCount = vecStartSDA.size();
		for (BYTE byIndex = byHeadLineCount; byIndex < byPatternCount; ++byIndex)
		{
			vecSCLPattern.push_back(vecStartSCL[byIndex]);
			vecSDAPattern.push_back(vecStartSDA[byIndex]);
		}
		pvecSCLPattern = &vecSCLPattern;
		pvecSDAPattern = &vecSDAPattern;
	}
	else
	{
		pvecSCLPattern = &vecStartSCL;
		pvecSDAPattern = &vecStartSDA;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load Start");
#endif // RECORD_TIME

	LoadPattern(*pvecSCLPattern, *pvecSDAPattern);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load SDA");
#endif // RECORD_TIME

	//Slave address
	BOOL bCheckACK = FALSE;
	vecSDAPattern.clear();
	GetSlavePattern(vecSDAPattern, FALSE, &bCheckACK);
	vector<char> vecNormalSCL;
	GetNormalSCLPattern(vecNormalSCL);
	LoadPattern(vecNormalSCL, vecSDAPattern);
	if (bCheckACK)
	{
		m_vecACKLineOffset.push_back(m_uCurLine - uOffset - 1);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load Register");
#endif // RECORD_TIME

	for (BYTE byIndex = 0; byIndex < m_byRegisterAddressByteCount;++byIndex)
	{
		GetRegisterPattern(byIndex, vecSDAPattern, &bCheckACK);
		LoadPattern(vecNormalSCL, vecSDAPattern);
		if (bCheckACK)
		{
			m_vecACKLineOffset.push_back(m_uCurLine - uOffset - 1);
		}
	}


	if (m_bRead)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Load Second Start");
#endif // RECORD_TIME

		vector<char> vecRestartSCL;
		vector<char> vecRestartSDA;
		GetRestartPattern(vecRestartSCL, vecRestartSDA);
		//Restart
		LoadPattern(vecRestartSCL, vecRestartSDA);

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Load Second Slave");
#endif // RECORD_TIME

		//Slave address
		GetSlavePattern(vecSDAPattern, TRUE, &bCheckACK);
		LoadPattern(vecNormalSCL, vecSDAPattern);
		if (bCheckACK)
		{
			m_vecACKLineOffset.push_back(m_uCurLine - uOffset - 1);
		}
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load Data");
#endif // RECORD_TIME

	m_uDataBaseOffset = m_uCurLine - uOffset;
	LoadDataPattern(uOffset);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Get Stop");
#endif // RECORD_TIME

	///<Stop pattern
	vector<char> vecStopSCL;
	vector<char> vecStopSDA;
	GetStopPattern(vecStopSCL, vecStopSDA);
	BOOL bLastLineSwitch = FALSE;
	if (m_bWithDRAM)
	{
		vecSCLPattern.clear();
		vecSDAPattern.clear();
		BYTE byLineCount = vecStopSCL.size() - byEndLineCount;
		for (BYTE byIndex = 0; byIndex < byLineCount;++byIndex)
		{
			vecSCLPattern.push_back(vecStopSCL[byIndex]);
			vecSDAPattern.push_back(vecStopSDA[byIndex]);
		}
		pvecSCLPattern = &vecSCLPattern;
		pvecSDAPattern = &vecSDAPattern;
		bLastLineSwitch = TRUE;
	}
	else
	{
		pvecSCLPattern = &vecStopSCL;
		pvecSDAPattern = &vecStopSDA;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load Stop");
#endif // RECORD_TIME

	LoadPattern(*pvecSCLPattern, *pvecSDAPattern, bLastLineSwitch, TRUE);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Download Pattern");
#endif // RECORD_TIME

	DownloadPattern();

	if (m_bWithDRAM)
	{
		m_uDRAMLineCount = m_uCurLine - m_uDRAMStartLine;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
	m_bLoad = TRUE;
	return 0;
}

char CI2CLine::GetLastStopPattern(BOOL bHighImpedance)
{
	return bHighImpedance ? 'X' : '1';
}

void CI2CLine::GetSlavePattern(std::vector<char>& vecSDAPattern, BOOL bSecond, BOOL* bCheckACK)
{
	vecSDAPattern.clear();
	Data2Pattern(m_bySlaveAddress, vecSDAPattern);
	vecSDAPattern.pop_back();
	if (bSecond)
	{
		vecSDAPattern.push_back('1');
	}
	else
	{
		vecSDAPattern.push_back('0');
	}
	vecSDAPattern.push_back('L');
	if (nullptr != bCheckACK)
	{
		*bCheckACK = TRUE;
	}
}

int CI2CLine::GetRegisterPattern(BYTE byRegisterIndex, std::vector<char>& vecSDAPattern, BOOL* bCheckACK)
{
	vecSDAPattern.clear();
	if (m_byRegisterAddressByteCount <= byRegisterIndex)
	{
		return -1;
	}
	Data2Pattern((m_uRegisterAddress >> (m_byRegisterAddressByteCount - byRegisterIndex - 1) * 8) & 0xFF, vecSDAPattern);
	vecSDAPattern.push_back('L');
	if (nullptr != bCheckACK)
	{
		*bCheckACK = TRUE;
	}
	return 0;
}

int CI2CLine::GetDataPattern(USHORT usSiteNo, UINT uDataByteIndex, std::vector<char>& vecSDAPattern, BOOL* bCheckACK)
{
	vecSDAPattern.clear();
	if (m_pSite->GetSiteCount() <= usSiteNo)
	{
		return -1;
	}
	if (m_uDataByteCount <= uDataByteIndex)
	{
		return -2;
	}
	if (m_bRead)
	{
		vecSDAPattern.assign(8, 'L');
	}
	else
	{
		int nData = m_pSiteData->GetSiteData(usSiteNo, uDataByteIndex);
		Data2Pattern(nData, vecSDAPattern);
	}
	if (m_bRead)
	{
		if (m_uDataByteCount == uDataByteIndex + 1)
		{
			vecSDAPattern.push_back('1');
		}
		else
		{
			vecSDAPattern.push_back('0');
		}
		if (nullptr != bCheckACK)
		{
			*bCheckACK = FALSE;
		}		
	}
	else
	{
		if (nullptr != bCheckACK)
		{
			*bCheckACK = TRUE;
		}
		vecSDAPattern.push_back('L');
	}
	return 0;
}

int CI2CLine::AllocateLine()
{
	UINT uLineCount = GetPatternCount();
	UINT uBRAMStartLine = 0;
	UINT uBRAMLineCount = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;
	int nRetVal = m_pRAMManage->AllocateLine(uLineCount, &uBRAMStartLine, &uBRAMLineCount, &uDRAMStartLine, &uDRAMLineCount);
	if (0 > nRetVal)
	{
		return -1;
	}
	if (uLineCount == uBRAMLineCount)
	{
		m_bWithDRAM = FALSE;
		m_uStartLine = uBRAMStartLine;
		m_uStopLine = m_uStartLine + uBRAMLineCount - 1;
	}
	else
	{
		m_bWithDRAM = TRUE;
		m_uDRAMStartLine = uDRAMStartLine;

		UINT uOddStartLine = 0;
		UINT uOddStopLine = 0;
		UINT uEvenStartLine = 0;
		UINT uEvenStopLine = 0;
		m_pRAMManage->GetCommonLine(&uOddStartLine, &uOddStopLine, &uEvenStartLine, &uEvenStopLine);
		if (0 == uLineCount % 2)
		{
			m_uStartLine = uEvenStartLine;
			m_uStopLine = uEvenStopLine;
		}
		else
		{
			m_uStartLine = uOddStartLine;
			m_uStopLine = uOddStopLine;
		}
	}
	return 0;
}

inline void CI2CLine::Data2Pattern(BYTE byData, std::vector<char>& vecPattern)
{
	char cPattern;
	vecPattern.clear();
	for (int nBitIndex = 7; nBitIndex >= 0; --nBitIndex)
	{
		if (0 != ((byData >> nBitIndex) & 0x01))
		{
			cPattern = '1';
		}
		else
		{
			cPattern = '0';
		}
		vecPattern.push_back(cPattern);
	}
}

inline int CI2CLine::LoadPattern(std::vector<char>& vecSCLPattern, std::vector<char>& vecSDAPattern, BOOL bLastLineSwitch, BOOL bStopPattern)
{
	if (0 == vecSDAPattern.size() || 0 == vecSDAPattern.size())
	{
		return 0;
	}
	else if (vecSDAPattern.size() != vecSCLPattern.size())
	{
		return -1;
	}

	USHORT usSiteCount = m_pSite->GetSiteCount();
	UINT uCurBaseLine = m_uCurLine;
	UINT uCurLine = m_uCurLine;
	CHANNEL_INFO SCLChannel;
	CHANNEL_INFO SDAChannel;
	CHANNEL_INFO* pChannel;

	char cStopPattern = '1';
	char cCurPattern = '1';
	vector<char>* pvecPattern = nullptr;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		cStopPattern = GetLastStopPattern(m_pSite->GetSiteStopStatus(usSiteNo));
		m_pSite->GetSiteChannel(usSiteNo, SCLChannel, SDAChannel);
		for (BYTE byIndex = 0; byIndex < 2; ++byIndex)
		{
			uCurLine = uCurBaseLine;
			if (0 == byIndex)
			{
				pChannel = &SCLChannel;
				pvecPattern = &vecSCLPattern;
			}
			else
			{
				pChannel = &SDAChannel;
				pvecPattern = &vecSDAPattern;
			}
			CI2CBoard* pBoard = m_pBoardManage->GetBoard(pChannel->m_bySlotNo, m_pAlarm);
			if (nullptr == pBoard)
			{
				continue;
			}
			UINT uCurPatternCount = pvecPattern->size();
			BOOL bSwitch = FALSE;
			for (UINT uIndex = 0; uIndex < uCurPatternCount; ++uIndex)
			{
				cCurPattern = pvecPattern->at(uIndex);
				if (uIndex + 1 == uCurPatternCount)
				{
					if (bLastLineSwitch)
					{
						bSwitch = TRUE;
					}
					if (bStopPattern && !m_bWithDRAM)
					{
						///<The last pattern of stop is modifiable
						cCurPattern = cStopPattern;
					}
				}
				pBoard->SetChannelPattern(pChannel->m_usChannel, uCurLine++, cCurPattern, "INC", 0, 0, bSwitch);
			}
		}
	}
	m_uCurLine += vecSCLPattern.size();
	return 0;
}

inline void CI2CLine::LoadDataPattern(UINT uOffset)
{
	vector<char> vecNormalSCL;
	GetNormalSCLPattern(vecNormalSCL);
	UINT uCurBaseLine = m_uCurLine;
	UINT uCurLine = m_uCurLine;
	CHANNEL_INFO SCLChannel;
	CHANNEL_INFO SDAChannel;
	CHANNEL_INFO* pChannel;
	vector<char>* pvecPattern = nullptr;
	vector<char> vecDataPattern;
	BOOL bCheckACK = FALSE;
	BOOL AddACK = TRUE;
	USHORT usSiteCount = m_pSite->GetSiteCount();
	CI2CBoard* pBoard = nullptr;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		m_pSite->GetSiteChannel(usSiteNo, SCLChannel, SDAChannel);
		for (BYTE byIndex = 0; byIndex < 2; ++byIndex)
		{
			bCheckACK = FALSE;
			uCurLine = uCurBaseLine;
			for (UINT uDataIndex = 0; uDataIndex < m_uDataByteCount; ++uDataIndex)
			{
				if (0 == byIndex)
				{
					pChannel = &SCLChannel;
					pvecPattern = &vecNormalSCL;
				}
				else
				{
					pChannel = &SDAChannel;

					GetDataPattern(usSiteNo, uDataIndex, vecDataPattern, &bCheckACK);
					pvecPattern = &vecDataPattern;
				}
				pBoard =m_pBoardManage->GetBoard(pChannel->m_bySlotNo, m_pAlarm);
				if (nullptr == pBoard)
				{
					continue;
				}
				UINT uCurPatternCount = pvecPattern->size();
				for (UINT uIndex = 0; uIndex < uCurPatternCount; ++uIndex)
				{
					pBoard->SetChannelPattern(pChannel->m_usChannel, uCurLine++, pvecPattern->at(uIndex), "INC");
				}
				if (AddACK && bCheckACK)
				{
					m_vecACKLineOffset.push_back(uCurLine - uOffset - 1);
				}
			}
		}
		AddACK = FALSE;
	}
	m_uCurLine += m_uDataByteCount * vecNormalSCL.size();
}

int CI2CLine::GetFailLineNo(USHORT usSiteNo, std::vector<int>& vecFailLine)
{
	vecFailLine.clear();
	CHANNEL_INFO Channel;
	int nRetVal = m_pSite->GetDataChannel(usSiteNo, Channel, TRUE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The site number is over range
			nRetVal = -1;
			break;
		case -2:
			///<The site is invalid
			nRetVal = -2;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	CI2CBoard* pBoard = m_pBoardManage->GetBoard(Channel.m_bySlotNo, m_pAlarm);
	if (nullptr == pBoard)
	{
		///<The board is not existed
		return -3;
	}
	pBoard->GetResult(Channel.m_usChannel, vecFailLine);
	return 0;
}

int CI2CLine::SetSiteData(CI2CSiteData& SiteData, USHORT usSiteNo, const std::vector<int>& vecFailLine)
{
	UINT uFailLineCount = vecFailLine.size();

	BYTE* pbyData = nullptr;
	try
	{
		pbyData = new BYTE[m_uDataByteCount];
		memset(pbyData, 0, m_uDataByteCount * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return -1;
	}
	UINT uLastDataLine = m_uDataBaseOffset + m_uDataByteCount * 9;
	UINT uDataLineOffset = 0;
	for (UINT uFailLineIndex = 0; uFailLineIndex < uFailLineCount;++uFailLineIndex)
	{
		uDataLineOffset = vecFailLine[uFailLineIndex];
		if (m_uDataBaseOffset > uDataLineOffset)
		{
			continue;
		}
		else if (uLastDataLine <= uDataLineOffset)
		{
			///<Not happened
			break;
		}
		uDataLineOffset -= m_uDataBaseOffset;

		pbyData[uDataLineOffset / 9] |= 1 << (7 - uDataLineOffset % 9);///<One ACK follow by each byte data
	}
	SiteData.SetSiteData(usSiteNo, pbyData, m_uDataByteCount);

	if (nullptr != pbyData)
	{
		delete[] pbyData;
		pbyData = nullptr;
	}
	SetNACK(SiteData, usSiteNo, vecFailLine);
	return 0;
}

inline void CI2CLine::SetNACK(CI2CSiteData& SiteData, USHORT usSiteNo, const std::vector<int>& vecFailLine)
{
	int nNACKIndex = 0;
	UINT uACKCount = m_vecACKLineOffset.size();
	UINT uFailCount = vecFailLine.size();
	for (UINT uACKIndex = 0; uACKIndex < uACKCount; ++uACKIndex)
	{
		for (UINT uFailIndex = 0; uFailIndex < uFailCount; ++uFailIndex)
		{
			if (m_vecACKLineOffset[uACKIndex] < vecFailLine[uFailIndex])
			{
				break;
			}
			else if (m_vecACKLineOffset[uACKIndex] == vecFailLine[uFailIndex])
			{
				nNACKIndex = uACKIndex + 1;
				break;
			}
		}
		if (0 != nNACKIndex)
		{
			break;
		}
	}
	SiteData.SetNACK(usSiteNo, nNACKIndex);
}

inline void CI2CLine::DownloadPattern()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("CI2CLine::DownloadPattern");
	CTimer::Instance()->Start("GetUseBoard");
#endif // RECORD_TIME

	USHORT usSiteCount = m_pSite->GetSiteCount();
	vector<BYTE> vecBoard;
	m_pSite->GetUseBoard(vecBoard, FALSE);

	for (auto Board : vecBoard)
	{

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Load");
#endif // RECORD_TIME

		CI2CBoard* pBoard = m_pBoardManage->GetBoard(Board, m_pAlarm);
		if (nullptr == pBoard)
		{
			continue;
		}
		pBoard->Load();
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
}

void CI2CLine::GetStartPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA)
{
	vecSCL.clear();
	vecSDA.clear();
	vecSCL.assign(3, '1');
	vecSDA.assign(2, '1');
	vecSDA.push_back('0');
}

void CI2CLine::GetRestartPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA)
{
	vecSCL.clear();
	vecSDA.clear();
	vecSCL.push_back('0');
	vecSCL.push_back('1');
	vecSCL.push_back('1');

	vecSDA.push_back('1');
	vecSDA.push_back('1');
	vecSDA.push_back('0');
}

void CI2CLine::GetStopPattern(std::vector<char>& vecSCL, std::vector<char>& vecSDA)
{
	vecSCL.clear();
	vecSDA.clear();
	vecSCL.push_back('0');
	vecSCL.push_back('1');
	vecSCL.push_back('1');
	vecSDA.push_back('0');
	vecSDA.push_back('0');
	vecSDA.push_back('1');

	vecSCL.push_back(GetLastStopPattern(TRUE));
	vecSDA.push_back(GetLastStopPattern(TRUE));
}

void CI2CLine::GetNormalSCLPattern(std::vector<char>& vecSCL)
{
	vecSCL.clear();
	vecSCL.assign(9, '0');
}

void CI2CLine::ChangeStopStatus(UINT uLineNo, BOOL bHigImpedance)
{
	///<Change the stop status in pattern
	USHORT usSiteCount = m_pSite->GetSiteCount();
	CI2CBoard* pBoard = nullptr;
	CHANNEL_INFO SCLChannel;
	CHANNEL_INFO SDAChannel;
	CHANNEL_INFO* pChannel = &SCLChannel;
	set<BYTE> setUsedBoard;
	char cPattern = 'X';
	BOOL bCurStopHighImpedance = TRUE;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		if (m_pSite->IsSiteValid(usSiteNo))
		{
			bCurStopHighImpedance = bHigImpedance;
		}
		else
		{
			bCurStopHighImpedance = m_pSite->GetSiteStopStatus(usSiteNo);
		}
		cPattern = GetLastStopPattern(bCurStopHighImpedance);
		m_pSite->GetSiteChannel(usSiteNo, SCLChannel, SDAChannel);
		for (BYTE byIndex = 0; byIndex < 2; ++byIndex)
		{
			if (0 == byIndex)
			{
				pChannel = &SCLChannel;
			}
			else
			{
				pChannel = &SDAChannel;
			}
			pBoard = m_pBoardManage->GetBoard(pChannel->m_bySlotNo, m_pAlarm);
			if (nullptr == pBoard)
			{
				continue;
			}
			pBoard->SetPatternRAM(TRUE);
			pBoard->SetChannelPattern(pChannel->m_usChannel, uLineNo, cPattern, "INC");
			setUsedBoard.insert(pChannel->m_bySlotNo);
		}
	}

	for (auto Board : setUsedBoard)
	{
		pBoard = m_pBoardManage->GetBoard(Board, m_pAlarm);
		if (nullptr == pBoard)
		{
			continue;
		}
		pBoard->Load();
	}
}
