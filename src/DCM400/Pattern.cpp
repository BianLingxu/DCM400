#include "pch.h"
#include "Pattern.h"
#include "CMDCode.h"
using namespace std;
CPattern::CPattern(CHardwareFunction& HardwareFunction, CDriverAlarm* pAlarm)
	: m_pHardware(&HardwareFunction)
	, m_pAlarm(pAlarm)
{

}

CPattern::~CPattern()
{

}

int CPattern::AddChannelPattern(USHORT usChannel, UINT uPatternLine, char cPattern, const CPatternCMD& PatternCMD)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (DCM400_MAX_PATTERN_COUNT <= uPatternLine)
	{
		return -2;
	}

	auto iterData = m_mapVectorData.find(uPatternLine);
	if (m_mapVectorData.end() == iterData)
	{
		CVectorData VectorData(m_pAlarm);
		m_mapVectorData.insert(make_pair(uPatternLine, VectorData));
		iterData = m_mapVectorData.find(uPatternLine);
	}
	int nRetVal = iterData->second.SetChannelData(usChannel, cPattern);
	if (0 != nRetVal)
	{
		///<The pattern is not supported
		return -3;
	}
	iterData->second.SetCommandData(PatternCMD.GetSpecifiedCMDData());

	auto iterCMD = m_mapCMDData.find(uPatternLine);
	if (m_mapCMDData.end() == iterCMD)
	{
		CCMDData CMDData;
		m_mapCMDData.insert(make_pair(uPatternLine, CMDData));
		iterCMD = m_mapCMDData.find(uPatternLine);
	}
	nRetVal = iterCMD->second.SetData(PatternCMD.GetGeneralCMDData());
	
	return 0;
}

int CPattern::Load()
{
	if (0 == m_mapCMDData.size() || 0 == m_mapVectorData.size())
	{
		return 0;
	}

	int nElementCount = (m_mapCMDData.rbegin()->first - m_mapCMDData.begin()->first + 1) * 8;
	ULONG* pulData = nullptr;
	
	try
	{
		pulData = new ULONG[nElementCount];
		memset(pulData, 0, nElementCount * sizeof(ULONG));
	}
	catch (const std::exception& e)
	{
		///<Allocate memory fail
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			m_pAlarm->SetAlarmMsg("Allocate memory fail:%s", e.what());
		}
		return -2;
	}

	int nStartLineNo = m_mapVectorData.begin()->first;
	int nElementIndex = 0;
	int nCurElementIndex = 0;

	///<Download pattern data
	for (auto& Data : m_mapVectorData)
	{
		nCurElementIndex = Data.first - nStartLineNo;
		if (nCurElementIndex == nElementIndex)
		{
			memcpy_s(&pulData[nCurElementIndex * 2], 2 * sizeof(ULONG), Data.second.GetVectorData(), 2 * sizeof(ULONG));
		}
		else
		{
			m_pHardware->ReadPatternMemory(nStartLineNo + nCurElementIndex, 1, pulData);
		}
		++nElementIndex;
	}
	m_pHardware->WritePatternMemory(nStartLineNo, m_mapVectorData.size(), pulData);

	///<Download command data
	memset(pulData, 0, nElementCount * sizeof(ULONG));
	nElementIndex = 0;
	nCurElementIndex = 0;

	///<Download pattern data
	for (auto& Data : m_mapVectorData)
	{
		nCurElementIndex = Data.first - nStartLineNo;
		if (nCurElementIndex == nElementIndex)
		{
			memcpy_s(&pulData[nCurElementIndex * 8], 8 * sizeof(ULONG), Data.second.GetVectorData(), 8 * sizeof(ULONG));
		}
		else
		{
			m_pHardware->ReadCMDMemory(nStartLineNo + nCurElementIndex, 1, pulData);
		}
		++nElementIndex;
	}
	m_pHardware->WritePatternMemory(nStartLineNo, m_mapVectorData.size(), pulData);
	m_mapVectorData.clear();
	m_mapCMDData.clear();
	if (nullptr != pulData)
	{
		delete[] pulData;
		pulData = nullptr;
	}
	return 0;
}

CVectorData::CVectorData(CDriverAlarm* pAlarm)
	: m_pAlarm(pAlarm)
{
	memset(m_aulData, 0, sizeof(m_aulData));
}

int CVectorData::SetChannelData(USHORT usChannel,  char cPattern)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	BYTE byCodeData = 0;
	switch (cPattern)
	{
	case '0':
		byCodeData = 1;
		break;
	case '1':
		byCodeData = 7;
		break;
	case 'H':
		byCodeData = 6;
		break;
	case 'L':
		byCodeData = 0;
		break;
	case 'X':
		byCodeData = 5;
		break;
	case 'M':
		byCodeData = 4;
		break;
	case 'V':
		byCodeData = 2;
		break;
	case 'S':
		byCodeData = 3;
		break;
	default:
		return -2;
		break;
	}

	BYTE byTargetBitPos = usChannel * 3;
	for (BYTE byBit = 0; byBit < 3; ++byBit, ++byTargetBitPos)
	{
		if (0 == (byCodeData >> byBit & 0x01))
		{
			m_aulData[byTargetBitPos / 32] &= ~(1 << byTargetBitPos % 32);
		}
		else
		{
			m_aulData[byTargetBitPos / 32] |= 1 << byTargetBitPos % 32;
		}
	}
	return 0;
}

void CVectorData::SetCommandData(USHORT usCMDData)
{
	m_aulData[1] &= 0x0000FFFF;
	m_aulData[1] |= usCMDData << 16;
}

const ULONG* CVectorData::GetVectorData()
{
	return m_aulData;
}

CCMDData::CCMDData()
{
	memset(m_aulData, 0, sizeof(m_aulData));
}

int CCMDData::SetData(const ULONG* pulData)
{
	if (nullptr == pulData)
	{
		return -1;
	}
	memcpy_s(m_aulData, sizeof(m_aulData), pulData, sizeof(m_aulData));
	return 0;
}

const ULONG* CCMDData::GetCMDData()
{
	return m_aulData;
}
