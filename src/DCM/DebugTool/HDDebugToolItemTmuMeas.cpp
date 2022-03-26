#include "HDDebugToolItemTMUMeas.h"
#include "SM8213.h"
CHDDebugToolItemTMUMeas::CHDDebugToolItemTMUMeas(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolItemTMUMeas::~CHDDebugToolItemTMUMeas()
{
}

int CHDDebugToolItemTMUMeas::Type() const
{
	return ITEM_TMU_MEAS;
}

const char* CHDDebugToolItemTMUMeas::Name() const
{
	return "TMU result";
}

int CHDDebugToolItemTMUMeas::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	string strMeasResult = "Board Not Existed";
	do
	{
		int nInstanceID = 0;
		USHORT usChannel = 0;
		double dMeasResult = 0;
		const char* lpszPinName = dcm_getPinName(nLogicChannel);
		int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
		if (0 >= nSlotNo)
		{
			return 0;
		}
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
		BOOL bDCMWithVector = TRUE;
		nInstanceID = dcm_GetPinInstanceID(nLogicChannel, bDCMWithVector);
		if (0 > nInstanceID || !bDCMWithVector)
		{
			return 0;
		}

		BYTE byMeasureMode = 0;
		UINT uSampleNum = 0;
		double dTimeout = 0;
		dcm_SetInstanceID(nInstanceID);
		int nRetVal = dcm_GetTMUMeasure(nSlotNo, usChannel, byMeasureMode, uSampleNum, dTimeout);
		if (0 != nRetVal)
		{
			strMeasResult = " ";
			break;
		}
		BOOL bUnknow = FALSE;
		TMU_MEAS_TYPE MeasType = TMU_MEAS_TYPE::FREQ;
		BYTE byMeasResultType = 0;
		switch (byMeasureMode)
		{
		case 1:
			MeasType = TMU_MEAS_TYPE::FREQ;
			break;
		case 2:
			MeasType = TMU_MEAS_TYPE::EDGE;
			break;
		case 3:
			MeasType = TMU_MEAS_TYPE::DELAY;
			break;
		default:
			bUnknow = TRUE;
			break;
		}
		if (bUnknow)
		{
			strMeasResult = "ERROR";
			break;
		}
		dcm_SetInstanceID(nInstanceID);
		dMeasResult = dcm_GetTMUMeasureResult(lpszPinName, nSiteNo, (BYTE)MeasType);
		double dGain = 0;
		char lpszUnit[4] = { 0 };
		double dAbsValue = fabs(dMeasResult);
		if (TMU_MEAS_TYPE::FREQ == MeasType)
		{
			if (1 - EQUAL_ERROR > dAbsValue)
			{
				dGain = 1e3;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "Hz");
			}
			else if (1e3 - EQUAL_ERROR > dAbsValue)
			{
				dGain = 1;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "KHz");
			}
			else
			{
				dGain = 1e-3;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "MHz");
			}
		}
		else
		{
			if (1 - EQUAL_ERROR > dAbsValue)
			{
				dGain = 1e3;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "ns");
			}
			else if(1e3 - EQUAL_ERROR > dAbsValue)
			{
				dGain = 1;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "us");				
			}
			else if (1e6 - EQUAL_ERROR > dAbsValue)
			{
				dGain = 1e-3;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "ms");
			}
			else
			{
				dGain = 1e-6;
				strcpy_s(lpszUnit, sizeof(lpszUnit), "s");
			}
		}
		char lpszMeasResult[32] = { 0 };
		sprintf_s(lpszMeasResult, sizeof(lpszMeasResult), "%.3f%s", dMeasResult * dGain, lpszUnit);
		strMeasResult = lpszMeasResult;
	} while (FALSE);

	Data = STSVariant(strMeasResult.c_str(), strMeasResult.length() + 1, STSVariant::DT_STRING);

	return 0;
}
