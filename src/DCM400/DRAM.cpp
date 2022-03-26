#include "pch.h"
#include "DRAM.h"
#include "Register.h"
#include "DCM400HardwareInfo.h"

#define REG_DRAM_DATA_ADDR           (0x0021)
#define REG_DRAM_WRITE_DATA          (0x0022)
#define REG_DRAM_READ_DATA           (0x0023)
#define REG_DRAM_ACCESS_CMD          (0x0024)
#define REG_DRAM_ACCESS_STA          (0x0025)

#define DRAM_INTF_BYTE_WIDTH         (8)
#define DRAM_BURST_BYTE_SIZE         (64)
#define DRAM_BURST_DWORD_SIZE        (DRAM_BURST_BYTE_SIZE / 4)
#define DRAM_SINGLE_RW_SIZE          (256 * DRAM_BURST_DWORD_SIZE)

#define DRAM_ACCESS_MAX_TIME         (100)

CDRAM::CDRAM(BYTE bySlotNo)
    : CBaseOperation(bySlotNo)
    , m_byControllerIndex(0)
    , m_ulDramAddr(0)
	, m_byFPGA(0)
    , m_bySelAdd(0)
	, m_bRead(FALSE)
{
}

int CDRAM::SetControllerIndex(BYTE byControllerIndex)
{
    if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
    {
        return -1;
    }
    m_byControllerIndex = byControllerIndex;
    SetModuleAddress(0);

    return 0;
}

int CDRAM::SetStartLineNo(int nType, ULONG ulStartByteNo)
{
    if (0 != ulStartByteNo % DRAM_BURST_BYTE_SIZE)
    {
        return -2;
    }

	switch (nType)
	{
	case CDRAM::MEM_TYPE::CMD_DRAM0:
		m_byFPGA = 0;
		m_bySelAdd = 0;
		break;
    case CDRAM::MEM_TYPE::CMD_DRAM1:
        m_byFPGA = 0;
        m_bySelAdd = 1;
        break;
	case CDRAM::MEM_TYPE::PAT_DRAM0:
		m_byFPGA = 1;
        m_bySelAdd = 2;
		break;
    case CDRAM::MEM_TYPE::PAT_DRAM1:
        m_byFPGA = 2;
        m_bySelAdd = 3;
        break;
	default:
		return -1;
		break;
	}

    m_ulDramAddr = ulStartByteNo & 0x3FFFFFFF;
    m_ulDramAddr /= DRAM_INTF_BYTE_WIDTH;

	return 0;
}

int CDRAM::Write(UINT uWriteDataCount, const ULONG* pulData)
{
	if (nullptr == pulData || 0 == uWriteDataCount)
	{
		return -1;
	}
    if (0 != uWriteDataCount % DRAM_BURST_DWORD_SIZE)
	{
		return -2;
	}

	m_bRead = FALSE;

	UINT uLeftDataCount = uWriteDataCount;
	UINT uCurWriteCount = 0;
	UINT uBaseDataIndex = 0;
	ULONG ulDramCMD = 0;

    while (0 != uLeftDataCount)
	{
        uCurWriteCount = DRAM_SINGLE_RW_SIZE > uLeftDataCount ? uLeftDataCount : DRAM_SINGLE_RW_SIZE;

		///<Set the DRAM start address
        SetDetailAddress(m_byFPGA, REG_DRAM_DATA_ADDR);
		BaseWrite(GetStartAddr());
		
		///<Write target data to FPGA
		SetDetailAddress(m_byFPGA, REG_DRAM_WRITE_DATA);
		for (UINT uDataIndex = 0; uDataIndex < uCurWriteCount; ++uDataIndex, ++uBaseDataIndex)
		{
			BaseWrite(pulData[uBaseDataIndex]);
		}

		///<Set the operation command
        SetDetailAddress(m_byFPGA, REG_DRAM_ACCESS_CMD);
        ulDramCMD = 1 << 16;
        ulDramCMD |= uCurWriteCount / DRAM_BURST_DWORD_SIZE;
        BaseWrite(ulDramCMD);

        m_ulDramAddr += uCurWriteCount * 4 / DRAM_INTF_BYTE_WIDTH;
        uLeftDataCount -= uCurWriteCount;
	}

	return 0;
}

int CDRAM::Read(UINT uReadDataCount, ULONG* pulData)
{
    if (nullptr == pulData || 0 == uReadDataCount)
    {
        return -1;
    }
    if (0 != uReadDataCount % DRAM_BURST_DWORD_SIZE)
    {
        return -2;
    }

    m_bRead = TRUE;

    UINT uLeftDataCount = uReadDataCount;
    UINT uCurReadCount = 0;
    UINT uBaseDataIndex = 0;
    ULONG ulDramCMD = 0;
    int nRetVal = 0;

    while (0 != uLeftDataCount)
    {
        uCurReadCount = DRAM_SINGLE_RW_SIZE > uLeftDataCount ? uLeftDataCount : DRAM_SINGLE_RW_SIZE;

        ///<Set the data start line number
        SetDetailAddress(m_byFPGA, REG_DRAM_DATA_ADDR);
        BaseWrite(GetStartAddr());

        ///<Set the operation command
        SetDetailAddress(m_byFPGA, REG_DRAM_ACCESS_CMD);
        ulDramCMD = 1 << 17;
        ulDramCMD |= uCurReadCount / DRAM_BURST_DWORD_SIZE;
        BaseWrite(ulDramCMD);

        ///<wait DRAM ready
        nRetVal = WaitReady();
        if (0 != nRetVal)
        {
            nRetVal = -3;
            break;
        }

        ///<Read data from FPGA
        SetDetailAddress(m_byFPGA, REG_DRAM_READ_DATA);
        for (UINT uDataIndex = 0; uDataIndex < uCurReadCount; ++uDataIndex, ++uBaseDataIndex)
        {
            pulData[uBaseDataIndex] = BaseRead();
        }

        m_ulDramAddr += uCurReadCount * 4 / DRAM_INTF_BYTE_WIDTH;
        uLeftDataCount -= uCurReadCount;
    }

    return nRetVal;
}

int CDRAM::WaitReady()
{
    ULONG ulStatus = 0;
    BYTE byTargetBit = 0;

	int nWaitTimes = DRAM_ACCESS_MAX_TIME;

    while (nWaitTimes >= 0)
	{
        SetDetailAddress(m_byFPGA, REG_DRAM_ACCESS_STA);
        ulStatus = BaseRead();
        byTargetBit = (m_bRead ? 1 : 0) + (0 == m_bySelAdd % 2 ? 0 : 2);
        if (0 == (ulStatus & (1 << byTargetBit)))
        {
            break;
        }
        nWaitTimes--;
	}

	if (0 >= nWaitTimes)
	{
		return -1;
	}

	return 0;
}

ULONG CDRAM::GetStartAddr()
{
    return m_bySelAdd << 30 | m_ulDramAddr;
}
