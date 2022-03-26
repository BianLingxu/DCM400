#include "ChannelMode.h"
using namespace std;
CChannelMode* CChannelMode::Instance()
{
	static CChannelMode ChannelStatus;
	return &ChannelStatus;
}

int CChannelMode::SetMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyChannelMode)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	if (m_mapChannelMode.end() != m_mapChannelMode.find(uControllerID))
	{
		return -1;
	}
	m_mapChannelMode.insert(make_pair(uControllerID, pbyChannelMode));
	return 0;
}

int CChannelMode::SetUnexpectMode(COperation& pOperation, const std::vector<USHORT>& vecChannel, CHANNEL_MODE UnexceptMode)
{
	UINT uControllerID = GetControllerID(pOperation.GetSlotNo(), pOperation.GetControllerIndex());
	auto iterController = m_mapChannelMode.find(uControllerID);
	if (m_mapChannelMode.end() == iterController)
	{
		return -1;
	}
	BYTE* pbyChannel = iterController->second;
	map<USHORT, ULONG> mapChannelValue;
	for (auto Channel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= Channel)
		{
			return -2;
		}
		if ((BYTE)UnexceptMode == pbyChannel[Channel])
		{
			pbyChannel[Channel] = (BYTE)CHANNEL_MODE::NEITHER_MODE;
			mapChannelValue.insert(make_pair(Channel, 1));
		}
	}
	if (0 == mapChannelValue.size())
	{
		return 0;
	}
	pOperation.Write305(0x0C, mapChannelValue);
	return 0;
}

int CChannelMode::UpdateMode(COperation& pOperation)
{
	UINT uControllerID = GetControllerID(pOperation.GetSlotNo(), pOperation.GetControllerIndex());
	auto iterController = m_mapChannelMode.find(uControllerID);
	if (m_mapChannelMode.end() == iterController)
	{
		return -1;
	}
	map<USHORT, ULONG> mapChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		mapChannel.insert(make_pair(usChannel, 0));
	}
	BYTE* pbyMode = iterController->second;
	pOperation.Read305(0x0C, mapChannel);
	BYTE byMode = (BYTE)CHANNEL_MODE::MCU_MODE;
	for (auto& Channel : mapChannel)
	{
		if (0 == (Channel.second & 0x01))
		{
			byMode = (BYTE)CHANNEL_MODE::MCU_MODE;
		}
		else if (1 == (Channel.second >> 2 & 0x01))
		{
			byMode = (BYTE)CHANNEL_MODE::PMU_MODE;
		}
		else
		{
			byMode = (BYTE)CHANNEL_MODE::NEITHER_MODE;
		}
		pbyMode[Channel.first] = byMode;
	}
	return 0;
}

int CChannelMode::SetChannelMode(COperation& Operation, const std::vector<USHORT>& vecChannel, CHANNEL_MODE ChannelMode)
{
	ULONG ulREGValue = 0;
	switch (ChannelMode)
	{
	case CChannelMode::CHANNEL_MODE::MCU_MODE:
		ulREGValue = 0;
		break;
	case CChannelMode::CHANNEL_MODE::PMU_MODE:
		ulREGValue = 5;
		break;
	case CChannelMode::CHANNEL_MODE::NEITHER_MODE:
		ulREGValue = 1;
		break;
	default:
		break;
	}
	BYTE* pbyMode = nullptr;
	auto iterChannel = m_mapChannelMode.find(GetControllerID(Operation.GetSlotNo(), Operation.GetControllerIndex()));
	if (m_mapChannelMode.end() != iterChannel)
	{
		pbyMode = iterChannel->second;
	}
	BOOL bCurModeDiff = TRUE;
	map<USHORT, ULONG> mapChannelValue;
	for (auto Channel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= Channel)
		{
			return -1;
		}
		bCurModeDiff = FALSE;
		if (nullptr != pbyMode)
		{
			if (pbyMode[Channel] != (BYTE)ChannelMode)
			{
				pbyMode[Channel] = (BYTE)ChannelMode;
				bCurModeDiff = TRUE;
			}
		}
		else
		{
			bCurModeDiff = TRUE;
		}
		if (bCurModeDiff)
		{
			mapChannelValue.insert(make_pair(Channel, ulREGValue));
		}
	}
	if (0 == mapChannelValue.size())
	{
		return 0;
	}
	Operation.Write305(0x0C, mapChannelValue);
	return 0;
}

int CChannelMode::SaveChannelMode(BYTE bySlotNo, BYTE byControllerIndex, const std::vector<USHORT>& vecChannel, CHANNEL_MODE ChannelMode)
{
	auto iterMode = m_mapChannelMode.find(GetControllerID(bySlotNo, byControllerIndex));
	if (m_mapChannelMode.end() == iterMode)
	{
		return -1;
	}
	for (auto Channel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= Channel)
		{
			continue;
		}
		iterMode->second[Channel] = (BYTE)ChannelMode;
	}
	return 0;
}

CChannelMode::CHANNEL_MODE CChannelMode::GetChannelMode(COperation& Operation, USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return CHANNEL_MODE::NEITHER_MODE;
	}
	map<USHORT, ULONG> mapREGValue = { {usChannel, 0} };
	Operation.Read305(0x0C, mapREGValue);
	ULONG ulREGValue = mapREGValue.begin()->second;

	CHANNEL_MODE CurMode = CHANNEL_MODE::NEITHER_MODE;

	if (0 == (ulREGValue & 0x01))
	{
		CurMode = CHANNEL_MODE::MCU_MODE;
	}
	else if (1 == (ulREGValue >> 2 & 0x01))
	{
		CurMode = CHANNEL_MODE::PMU_MODE;
	}
	vector<USHORT> vecChannel = { usChannel };
	SaveChannelMode(Operation.GetSlotNo(), Operation.GetControllerIndex(), vecChannel, CurMode);
	return CurMode;
}

CChannelMode::CChannelMode()
{

}

UINT CChannelMode::GetControllerID(BYTE bySlotNo, BYTE byControllerIndex)
{
	return bySlotNo << 24 | byControllerIndex;
}

