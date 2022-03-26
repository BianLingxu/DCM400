#include "pch.h"
#include "Operation.h"
#include "BRAM.h"
#include "DRAM.h"
#include "Register.h"

COperation::COperation(BYTE bySlotNo)
	: m_bySlotNo(bySlotNo)
	, m_Register(bySlotNo)
	, m_BRAM(bySlotNo)
	, m_DRAM(bySlotNo)
	, m_byControllerIndex(0)
{
}

COperation::~COperation()
{
}

BYTE COperation::GetSlotNo() const
{
	return m_bySlotNo;
}

int COperation::SetControllerIndex(BYTE byControllerIndex)
{
	int nRetVal = 0;

	m_byControllerIndex = byControllerIndex;

    do {
        nRetVal = m_Register.SetControllerIndex(byControllerIndex);
        if (0 != nRetVal)
        {
            break;
        }

        nRetVal = m_BRAM.SetControllerIndex(byControllerIndex);
        if (0 != nRetVal)
        {
            break;
        }

        nRetVal = m_DRAM.SetControllerIndex(byControllerIndex);
        if (0 != nRetVal)
        {
            break;
        }
    } while (0);

	return nRetVal;
}

BYTE COperation::GetControllerIndex() const
{
	return m_byControllerIndex;
}

ULONG COperation::ReadRegister(USHORT usREGAddress, BOOL bShared, BYTE byFPGAAddr)
{
    m_Register.SetRegisterType(bShared);
    return m_Register.Read(usREGAddress, byFPGAAddr);
}

int COperation::WriteRegister(USHORT usREGAddress, ULONG ulData, BOOL bShared, BYTE byFPGAAddr)
{
    m_Register.SetRegisterType(bShared);
    return m_Register.Write(usREGAddress, ulData, byFPGAAddr);
}

int COperation::ReadBRAM(BRAM_TYPE RAMType, USHORT usRAMAddress, UINT uReadDataCount, ULONG* pulDataBuff, BOOL bMainMem)
{
    if (0 == uReadDataCount || nullptr == pulDataBuff)
    {
        return -2;
    }

	switch (RAMType)
	{
	case COperation::BRAM_TYPE::PAT_LIST:
		break;
	case COperation::BRAM_TYPE::DDR_INFO:
		break;
	case COperation::BRAM_TYPE::PRIME_RAM:
		break;
	case COperation::BRAM_TYPE::GLO_T0_SET:
		break;
	case COperation::BRAM_TYPE::T0_RAM:
		break;
	case COperation::BRAM_TYPE::BRAM5:
		break;
	case COperation::BRAM_TYPE::TG_DDR_INFO:
		break;
	case COperation::BRAM_TYPE::MEM_TIMMING:
		break;
	case COperation::BRAM_TYPE::MEM_DATA_SRC_SEL:
		break;
	case COperation::BRAM_TYPE::MEM_RSU:
		break;
	case COperation::BRAM_TYPE::GLO_TIMINGSET:
		break;
	case COperation::BRAM_TYPE::MEM_ADC:
		break;
	default:
		return -1;
		break;
	}

	m_BRAM.SetAddress((int)RAMType, usRAMAddress);

	return m_BRAM.Read(uReadDataCount, pulDataBuff, bMainMem);
}

int COperation::WriteBRAM(BRAM_TYPE BRAMType, USHORT usRAMAddress, UINT uWriteDataCount, const ULONG* pulData)
{
    if (0 == uWriteDataCount || nullptr == pulData)
    {
        return -2;
    }

	switch (BRAMType)
	{
	case COperation::BRAM_TYPE::PAT_LIST:
		break;
	case COperation::BRAM_TYPE::DDR_INFO:
		break;
	case COperation::BRAM_TYPE::PRIME_RAM:
		break;
	case COperation::BRAM_TYPE::GLO_T0_SET:
		break;
	case COperation::BRAM_TYPE::T0_RAM:
		break;
	case COperation::BRAM_TYPE::BRAM5:
		break;
	case COperation::BRAM_TYPE::TG_DDR_INFO:
		break;
	case COperation::BRAM_TYPE::MEM_TIMMING:
		break;
	case COperation::BRAM_TYPE::MEM_DATA_SRC_SEL:
		break;
	case COperation::BRAM_TYPE::MEM_RSU:
		break;
	case COperation::BRAM_TYPE::GLO_TIMINGSET:
		break;
	case COperation::BRAM_TYPE::MEM_ADC:
		break;
	default:
		return -1;
		break;
	};

	m_BRAM.SetAddress((int)BRAMType, usRAMAddress);

	return m_BRAM.Write(uWriteDataCount, pulData);
}

int COperation::ReadDRAM(DRAM_TYPE DRAMType, UINT uStartLineNo, UINT uReadDataCount, ULONG* pulDataBuff)
{
    int nRetVal;

    if (0 == uReadDataCount || nullptr == pulDataBuff)
    {
        return -2;
    }

	switch (DRAMType)
	{
	case COperation::DRAM_TYPE::CMD_DRAM0:
		break;
	case COperation::DRAM_TYPE::CMD_DRAM1:
		break;
	case COperation::DRAM_TYPE::PAT_DRAM0:
		break;
	case COperation::DRAM_TYPE::PAT_DRAM1:
		break;
	default:
		return -1;
		break;
	}

    nRetVal = m_DRAM.SetStartLineNo((CDRAM::MEM_TYPE)DRAMType, uStartLineNo);
	if (0 != nRetVal)
	{
		return -3;
	}

	nRetVal = m_DRAM.Read(uReadDataCount, pulDataBuff);
	if (0 != nRetVal)
	{
		return -4;
	}

	return 0;
}

int COperation::WriteDRAM(DRAM_TYPE DRAMType, UINT uStartBits, UINT uWriteDataCount, const ULONG* pulData)
{
    int nRetVal;

    if (0 == uWriteDataCount || nullptr == pulData)
    {
        return -2;
    }

	switch (DRAMType)
	{
	case COperation::DRAM_TYPE::CMD_DRAM0:
		break;
	case COperation::DRAM_TYPE::CMD_DRAM1:
		break;
	case COperation::DRAM_TYPE::PAT_DRAM0:
		break;
	case COperation::DRAM_TYPE::PAT_DRAM1:
		break;
	default:
		return -1;
		break;
	}

	nRetVal = m_DRAM.SetStartLineNo((CDRAM::MEM_TYPE)DRAMType, uStartBits);
	if (0 != nRetVal)
	{
		return -3;
	}

	m_DRAM.Write(uWriteDataCount, pulData);
	
	return 0;
}

void COperation::WaitUs(double dUs)
{	
	m_Register.WaitUs(dUs);
}
