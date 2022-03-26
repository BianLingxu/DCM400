#include "HDDebugToolItemVOH.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemVOH::HDDebugToolItemVOH(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemVOH::~HDDebugToolItemVOH()
{

}

int HDDebugToolItemVOH::Type() const
{
	return ITEM_VOH;
}

const char* HDDebugToolItemVOH::Name() const
{
	return "VOH";
}

int HDDebugToolItemVOH::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	double getValue = 0;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		dcm_GetLevelSettingValue(nSlotNo, usChannel, DCM_VOH, getValue);
	}

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.4fV", getValue);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}