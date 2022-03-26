#include "HDDebugToolItemPinMode.h"
#include "SM8213.h"
CHDDebugToolItemPinMode::CHDDebugToolItemPinMode(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{

}

CHDDebugToolItemPinMode::~CHDDebugToolItemPinMode()
{

}

int CHDDebugToolItemPinMode::Type() const
{
	return ITEM_PIN_MODE;
}

const char* CHDDebugToolItemPinMode::Name() const
{
	return "Pin Mode";
}

int CHDDebugToolItemPinMode::GetData(int nSiteNo, int nPinNo, STSVariant& data, STSVariant& mark) const
{
	USHORT usChannel = 0;
	int nMode = 2;
	int nSlotNo = dcm_GetPinSlotChannel(nPinNo, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		nMode = dcm_GetChannelMode(nSlotNo, usChannel);
	}
	string strMode = "ERROR";
	switch (nMode)
	{
	case 0:
		strMode = "MCU";
		break;
	case 1:
		strMode = "PMU";
		break;
	case 2:
		strMode = "NEITHER";
		break;
	default:
		strMode = "ERROR";
		break;
	}
	data = STSVariant(strMode.c_str(), strMode.length() + 1, STSVariant::DT_STRING);

	return 0;
}
