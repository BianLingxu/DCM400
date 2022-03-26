#include "HDDebugToolItemVIH.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemVIH::HDDebugToolItemVIH(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemVIH::~HDDebugToolItemVIH()
{

}

int HDDebugToolItemVIH::Type() const
{
	return ITEM_VIH;
}

const char* HDDebugToolItemVIH::Name() const
{
	return "VIH";
}

int HDDebugToolItemVIH::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	double getValue = 0;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		dcm_GetLevelSettingValue(nSlotNo, usChannel, DCM_VIH, getValue);
	}

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.4fV", getValue);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}