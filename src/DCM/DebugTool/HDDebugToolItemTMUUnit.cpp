#include "HDDebugToolItemTMUUnit.h"
#include "SM8213.h"
CHDDebugToolItemTMUUnit::CHDDebugToolItemTMUUnit(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUUnit::~CHDDebugToolItemTMUUnit()
{
}

int CHDDebugToolItemTMUUnit::Type() const
{
	return ITEM_TMU_CHANNEL;
}

const char* CHDDebugToolItemTMUUnit::Name() const
{
	return "TMU unit";
}

int CHDDebugToolItemTMUUnit::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	string strUnit;
	BYTE byTMUMeasMode = 0;
	USHORT usChannel = 0;
	int nRetVal = -1;
	int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
		char lpszResult[32] = { 0 };
		BOOL bDCMWithVector = TRUE;
		int nInstanceID = dcm_GetPinInstanceID(nLogicChannel, bDCMWithVector);
		if (0 > nInstanceID || !bDCMWithVector)
		{
			return 0;
		}

		dcm_SetInstanceID(nInstanceID);
		nRetVal = dcm_GetTMUConnectUnit(nSlotNo, usChannel);
		if (0 > nRetVal)
		{
			sprintf_s(lpszResult, sizeof(lpszResult), "Not connected");
		}
		else
		{
			sprintf_s(lpszResult, sizeof(lpszResult), "Unit %d", nRetVal + 1);
		}
		strUnit = lpszResult;
	}
	else
	{
		strUnit = "ERROR";
	}
	Data = STSVariant(strUnit.c_str(), strUnit.length() + 1, STSVariant::DT_STRING);
	return 0;
}
