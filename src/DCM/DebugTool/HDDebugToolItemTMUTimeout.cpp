#include "HDDebugToolItemTMUTimeout.h"
#include "SM8213.h"
CHDDebugToolItemTMUTimeout::CHDDebugToolItemTMUTimeout(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUTimeout::~CHDDebugToolItemTMUTimeout()
{
}

int CHDDebugToolItemTMUTimeout::Type() const
{
	return ITEM_TMU_TIMEOUT;
}

const char* CHDDebugToolItemTMUTimeout::Name() const
{
	return "TMU timeout";
}

int CHDDebugToolItemTMUTimeout::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	double dTimeout = 0;
	USHORT usChannel = 0;
	int nRetVal = -1;
	string strTimeout = "Board Not Existed";
	int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
		BOOL bDCMWithVector = TRUE;
		int nInstanceID = dcm_GetPinInstanceID(nLogicChannel, bDCMWithVector);
		if (0 > nInstanceID || !bDCMWithVector)
		{
			return 0;
		}

		dcm_SetInstanceID(nInstanceID);
		UINT uSampleNum = 0;
		BYTE byTMUMeasMode = 0;
		nRetVal = dcm_GetTMUMeasure(nSlotNo, usChannel, byTMUMeasMode, uSampleNum, dTimeout);
		if (0 != nRetVal)
		{
			strTimeout = " ";
		}
		else
		{
			char lpszTimeout[32] = { 0 };
			sprintf_s(lpszTimeout, sizeof(lpszTimeout), "%.6fms", dTimeout);
			strTimeout = lpszTimeout;
		}
	}
	Data = STSVariant(strTimeout.c_str(), strTimeout.length() + 1, STSVariant::DT_STRING);
	return 0;
}
