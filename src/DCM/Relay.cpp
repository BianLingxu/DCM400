#include "Relay.h"
#include <set>
using namespace std;
#define CAL_DUTG0K	79
#define CAL_DUTG1K	78
#define CAL_DUTG2K	77
#define CAL_DUTG3K	76
#define DC_FORCEK	72
#define DC_SENSEK	73
#define CAL_FORCEK	79
#define CAL_SENSEK	78
#define CAL_GUARDK	77


BYTE g_byFuncInvCtrlBit[TOTAL_RELAY_COUNT] = { 61, 63, 57, 59, 58, 56, 62, 60, 50, 48, 54, 52, 53, 55, 49, 51, 70, 71, 47, 45, 43, 41, 40, 42, 44, 46, 32, 34, 36, 38, 39, 37,
 35, 33, 69, 68, 67, 66, 19, 17, 23, 21, 20, 22, 16, 18, 28, 30, 26, 24, 25, 27, 29, 31, 64, 65, 1 , 3 , 5 , 7 , 6 , 4 , 2 ,0, 14, 12, 10, 8 , 9 , 11, 13, 15};

BYTE g_byDCInvCtrlBit[TOTAL_RELAY_COUNT] = { 63, 61, 59, 57, 56, 58, 60, 62, 48, 50, 52, 54, 55, 53, 51, 49, 71, 70, 47, 45, 43, 41, 40, 42, 44, 46, 32, 34, 36, 38, 39, 37,
35, 33, 68, 69, 66, 67, 17, 19, 21, 23, 22, 20, 18, 16, 30, 28, 26, 24, 25, 27, 29, 31, 64, 65, 1 , 3 , 5 , 7 , 6 , 4 , 2 ,0, 14, 12, 10, 8 , 9 , 11, 13, 15 };

BYTE g_byFuncCtrlBit[TOTAL_RELAY_COUNT] = { 63, 56, 62, 57, 61, 58, 60, 59, 67, 68, 66, 69, 65, 70, 64, 71, 44, 39, 45, 38, 42, 41, 43, 40, 49, 50, 48, 51, 46, 52, 47, 53,
26, 33, 27, 32, 28, 31, 29, 30, 22, 21, 23, 20, 24, 19, 25, 18, 9, 14, 8, 15, 11, 12, 10, 13, 5, 2, 4, 3, 7,0, 6, 1, 54, 55, 37, 36, 35, 34, 16, 17 };

BYTE g_byDCCtrlBit[TOTAL_RELAY_COUNT] = { 63, 56, 62, 57, 61, 58, 60, 59, 67, 68, 66, 69, 65, 70, 64, 71, 45, 38, 44, 39, 43, 40, 42, 41, 49, 50, 48, 51, 47, 52, 46, 53,
26, 33, 27, 32, 28, 31, 29, 30, 22, 21, 23, 20, 24, 19, 25, 18, 8, 15, 9, 14, 10, 13, 11, 12, 4, 3, 5, 2, 6, 1, 7,0, 54, 55, 36, 37, 34, 35, 17, 16 };

CRelay::CRelay(COperation& Operation)
	: m_pulFunctionREGData(nullptr)
	, m_pulDCREGData(nullptr)
{
	m_pRelaySafety = CRelaySafety::Instance();
	m_pOperation = &Operation;
	CRelayRigister::Instance()->GetFuncRelayREG(m_pOperation->GetSlotNo(), m_pulFunctionREGData);
	CRelayRigister::Instance()->GetDCRelayREG(m_pOperation->GetSlotNo(), m_pulDCREGData);
	m_bConnect = FALSE;
	m_bOperation = FALSE;
	m_bDCRelay = FALSE;
}

int CRelay::FunctionRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect)
{
	m_bOperation = FALSE;
	m_bDCRelay = FALSE;
	int nRetVal = 0;
	m_bConnect = bConnect;
	BYTE bySlotNo = m_pOperation->GetSlotNo();
	m_pRelaySafety->Apply(bySlotNo);

	for (USHORT usChannel : vecChannel)
	{
		nRetVal = SetFuncRelay(usChannel);
		if (0 != nRetVal)
		{
			m_pRelaySafety->Release(bySlotNo);
			return -1;
		}
	}
	if (m_bOperation)
	{
		UpdateRelay();
		UINT uDelay = 100;
		if (bConnect)
		{
			uDelay = 500;
		}
		m_pOperation->WaitUs(uDelay);
	}
	m_pRelaySafety->Release(bySlotNo);
	return 0;
}

int CRelay::CalRelay(USHORT usChannel, BOOL bConnect)
{
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	BYTE bySlotNo = m_pOperation->GetSlotNo();
	m_pRelaySafety->Apply(bySlotNo);

	m_bOperation = FALSE;
	m_bConnect = bConnect;
	memset(m_pulFunctionREGData, 0, RELAY_REGISTER_COUNT * sizeof(ULONG));
	memset(m_pulDCREGData, 0, RELAY_REGISTER_COUNT * sizeof(ULONG));
	if (m_bConnect)
	{
		SetFuncRelay(usChannel);
		SetDCRelay(usChannel);
		SetCalRelay(usChannel);
	}
	else
	{
		///<Get the relay status of the board
		m_bDCRelay = TRUE;
		m_bOperation = TRUE;
	}
	if (m_bOperation)
	{
		UpdateRelay();
		m_pRelaySafety->Release(bySlotNo);

		m_pOperation->WaitUs(700);
	}
	else
	{
		m_pRelaySafety->Release(bySlotNo);
	}
	return 0;
}

int CRelay::SetHighVoltageRelay(USHORT usChannel, BOOL bConnect)
{
	///<0, 2, 16, 18, 32, 34, 48, 50
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	if (0 != usChannel % DCM_CHANNELS_PER_CONTROL && 2 != usChannel % DCM_CHANNELS_PER_CONTROL)
	{
		return -2;
	}

	BYTE bySlotNo = m_pOperation->GetSlotNo();
	m_pRelaySafety->Apply(bySlotNo);

	m_bConnect = bConnect;
	m_bOperation = FALSE;
	m_bDCRelay = FALSE;
	BYTE byRelayIndex = (usChannel % DCM_CHANNELS_PER_CONTROL / 2) + (usChannel / DCM_CHANNELS_PER_CONTROL) * 2 + DCM_MAX_CHANNELS_PER_BOARD;

	BYTE byREGIndex = g_byFuncCtrlBit[byRelayIndex] / 32; 
	BitSet(&m_pulFunctionREGData[byREGIndex], g_byFuncCtrlBit[byRelayIndex] % 32);
	if (m_bOperation)
	{
		UpdateRelay();

		m_pRelaySafety->Release(bySlotNo);

		UINT uDelay = 100;
		if (bConnect)
		{
			uDelay = 500;
		}
		m_pOperation->WaitUs(uDelay);
	}
	else
	{
		m_pRelaySafety->Release(bySlotNo);
	}
	return 0;
}

int CRelay::GetConnectedChannel(RELAY_TYPE RelayType, std::vector<USHORT>& vecConnectedChannel)
{
	vecConnectedChannel.clear();
	BYTE* pbyInvCtrlBit = g_byFuncInvCtrlBit;

	BOOL bRelayStatus[TOTAL_RELAY_COUNT] = { 0 };///<The function relay status of each channel and high voltage channel
	BYTE* pbyCtrlBit = g_byFuncCtrlBit;
	BYTE byREGOffset = 0;
	BOOL bFuncRelay = TRUE;
	switch (RelayType)
	{
	case RELAY_TYPE::FUNC_RELAY:
	case RELAY_TYPE::HIGH_VOLTAGE_RELAY:
		pbyInvCtrlBit = g_byFuncInvCtrlBit;
		break;
	case RELAY_TYPE::DC_RELAY:
		pbyInvCtrlBit = g_byDCInvCtrlBit;
		bFuncRelay = FALSE;
		byREGOffset = 3;
		break;
	default:
		return -1;
		break;
	}
	BYTE byBaseIndex = 0;

	BYTE bySlotNo = m_pOperation->GetSlotNo();
	m_pRelaySafety->Apply(bySlotNo);

	for (int nREGIndex = 0; nREGIndex < RELAY_REGISTER_COUNT;++nREGIndex)
	{
		byBaseIndex = nREGIndex * 32;
		ULONG ulData = m_pOperation->ReadBoard(0xA010 + byREGOffset + nREGIndex);
		for (int nIndex = 0; nIndex < 32; ++nIndex)
		{
			if (TOTAL_RELAY_COUNT <= nIndex + byBaseIndex)
			{
				break;
			}
			bRelayStatus[pbyInvCtrlBit[nIndex + byBaseIndex]] = (ulData >> nIndex) & 0x01;
		}
	}
	if (RELAY_TYPE::HIGH_VOLTAGE_RELAY != RelayType)
	{
		for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
		{
			if (bRelayStatus[usChannel])
			{
				vecConnectedChannel.push_back(usChannel);
			}
		}
		SynREGValue(bRelayStatus, bFuncRelay);
	}
	else
	{
		for (BYTE byBaseIndex = 0, nRelayIndex = DCM_MAX_CHANNELS_PER_BOARD; byBaseIndex < 8; ++byBaseIndex, ++nRelayIndex)
		{
			if (bRelayStatus[nRelayIndex])
			{
				vecConnectedChannel.push_back(byBaseIndex / 2 * DCM_CHANNELS_PER_CONTROL + byBaseIndex % 2 * 2);
			}
		}
	}
	m_pRelaySafety->Release(bySlotNo);
	return 0;
}

int CRelay::GetRelayChannel(RELAY_TYPE RelayType, std::vector<USHORT>& vecChannel)
{
	vecChannel.clear();
	switch (RelayType)
	{
	case RELAY_TYPE::FUNC_RELAY:
	case RELAY_TYPE::DC_RELAY:
		for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD;++usChannel)
		{
			vecChannel.push_back(usChannel);
		}
		break;
	case RELAY_TYPE::HIGH_VOLTAGE_RELAY:
		for (USHORT usChannel = 0; usChannel <8;++usChannel)
		{
			if (0 == usChannel % 2)
			{
				vecChannel.push_back(usChannel * 8);
			}
			else
			{
				vecChannel.push_back((usChannel - 1) * 8 + 2);
			}
		}
		break;
	default:
		return -1;
		break;
	}
	return 0;
}

int CRelay::SetFuncRelay(USHORT usChannel)
{
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	BYTE byREGIndex = g_byFuncCtrlBit[usChannel] / 32;
	BitSet(&m_pulFunctionREGData[byREGIndex], g_byFuncCtrlBit[usChannel] % 32);
	return 0;
}

int CRelay::SetCalRelay(USHORT usChannel)
{
	if (TOTAL_RELAY_COUNT <= usChannel)
	{
		return -1;
	}
	m_bDCRelay = TRUE;
	BYTE byBitIndex = usChannel / 16;
	if (4 <= byBitIndex)
	{
		///<The high voltage relay
		byBitIndex = (usChannel - DCM_MAX_CHANNELS_PER_BOARD) / 2;
	}
	
	BYTE byCtrlBit[4] = { CAL_DUTG0K - 64, CAL_DUTG1K - 64, CAL_DUTG2K - 64, CAL_DUTG3K - 64 };

	BitSet(&m_pulFunctionREGData[2], byCtrlBit[byBitIndex]);

	BYTE bySetDCREG[5] = { DC_FORCEK - 64, DC_SENSEK - 64, CAL_FORCEK - 64, CAL_SENSEK - 64, CAL_GUARDK - 64 };
	
	for (auto Bit : bySetDCREG)
	{
		BitSet(&m_pulDCREGData[2], Bit);
	}

	return 0;
}

int CRelay::SetDCRelay(USHORT usChannel)
{
	if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	m_bDCRelay = TRUE;
	BYTE byREGIndex = g_byDCCtrlBit[usChannel] / 32;
	BitSet(&m_pulDCREGData[byREGIndex], g_byDCCtrlBit[usChannel] % 32);
	return 0;
}

void CRelay::UpdateRelay()
{
	for (BYTE byREGIndex = 0; byREGIndex < RELAY_REGISTER_COUNT;++byREGIndex)
	{
		m_pOperation->WriteBoard(0x20 + byREGIndex, m_pulFunctionREGData[byREGIndex]);
	}
	m_pOperation->WriteBoard(0x23, 1);
	m_pOperation->WriteBoard(0x23, 0);

	if (m_bDCRelay)
	{
		for (BYTE byREGIndex = 0; byREGIndex < RELAY_REGISTER_COUNT; ++byREGIndex)
		{
			m_pOperation->WriteBoard(0x40 + byREGIndex, m_pulDCREGData[byREGIndex]);
		}
		m_pOperation->WriteBoard(0x43, 1);
		m_pOperation->WriteBoard(0x43, 0);
	}
}

inline void CRelay::BitSet(ULONG* pulData, BYTE byBitIndex)
{
	if (nullptr == pulData)
	{
		return;
	}
	int nData = 1 << byBitIndex;
	if (m_bConnect)
	{
		if (!m_bOperation && 0 == (*pulData >> byBitIndex & 0x01))
		{
			m_bOperation = TRUE;
		}
		*pulData |= nData;
	}
	else
	{
		if (!m_bOperation && 1 == (*pulData >> byBitIndex & 0x01))
		{
			m_bOperation = TRUE;
		}
		*pulData &= ~nData;
	}
}

inline void CRelay::SynREGValue(const BOOL* bRelayStatus, BOOL bFuncRelay)
{
	memset(m_pulFunctionREGData, 0, RELAY_REGISTER_COUNT * sizeof(ULONG));
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		m_bConnect = bRelayStatus[usChannel];
		if (bFuncRelay)
		{
			SetFuncRelay(usChannel);
		}
		else
		{
			SetDCRelay(usChannel);
		}
	}
}

int CRelayRigister::SetDCRelayMem(BYTE bySlotNo, ULONG* pulRelayREG)
{
	auto iterSlot = m_mapCalRelay.find(bySlotNo);
	if (m_mapCalRelay.end() == iterSlot)
	{
		m_mapCalRelay.insert(make_pair(bySlotNo, nullptr));
		iterSlot = m_mapCalRelay.find(bySlotNo);
	}
	iterSlot->second = pulRelayREG;
	return 0;
}

int CRelayRigister::GetDCRelayREG(BYTE bySlotNo, ULONG*& pulDCRelay)
{
	auto iterSlot = m_mapCalRelay.find(bySlotNo);
	if (m_mapCalRelay.end() == iterSlot)
	{
		return -1;
	}
	pulDCRelay = iterSlot->second;
	return 0;
}

CRelayRigister::CRelayRigister()
{

}

CRelayRigister* CRelayRigister::Instance()
{
	static CRelayRigister RelayRegister;
	return &RelayRegister;
}

CRelayRigister::~CRelayRigister()
{
}

int CRelayRigister::SetFuncRelayMem(BYTE bySlotNo, ULONG* pulFuncRelay)
{
	auto iterSlot = m_mapFunctionRelay.find(bySlotNo);
	if (m_mapFunctionRelay.end() == iterSlot)
	{
		m_mapFunctionRelay.insert(make_pair(bySlotNo, nullptr));
		iterSlot = m_mapFunctionRelay.find(bySlotNo);
	}
	iterSlot->second = pulFuncRelay;
	return 0;
}

int CRelayRigister::GetFuncRelayREG(BYTE bySlotNo, ULONG*& pulFuncRelay)
{
	auto iterSlot = m_mapFunctionRelay.find(bySlotNo);
	if (m_mapFunctionRelay.end() == iterSlot)
	{
		return -1;
	}
	pulFuncRelay = iterSlot->second;
	return 0;
}

CRelaySafety* CRelaySafety::Instance()
{
	static CRelaySafety RelaySafety;
	return &RelaySafety;
}

void CRelaySafety::Apply(BYTE bySlotNo)
{
	EnterCriticalSection(GetCritical(bySlotNo));
}

void CRelaySafety::Release(BYTE bySlotNo)
{
	LeaveCriticalSection(GetCritical(bySlotNo));
}

CRelaySafety::~CRelaySafety()
{
	for (auto& Critical : m_mapSlotSafety)
	{
		if (nullptr != Critical.second)
		{
			DeleteCriticalSection(Critical.second);
			delete Critical.second;
			Critical.second = nullptr;
		}
	}
	m_mapSlotSafety.clear();
	DeleteCriticalSection(&m_criDistribution);
}

CRelaySafety::CRelaySafety()
{
	InitializeCriticalSection(&m_criDistribution);
}

CRITICAL_SECTION* CRelaySafety::GetCritical(BYTE bySlotNo)
{
	CRITICAL_SECTION* pCritical = nullptr;
	EnterCriticalSection(&m_criDistribution);
	auto iterCritical = m_mapSlotSafety.find(bySlotNo);
	if (m_mapSlotSafety.end() == iterCritical)
	{
		pCritical = new CRITICAL_SECTION;
		InitializeCriticalSection(pCritical);
		m_mapSlotSafety.insert(make_pair(bySlotNo, pCritical));
	}
	else
	{
		pCritical = iterCritical->second;
	}
	LeaveCriticalSection(&m_criDistribution);
	return pCritical;
}
