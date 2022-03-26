#include "HDDebugToolItemVOL.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

HDDebugToolItemVOL::HDDebugToolItemVOL(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemVOL::~HDDebugToolItemVOL()
{

}

int HDDebugToolItemVOL::Type() const
{
	return ITEM_VOL;
}

const char* HDDebugToolItemVOL::Name() const
{
	return "VOL";
}

int HDDebugToolItemVOL::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	double getValue = 0;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		dcm_GetLevelSettingValue(nSlotNo, usChannel, DCM_VOL, getValue);
	}

	sts_sprintf(buf, STS_ARRAY_SIZE(buf), "%.4fV", getValue);
	std::string str = buf;
	data = STSVariant(str.c_str(), str.length() + 1, STSVariant::DT_STRING);

	return 0;
}