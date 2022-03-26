#include "HDDebugToolItemPmuSamples.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuSamples::HDDebugToolItemPmuSamples(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuSamples::~HDDebugToolItemPmuSamples()
{

}

int HDDebugToolItemPmuSamples::Type() const
{
	return ITEM_PMU_SAMPLES;
}

const char* HDDebugToolItemPmuSamples::Name() const
{
	return "PMU_SAMPLES";
}

int HDDebugToolItemPmuSamples::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	int nSampleTimes = 0;
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
	nSampleTimes = dcm_getSampleTimes(logicChannel, site);
	if (0 > nSampleTimes)
	{
		nSampleTimes = 0;
	}

	data = STSVariant(nSampleTimes);

	return 0;
}