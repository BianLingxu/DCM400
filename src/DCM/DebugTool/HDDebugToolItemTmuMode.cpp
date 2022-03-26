#include "HDDebugToolItemTMUMode.h"
#include "SM8213.h"

CHDDebugToolItemTMUMode::CHDDebugToolItemTMUMode(HDDebugTool* pDebugTool)
	:HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUMode::~CHDDebugToolItemTMUMode()
{
}

int CHDDebugToolItemTMUMode::Type() const
{
	return ITEM_TMU_MODE;
}

const char* CHDDebugToolItemTMUMode::Name() const
{
	return "TMU mode";
}

int CHDDebugToolItemTMUMode::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	string strMode = "Board Not Existed";
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
		BOOL bDCMWithVector = TRUE;
		int nInstanceID = dcm_GetPinInstanceID(nLogicChannel, bDCMWithVector);
		if (0 > nInstanceID || !bDCMWithVector)
		{
			return 0;
		}

		dcm_SetInstanceID(nInstanceID);
		UINT uSampleNum = 0;
		double dTimeout = 0;
		nRetVal = dcm_GetTMUMeasure(nSlotNo, usChannel, byTMUMeasMode, uSampleNum, dTimeout);
		if (0 != nRetVal)
		{
			strMode = " ";
		}
		else
		{
			switch (byTMUMeasMode)
			{
			case (BYTE)TMU_MEAS_MODE::DUTY_PERIOD:
				strMode = "FREQ_DUTY";
				break;
			case (BYTE)TMU_MEAS_MODE::EDGE_TIME:
				strMode = "EDGE";
				break;
			case (BYTE)TMU_MEAS_MODE::SIGNAL_DELAY:
				strMode = "DELAY";
				break;
			default:
				strMode = " ";
				break;
			}

		}
	}

	Data = STSVariant(strMode.c_str(), strMode.length() + 1, STSVariant::DT_STRING);
	return 0;
}
