#include "pch.h"
#include "Register.h"
#include "DCM400HardwareInfo.h"

CRegister::CRegister(BYTE bySlotNo)
	: CBaseOperation(bySlotNo)
	, m_byModuleAddress(0)
{
}

int CRegister::SetControllerIndex(BYTE byControllerIndex)
{
	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
	{
		return -1;
	}
	m_byModuleAddress = byControllerIndex + 1;

	return 0;
}

int CRegister::SetRegisterType(BOOL bShared)
{
    BYTE byModuleAddr;
    
    byModuleAddr = bShared ? 0 : m_byModuleAddress;
	SetModuleAddress(byModuleAddr);

	return 0;
}

ULONG CRegister::Read(USHORT usRegisterAddress, BYTE byFPGAAddr)
{
    SetDetailAddress(byFPGAAddr, usRegisterAddress & 0x3FF);
	return BaseRead();
}

int CRegister::Write(USHORT usRegisterAddress, ULONG ulData, BYTE byFPGAAddr)
{
    SetDetailAddress(byFPGAAddr, usRegisterAddress & 0x3FF);
	BaseWrite(ulData);

	return 0;
}