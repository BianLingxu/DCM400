#include "HDDebugToolItemOut.h"
#include <windows.h>
#include <string>
#include "SM8213.h"

HDDebugToolItemOut::HDDebugToolItemOut(HDDebugTool * debugTool)
	: HDDebugToolItem(debugTool)
{

}

HDDebugToolItemOut::~HDDebugToolItemOut()
{

}

int HDDebugToolItemOut::Type() const
{
	return ITEM_OUT;
}

const char* HDDebugToolItemOut::Name() const
{
	return "OUT";
}

int HDDebugToolItemOut::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
	char buf[1024] = { '\0' };
	int relayStatus = BOARD_NOT_INSERT_ERROR;

	USHORT usChannel = 0;
	int nSlotNo = dcm_GetPinSlotChannel(logicChannel, site, usChannel);
	if (0 < nSlotNo)
	{
		relayStatus = dcm_GetRelayStatus(nSlotNo, usChannel, 0);
	}

	std::string str_relay = "";

	switch (relayStatus)
	{
	case 0:
		str_relay = "OFF";
		break;
	case 1:
		str_relay = "ON";
		break;
	case -1:
		str_relay = "ChExceed";
		break;
	case -2:
		///<The relay type is error, not will happen
		break;
	case -3:
		str_relay = "NoBoard";
		break;
	case -4:
		str_relay = "NoTypeRelay";
		break;
	default:
		break;
	}

	data = STSVariant(str_relay.c_str(), str_relay.length() + 1, STSVariant::DT_STRING);

	return 0;
}