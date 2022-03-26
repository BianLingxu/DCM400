#include "pch.h"
#include "BRAM.h"
#include "DCM400HardwareInfo.h"

#define REG_BRAM_WRITE_ADDR          (0x0000)
#define REG_BRAM_WRITE_DATA          (0x0001)
#define REG_BRAM_READ_ADDR           (0x0002)
#define REG_BRAM_READ_DATA           (0x0003)

CBRAM::CBRAM(BYTE bySlotNo)
	: CBaseOperation(bySlotNo)
    , m_byControllerIndex(0)
    , m_byFPGA(0)
    , m_byAdditionFPGA(0)
{
    m_ulStartAddress = 0;
}

int CBRAM::SetControllerIndex(BYTE byControllerIndex)
{
    if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
    {
        return -1;
    }

    m_byControllerIndex = byControllerIndex;
    SetModuleAddress(byControllerIndex + 1);

    return 0;
}

int CBRAM::SetAddress(int nMemType, USHORT usLow16Data)
{
    switch (nMemType)
    {
    case CBRAM::PAT_LIST:
        m_byFPGA = 0;
        m_byAdditionFPGA = (m_byControllerIndex < 2) ? 1 : 2;
        break;
    case CBRAM::DDR_INFO:
        m_byFPGA = 0;
        m_byAdditionFPGA = 0;
        break;
    case CBRAM::PRIME_RAM:
        m_byFPGA = 0;
        m_byAdditionFPGA = 0;
        break;
    case CBRAM::GLO_T0_SET:
        m_byFPGA = 0;
        m_byAdditionFPGA = (m_byControllerIndex < 2) ? 1 : 2;
        break;
    case CBRAM::T0_RAM:
        m_byFPGA = 0;
        m_byAdditionFPGA = (m_byControllerIndex < 2) ? 1 : 2;
        break;
    case CBRAM::BRAM5:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::TG_DDR_INFO:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::MEM_TIMMING:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::MEM_DATA_SRC_SEL:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::MEM_RSU:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::GLO_TIMINGSET:
        m_byFPGA = (m_byControllerIndex < 2) ? 1 : 2;;
        m_byAdditionFPGA = m_byFPGA;
        break;
    case CBRAM::MEM_ADC:
        m_byFPGA = 3;
        m_byAdditionFPGA = 3;
        break;
    default:
        return -1;
        break;
    }
	
    m_ulStartAddress = (nMemType << 16) | usLow16Data;

	return 0;
}

int CBRAM::Write(UINT uWriteDataCount, const ULONG* pulData)
{
    if (0 == uWriteDataCount || nullptr == pulData)
    {
        return -1;
    }

	SetDetailAddress(m_byFPGA, REG_BRAM_WRITE_ADDR);
	BaseWrite(m_ulStartAddress);

    SetDetailAddress(m_byFPGA, REG_BRAM_WRITE_DATA);
	for (UINT uDataIndex = 0; uDataIndex < uWriteDataCount; ++uDataIndex)
	{
		BaseWrite(pulData[uDataIndex]);
	}

    return 0;
}

int CBRAM::Read(UINT uReadDataCount, ULONG* pulData, BOOL bMainMem)
{
    BYTE byFPGA;

    if (0 == uReadDataCount ||nullptr == pulData)
    {
        return -1;
    }

    byFPGA = bMainMem ? m_byFPGA : m_byAdditionFPGA;
    SetDetailAddress(byFPGA, REG_BRAM_READ_ADDR);
    BaseWrite(m_ulStartAddress);
    
    SetDetailAddress(byFPGA, REG_BRAM_READ_DATA);
    for (UINT uDataIndex = 0; uDataIndex < uReadDataCount;++uDataIndex)
    {
        pulData[uDataIndex] = BaseRead();
    }

    return 0;
}
