#include "HDDebugToolItemPmuForce.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuForce::HDDebugToolItemPmuForce(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuForce::~HDDebugToolItemPmuForce()
{

}

int HDDebugToolItemPmuForce::Type() const
{
	return ITEM_PMU_FORCE;
}

const char* HDDebugToolItemPmuForce::Name() const
{
	return "PMU_FORCE";
}

int HDDebugToolItemPmuForce::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	BYTE PMUMode = 0xFF;
	BYTE IRange = 0xFF;
	double dGetValue = 0;

	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (1 != nMode)
		{
			return 0;
		}
		dGetValue = dcm_GetPPMUSetValue(nSlotNo, usChannel, PMUMode, IRange);
	}
	else
	{
		return 0;
	}

	char unit[4] = { 0 };
	double dGain = 1;	

	double dAbsForceVlue = fabs(dGetValue);

	if (dAbsForceVlue > 1E-15)
	{
		if (dAbsForceVlue < 0.000001)
		{
			dGain = 1e9;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "n");
		}
		else if (dAbsForceVlue < 0.001)
		{
			dGain = 1e6;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "u");
		}
		else if (dAbsForceVlue < 1)
		{
			dGain = 1e3;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "m");
		}
		else
		{
			dGain = 1;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "");
		}
	}
	else
	{
		dGain = 1;
		sts_strcpy(unit, STS_ARRAY_SIZE(unit), "");
	}
	double tempGetValue = dGetValue * dGain;

	switch (PMUMode)
	{
	case (int)PMU_MODE::FVMI:
	case (int)PMU_MODE::FVMV:
		sts_strcat(unit, STS_ARRAY_SIZE(unit), "V");
		break;
	case (int)PMU_MODE::FIMI:
	case (int)PMU_MODE::FIMV:
		sts_strcat(unit, STS_ARRAY_SIZE(unit), "A");
		break;	
	default:
		sts_strcat(unit, STS_ARRAY_SIZE(unit), "V");
		break;
	}
	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.3f%s", tempGetValue, unit);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}