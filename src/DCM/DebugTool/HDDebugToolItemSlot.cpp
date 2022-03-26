#include "HDDebugToolItemSlot.h"
#include "SM8213.h"
CHDDebugToolItemSlot::CHDDebugToolItemSlot(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{

}

CHDDebugToolItemSlot::~CHDDebugToolItemSlot()
{

}

int CHDDebugToolItemSlot::Type() const
{
	return ITEM_PIN_SLOT;
}

const char* CHDDebugToolItemSlot::Name() const
{
	return "SLOT";
}

int CHDDebugToolItemSlot::GetData(int nSiteNo, int nPinNo, STSVariant& data, STSVariant& mark) const
{
	USHORT usChannel = 0;
	int nMode = 2;
	int nSlotNo = dcm_GetPinSlotChannel(nPinNo, nSiteNo, usChannel);
	char lpszSlot[32] = { 0 };
	if (0 < nSlotNo)
	{
		sprintf_s(lpszSlot, sizeof(lpszSlot), "S%d_%d", nSlotNo, usChannel);
	}

	data = STSVariant(lpszSlot, strlen(lpszSlot) + 1, STSVariant::DT_STRING);

	return 0;
}

