#include "Bind.h"
#include "STSSP8201.h"
using namespace std;
CBindInfo* CBindInfo::Instance()
{
	static CBindInfo BindInfo;
	return &BindInfo;
}
BOOL CBindInfo::IsBind()
{
	return 0 == m_setFollowSlot.size() ? FALSE : TRUE;
}

BYTE CBindInfo::GetBindInfo(std::set<BYTE>& setSlot, std::set<BYTE>& setController)
{
	setSlot = m_setFollowSlot;
	setController = m_setBindController;
	return m_byTargetSlot;
}

int CBindInfo::Bind(const std::set<BYTE>& setSlot, const std::set<BYTE>& setController, BYTE byTargetSlot)
{
	if (0 == byTargetSlot)
	{
		return -1;
	}
	BYTE byControllerCount = setController.size();
	if (0 == byControllerCount)
	{
		return -2;
	}
	if (0 == byTargetSlot)
	{
		return -3;
	}
	if (0 != m_setFollowSlot.size())
	{
		return -4;
	}
	m_byTargetSlot = byTargetSlot;
	m_setFollowSlot = setSlot;
	m_setBindController = setController;
	BYTE byBindController = 0;
	for (auto Controller : m_setBindController)
	{
		byBindController |= 1 << Controller;
	}
	for (auto Slot : setSlot)
	{
		///<Write bind information to board
		Bind(Slot, byBindController, m_byTargetSlot);
	}
	return 0;
}

int CBindInfo::GetBindControllerCount()
{
	return m_setFollowSlot.size() * m_setBindController.size();
}

CBindInfo::CBindInfo()
{
}

CBindInfo::~CBindInfo()
{
}

void CBindInfo::ClearBind()
{
	for (auto BindBoard : m_setFollowSlot)
	{
		Bind(BindBoard, 0, BindBoard);
	}
	m_setFollowSlot.clear();
	m_setBindController.clear();
	m_byTargetSlot = -1;
}


inline void CBindInfo::Bind(BYTE byFollowSlot, BYTE byBindData, BYTE byTargetSlot)
{
	ULONG ulAddress = (0x3F1 << 14) + ((byFollowSlot - 1) << 2);
	ULONG ulData = (byBindData << 16) + byTargetSlot;
	write_dw(ulAddress, ulData);
}

