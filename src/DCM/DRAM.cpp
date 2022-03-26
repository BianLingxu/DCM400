#include "DRAM.h"
#include "Register.h"

#define DRAM_ADDR_RW (0x400 & 0x3FF)
#define DRAM_DATA_WR (0x401 & 0x3FF)
#define DRAM_DATA_RD (0x402 & 0x3FF)
#define DRAM_CMD_REQ (0x405 & 0x3FF)
#define DRAM_STA_CHK (0x406 & 0x3FF)

CDRAM::CDRAM(BYTE bySlotNo)
	: CCOM(bySlotNo)
{
	m_ulStartLineNo = 0;
	m_ulStartAddress = 0;
	SetSelectAddress(DRAM_ADDR_RW, DRAM_ADDR_RW);
	SetDataAddress(DRAM_DATA_RD, DRAM_DATA_WR);
	SetRequestAddress(DRAM_CMD_REQ);
	SetStatusAddress(DRAM_STA_CHK, 100);
	m_bySlot = bySlotNo;
}

int CDRAM::SetStartLineNo(int nType, ULONG ulStartLineNo)
{
	switch (MEM_TYPE(nType))
	{
	case MEM_TYPE::DRAM1:
		break;
	case MEM_TYPE::DRAM2:
		break;
	case MEM_TYPE::DRAM3:
		break;
	case MEM_TYPE::DRAM4:
		break;
	case MEM_TYPE::DRAM5:
		break;
	case MEM_TYPE::DRAM6:
		break;
	default:
		return -1;
		break;
	}
	if (DCM_DRAM_LINE_COUNT < ulStartLineNo)
	{
		return -2;
	}

	m_ulStartAddress = (nType << 25) | (ulStartLineNo);

	return 0;
}
int CDRAM::Write(UINT uWriteDataCount, const ULONG* pulData)
{
	if (DCM_DRAM_LINE_COUNT < m_ulStartLineNo + uWriteDataCount)
	{
		return -2;
	}

	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -3;
	}

	SetStartAddress(m_ulStartAddress);
	CCOM::Write(uWriteDataCount, pulData);

	return 0;
}

int CDRAM::SetControllerIndex(BYTE byControllerIndex)
{
	m_byControllerIndex = byControllerIndex;
	CCOM::SetControllerIndex(m_byControllerIndex);

	return 0;
}

int CDRAM::Read(UINT uReadDataCount, ULONG* pulData)
{
	ULONG regValue(0), timeVal(0);

	if (0 == uReadDataCount || nullptr == pulData)
	{
		return -2;
	}

	if (DCM_DRAM_LINE_COUNT < m_ulStartLineNo + uReadDataCount)
	{
		return -3;
	}

	///<Check whether the DRAM is ready
	CRegister Register(m_bySlot);
	Register.SetControllerIndex(m_byControllerIndex);
	Register.SetRegisterType(CRegister::REG_TYPE::COM_REG);
	ULONG ulData = 0;
	int nMaxCheckTimes = 100;
	int nCheckTimes = 0;
	do
	{
		ulData = Register.Read(0x0406);
		++nCheckTimes;
	} while (0 != ulData && nCheckTimes < nMaxCheckTimes);
	if (nMaxCheckTimes <= nCheckTimes)
	{
		return -4;
	}

	SetStartAddress(m_ulStartAddress);
	CCOM::Read(uReadDataCount, pulData);

	return 0;
}

ULONG CDRAM::GetStatus()
{
	return 0xFFFFFFFF;
}

int CDRAM::SetRegisterType(USHORT usType)
{
	return -1;
}

int CDRAM::SetAddress(int nType, USHORT usAddress)
{
	return -1;
}

ULONG CDRAM::Read(USHORT usAddress)
{
	return -1;
}

int CDRAM::Write(USHORT usAddress, ULONG ulData)
{
	return -1;
}

int CDRAM::Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CDRAM::Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData)
{
	return -1;
}

int CDRAM::Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}

int CDRAM::Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}
