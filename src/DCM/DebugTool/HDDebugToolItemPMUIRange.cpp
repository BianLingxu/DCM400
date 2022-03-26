#include "HDDebugToolItemPmuIRange.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuIRange::HDDebugToolItemPmuIRange(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuIRange::~HDDebugToolItemPmuIRange()
{

}

int HDDebugToolItemPmuIRange::Type() const
{
	return ITEM_PMU_IRNG;
}

const char* HDDebugToolItemPmuIRange::Name() const
{
	return "PMU_IRNG";
}

int HDDebugToolItemPmuIRange::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	BYTE IRange = 0xFF;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (1 != nMode)
		{
			return 0;
		}
		BYTE PMUMode = 0;
		dcm_GetPPMUSetValue(nSlotNo, usChannel, PMUMode, IRange);
	}

	string strIRange = "";
	switch (IRange)
	{
	case (int)PMU_IRANGE::IRANGE_2UA:
		strIRange = "2uA";
		break;
	case (int)PMU_IRANGE::IRANGE_20UA:
		strIRange = "20uA";
		break;
	case (int)PMU_IRANGE::IRANGE_200UA:
		strIRange = "200uA";
		break;
	case (int)PMU_IRANGE::IRANGE_2MA:
		strIRange = "2mA";
		break;
	case (int)PMU_IRANGE::IRANGE_32MA:
		strIRange = "32mA";
		break;
	default:
		strIRange = "ERROR";
		break;
	}

	data = STSVariant(strIRange.c_str(), strIRange.length() + 1, STSVariant::DT_STRING);

	return 0;
}