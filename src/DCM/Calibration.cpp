#include "Calibration.h"
using namespace std;

CCalibration* CCalibration::Instance()
{
	static CCalibration Calibration;
	return &Calibration;
}

int CCalibration::SetCalibrationMemory(BYTE bySlotNo, BYTE byController, CAL_DATA* pCalData)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	if (m_mapCalibrationData.end() != m_mapCalibrationData.find(uControllerID))
	{
		return -1;
	}

	m_mapCalibrationData.insert(make_pair(uControllerID, pCalData));
	return 0;
}

int CCalibration::SetCalibration(BYTE bySlotNo, BYTE byController, const CAL_DATA* pCalData)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapCalibrationData.find(uControllerID);
	if (m_mapCalibrationData.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	memcpy_s(iterController->second, DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA), pCalData, DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA));
	return 0;
}

int CCalibration::SetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, const CAL_DATA* pCalData)
{
	auto iterController = m_mapCalibrationData.find(GetControllerID(bySlotNo, byController));
	if (m_mapCalibrationData.end() == iterController)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	memcpy_s(&iterController->second[usChannel], sizeof(CAL_DATA), pCalData, sizeof(CAL_DATA));

	return 0;
}

int CCalibration::SetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, CAL_TYPE CalType, PMU_IRANGE IRange, float fGain, float fOffset)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapCalibrationData.find(uControllerID);
	if (m_mapCalibrationData.end() == iterController)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	float* pfGain = nullptr;
	float* pfOffset = nullptr;
	if (nullptr == pfGain)
	{
		switch (IRange)
		{
		case PMU_IRANGE::IRANGE_2UA:
			break;
		case PMU_IRANGE::IRANGE_20UA:
			break;
		case PMU_IRANGE::IRANGE_200UA:
			break;
		case PMU_IRANGE::IRANGE_2MA:
			break;
		case PMU_IRANGE::IRANGE_32MA:
			break;
		default:
			return -4;
			break;
		}
	}
	BYTE byIRangeCode = (BYTE)IRange;
	switch (CalType)
	{
	case CCalibration::CAL_TYPE::CAL_DVH:
		pfGain = iterController->second[usChannel].m_fDVHGain;
		pfOffset = iterController->second[usChannel].m_fDVHOffset;
		break;
	case CCalibration::CAL_TYPE::CAL_DVL:
		pfGain = iterController->second[usChannel].m_fDVLGain;
		pfOffset = iterController->second[usChannel].m_fDVLOffset;
		break;
	case CCalibration::CAL_TYPE::CAL_FV:
		pfGain = &iterController->second[usChannel].m_fFVGain[byIRangeCode];
		pfOffset = &iterController->second[usChannel].m_fFVOffset[byIRangeCode];
		break;
	case CCalibration::CAL_TYPE::CAL_FI:
		pfGain = &iterController->second[usChannel].m_fFIGain[byIRangeCode];
		pfOffset = &iterController->second[usChannel].m_fFIOffset[byIRangeCode];
		break;
	case CCalibration::CAL_TYPE::CAL_MV:
		pfGain = iterController->second[usChannel].m_fMVGain;
		pfOffset = iterController->second[usChannel].m_fMVOffset;
		break;
	case CCalibration::CAL_TYPE::CAL_MI:
		pfGain = &iterController->second[usChannel].m_fMIGain[byIRangeCode];
		pfOffset = &iterController->second[usChannel].m_fMIOffset[byIRangeCode];
		break;
	default:
		return -3;
		break;
	}
	*pfGain = fGain;
	*pfOffset = fOffset;

	return 0;
}

int CCalibration::ResetCalibrationData(BYTE bySlotNo, BYTE byController, USHORT usChannel)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterControler = m_mapCalibrationData.find(uControllerID);
	if (m_mapCalibrationData.end() == iterControler || nullptr == iterControler->second)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	CAL_DATA CalData;
	GetDefaultCalibrationData(CalData);
	memcpy_s(&iterControler->second[usChannel], sizeof(CAL_DATA), &CalData, sizeof(CAL_DATA));
	return 0;
}

int CCalibration::ResetCalibrationData(BYTE bySlotNo, BYTE byController)
{
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterControler = m_mapCalibrationData.find(uControllerID);
	if (m_mapCalibrationData.end() == iterControler || nullptr == iterControler->second)
	{
		return -1;
	}
	CAL_DATA CalData;
	GetDefaultCalibrationData(CalData);
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		memcpy_s(&iterControler->second[usChannel], sizeof(CAL_DATA), &CalData, sizeof(CAL_DATA));
	}
	return 0;
}

int CCalibration::GetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, CAL_DATA& CalData)
{
	auto iterController = m_mapCalibrationData.find(GetControllerID(bySlotNo, byController));
	if (m_mapCalibrationData.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -2;
	}
	memcpy_s(&CalData, sizeof(CalData), &iterController->second[usChannel], sizeof(CalData));
	return 0;
}

int CCalibration::GetCalibration(BYTE bySlotNo, BYTE byController, CAL_DATA* pCalData)
{
	auto iterController = m_mapCalibrationData.find(GetControllerID(bySlotNo, byController));
	if (m_mapCalibrationData.end() == iterController || nullptr == iterController->second)
	{
		return -1;
	}
	if (nullptr == pCalData)
	{
		return -2;
	}
	memcpy_s(pCalData, DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA), iterController->second, DCM_CHANNELS_PER_CONTROL * sizeof(CAL_DATA));
	return 0;
}

CCalibration::CCalibration()
{
}

inline UINT CCalibration::GetControllerID(BYTE bySlotNo, BYTE byController)
{
	return bySlotNo << 24 | byController;
}

inline void CCalibration::GetDefaultCalibrationData(CAL_DATA& CalData)
{
	CalData.m_fDVHGain[0] = 1;
	CalData.m_fDVHOffset[0] = 0;
	CalData.m_fDVLGain[0] = 1;
	CalData.m_fDVLOffset[0] = 0;
	CalData.m_fMVGain[0] = 1;
	CalData.m_fMVOffset[0] = 0;
	CalData.m_fFVGain[0] = 1;
	CalData.m_fFVOffset[0] = 0;
	for (BYTE byRange = 0; byRange < PMU_IRANGE_COUNT; ++byRange)
	{
		CalData.m_fFIGain[byRange] = 1;
		CalData.m_fFIOffset[byRange] = 0;
		CalData.m_fMIGain[byRange] = 1;
		CalData.m_fMIOffset[byRange] = 0;
	}
}
