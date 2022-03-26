#include "HDDebugToolItemTMUSampleNum.h"
#include "SM8213.h"
CHDDebugToolTMUSampleNum::CHDDebugToolTMUSampleNum(HDDebugTool* pDebugTool)
	: HDDebugToolItem(pDebugTool)
{
}

CHDDebugToolTMUSampleNum::~CHDDebugToolTMUSampleNum()
{
}

int CHDDebugToolTMUSampleNum::Type() const
{
	return ITEM_TMU_CYLCETIMES;
}

const char* CHDDebugToolTMUSampleNum::Name() const
{
	return "TMU sample number";
}

int CHDDebugToolTMUSampleNum::GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const
{
	UINT uSampleNum = 0;
	USHORT usChannel = 0;
	int nRetVal = -1;
	string strSampleNum = "Board Not Existed";
	int nSlotNo = dcm_GetPinSlotChannel(nLogicChannel, nSiteNo, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
		BYTE byTMUMeasMode = 0;
		double dTimeout = 0;
		nRetVal = dcm_GetTMUMeasure(nSlotNo, usChannel, byTMUMeasMode, uSampleNum, dTimeout);

		if (0 != nRetVal)
		{
			strSampleNum = " ";
		}
		else
		{
			char lpszSampleNum[32] = { 0 };
			sprintf_s(lpszSampleNum, sizeof(lpszSampleNum), "%d", uSampleNum);
			strSampleNum = lpszSampleNum;
		}
	}
	Data = STSVariant(strSampleNum.c_str(), strSampleNum.length() + 1, STSVariant::DT_STRING);
	return 0;
}
