#include "PMU.h"
#include "HardwareFunction.h"
using namespace std;

#define BIND_SET_BEIGIN(bySlotNo, byController, vecChannel)\
set<BYTE> setSlot;\
set<BYTE> setController;\
CBindInfo::Instance()->GetBindInfo(setSlot, setController);\
setSlot.insert(bySlotNo);\
setController.insert(byController);\
for (auto bySlot : setSlot){\
	for (auto byController : setController){\
	 uControllerID = GetControllerID(bySlot, byController);

#define BIND_SET_END }\
}

int CPMU::SetAverageDataMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusAverageData)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapAverageData.find(uControllerID);
	if (m_mapAverageData.end() != iterController)
	{
		return -1;
	}

	m_mapAverageData.insert(make_pair(uControllerID, pusAverageData));
	return 0;
}

int CPMU::SetSampleModeMemory(BYTE bySlotNo, BYTE byControllerIndex, UINT* puTimes, double* pdPeriod)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapSampleSetting.find(uControllerID);
	if (m_mapSampleSetting.end() != iterController)
	{
		return -1;
	}
	m_mapSampleSetting.insert(make_pair(uControllerID, SAMPLE_SETTING()));
	iterController = m_mapSampleSetting.find(uControllerID);
	iterController->second.m_pdPeriod = pdPeriod;
	iterController->second.m_puSampleTimes = puTimes;
	return 0;
}

int CPMU::SetMeasureModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucMeasureType)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapMeasureMode.find(uControllerID);
	if (m_mapMeasureMode.end() != iterController)
	{
		return -1;
	}
	m_mapMeasureMode.insert(make_pair(uControllerID, pucMeasureType));
	return 0;
}

int CPMU::SetLatestMeasureModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucMeasureType)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapLatestMeasureMode.find(uControllerID);
	if (m_mapLatestMeasureMode.end() != iterController)
	{
		return -1;
	}
	m_mapLatestMeasureMode.insert(make_pair(uControllerID, pucMeasureType));
	return 0;
}

int CPMU::SetForceModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucForceMode, unsigned char* pucIRange)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterController = m_mapForceMode.find(uControllerID);
	if (m_mapForceMode.end() != iterController)
	{
		return -1;
	}

	m_mapForceMode.insert(make_pair(uControllerID, FORCE_MODE()));
	iterController = m_mapForceMode.find(uControllerID);
	iterController->second.m_pucForceMode = pucForceMode;
	iterController->second.m_pucIRange = pucIRange;
	return 0;
}

int CPMU::SaveAverageData(COperation& Operation, BYTE byGetChip, UINT uSampleCount, BYTE byChipEvenChannel)
{
	if (0 == uSampleCount)
	{
		return -1;
	}
	ULONG ulAD7606[DCM_CHANNELS_PER_CONTROL / 2] = { 0 };
	int nRetVal = Operation.Read7606(byGetChip, uSampleCount, ulAD7606);
	if (0 != nRetVal)
	{
		return -2;
	}
	BYTE bySlotNo = Operation.GetSlotNo();
	BYTE byControllerIndex = Operation.GetControllerIndex();
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterData = m_mapAverageData.find(uControllerID);
	if (m_mapAverageData.end() == iterData)
	{
		return -3;
	}

	USHORT usChannelIndex = 0;

	USHORT usChannel = 0;
	USHORT usChannelCount = DCM_CHANNELS_PER_CONTROL / 2;
	for (USHORT usChannelIndex = 0; usChannelIndex < usChannelCount; ++usChannelIndex)
	{
		if (0 == (byGetChip >> usChannelIndex & 0x01))
		{
			continue;
		}
		if (0 == (byChipEvenChannel >> usChannelIndex & 0x01))
		{
			usChannel = usChannelIndex * 2 + 1;
		}
		else
		{
			usChannel = usChannelIndex * 2;
		}
		iterData->second[usChannel] = ulAD7606[usChannelIndex];
	}
	return 0;
}

void CPMU::SetSampleSetting(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, UINT uSampleCount, double dPeriod)
{
	UINT uControllerID = 0;
	BIND_SET_BEIGIN(bySlotNo, byController, vecChannel)
	{
		auto iterLatestMeasMode = m_mapLatestMeasureMode.find(uControllerID);
		auto iterCurMeasMode = m_mapMeasureMode.find(uControllerID);

		auto iterSample = m_mapSampleSetting.find(uControllerID);
		if (m_mapSampleSetting.end() != iterSample)
		{
			for (USHORT usChannel : vecChannel)
			{
				iterSample->second.m_pdPeriod[usChannel] = dPeriod;
				iterSample->second.m_puSampleTimes[usChannel] = uSampleCount;
				
				iterCurMeasMode->second[usChannel] = iterLatestMeasMode->second[usChannel];
			}

		}
	}
	BIND_SET_END
}

int CPMU::GetSampleSetting(BYTE bySlotNo, BYTE byController, USHORT usChannel, UINT& uSampleCount, double& dPeriod)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterSample = m_mapSampleSetting.find(uControllerID);
	if (m_mapSampleSetting.end() == iterSample)
	{
		return -1;
	}
	else if (nullptr == iterSample->second.m_pdPeriod || nullptr == iterSample->second.m_puSampleTimes)
	{
		return -2;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -3;
	}
	uSampleCount = iterSample->second.m_puSampleTimes[usChannel];
	dPeriod = iterSample->second.m_pdPeriod[usChannel];
	return 0;
}

void CPMU::SetMeasureMode(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, unsigned char ucMeasureMode)
{
	UINT uControllerID = 0;
	BIND_SET_BEIGIN(bySlotNo, byController, vecChannel)
	{
		auto iterControllerID = m_mapLatestMeasureMode.find(uControllerID);
		if (m_mapLatestMeasureMode.end() != iterControllerID)
		{
			for (USHORT usChannel : vecChannel)
			{
				if (DCM_CHANNELS_PER_CONTROL <= usChannel)
				{
					continue;
				}
				iterControllerID->second[usChannel] = ucMeasureMode;
			}
		}
	}
	BIND_SET_END
}

int CPMU::GetMeasureMode(BYTE bySlotNo, BYTE byController, USHORT usChannel, unsigned char& ucMeasureMode)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapMeasureMode.find(uControllerID);
	if (m_mapMeasureMode.end() == iterController)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	ucMeasureMode = iterController->second[usChannel];
	return 0;
}

int CPMU::SetForceMode(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, unsigned char ucForceMode, unsigned char ucIRange)
{
	UINT uControllerID = 0;
	BIND_SET_BEIGIN(bySlotNo, byController, vecChannel)
	{
		auto iterControllerID = m_mapForceMode.find(uControllerID);
		if (m_mapForceMode.end() != iterControllerID)
		{
			for (USHORT usChannel : vecChannel)
			{
				if (DCM_CHANNELS_PER_CONTROL <= usChannel)
				{
					continue;
				}
				iterControllerID->second.m_pucForceMode[usChannel] = ucForceMode;
				iterControllerID->second.m_pucIRange[usChannel] = ucIRange;
			}
		}
	}
	BIND_SET_END
	return 0;
}

int CPMU::GetForceMode(BYTE bySlotNo, BYTE byController, USHORT usChannel, unsigned char& ucForceMode, unsigned char& ucIRange)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapForceMode.find(uControllerID);
	if (m_mapForceMode.end() == iterController)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	ucForceMode = iterController->second.m_pucForceMode[usChannel];
	ucIRange = iterController->second.m_pucIRange[usChannel];
	return 0;
}

int CPMU::GetAverageData(BYTE bySlotNo, BYTE byControllerIndex, USHORT usChannel)
{
	UINT uControllerID = GetControllerID(bySlotNo, byControllerIndex);
	auto iterData = m_mapAverageData.find(uControllerID);
	if (m_mapAverageData.end() == iterData)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}

	if (nullptr == iterData->second)
	{
		return -1;
	}
	return iterData->second[usChannel];
}

CPMU::CPMU()
{

}

inline UINT CPMU::GetControllerID(BYTE bySlotNo, BYTE byControllerIndex)
{
	return bySlotNo << 24 | byControllerIndex;
}

CPMU* CPMU::Instance()
{
	static CPMU PMUData;
	return &PMUData;
}

CPMU::~CPMU()
{
}