#include "pch.h"
#include "TMU.h"
using namespace std;

CTMU* CTMU::Instance()
{
    static CTMU TMU;
    return &TMU;
}

int CTMU::SetUnitChannelMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusUnitChannel)
{
    UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
    if (m_mapChannel.end() != m_mapChannel.find(uControllerID))
    {
        return -1;
    }
    if (nullptr == pusUnitChannel)
    {
        return -1;
    }
    m_mapChannel.insert(make_pair(uControllerID, pusUnitChannel));
    return 0;
}

int CTMU::SetModeMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyUnitMode)
{
    UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
    if (m_mapMode.end() != m_mapMode.find(uControllerID))
    {
        return -1;
    }
    if (nullptr == pbyUnitMode)
    {
        return -2;
    }
    m_mapMode.insert(make_pair(uControllerID, pbyUnitMode));
    return 0;
}

int CTMU::SetTriggerEdgeMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyTriggerEdge)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	if (m_mapTriggerEdge.end() != m_mapTriggerEdge.find(uControllerID))
	{
		return -1;
	}
	if (nullptr == pbyTriggerEdge)
	{
		return -2;
	}
	m_mapTriggerEdge.insert(make_pair(uControllerID, pbyTriggerEdge));
	return 0;
}

int CTMU::SetHoldOffMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusHolfOffTime, USHORT* pusHoldOffNum)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	if (m_mapHoldOffTime.end() != m_mapHoldOffTime.find(uControllerID))
	{
		return -1;
	}
	if (nullptr == pusHolfOffTime || nullptr == pusHoldOffNum)
	{
		return -2;
	}
	m_mapHoldOffTime.insert(make_pair(uControllerID, pusHolfOffTime));
	m_mapHoldOffNum.insert(make_pair(uControllerID, pusHoldOffNum));
    return 0;
}

int CTMU::SetSampleNumberMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusSampleNum)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	if (m_mapSampleNum.end() != m_mapSampleNum.find(uControllerID))
	{
		return -1;
	}
	if (nullptr == pusSampleNum)
	{
		return -2;
	}
	m_mapSampleNum.insert(make_pair(uControllerID, pusSampleNum));
	return 0;
}

int CTMU::SetTimeoutMemory(BYTE bySlotNo, BYTE byControllerIndex, float* pfTimeout)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	if (m_mapTimeout.end() != m_mapTimeout.find(uControllerID))
	{
		return -1;
	}
	if (nullptr == pfTimeout)
	{
		return -2;
	}
	m_mapTimeout.insert(make_pair(uControllerID, pfTimeout));
	return 0;
}

int CTMU::SetChannel(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usChannel)
{
    UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
    auto iterController = m_mapChannel.find(uControllerID);
    if (m_mapChannel.end() == iterController)
    {
        return -1;
    }
    if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnitIndex)
    {
        return -2;
    }
    if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
    {
        return -3;
    }
    iterController->second[byUnitIndex] = usChannel;
    return 0;
}

int CTMU::GetChannel(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex)
{
    UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
    auto iterController = m_mapChannel.find(uControllerID);
    if (m_mapChannel.end() == iterController || nullptr == iterController->second)
    {
        return -1;
    }
    if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
    {
        return -2;
    }

    return iterController->second[byUnitIndex];
}

int CTMU::SetMode(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, BYTE byMode)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapMode.find(uControllerID);
	if (m_mapMode.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
    iterController->second[byUnitIndex] = byMode;
    return 0;
}

int CTMU::GetMode(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapMode.find(uControllerID);
	if (m_mapMode.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
    return iterController->second[byUnitIndex];
}

int CTMU::SetTriggerEdge(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, BOOL bRaiseEdge)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapTriggerEdge.find(uControllerID);
	if (m_mapTriggerEdge.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	iterController->second[byUnitIndex] = bRaiseEdge;
    return 0;
}

int CTMU::GetTriggerEdge(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapTriggerEdge.find(uControllerID);
	if (m_mapTriggerEdge.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	return iterController->second[byUnitIndex];
}

int CTMU::SetHoldOff(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usHoldOffTime, USHORT usHoldOffNum)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterTimeController = m_mapHoldOffTime.find(uControllerID);
	auto iterNumController = m_mapHoldOffNum.find(uControllerID);
	if (m_mapHoldOffTime.end() == iterTimeController || nullptr == iterTimeController->second
		|| m_mapHoldOffNum.end() == iterNumController || nullptr == iterNumController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	iterTimeController->second[byUnitIndex] = usHoldOffTime;
	iterNumController->second[byUnitIndex] = usHoldOffNum;
	return 0;
}

int CTMU::GetHoldOff(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT& usHoldOffTime, USHORT& usHolfOffNum)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterTimeController = m_mapHoldOffTime.find(uControllerID);
	auto iterNumController = m_mapHoldOffNum.find(uControllerID);
	if (m_mapHoldOffTime.end() == iterTimeController || nullptr == iterTimeController->second
		|| m_mapHoldOffNum.end() == iterNumController || nullptr == iterNumController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	usHoldOffTime = iterTimeController->second[byUnitIndex];
	usHolfOffNum = iterNumController->second[byUnitIndex];
	return 0;
}

int CTMU::SetSampleNumber(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usSampleNum)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapSampleNum.find(uControllerID);
	if (m_mapSampleNum.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	iterController->second[byUnitIndex] = usSampleNum;
	return 0;
}

int CTMU::GetSampleNumber(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapSampleNum.find(uControllerID);
	if (m_mapSampleNum.end() == iterController)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}

	return iterController->second[byUnitIndex];
}

int CTMU::SetTimeout(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, float fTimeout)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapTimeout.find(uControllerID);
	if (m_mapTimeout.end() == iterController)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	iterController->second[byUnitIndex] = fTimeout;
	return 0;
}

float CTMU::GetTimeout(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapTimeout.find(uControllerID);
	if (m_mapTimeout.end() == iterController)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER < byUnitIndex)
	{
		return -2;
	}
	return iterController->second[byUnitIndex];
}

CTMU::CTMU()
{
}

BYTE CTMU::GetControllerID(BYTE bySlotNo, BYTE byControllerIndex)
{
    return bySlotNo <<8 | byControllerIndex;
}
