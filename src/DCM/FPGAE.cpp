#include "FPGAE.h"

CFPGAE::CFPGAE(BYTE bySlotNo)
	: CBaseOperation(bySlotNo)
{
	SetModuleAddress(5);
}

int CFPGAE::SetControllerIndex(BYTE byControllerIndex)
{
	SetModuleAddress(5);
	return 0;
}

int CFPGAE::Read(USHORT usAddress, UINT uWriteDataCount, ULONG* pulData)
{
	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -2;
	}
	SetFunctionRegisterAddress((usAddress >> 10) & 0x03, usAddress & 0x3FF);
	for (UINT uDataIndex = 0; uDataIndex < uWriteDataCount;++uDataIndex)
	{
		pulData[uDataIndex] = BaseRead();
	}
	return 0;
}

int CFPGAE::Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData)
{
	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -2;
	}
	SetFunctionRegisterAddress((usAddress >> 10) & 0x03, usAddress & 0x3FF);
	for (UINT uDataIndex = 0; uDataIndex < uWriteDataCount; ++uDataIndex)
	{
		BaseWrite(pulData[uDataIndex]);
	}
	return 0;
}

ULONG CFPGAE::GetStatus()
{
	return 0xFFFFFFFF;
}

int CFPGAE::SetRegisterType(USHORT usType)
{
	return -1;
}

int CFPGAE::SetAddress(int nType, USHORT usAddress)
{
	return -1;
}

int CFPGAE::SetStartLineNo(int nType, ULONG ulStartLineNo)
{
	return -1;
}

ULONG CFPGAE::Read(USHORT usAddress)
{
	return -1;
}

int CFPGAE::Write(USHORT usAddress, ULONG ulData)
{
	return -1;
}

int CFPGAE::Read(UINT uReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CFPGAE::Write(UINT uWriteDataCount, const ULONG* pulData)
{
	return -1;
}


int CFPGAE::Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}

int CFPGAE::Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}
