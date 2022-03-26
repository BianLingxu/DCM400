#include "HDDebugToolItemTMUHoldOffTime.h"
#include "SM8213.h"
CHDDebugToolItemTMUHoldOffTime::CHDDebugToolItemTMUHoldOffTime(HDDebugTool* pDebugTool)
	:HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUHoldOffTime::~CHDDebugToolItemTMUHoldOffTime()
{
}

int CHDDebugToolItemTMUHoldOffTime::Type() const
{
	return ITEM_TMU_HOLD_OFF_TIME;
}

const char* CHDDebugToolItemTMUHoldOffTime::Name() const
{
	return "TMU Hold Off Time";
}

int CHDDebugToolItemTMUHoldOffTime::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	USHORT usHoldOffTime = 0;
	USHORT usChannel = 0;
	int nRetVal = -1;
	string strHoldOffTime = "Board Not Existed";
	int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
		BOOL bRaiseEdge = FALSE;
		USHORT usHoldOffNum = 0;
		nRetVal = dcm_GetTMUParameter(nSlotNo, usChannel, bRaiseEdge, usHoldOffTime, usHoldOffNum);
		if (0 != nRetVal)
		{
			strHoldOffTime = " ";
		}
		else
		{
			char lpszValue[32] = { 0 };
			sprintf_s(lpszValue, sizeof(lpszValue), "%dns", usHoldOffTime);
			strHoldOffTime = lpszValue;
		}
	}
	Data = STSVariant(strHoldOffTime.c_str(), strHoldOffTime.length() + 1, STSVariant::DT_STRING);
	return 0;
}

