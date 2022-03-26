#include "HDDebugToolItemPmuMeas.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuMeas::HDDebugToolItemPmuMeas(HDDebugTool * debugTool)
    : HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuMeas::~HDDebugToolItemPmuMeas()
{

}

int HDDebugToolItemPmuMeas::Type() const
{
	return ITEM_PMU_MEAS;
}
const char * HDDebugToolItemPmuMeas::Name() const
{
    return "PMU_MEAS";
}

int HDDebugToolItemPmuMeas::CanModifyData() const
{
    return 0;
}

int HDDebugToolItemPmuMeas::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	USHORT usChannel = 0;
	double dGetValue = 0;

	const char* lpszPinName = dcm_getPinName(logicChannel);
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (1 != nMode)
		{
			return 0;
		}
		BOOL bDCMWithVector = TRUE;
		int nInstanceID = dcm_GetPinInstanceID(logicChannel, bDCMWithVector);
		if (!bDCMWithVector)
		{
			return 0;
		}
		dcm_SetInstanceID(nInstanceID);
		dGetValue = dcm_getPpmuMultiMeasResult(lpszPinName, site, AVERAGE_RESULT);
	}
	else
	{
		return 0;
	}

	char unit[4] = { 0 };
	double dGain = 1;
	double dAbsMeas = fabs(dGetValue);
	if (dAbsMeas > 1E-15)
	{
		if (dAbsMeas < 0.000001)
		{
			dGain = 1e9;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "n");
		}
		else if (dAbsMeas < 0.001)
		{
			dGain = 1e6;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "u");
		}
		else if (dAbsMeas < 1)
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
	BYTE byIRANGE = 0;
	BYTE byMeasType = 0;

	int nRetVal = dcm_getPpmuMeasStatus(logicChannel, site, byIRANGE, byMeasType);

	if (0 == nRetVal)
	{
		if (0 == byMeasType)//MV
		{
			sts_strcat(unit, STS_ARRAY_SIZE(unit), "V");
		}
		else
		{
			sts_strcat(unit, STS_ARRAY_SIZE(unit), "A");
		}
	}
	else
	{
		sts_strcat(unit, STS_ARRAY_SIZE(unit), "V");
	}

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.3f%s", tempGetValue, unit);

	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

    return 0;
}