#include "HDDebugToolItemMcuMeas.h"
#include <windows.h>
#include <string>
#include "SM8213.H"

extern BYTE g_byTMUDepthAddress[DCM_MAX_BOARD_NUM * DCM_MAX_CHANNELS_PER_BOARD];
HDDebugToolItemMcuMeas::HDDebugToolItemMcuMeas(HDDebugTool * debugTool)
    : HDDebugToolItem(debugTool)
{

}

HDDebugToolItemMcuMeas::~HDDebugToolItemMcuMeas()
{

}

int HDDebugToolItemMcuMeas::Type() const
{
    return ITEM_MCU_MEAS;
}
const char * HDDebugToolItemMcuMeas::Name() const
{
    return "MCU Result";
}

int HDDebugToolItemMcuMeas::CanModifyData() const
{
    return 0;
}

int HDDebugToolItemMcuMeas::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	short channel = 0;
	double getValue = 1;
	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		int nMode = dcm_GetChannelMode(nSlotNo, usChannel);
		if (0 != nMode)
		{
			return 0;
		}
	}
	const char* lpszPinName = dcm_getPinName(logicChannel);
	BOOL bBoardExist = dcm_getPinChannel(logicChannel, site, channel);
	if (bBoardExist)
	{		
		getValue = dcm_getmcupinresult(lpszPinName, site);
	}
	std::string strMcuResult = "";
	if (0 == getValue)
	{
		strMcuResult = "PASS";
	}
	else
	{
		strMcuResult = "FAIL";
	}

	data = STSVariant(strMcuResult.c_str(), strMcuResult.length() + 1, STSVariant::DT_STRING);

    return 0;
}