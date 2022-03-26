#include "HDDebugToolItemTMUHoldOffNum.h"
#include "SM8213.h"

CHDDebugToolItemTMUHoldOffNum::CHDDebugToolItemTMUHoldOffNum(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUHoldOffNum::~CHDDebugToolItemTMUHoldOffNum()
{
}

int CHDDebugToolItemTMUHoldOffNum::Type() const
{
	return ITEM_TMU_HOLD_OFF_NUM;
}

const char* CHDDebugToolItemTMUHoldOffNum::Name() const
{
	return "TMU Hold off number";
}

int CHDDebugToolItemTMUHoldOffNum::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	USHORT usHoldOffNum = 0;
	USHORT usChannel = 0;
	int nRetVal = -1;
	string strHoldOffNum = "Board Not Existed";
	int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			///<Not MCU mode
			return 0;
		}
		BOOL bRaiseEdge = FALSE;
		USHORT usHoldOffTime = 0;
		nRetVal = dcm_GetTMUParameter(nSlotNo, usChannel, bRaiseEdge, usHoldOffTime, usHoldOffNum);
		if (0 != nRetVal)
		{
			strHoldOffNum = " ";
		}
		else
		{
			char lpszValue[32] = { 0 };
			sprintf_s(lpszValue, sizeof(lpszValue), "%d", usHoldOffNum);
			strHoldOffNum = lpszValue;
		}
	}

	Data = STSVariant(strHoldOffNum.c_str(), strHoldOffNum.length() + 1, STSVariant::DT_STRING);
	return 0;
}
