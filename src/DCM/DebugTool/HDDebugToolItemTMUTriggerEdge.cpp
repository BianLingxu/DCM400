#include "HDDebugToolItemTMUTriggerEdge.h"
#include "SM8213.h"
CHDDebugToolItemTriggerEdge::CHDDebugToolItemTriggerEdge(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTriggerEdge::~CHDDebugToolItemTriggerEdge()
{
}

int CHDDebugToolItemTriggerEdge::Type() const
{
	return ITEM_TMU_TRIGGER_EDGE;
}

const char* CHDDebugToolItemTriggerEdge::Name() const
{
	return "Trigger edge";
}

int CHDDebugToolItemTriggerEdge::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	BOOL bRaiseEdge = FALSE;
	USHORT usChannel = 0;
	int nRetVal = -1;
	string strTriggerEdge = "Board Not Existed";
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
		USHORT usHoldOffTime = 0;
		USHORT usHoldOffNum = 0;
		nRetVal = dcm_GetTMUParameter(nSlotNo, usChannel, bRaiseEdge, usHoldOffTime, usHoldOffNum);
		if (0 != nRetVal)
		{
			strTriggerEdge = " ";
		}
		else
		{
			if (bRaiseEdge)
			{
				strTriggerEdge = "Raise";
			}
			else
			{
				strTriggerEdge = "Fall";
			}
		}
	}

	Data = STSVariant(strTriggerEdge.c_str(), strTriggerEdge.length() + 1, STSVariant::DT_STRING);
	return 0;
}
