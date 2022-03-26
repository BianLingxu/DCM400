#include "HDDebugToolItemVIL.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemVIL::HDDebugToolItemVIL(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemVIL::~HDDebugToolItemVIL()
{

}

int HDDebugToolItemVIL::Type() const
{
	return ITEM_VIL;
}

const char* HDDebugToolItemVIL::Name() const
{
	return "VIL";
}

int HDDebugToolItemVIL::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	double getValue = 0;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		dcm_GetLevelSettingValue(nSlotNo, usChannel, DCM_VIL, getValue);
	}

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.4fV", getValue);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}