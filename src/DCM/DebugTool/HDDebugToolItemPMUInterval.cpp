#include "HDDebugToolItemPmuInterval.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuInterval::HDDebugToolItemPmuInterval(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuInterval::~HDDebugToolItemPmuInterval()
{

}

int HDDebugToolItemPmuInterval::Type() const
{
	return ITEM_PMU_INTERVAL;
}

const char* HDDebugToolItemPmuInterval::Name() const
{
	return "PMU_INTERVAL";
}

int HDDebugToolItemPmuInterval::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	int nSamplePeriod = 0;	
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (1 != nMode)
		{
			return 0;
		}
	}
	nSamplePeriod = dcm_getSampleInterval(logicChannel, site);
	if (0 > nSamplePeriod)
	{
		nSamplePeriod = 0;
	}

	char unit[3] = { 0 };
	double dGain = 1;
	if (0 < nSamplePeriod)
	{
		if (1 > nSamplePeriod )
		{
			dGain = 1e3;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "ns");
		}
		else if (1e3 > nSamplePeriod)
		{
			dGain = 1;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "us");
		}
		else if (1e6 > nSamplePeriod)
		{
			dGain = 1e-3;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "ms");
		}
		else
		{
			dGain = 1e6;
			sts_strcpy(unit, STS_ARRAY_SIZE(unit), "s");
		}
	}
	else
	{
		dGain = 1e6;
		sts_strcpy(unit, STS_ARRAY_SIZE(unit), "s");
	}

	double samplePeriodData = nSamplePeriod * dGain;

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.2f%s", samplePeriodData, unit);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}