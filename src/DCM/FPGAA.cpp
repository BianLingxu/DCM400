#include "FPGAA.h"

CFPGAA::CFPGAA(BYTE bySlotNo)
	: CBaseOperation(bySlotNo)
{
	m_byFunctionAddress =  MODULE_TYPE::CAL_MODULE;
}

int CFPGAA::SetControllerIndex(BYTE byControllerIndex)
{
	if (DCM_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
	{
		return -1;
	}
	SetModuleAddress(byControllerIndex + 1);
	return 0;
}

int CFPGAA::SetModuleType(MODULE_TYPE ModuleType)
{
	switch (ModuleType)
	{
	case MODULE_TYPE::SYS_MODULE:
		break;
	case MODULE_TYPE::COM_MODULE:
		break;
	case MODULE_TYPE::FUN_MODULE:
		break;
	case MODULE_TYPE::TMU_MODULE:
		break;
	case MODULE_TYPE::CAL_MODULE:
		break;
	default:
		return -1;
		break;
	}
	m_byFunctionAddress = ModuleType;
	return 0;
}

ULONG CFPGAA::Read(USHORT usREGAddress)
{
	SetFunctionRegisterAddress((BYTE)m_byFunctionAddress, usREGAddress);
	return BaseRead();
}

int CFPGAA::Write(USHORT usREGAddress, ULONG ulData)
{
	SetFunctionRegisterAddress((BYTE)m_byFunctionAddress, usREGAddress);
	BaseWrite(ulData);
	return 0;
}
