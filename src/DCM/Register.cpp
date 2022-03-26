#include "Register.h"

CRegister::CRegister(BYTE bySlotNo)
	: CFPGAA(bySlotNo)
{
}

int CRegister::SetRegisterType(USHORT usRegisterType)
{
	switch (usRegisterType)
	{
	case CRegister::SYS_REG:
		break;
	case CRegister::COM_REG:
		break;
	case CRegister::FUNC_REG:
		break;
	case CRegister::TMU_REG:
		break;
	case CRegister::CAL_REG:
		break;
	default:
		return -1;
		break;
	}
	SetModuleType(CFPGAA::MODULE_TYPE(usRegisterType));
	return 0;
}

ULONG CRegister::Read(USHORT usRegisterAddress)
{
	return CFPGAA::Read(usRegisterAddress);
}

int CRegister::Write(USHORT usRegisterAddress, ULONG ulData)
{
	return CFPGAA::Write(usRegisterAddress, ulData);
}

ULONG CRegister::GetStatus()
{
	return 0xFFFFFFFF;
}


int CRegister::SetAddress(int nType, USHORT usAddress)
{
	return -1;
}

int CRegister::SetStartLineNo(int nType, ULONG ulStartLineNo)
{
	return -1;
}

int CRegister::Read(UINT uReadDataCount, ULONG* pulDataBuff)
{
	return 0;
}

int CRegister::Write(UINT uWriteDataCount, const ULONG* pulData)
{
	return 0;
}

int CRegister::Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CRegister::Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData)
{
	return -1;
}

int CRegister::Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}

int CRegister::Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}
