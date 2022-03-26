#include "HDDebugToolItemPmuMode.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemPmuMode::HDDebugToolItemPmuMode(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemPmuMode::~HDDebugToolItemPmuMode()
{

}

int HDDebugToolItemPmuMode::Type() const
{
	return ITEM_PMU_MODE;
}

const char* HDDebugToolItemPmuMode::Name() const
{
	return "PMU_MODE";
}

int HDDebugToolItemPmuMode::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	BYTE PMUMode = 255;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (1 != nMode)
		{
			return 0;
		}
		BYTE IRange = 0xFF;
		dcm_GetPPMUSetValue(nSlotNo, usChannel, PMUMode, IRange);
	}

	string strMode = "";
	switch (PMUMode)
	{
	case (int)PMU_MODE::FIMV:
		strMode = "FIMV";
		break;
	case (int)PMU_MODE::FIMI:
		strMode = "FIMI";
		break;
	case (int)PMU_MODE::FVMI:
		strMode = "FVMI";
		break;
	case (int)PMU_MODE::FVMV:
		strMode = "FVMV";
		break;
		strMode = "ERROR";
	default:
		break;
	}

	data = STSVariant(strMode.c_str(), strMode.length() + 1, STSVariant::DT_STRING);

	return 0;
}