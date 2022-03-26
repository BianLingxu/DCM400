#include "BRAM.h"

#define BRAM_ADDR_WR (0x407 & 0x3FF)
#define BRAM_DATA_WR (0x408 & 0x3FF)
#define BRAM_ADDR_RD (0x409 & 0x3FF)
#define BRAM_DATA_RD (0x40A & 0x3FF)
#define BRAM_CMD_REQ (0x0000)
#define BRAM_STA_CHK (0x0000)

CBRAM::CBRAM(BYTE bySlotNo)
	: CCOM(bySlotNo)
{
    m_ulStartAddress = 0;
	SetSelectAddress(BRAM_ADDR_RD, BRAM_ADDR_WR);
	SetDataAddress(BRAM_DATA_RD, BRAM_DATA_WR);
	SetRequestAddress(BRAM_CMD_REQ);   // not support for BRAM
	SetStatusAddress(BRAM_STA_CHK, 0); // not support for BRAM
}

ULONG CBRAM::GetStatus()
{
	return 0xFFFFFFFF;
}

int CBRAM::SetAddress(int nMemType, USHORT usLow16Data)
{
	switch (nMemType)
	{
    case CBRAM::IMM1:
        break;
    case CBRAM::IMM2:
        break;
    case CBRAM::IMM3:
        break;
    case CBRAM::FM:
        break;
    case CBRAM::MM:
        break;
    case CBRAM::IOM:
        break;
    case CBRAM::PMU_BUF:
        break;
    case CBRAM::MEM_PERIOD:
        break;
    case CBRAM::MEM_RSU:
        break;
    case CBRAM::MEM_HIS:
        break;
    case CBRAM::MEM_TIMING:
        break;
    default:
        return -1;
        break;
	}

    m_ulStartAddress = (nMemType << 16) | usLow16Data;

	return 0;
}

int CBRAM::SetRegisterType(USHORT usType)
{
	return -1;
}

int CBRAM::SetStartLineNo(int nType, ULONG ulStartLineNo)
{
	return -1;
}

ULONG CBRAM::Read(USHORT usAddress)
{
	return -1;
}

int CBRAM::Write(USHORT usAddress, ULONG ulData)
{
	return -1;
}

int CBRAM::Read(USHORT usAddress, UINT uiReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CBRAM::Write(USHORT usAddress, UINT uiWriteDataCount, const ULONG* pulData)
{
	return -1;
}

int CBRAM::Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}

int CBRAM::Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	return -1;
}

int CBRAM::Write(UINT uWriteDataCount, const ULONG* pulData)
{
    if (0 == uWriteDataCount || nullptr == pulData)
    {
        return -2;
    }

	SetStartAddress(m_ulStartAddress);
	CCOM::Write(uWriteDataCount, pulData);

    return 0;
}

int CBRAM::Read(UINT uReadDataCount, ULONG* pulData)
{
    if (0 == uReadDataCount ||nullptr == pulData)
    {
        return -2;
    }

	SetStartAddress(m_ulStartAddress);
	CCOM::Read(uReadDataCount, pulData);

    return 0;
}
