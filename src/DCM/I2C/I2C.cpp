#include "I2C.h"
#include "..\HardwareFunction.h"
#include "I2CBoard.h"
#include "..\ChannelClassify.h"
#include "I2CLineInfo.h"
#include <set>
#define I2C_READ_INDEX 0
#define I2C_WRITE_INDEX 1
#define I2CTIMESET TIMESET_COUNT - 1
using namespace std;

#ifdef RECORD_TIME
#include "..\Timer.h"
#endif // RECORD_TIME

CI2C::CI2C(CDriverAlarm* pAlarm)
{
	m_pBoardManage = CI2CManage::Instance()->GetBoardManage(this);
	m_pRAMManage = CI2CManage::Instance()->GetRAMManage(this);
	m_byRegisterByteCount = 1;
	m_dPeriod = MIN_PERIOD;
	m_pAlarm = pAlarm;
	m_bLatestRead = FALSE;
	m_bLastRunSuccess = TRUE;
	m_bLoadCommonLine = FALSE;
	Initialize(TRUE);
	Initialize(FALSE);
}

CI2C::~CI2C()
{
	Reset();
	if (nullptr != m_I2C[I2C_READ_INDEX])
	{
		delete m_I2C[I2C_READ_INDEX];
		m_I2C[I2C_READ_INDEX] = nullptr;
	}
	if (nullptr != m_I2C[I2C_WRITE_INDEX])
	{
		delete m_I2C[I2C_WRITE_INDEX];
		m_I2C[I2C_WRITE_INDEX] = nullptr;
	}
	DeleteMemory(m_mapBoard);
}

CDriverAlarm* CI2C::GetAlarm()
{
	return m_pAlarm;
}

int CI2C::Set(double dPeriod, USHORT usSiteCount, REG_MODE RegisterMode, const char* lpszSCL, const char* lpszSDA)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CSet");
	CTimer::Instance()->Start("CheckPeriod");
#endif // RECORD_TIME

	if (MIN_PERIOD - EQUAL_ERROR > dPeriod || MAX_PERIOD + EQUAL_ERROR < dPeriod)
	{
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PERIOD_ERROR);
		double dUserSetPeriod = dPeriod;
		if (MIN_PERIOD - EQUAL_ERROR > dPeriod)
		{
			dPeriod = MIN_PERIOD;
		}
		else
		{
			dPeriod = MAX_PERIOD;
		}
		m_pAlarm->SetAlarmMsg("The period(%.0fns) of I2C is over range([%.0f,%.0f]), and will be set to %.0fns.", dUserSetPeriod, MIN_PERIOD, MAX_PERIOD, dPeriod);
		m_pAlarm->SetParamName("Time");
		m_pAlarm->Output(FALSE);
	}
	m_dPeriod = dPeriod;
	int nMaxSiteCount = DCM_MAX_SITE_COUNT / 2;
	if (0 == usSiteCount || nMaxSiteCount < usSiteCount)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_COUNT_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The site count(%d) is over range([%d,%d]).", usSiteCount, 1, nMaxSiteCount);
		return -1;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Reset");
#endif // RECORD_TIME

	BOOL bSiteCountSame = TRUE;

	USHORT usCurSiteCount = m_I2CSite.GetSiteCount();
	if (usSiteCount != usCurSiteCount)
	{
		bSiteCountSame = FALSE;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Check Register");
#endif // RECORD_TIME

	switch (RegisterMode)
	{
	case CI2C::REG_MODE::REG_8:
		break;
	case CI2C::REG_MODE::REG_16:
		break;
	case CI2C::REG_MODE::REG_24:
		break;
	case CI2C::REG_MODE::REG_32:
		break;
	default:
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_MODE_ERROR);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The byte count(%d) of register address is over range([%d,%d]).", (BYTE)RegisterMode + 1, 0, CI2C::REG_MODE::REG_32);
		return -2;
		break;
	}
	m_byRegisterByteCount = (BYTE)RegisterMode + 1;

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetREG");
#endif // RECORD_TIME

	CI2CBase* pI2C = m_I2C[I2C_READ_INDEX];
	if (nullptr != pI2C)
	{
		pI2C->SetREGByte(m_byRegisterByteCount);
	}
	pI2C = m_I2C[I2C_WRITE_INDEX];
	if (nullptr != pI2C)
	{
		pI2C->SetREGByte(m_byRegisterByteCount);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("ParseChannel");
#endif // RECORD_TIME

	BOOL bParseChannel = TRUE;
	if (bSiteCountSame && 0 == m_strSCLChannel.compare(lpszSCL) && 0 == m_strSDAChannel.compare(lpszSDA))
	{
		bParseChannel = FALSE;
	}
	if (bParseChannel)
	{
		Reset();
		m_I2CSite.SetSiteCount(usSiteCount);
		m_strSCLChannel.clear();
		m_strSDAChannel.clear();
		m_bLoadCommonLine = FALSE;///<Reload common line


#ifdef RECORD_TIME
		CTimer::Instance()->Start("SlotParse SCL");
#endif // RECORD_TIME

		vector<CHANNEL_INFO> vecSCL;
		vector<CHANNEL_INFO> vecSDA;

		auto ParseChannel = [&](BOOL bSCL)->int
		{
			auto pvecChannel = &vecSCL;
			auto lpszChannel = lpszSCL;
			if (!bSCL)
			{
				pvecChannel = &vecSDA;
				lpszChannel = lpszSDA;
			}
			int nRetVal = SlotParse(lpszChannel, *pvecChannel);
			if (0 != nRetVal)
			{
				if (bSCL)
				{
					m_pAlarm->SetParamName("lpszSCLChannel");
				}
				else
				{
					m_pAlarm->SetParamName("lpszSDAChannel");
				}
				switch (nRetVal)
				{
				case -1:
					///<parameter is nullptr
					nRetVal = -3;
					break;
				case -2:
					///<parameter is blank
					nRetVal = -4;
					break;
				case -3:
				case -4:
				case -5:
				case -6:
					///<Format error
					nRetVal = -5;
					break;
				case -7:
					///<Channel over range
					nRetVal = -6;
					break;
				case -8:
					///<Channel over range or not existed
					nRetVal = -7;
					break;
				default:
					break;
				}
				return nRetVal;
			}
			return 0;
		};
		///<Parse SCL
		int nRetVal = ParseChannel(TRUE);
		if (0 != nRetVal)
		{
			m_I2CSite.SetSiteCount(0);
			return nRetVal;
		}
		///<Parse SDA
		nRetVal = ParseChannel(FALSE);
		if (0 != nRetVal)
		{
			m_I2CSite.SetSiteCount(0);
			return nRetVal;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Check Site");
#endif // RECORD_TIME

		if (usSiteCount != vecSCL.size() || usSiteCount != vecSDA.size())
		{
			m_I2CSite.SetSiteCount(0);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_NOT_EQUAL_SITE_COUNT);
			USHORT uChannelCount = vecSDA.size();
			char lpszCurChanneType[16] = { 0 };
			if (usSiteCount != vecSCL.size())
			{
				uChannelCount = vecSCL.size();
				strcpy_s(lpszCurChanneType, sizeof(lpszCurChanneType), "lpszSCLChannel");
			}
			else
			{
				strcpy_s(lpszCurChanneType, sizeof(lpszCurChanneType), "lpszSDAChannel");
			}
			m_pAlarm->SetAlarmMsg("The channel number(%d) of \"%s\" is not equal to site count(%d).", uChannelCount, lpszCurChanneType, usSiteCount);
			return -8;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Check Channel");
#endif // RECORD_TIME

		nRetVal = CheckChannel(vecSCL, vecSDA);
		if (0 != nRetVal)
		{
			m_I2CSite.SetSiteCount(0);
			return -9;
		}

		m_strSCLChannel = lpszSCL;
		m_strSDAChannel = lpszSDA;

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Set Channel");
#endif // RECORD_TIME

		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
		{
			m_I2CSite.SetSiteChannel(usSiteNo, vecSCL[usSiteNo], vecSDA[usSiteNo]);
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Set Period");
#endif // RECORD_TIME

	m_pBoardManage->SetSite(m_I2CSite);

	SetPeriod(FALSE);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Set Edge SCL");
#endif // RECORD_TIME

	double dEdge[EDGE_COUNT] = { 0 };

	dEdge[0] = 10;
	dEdge[1] = 10 + m_dPeriod / 2;
	dEdge[2] = 10;
	dEdge[3] = 10 + m_dPeriod / 2;
	dEdge[4] = 10 + m_dPeriod / 2;
	dEdge[5] = m_dPeriod * 3 / 4;
	SetEdge(TRUE, dEdge, FALSE);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Set Edge SDA");
#endif // RECORD_TIME

	dEdge[0] = m_dPeriod / 4;//T1R
	dEdge[1] = m_dPeriod * 3 / 4;///<T1F
	dEdge[2] = dEdge[0];//IOR
	dEdge[3] = 10 + m_dPeriod / 2;///<IOF
	dEdge[4] = 10 + m_dPeriod / 2;///<STBR
	dEdge[5] = m_dPeriod * 3 / 4;///<STBF
	SetEdge(FALSE, dEdge, FALSE);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("LoadCommonPattern");
#endif // RECORD_TIME
	if (bParseChannel)
	{
		LoadCommonPattern();
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\SetTime.csv");
#endif // RECORD_TIME

	return 0;
}

int CI2C::Set(double dPeriod, REG_MODE RegMode, const std::vector<CHANNEL_INFO>& vecSCL, const std::vector<CHANNEL_INFO>& vecSDA)
{
	if (vecSDA.size() != vecSCL.size())
	{
		return -1;
	}
	USHORT usSiteCount = vecSCL.size();
	m_I2CSite.SetSiteCount(usSiteCount);
	int nRetVal = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		nRetVal = m_I2CSite.SetSiteChannel(usSiteNo, vecSCL[usSiteNo], vecSDA[usSiteNo]);
		if (0 != nRetVal)
		{
			m_I2CSite.Reset();
			return -2;
		}
	}
	m_dPeriod = dPeriod;
	m_byRegisterByteCount = (BYTE)RegMode;
	
	return 0;
}

double CI2C::GetPeriod()
{
	return m_dPeriod;
}

int CI2C::GetRegisterByteCount()
{
	return m_byRegisterByteCount;
}

void CI2C::SetStopStatus(BOOL bHighImpedance)
{
	for (auto& pI2C : m_I2C)
	{
		if (nullptr == pI2C)
		{
			continue;
		}
		pI2C->SetStopStatus(bHighImpedance);
	}
	CI2CLine I2CLine(TRUE, m_I2CSite, m_pAlarm, m_pBoardManage, m_pRAMManage);
	I2CLine.ChangeCommonLineStopStatus(bHighImpedance);
	m_I2CSite.SetSiteStopStatus(bHighImpedance);
}

int CI2C::SetPinLevel(double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	vector<BYTE> vecUseBoard;
	m_I2CSite.GetUseBoard(vecUseBoard);
	int nRetVal = 0;
	BYTE byUseBoardCount = vecUseBoard.size();
	for (auto Slot : vecUseBoard)
	{
		CI2CBoard* pI2CBoard = m_pBoardManage->GetBoard(Slot, m_pAlarm);
		if (nullptr == pI2CBoard)
		{
			continue;
		}
		pI2CBoard->SetSite(m_I2CSite);
		nRetVal = pI2CBoard->SetPinLevel(dVIH, dVIL, dVOH, dVOL, byChannelType);
		if (0 != nRetVal)
		{
			nRetVal = -2;
			break;
		}
	}

	return nRetVal;
}

void CI2C::SetOperationParam(BOOL bRead, BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataBytesCount)
{
	m_bLatestRead = bRead;
	
	BYTE byI2CIndex = I2C_WRITE_INDEX;
	if (bRead)
	{
		byI2CIndex = I2C_READ_INDEX;
	}
	CI2CBase* pI2C = m_I2C[byI2CIndex];
	if (nullptr == pI2C)
	{
		Initialize(TRUE);
	}
	pI2C->SetAddress(bySlaveAddress, uRegisterAddress, uDataBytesCount);
}

inline int CI2C::Run()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CRun");
	CTimer::Instance()->Start("Initialize");
#endif // RECORD_TIME

	BYTE byIndex = I2C_WRITE_INDEX;
	if (m_bLatestRead)
	{
		byIndex = I2C_READ_INDEX;
	}
	CI2CBase* pI2C = m_I2C[byIndex];
	if (nullptr == pI2C)
	{
		Initialize(m_bLatestRead);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Disconnect");
#endif // RECORD_TIME

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("I2CRunVector");
#endif // RECORD_TIME
	int nRetVal = Run(m_bLatestRead);
	if (0 != nRetVal)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
		nRetVal = -1;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

	return nRetVal;
}

int CI2C::GetReadData(USHORT usSiteNo, UINT uDataByteIndex)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	CI2CBase* pI2C = m_I2C[I2C_READ_INDEX];
	if (nullptr == pI2C || !m_bLatestRead || !m_bLastRunSuccess)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_READ_ERROR);
		m_pAlarm->SetAlarmMsg("The latest ran is not I2C read operation");
		return -2;
	}
	if (m_I2CSite.GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The site number(%d) is over range([%d,%d]).", usSiteNo, 0, m_I2CSite.GetSiteCount() - 1);
		return -3;
	}
	m_pAlarm->SetSite(usSiteNo);

	if (!m_I2CSite.IsSiteValid(usSiteNo))
	{
// 		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_INVALID);
// 		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_SITE_INVALID);
// 		m_pAlarm->SetAlarmMsg("The SITE_%d is invalid.", usSiteNo + 1);
		return -4;
	}

	if (pI2C->GetDataByteCount() <= uDataByteIndex)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_LENGTH_ERROR);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The data index(%d) is over range([%d,%d]).", uDataByteIndex, 0, pI2C->GetDataByteCount() - 1);
		return -5;
	}
	int nRetVal = pI2C->GetSiteData(usSiteNo, uDataByteIndex);
	if (0 != nRetVal)
	{
		///<Not will happened
		switch (nRetVal)
		{
		case -1:
			///<The site number is over range
			nRetVal = -3;
			break;
		case -2:
			///<The site is invalid
			nRetVal = -3;
			break;
		case -3:
			///<The data index is over range
			nRetVal = -5;
			break;
		case -4:
			///<The data of the site is not existed
			nRetVal = -6;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CI2C::SetSiteData(USHORT usSiteNo, const BYTE* pbyData)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	if (m_bLatestRead)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_FUNCTION_USE_ERROR);
		m_pAlarm->SetAlarmMsg("Can't set read data.");
		return -2;
	}
	CI2CBase* pI2C = m_I2C[I2C_WRITE_INDEX];
	if (nullptr == pI2C)
	{
		return -2;
	}
	if (m_I2CSite.GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The site number(%d) is over range([%d,%d]).", usSiteNo, 0, m_I2CSite.GetSiteCount() - 1);
		return -3;
	}
	if (nullptr == pbyData)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetAlarmMsg("The point to write data of SITE_%d is nullptr.", usSiteNo + 1);
		return -4;
	}
	int nRetVal = pI2C->SetSiteData(usSiteNo, pbyData);
	if (0 != nRetVal)
	{
		nRetVal = -5;
	}
	return nRetVal;
}

int CI2C::SetBaseLine(int nRAMBaseLine, int nDRAMBaseLine)
{
	if (DCM_BRAM_PATTERN_LINE_COUNT <= nRAMBaseLine + m_pRAMManage->GetCommonLineCount())
	{
		return -1;
	}
	if (DCM_DRAM_PATTERN_LINE_COUNT <= nDRAMBaseLine)
	{
		return -2;
	}
	m_pRAMManage->Reset();
	m_pRAMManage->SetBaseRAM(nRAMBaseLine, nDRAMBaseLine);
	for (int nTypeIndex = 0; nTypeIndex < 2;++nTypeIndex)
	{
		if (nullptr != m_I2C[nTypeIndex])
		{
			m_I2C[nTypeIndex]->Reset();
		}
	}
	m_bLoadCommonLine = FALSE;
	return 0;
}

int CI2C::GetNACKIndex(USHORT usSiteNo)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set I2C channel through I2CSet before.");
		return -1;
	}
	if (m_I2CSite.GetSiteCount() <= usSiteNo)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetAlarmMsg("The site number(%d) is over range([%d,%d]).", usSiteNo, 0, m_I2CSite.GetSiteCount() - 1);
		return -2;
	}
	if (!m_I2CSite.IsSiteValid(usSiteNo))
	{
// 		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_SITE_INVALID);
// 		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_SITE_INVALID);
// 		m_pAlarm->SetAlarmMsg("The SITE_%d is invalid.", usSiteNo + 1);
		return -3;
	}
	m_pAlarm->SetSite(usSiteNo);
	if (!m_bLastRunSuccess)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_OPERATION_ERROR);
		m_pAlarm->SetAlarmMsg("No I2C operation before.");
		return -4;
	}
	BYTE byIndex = I2C_WRITE_INDEX;
	if (m_bLatestRead)
	{
		byIndex = I2C_READ_INDEX;
	}
	CI2CBase* pI2C = m_I2C[byIndex];
	if (nullptr == pI2C)
	{
		//Not will happen
	}
	int nRetVal = pI2C->GetNACKIndex(usSiteNo);
	if (0 > nRetVal)
	{
		///<Not will happened
		switch (nRetVal)
		{
		case -1:
			///<The site number is over range
			nRetVal = -2;
			break;
		case -2:
			///<Not ran before
			nRetVal = -4;
			break;
		case -3:
			///<The site is invalid
			nRetVal = -3;
			break;
		case -4:
			///<The board of the site is not existed
			nRetVal = -5;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

USHORT CI2C::GetSiteCount()
{
	return m_I2CSite.GetSiteCount();
}

int CI2C::GetLatestMemoryInfo(UINT& uStartLine, UINT& uLineCount, BOOL& bWithDRAM, UINT& uLineCountBeforeOut, UINT& uDRAMStartLine, UINT& uDRAMLineCount)
{
	BYTE byType = I2C_WRITE_INDEX;
	if (m_bLatestRead)
	{
		byType = I2C_READ_INDEX;
	}
	if (nullptr == m_I2C[byType])
	{
		return -1;
	}
	int nRetVal = m_I2C[byType]->GetLatestMemoryInfo(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);
	if (0 != nRetVal)
	{
		nRetVal = -1;
	}
	return nRetVal;
}

BOOL CI2C::IsSiteValid(USHORT usSiteNo) const
{
	return m_I2CSite.IsSiteValid(usSiteNo);
}

void CI2C::SetExistedBoard(const std::vector<BYTE>& vecBoard)
{
	m_pBoardManage->SetValidBoard(vecBoard);
}

int CI2C::SetDynamicLoad(BYTE byChannelType, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	vector<BYTE> vecUseBoard;
	m_I2CSite.GetUseBoard(vecUseBoard);
	int nRetVal = 0;
	BYTE byUseBoardCount = vecUseBoard.size();
	for (auto Slot : vecUseBoard)
	{
		CI2CBoard* pI2CBoard = m_pBoardManage->GetBoard(Slot, m_pAlarm);
		if (nullptr == pI2CBoard)
		{
			continue;
		}
		pI2CBoard->SetSite(m_I2CSite);
		nRetVal = pI2CBoard->SetDynamicLoad(byChannelType, bEnable, dIOH,dIOL, dVTVoltValue, dClampHighVoltValue, dClampLowVoltValue);
		if (0 != nRetVal)
		{
			break;
		}
	}

	return nRetVal;
}

void CI2C::GetChannel(std::vector<CHANNEL_INFO>& vecChannel) const
{
	m_I2CSite.GetAllChannel(vecChannel);
}

int CI2C::GetChannel(USHORT usSiteNo, BOOL bSCL, CHANNEL_INFO& Channel) const
{
	CHANNEL_INFO OtherChannel;
	CHANNEL_INFO& SCLChannel = bSCL ? Channel : OtherChannel;
	CHANNEL_INFO& SDAChannel = bSCL ? OtherChannel : Channel;
	int nRetVal = m_I2CSite.GetSiteChannel(usSiteNo, SCLChannel, SDAChannel);
	if (0 > nRetVal)
	{
		return -1;
	}
	return 0;
}

void CI2C::EnableCopmareShield(BOOL bEnable)
{
	m_pBoardManage->EnableCopmareShield(bEnable);
}

inline void CI2C::Initialize(BOOL bRead)
{
	int nIndex = I2C_WRITE_INDEX;
	if (bRead)
	{
		nIndex = I2C_READ_INDEX;
	}
	CI2CBase* pI2C = m_I2C[nIndex];
	if (nullptr == pI2C)
	{
		pI2C = new CI2CBase(bRead, m_I2CSite, m_pAlarm, m_pBoardManage, m_pRAMManage);
		pI2C->SetREGByte(m_byRegisterByteCount);
		m_I2C[nIndex] = pI2C;
	}
	CI2CLineInfo::Instance()->SetI2CBase(m_I2C[0], m_I2C[1]);
}

int CI2C::Run(BOOL bRead)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CRunWrite");
	CTimer::Instance()->Start("RunI2CBase");
#endif // RECORD_TIME

	m_bLastRunSuccess = FALSE;
	BYTE byI2CIndex = I2C_WRITE_INDEX;
	if (bRead)
	{
		byI2CIndex = I2C_READ_INDEX;
	}
	CI2CBase* pI2C = m_I2C[byI2CIndex];
	if (nullptr == pI2C)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		return -1;
	}
	int nRetVal = pI2C->Run();

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("FreeLine");
#endif // RECORD_TIME
	if (0 == nRetVal)
	{
		m_bLastRunSuccess = TRUE;
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		return 0;
	}
	
	nRetVal = CI2CLineInfo::Instance()->FreeLine(nRetVal);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("RunAgain");
#endif // RECORD_TIME
	if (0 == nRetVal)
	{
		pI2C->Run();
	}
	else
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		return -2;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

	return 0;
}

void CI2C::LoadCommonPattern()
{
	if (m_bLoadCommonLine)
	{
		return;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Start("Create I2CLine");
#endif // RECORD_TIME


	CI2CLine I2CLine(TRUE, m_I2CSite, m_pAlarm, m_pBoardManage, m_pRAMManage);
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Load common line");
#endif // RECORD_TIME
	I2CLine.InitialCommonLine();
	m_bLoadCommonLine = TRUE;


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

}

void CI2C::Reset(BOOL bIncludeChannel)
{
	if (bIncludeChannel)
	{
		m_I2CSite.Reset();
	}
	m_I2C[I2C_READ_INDEX]->Reset();
	m_I2C[I2C_WRITE_INDEX]->Reset();
	m_pRAMManage->Clear();
	m_pBoardManage->Reset();
	m_strSCLChannel.clear();
	m_strSDAChannel.clear();
}

int CI2C::Connect(BOOL bConnect)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}

	vector<USHORT> vecChannel;
	vector<BYTE> vecUseBoard;
	m_I2CSite.GetUseBoard(vecUseBoard);
	for (auto Board : vecUseBoard)
	{
		auto iterBoard = m_mapBoard.find(Board);
		if (m_mapBoard.end() == iterBoard)
		{
			CHardwareFunction* pHardware = new CHardwareFunction(Board, m_pAlarm);
			m_mapBoard.insert(make_pair(Board, pHardware));
			iterBoard = m_mapBoard.find(Board);
		}
		if (!iterBoard->second->IsBoardExisted())
		{
			continue;
		}
		m_I2CSite.GetBoardChannel(Board, vecChannel);
		iterBoard->second->SetFunctionRelay(vecChannel, bConnect);
	}
	return 0;
}

int CI2C::SetPeriod(double dPeriod)
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	m_dPeriod = dPeriod;
	int nRetVal = SetPeriod();
	if (0 != nRetVal)
	{
		return -2;
	}
	return 0;
}

int CI2C::Write(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount, const BYTE* const pbyWriteData[])
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("Write, 0x%02X", (int)bySlaveAddress);
	CTimer::Instance()->Start("CheckParam");
#endif // RECORD_TIME

	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	if (I2C_WRITE_MAX_BYTE_COUNT < uDataByteCount || 0 == uDataByteCount)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_SIZE_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("nDataLength");
		m_pAlarm->SetAlarmMsg("The I2C write byte(%d) is over range[1, 1000].", uDataByteCount);
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
		return -2;
	}
	if (nullptr == pbyWriteData)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("pulDataArray");
		m_pAlarm->SetAlarmMsg("The address point of write data is nullptr.");
		return -3;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("OperationParam");
#endif // RECORD_TIME

	int nRetVal = 0;
	SetOperationParam(FALSE, bySlaveAddress, uRegisterAddress, uDataByteCount);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetSiteData");
#endif // RECORD_TIME

	USHORT uSiteCount = GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
	{
		nRetVal = SetSiteData(usSiteNo, pbyWriteData[usSiteNo]);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -2:
			case -3:
				//Site over range, checked
				break;
			case -4:
				//The array point is nullptr
				nRetVal = -4;
				break;
			default:
				break;
			}
			return nRetVal;
			break;
		}
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Run");
#endif // RECORD_TIME

	nRetVal = Run();
	if (0 != nRetVal)
	{
		//Line not enough
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		nRetVal = -3;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\I2CWrite_MultiTask.csv");
#endif // RECORD_TIME

	return nRetVal;
}

int CI2C::Write(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount, const BYTE* pbyWriteData)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("Write, 0x%02X", (int)bySlaveAddress);
	CTimer::Instance()->Start("CheckParam");
#endif // RECORD_TIME

	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	if (I2C_WRITE_MAX_BYTE_COUNT < uDataByteCount || 0 == uDataByteCount)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_SIZE_OVER_RANGE);
		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
		m_pAlarm->SetParamName("nDataLength");
		m_pAlarm->SetAlarmMsg("The I2C write byte(%d) is over range[1, 1000].", uDataByteCount);
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
		return -2;
	}
	if (nullptr == pbyWriteData)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetParamName("pbyWriteData");
		m_pAlarm->SetAlarmMsg("The point of  data written is nullptr.");
		return -3;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("OperationParam");
#endif // RECORD_TIME

	int nRetVal = 0;
	SetOperationParam(FALSE, bySlaveAddress, uRegisterAddress, uDataByteCount);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SetSiteData");
#endif // RECORD_TIME

	USHORT uSiteCount = GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
	{
		nRetVal = SetSiteData(usSiteNo, pbyWriteData);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -2:
			case -3:
				//Site over range, checked
				break;
			case -4:
				//The array point is nullptr
				nRetVal = -4;
				break;
			default:
				break;
			}
			return nRetVal;
			break;
		}
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Run");
#endif // RECORD_TIME

	nRetVal = Run();
	if (0 != nRetVal)
	{
		//Line not enough
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		nRetVal = -3;
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\I2CWrite.csv");
#endif // RECORD_TIME

	return nRetVal;
}

int CI2C::Read(BYTE bySlaveAddress, UINT uRegisterAddress, UINT uDataByteCount)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("I2CRead 0x%02X", (int)bySlaveAddress);
	CTimer::Instance()->Start("CheckParam");
#endif // RECORD_TIME
	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		return -1;
	}
	if (I2C_READ_MAX_BYTE_COUNT < uDataByteCount || 0 == uDataByteCount)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_DATA_SIZE_OVER_RANGE);
		m_pAlarm->SetParamName("nDataCount");
		m_pAlarm->SetAlarmMsg("The I2C read byte(%d) is over range[1, 100].", uDataByteCount);
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
		return -3;
	}
	int nRetVal = 0;

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("OperationParam");
#endif // RECORD_TIME

	SetOperationParam(TRUE, bySlaveAddress, uRegisterAddress, uDataByteCount);

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Run");
#endif // RECORD_TIME

	nRetVal = Run();
	if (0 != nRetVal)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		nRetVal = -2;
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\I2CRead.csv");
#endif // RECORD_TIME
	return nRetVal;
}

int CI2C::GetReadByteCount()
{
	if (0 == m_I2CSite.GetSiteCount())
	{
		return -1;
	}
	if (!m_bLatestRead)
	{
		return -2;
	}
	return m_I2C[I2C_READ_INDEX]->GetDataByteCount();
}

int CI2C::SlotParse(const char* lpszPin, std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	if (nullptr == lpszPin)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetAlarmMsg("The point of channel information string is nullptr.");
		return -1;
	}
	
	int nRetVal = 0;
	string strSitePin;
	string strPin = lpszPin;
	strPin.erase(0, strPin.find_first_not_of(' '));
	strPin.erase(strPin.find_last_not_of(' ') + 1);
	if (0 == strPin.size())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_BLANK);
		m_pAlarm->SetAlarmMsg("The channel information string is blank.");
		return -2;
	}
	int nPos = 0;
	while (-1 != nPos)
	{
		USHORT usCurSiteNo = vecChannel.size();

#ifdef RECORD_TIME
		CTimer::Instance()->Start("Get SITE_%d Channel", usCurSiteNo + 1);
#endif // RECORD_TIME

		nPos = strPin.find(",");
		if (-1 == nPos)
		{
			if (0 == strPin.size())
			{
				continue;
			}
			strSitePin = strPin;
			strPin.clear();
		}
		else
		{
			strSitePin = strPin.substr(0, nPos);
			strPin.erase(0, nPos + 1);
		}

		nPos = strSitePin.find("S");
		string strTemp = strSitePin;
		if (-1 == nPos)
		{
			nPos = strSitePin.find("s");
			if (-1 == nPos)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
				m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s) is wrong, no slot sign \"S\" or \"s\".",
					lpszPin, usCurSiteNo + 1, strSitePin.c_str());
				nRetVal = -3;
				break;
			}
		}
		strTemp.erase(0, nPos + 1);
		nPos = strTemp.find("_");
		if (-1 == nPos)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s), no channel sign \"_\".",
				lpszPin, usCurSiteNo + 1, strSitePin.c_str());
			nRetVal = -4;
			break;
		}
		else if (0 == nPos)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s) is wrong, no slot information.",
				lpszPin, usCurSiteNo + 1, strSitePin.c_str());
			nRetVal = -5;
			break;
		}
		CHANNEL_INFO Channel;
		string strSlot = strTemp.substr(0, nPos);
		Channel.m_bySlotNo = atoi(strSlot.c_str());
		if (0 == Channel.m_bySlotNo && 0 != strSlot[0])
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s) is wrong, the slot(%s) is error.",
				lpszPin, usCurSiteNo + 1, strSitePin.c_str(), strSlot.c_str());
			nRetVal = -6;
			break;
		}
		strTemp.erase(0, nPos + 1); 
		if (0 == strTemp.size())
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s) is wrong, no channel information.",
				lpszPin, usCurSiteNo + 1, strSitePin.c_str());
			nRetVal = -5;
			break;
		}
		Channel.m_usChannel = atoi(strTemp.c_str());
		if (0 == Channel.m_usChannel && '0' != strTemp[0])
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel information format(%s) of SITE_%d(%s) is wrong, the channel(%s) is error.",
				lpszPin, usCurSiteNo + 1, strSitePin.c_str(), strTemp.c_str());
			nRetVal = -6;
			break;
		}
		if (DCM_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG);
			m_pAlarm->SetAlarmMsg("The channel(%d) of SITE_%d(%s) is over range[0,%d].", 
				Channel.m_usChannel, usCurSiteNo + 1, strSitePin.c_str(), DCM_MAX_CHANNELS_PER_BOARD - 1);
			nRetVal = -7;
			break;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Check channel existed");

		CTimer::Instance()->Start("Create Board");
#endif // RECORD_TIME

		CI2CBoard* pBoard = m_pBoardManage->GetBoard(Channel.m_bySlotNo, m_pAlarm);

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("Check Channel");
#endif // RECORD_TIME
		
		if (nullptr == pBoard && m_I2CSite.IsSiteValid(usCurSiteNo))
		{
			//The channel used is not existed
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
			m_pAlarm->SetSite(usCurSiteNo);
			m_pAlarm->SetAlarmMsg("The channel in SITE_%d(S%d_%d) is not existed.", usCurSiteNo + 1, Channel.m_bySlotNo, Channel.m_usChannel);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -8;

#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
#endif // RECORD_TIME
			break;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		nRetVal = 0;

		vecChannel.push_back(Channel);
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

	return nRetVal;
}

int CI2C::CheckChannel(const std::vector<CHANNEL_INFO>& vecSCL, const std::vector<CHANNEL_INFO>& vecSDA)
{
	USHORT usSiteCount = vecSCL.size();
	if (usSiteCount != vecSDA.size())
	{
		return -1;
	}
	int nRetVal = 0;
	BOOL bConflict = FALSE;
	UINT uChannel = 0;
	char lpszChannel[32] = { 0 };
	CHANNEL_INFO Channel;
	set<UINT> setChannel;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		for (BYTE byIndex = 0; byIndex < 2;++byIndex)
		{
			if (0 == byIndex)
			{
				Channel = vecSCL[usSiteNo];
			}
			else
			{
				Channel = vecSDA[usSiteNo];
			}
			uChannel = (Channel.m_bySlotNo << 24) | Channel.m_usChannel;
			if (setChannel.end() != setChannel.find(uChannel))
			{
				bConflict = TRUE;
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_CHANNEL_CONFICT);
				if (0 == byIndex)
				{
					m_pAlarm->SetParamName("lpszSCLChannel");
					strcpy_s(lpszChannel, sizeof(lpszChannel), "lpszSCLChannel");
				}
				else
				{
					m_pAlarm->SetParamName("lpszSDAChannel");
					strcpy_s(lpszChannel, sizeof(lpszChannel), "lpszSDAChannel");
				}
				m_pAlarm->SetAlarmMsg("The channel of SITE_%d(%s:S%d_%d) has been used in other site.",
					usSiteNo + 1, lpszChannel, Channel.m_bySlotNo, Channel.m_usChannel);
				nRetVal = -2;
				break;
			}
			setChannel.insert(uChannel);
		}
		if (bConflict)
		{
			break;
		}
	}
	setChannel.clear();
	return nRetVal;
}

int CI2C::SetEdge(BOOL bSCL, const double* pdEdge, BOOL bOnlyValidSite)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("SetEdge, %d", bSCL);
	CTimer::Instance()->Start("Start");
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

	if (0 == m_I2CSite.GetSiteCount())
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_I2C_NOT_SET_SITE);
		m_pAlarm->SetAlarmMsg("Not set the channel information through I2CSet before.");
		return -1;
	}
	if (nullptr == pdEdge)
	{
		return -2;
	}
	WAVE_FORMAT WaveFormat;
	if (bSCL)
	{
		WaveFormat = WAVE_FORMAT::RO;
	}
	else
	{
		WaveFormat = WAVE_FORMAT::NRZ;
	}
	int nRetVal = 0;
	map<BYTE, vector<USHORT>> mapChannel;
	m_I2CSite.GetChannel(bSCL, mapChannel, bOnlyValidSite);
	for (auto& Channel : mapChannel)
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Start("CreateBoard %d", Channel.first);
#endif // RECORD_TIME

		CI2CBoard* pI2CBoard = m_pBoardManage->GetBoard(Channel.first, m_pAlarm);

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
		CTimer::Instance()->Start("SetBoardEdge");
#endif // RECORD_TIME

#ifdef RECORD_TIME
		CTimer::Instance()->Start("CheckBoardExisted S%d", Channel.first);
#endif // RECORD_TIME
		if (nullptr != pI2CBoard)
		{
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
			CTimer::Instance()->Start("SetBoardEdge S%d", Channel.first);
#endif // RECORD_TIME
			nRetVal = pI2CBoard->SetEdge(Channel.second, pdEdge, WaveFormat, IO_FORMAT::NRZ);
			if (0 != nRetVal)
			{
				switch (nRetVal)
				{
				case -1:
					///<Board not existed, ignore
					break;
				case -2:
					///<The point of edge is nullptr,checked
					break;
				case -3:
					///<The format is error, not happened
					break;
				case -4:
					///<The edge is over range
					nRetVal = -3;
				default:
					break;
				}
				break;
			}
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
#endif // RECORD_TIME
		}
#ifdef RECORD_TIME
		else
		{
			CTimer::Instance()->Stop();
		}
#endif // RECORD_TIME

#ifdef RECORD_TIME
		CTimer::Instance()->Stop(); 
#endif // RECORD_TIME

	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Print("D:\\SetTime.csv");
#endif // RECORD_TIME

	return nRetVal;
}

int CI2C::SetPeriod(BOOL bOnlyValidSite)
{
	vector<BYTE> vecUseBoard;
	m_I2CSite.GetUseBoard(vecUseBoard,bOnlyValidSite);
	int nRetVal = 0;
	BYTE byUseBoardCount = vecUseBoard.size();
	for (auto Board : vecUseBoard)
	{
		CI2CBoard* pI2CBoard = m_pBoardManage->GetBoard(Board, m_pAlarm);
		if (nullptr == pI2CBoard)
		{
			continue;
		}
		nRetVal = pI2CBoard->SetPeriod(m_dPeriod, bOnlyValidSite);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Not set site information, not happen
				break;
			case -2:
				///<The board is not existed
				break;
			case -3:
				///<The period is over range
				nRetVal = -1;
			default:
				break;
			}
			return nRetVal;
		}
	}
	return 0;
}

template<typename Key, typename Value>
inline void CI2C::DeleteMemory(std::map<Key, Value>& mapParam)
{
	for (auto& Item : mapParam)
	{
		if (nullptr != Item.second)
		{
			delete Item.second;
			Item.second = nullptr;
		}
	}
	mapParam.clear();
}