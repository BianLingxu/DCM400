#include "pch.h"
#include "Relay.h"
#define HIGH_VOLTAGE_RELAY_COUNT 8
#define FUNC_RELAY_COUNT 2

USHORT g_ausHighVoltageChannel[HIGH_VOLTAGE_RELAY_COUNT] = { 0, 2, 16, 18, 32, 34, 48, 50 };///<The channels with high voltage relay

CRelay::CRelay(COperation& Operation)
	: m_pOperation(&Operation)
{
}

int CRelay::FunctionRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect)
{
	BYTE bySlotNo = m_pOperation->GetSlotNo();
	CRelaySafety::Instance()->Apply(bySlotNo);
	USHORT usChannel = 0;
	ULONG* pulREG = nullptr;
	CRelayRigister::Instance()->GetRelayREG(bySlotNo, pulREG);
	ULONG aulREG[FUNC_RELAY_COUNT] = { 0 };
	memcpy_s(aulREG, sizeof(aulREG), pulREG, sizeof(aulREG));
	for (auto Channel : vecChannel)
	{
		if (DCM400_MAX_CHANNELS_PER_BOARD <= Channel)
		{
			///<The channel is over range
			return -1;
		}
		if (bConnect)
		{
			pulREG[Channel / 32] |= (1 << (Channel % 32));
		}
		else
		{
			pulREG[Channel / 32] &= ~(1 << (Channel % 32));
		}
	}
	BOOL bAction = FALSE;

	for (int nREGIndex = 0; nREGIndex < FUNC_RELAY_COUNT;++nREGIndex)
	{
		if (aulREG[nREGIndex] != pulREG[nREGIndex])
		{
			bAction = TRUE;
			m_pOperation->WriteRegister(0x15 + nREGIndex, pulREG[nREGIndex], TRUE, 3);
		}
	}

	if (bAction)
	{
		DWORD dwDelay = 100;
		if (bConnect)
		{
			dwDelay = 500;
		}
		m_pOperation->WaitUs(dwDelay);
	}
	CRelaySafety::Instance()->Release(bySlotNo);
	return 0;
}

int CRelay::SetHighVoltageRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect)
{
	BYTE bySlotNo = m_pOperation->GetSlotNo();
	CRelaySafety::Instance()->Apply(bySlotNo);
	ULONG* pulREG = nullptr;///<The relay register memory
	CRelayRigister::Instance()->GetRelayREG(bySlotNo, pulREG);

	ULONG ulREG = pulREG[2];
	for (auto Channel : vecChannel)
	{
		BYTE byRelayIndex = 0;
		for (; byRelayIndex < HIGH_VOLTAGE_RELAY_COUNT; byRelayIndex++)
		{
			if (Channel == g_ausHighVoltageChannel[byRelayIndex])
			{
				break;
			}
		}
		if (byRelayIndex >= HIGH_VOLTAGE_RELAY_COUNT)
		{
			CRelaySafety::Instance()->Release(bySlotNo);
			return -1;
		}
		if (bConnect)
		{
			ulREG |= (1 << byRelayIndex);
		}
		else
		{
			ulREG &= ~(1 << byRelayIndex);
		}
	}
	
	if (ulREG != pulREG[2])
	{
		pulREG[2] = ulREG;
		m_pOperation->WriteRegister(0x17, ulREG, TRUE, 3);
		DWORD dwDelay = 100;
		if (bConnect)
		{
			dwDelay = 500;
		}
		m_pOperation->WaitUs(dwDelay);
	}

	CRelaySafety::Instance()->Release(bySlotNo);
	return 0;
}

int CRelay::GetConnectedChannel(RELAY_TYPE RelayType, std::vector<USHORT>& vecConnectedChannel)
{
	BYTE bySlotNo = m_pOperation->GetSlotNo();
	CRelaySafety::Instance()->Apply(bySlotNo);
	ULONG* pulREG = nullptr;
	CRelayRigister::Instance()->GetRelayREG(bySlotNo, pulREG);
	vecConnectedChannel.clear();

	if (RELAY_TYPE::FUNC_RELAY == RelayType)
	{
		for (int nREGIndex = 0; nREGIndex < FUNC_RELAY_COUNT;++nREGIndex)
		{
			pulREG[nREGIndex] = m_pOperation->ReadRegister(0x15 + nREGIndex, TRUE, 3);
		}
		for (USHORT usChannel = 0; usChannel < DCM400_MAX_CHANNELS_PER_BOARD;++usChannel)
		{
			if (pulREG[usChannel / 32] & (1 << (usChannel % 32)))
			{
				vecConnectedChannel.push_back(usChannel);
			}
		}
	}
	else
	{
		pulREG[2] = m_pOperation->ReadRegister(0x17, TRUE, 3);
		for (BYTE byRelayIndex = 0; byRelayIndex < HIGH_VOLTAGE_RELAY_COUNT; ++byRelayIndex)
		{
			if (pulREG[2] & (1 << byRelayIndex))
			{
				vecConnectedChannel.push_back(g_ausHighVoltageChannel[byRelayIndex]);
			}
		}
	}
	CRelaySafety::Instance()->Release(bySlotNo);
	return 0;
}

void CRelay::GetHighVoltageChannel(std::set<USHORT>& setChannel)
{
	for (auto Channel : g_ausHighVoltageChannel)
	{
		setChannel.insert(Channel);
	}
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

int CRelayRigister::SetRelayMem(BYTE bySlotNo, ULONG* pulFuncRelay)
{
	auto iterSlot = m_mapRelayREG.find(bySlotNo);
	if (m_mapRelayREG.end() == iterSlot)
	{
		m_mapRelayREG.insert(make_pair(bySlotNo, nullptr));
		iterSlot = m_mapRelayREG.find(bySlotNo);
	}
	iterSlot->second = pulFuncRelay;
	return 0;
}

int CRelayRigister::GetRelayREG(BYTE bySlotNo, ULONG*& pulFuncRelay)
{
	auto iterSlot = m_mapRelayREG.find(bySlotNo);
	if (m_mapRelayREG.end() == iterSlot)
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
