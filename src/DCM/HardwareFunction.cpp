#include "HardwareFunction.h"
#include "STSMD5.h"
#include "STSSP8201.h"
#include "FlashInfo.h"
#include "Calibration.h"
#include "PMU.h"
#include "TMU.h"
#include "Period.h"
#include "ChannelMode.h"
#include <iterator>
using namespace std;

//*******************Flash operation related****************************//
//***FLASH Controller offset Address***//
//The SPI offset address which the flash address will be written to.
#define	FLASH_ADDR_ADDR			0x0012
//The SPI offset address which the data number written to or read from flash will be written to.
#define	FLASH_DATA_LENGTH_ADDR	0x0013
//The SPI offset address will write Controller to.
#define	FLASH_CTRL_ADDR			0x0016
//The SPI offset address which the data written to flash will be written to.
#define	FLASH_DATA_ADDR			0x0010
//The SPI offset address which the command of flash will be written to.
#define	FLASH_CMD_ADDR			0x0011
//The SPI offset address which get the data read from flash.
#define	FLASH_READ_DATA_ADDR	0x0003

//**The command about flash read and write operations**//
///<The command which enable flash write operation
#define	FLASH_CMD_WRITE_ENABLE	0x06
///<The command which page program write flash
#define FLASH_CMD_PAGE_PROG		0x02
///<The command of read flash data.
#define FLASH_CMD_READ_ENABLE	0x03
///<The command which erase sector data of flash
#define FLAH_CMD_EARSH_SECTOR	0xD8
//The command of read flash status.
#define FLASH_CMD_READ_STATUS	0x05
///<The command of read flash ID.
#define FLASH_CMD_READ_ID		0x9F
#define FLASH_PP_MAX_CYCLE		10e3		//us
#define POLLING_INTERVAL	100			//query interval, unit us
#define FLASH_WAIT_TIMES (FLASH_PP_MAX_CYCLE / POLLING_INTERVAL)

#define REWRITE_TIMES_AFTER_FAIL	2
#define REREAD_TIMES_AFTER_FAIL		2

#define FLASH_ID 0x00202016

///<The begin code of bind read
#define BIND_READ_BEGIN(pOperation, byCurrentSlot) \
set<BYTE> setSlot;\
set<BYTE> setController;\
BYTE byTargetSlot = CBindInfo::Instance()->GetBindInfo(setSlot, setController);\
CBindInfo::Instance()->ClearBind();\
for (auto Slot : setSlot){\
	COperation Operation(Slot);\
	pOperation = &Operation;\
	for (auto Controller : setController){\
		Operation.SetControllerIndex(Controller);

///<The end code of bind read
#define  BIND_READ_END \
		}\
	}\
CBindInfo::Instance()->Bind(setSlot, setController, byTargetSlot);

CHardwareFunction::CHardwareFunction(BYTE bySlotNo, CDriverAlarm * pAlarm)
	: m_Operation(bySlotNo)
	, m_pRelay(nullptr)
	, m_bLatestRanWithDRAM(FALSE)
	, m_bBRAMFailMemoryFilled(FALSE)
	, m_bDRAMFailMemoryFilled(FALSE)
	, m_pAlarm(pAlarm)
	, m_uSampleTimes(0)
	, m_ulBoardVer(-1)
	, m_bSelectFail(FALSE)
{
	m_bySlotNo = bySlotNo;
	m_byControllerIndex = -1;
	m_bGetCalibrationData = FALSE;
	memset(m_byPMUStatus, 0, sizeof(m_byPMUStatus));
	memset(m_byPMUMeasureChip, 0, sizeof(m_byPMUMeasureChip));
	memset(m_byPMUMeasureChipEven, 0, sizeof(m_byPMUMeasureChipEven));

}

CHardwareFunction::~CHardwareFunction()
{
	if (nullptr != m_pRelay)
	{
		delete m_pRelay;
		m_pRelay = nullptr;
	}
}

int CHardwareFunction::SetControllerIndex(BYTE byControllerIndex)
{
	if (DCM_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
	{
		return -1;
	}
	m_byControllerIndex = byControllerIndex;
	m_Operation.SetControllerIndex(byControllerIndex);
	return 0;
}

int CHardwareFunction::GetControllerIndex() const
{
	return m_byControllerIndex;
}

BYTE CHardwareFunction::GetSlotNo() const
{
	return m_bySlotNo;
}

BOOL CHardwareFunction::IsBoardExisted()
{
#ifdef _OFFLINE
	return TRUE;
#endif // _OFFLINE

	ULONG ulData = m_Operation.ReadBoard(0xA001);
	if (0xFFFFFFFF != ulData)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CHardwareFunction::IsControllerExist()
{
#ifdef _OFFLINE
	return TRUE;
#endif // _OFFLINE

	ULONG ulREGValue;
	BOOL bExisted = TRUE;

	ULONG ulDate = m_Operation.ReadRegister(COperation::REG_TYPE::SYS_REG, 0x0003);
	do
	{
		// read FPGA version register 
		ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::SYS_REG, 0x0000);
		if (ulDate == ulREGValue || 0x8212 != (ulREGValue >> 16 & 0xFFFF))
		{
			bExisted = FALSE;
			break;
		}
	} while (0);

	return bExisted;
}

ULONG CHardwareFunction::GetFlashID()
{
	DWORD dwFlashID = 0;

	//Write Controller word
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x00008000);
	//Write flash ID request command
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_READ_ID);

	m_Operation.WaitUs(200);
	return m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
}

BOOL CHardwareFunction::CheckFlashID()
{
	DWORD dwReadID = GetFlashID();
	if (FLASH_ID == dwReadID)
	{
		return TRUE;
	}
	return FALSE;
}

int CHardwareFunction::ReadFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usReadByteCount, BYTE *pbyData)
{
	if (SECTOR_COUNT <= bySectorNo)
	{
		return -1;
	}
	if (SECTOR_PAGE_COUNT <= byPageNo)
	{
		return -2;
	}
	if (FLASH_PAGE_SIZE <= byOffset)
	{
		return -3;
	}
	if (FLASH_PAGE_SIZE < usReadByteCount + byOffset)
	{
		return -4;
	}
	if (nullptr == pbyData || 0 == usReadByteCount)
	{
		return -5;
	}
	DWORD dwFlashID = GetFlashID();
	if (FLASH_ID != dwFlashID)
	{
		return -6;
	}

	DWORD dwReadStartAddr = bySectorNo * FLASH_SECTOR_SIZE + byPageNo * FLASH_PAGE_SIZE + byOffset;
	m_Operation.WriteBoard(FLASH_ADDR_ADDR, dwReadStartAddr);

	DWORD dwReadData = 0;
	int nIntNum = usReadByteCount >> 2;
	int nLeftNum = usReadByteCount % 4;
	BYTE byReadLengthDword = nIntNum + ((nLeftNum > 0) ? 1 : 0);
	//Load data length
	m_Operation.WriteBoard(FLASH_DATA_LENGTH_ADDR, byReadLengthDword);
	//Write Controller word, reset FIFO
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x80000000);
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x00000000);
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_READ_ENABLE);
	m_Operation.WaitUs(200);
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x20000000);
	for (int i = 0; i < nIntNum; ++i)
	{
		dwReadData = m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
		pbyData[i * 4 + 0] = (dwReadData >> 24) & 0x00FF;
		pbyData[i * 4 + 1] = (dwReadData >> 16) & 0x00FF;
		pbyData[i * 4 + 2] = (dwReadData >> 8) & 0x00FF;
		pbyData[i * 4 + 3] = dwReadData & 0x00FF;
	}
	if (0 < nLeftNum)
	{
		dwReadData = m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
		for (int i = 0; i < nLeftNum; ++i)
		{
			pbyData[nIntNum * 4 + i] = (dwReadData >> (24 - i * 8)) & 0x000000FF;
		}
	}
	return 0;
}

int CHardwareFunction::WriteFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usWriteByteCount, BYTE *pbyWriteData)
{
	if (SECTOR_COUNT <= bySectorNo)
	{
		return -1;
	}
	if (SECTOR_PAGE_COUNT <= byPageNo)
	{
		return -2;
	}
	if (FLASH_PAGE_SIZE <= byOffset)
	{
		return -3;
	}
	if (FLASH_PAGE_SIZE < usWriteByteCount + byOffset)
	{
		return -4;
	}
	if (nullptr == pbyWriteData || 0 == usWriteByteCount)
	{
		return -5;
	}

	DWORD dwFlashID = GetFlashID();
	if (FLASH_ID != dwFlashID)
	{
		return -6;
	}

	DWORD dwWriteStartAddr = bySectorNo * FLASH_SECTOR_SIZE + byPageNo * FLASH_PAGE_SIZE + byOffset;
	//Load flash start address.
	m_Operation.WriteBoard(FLASH_ADDR_ADDR, dwWriteStartAddr);

	int nIntNum = usWriteByteCount >> 2;
	int nLeftNum = usWriteByteCount % 4;
	BYTE byWriteLengthDword = nIntNum + ((nLeftNum > 0) ? 1 : 0);
	//Load data length
	m_Operation.WriteBoard(FLASH_DATA_LENGTH_ADDR, byWriteLengthDword);

	//Write Controller word, reset FIFO
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x80000000);
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x00000000);

	//Load data into FIFO
	DWORD dwWriteData = 0;
	int i = 0;
	for (i = 0; i < nIntNum; ++i)
	{
		dwWriteData = (pbyWriteData[i * 4] << 24) | (pbyWriteData[i * 4 + 1] << 16)
			| (pbyWriteData[i * 4 + 2] << 8) | pbyWriteData[i * 4 + 3];
		m_Operation.WriteBoard(FLASH_DATA_ADDR, dwWriteData);
	}
	dwWriteData = 0;
	if (0 != nLeftNum)
	{
		for (i = 0; i < nLeftNum; ++i)
		{
			dwWriteData += (pbyWriteData[nIntNum * 4 + i] << (24 - i * 8));
		}
		dwWriteData |= (0xFFFFFFFF >> (nLeftNum * 8));//The invalid bit is set to 1.
		m_Operation.WriteBoard(FLASH_DATA_ADDR, dwWriteData);
	}
	//Enable write
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_WRITE_ENABLE);
	m_Operation.WaitUs(20);
	//Enable page program
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_PAGE_PROG);
	m_Operation.WaitUs(200);

	//Wait for the page program completed
	BYTE byFlashStatus = 0;
	int nPollTimes = 0;
	do
	{
		byFlashStatus = GetFlashStatus();
		DelayUs(POLLING_INTERVAL);
		++nPollTimes;
	} while ((byFlashStatus & 0x01) && (nPollTimes < FLASH_WAIT_TIMES));

	BYTE *pbyReadData = new BYTE[usWriteByteCount];
	int nErrorNum = 0;
	ReadFlash(bySectorNo, byPageNo, byOffset, usWriteByteCount, pbyReadData);

	if (0 != memcmp(pbyReadData, pbyWriteData, usWriteByteCount))
	{
		for (i = 0; i < usWriteByteCount; ++i)
		{
			if (pbyReadData[i] != pbyWriteData[i])
			{
				++nErrorNum;
			}
		}
	}
	delete[] pbyReadData;
	pbyReadData = nullptr;
	return nErrorNum;
}

int CHardwareFunction::EraseFlash(BYTE bySectorNo)
{
	if (SECTOR_COUNT <= bySectorNo)
	{
		return -1;
	}
	//load flash start addr
	m_Operation.WriteBoard(FLASH_ADDR_ADDR, bySectorNo * FLASH_SECTOR_SIZE);

	//write flash cmd
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_WRITE_ENABLE);
	m_Operation.WaitUs(20);
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLAH_CMD_EARSH_SECTOR);
	//Wait for the sector erase completed.
	BYTE byFlashStatus = 0;
	int nPollTimes = 0;
	do
	{
		byFlashStatus = GetFlashStatus();
		DelayUs(POLLING_INTERVAL);
		++nPollTimes;
	} while ((byFlashStatus & 0x01) && nPollTimes < 10000);

	if (FLASH_WAIT_TIMES <= nPollTimes)
	{
		return -2;
	}
	return 0;
}

USHORT CHardwareFunction::GetBoardLogicRevision()
{
	ULONG ulREGValue = read_dw(m_bySlotNo << 18);
	m_ulBoardVer = ulREGValue & 0xFFFF;
	return m_ulBoardVer;
}

USHORT CHardwareFunction::GetControllerLogicRevision()
{
	return m_Operation.ReadRegister(COperation::REG_TYPE::SYS_REG, 0x0000) & 0xFFFF;
}

int CHardwareFunction::InitMCU(const std::vector<USHORT>& vecChannel)
{
	SetChannelStatus(vecChannel, CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE);
	return 0;
}

int CHardwareFunction::SetFunctionRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect)
{
	if (nullptr == m_pRelay)
	{
		m_pRelay = new CRelay(m_Operation);
	}
	int nRetVal = m_pRelay->FunctionRelay(vecChannel, bConnect);
	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CHardwareFunction::GetConnectChannel(std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType)
{
	if (nullptr == m_pRelay)
	{
		m_pRelay = new CRelay(m_Operation);
	}

	int nRetVal = m_pRelay->GetConnectedChannel(RelayType, vecChannel);
	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CHardwareFunction::SetPinLevel(const std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVT, double dVOH, double dVOL, double dClampHigh, double dClampLow)
{
	std::map<USHORT, ULONG> mapVIH, mapVIL, mapVT, mapVOH, mapVOL, mapVCH, mapVCL;
	std::map<USHORT, ULONG> mapPePmu, mapChState, mapPmuState;
	ULONG ulVIH(0), ulVIL(0), ulVT(0), ulVOH(0), ulVOL(0), ulVCH(0), ulVCL(0);
	USHORT index = 0;
	int nRetVal = 0;

	do
	{
		if (PIN_LEVEL_MIN > dVIH || PIN_LEVEL_MAX < dVIH)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dVIH");
				m_pAlarm->SetAlarmMsg("The VIH(%.1f) is over range[%.1f,%.1f].", dVIH, PIN_LEVEL_MIN, PIN_LEVEL_MAX);
			}
			nRetVal = -2;
			break;
		}
		if (PIN_LEVEL_MIN > dVIL || PIN_LEVEL_MAX < dVIL)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dVIL");
				m_pAlarm->SetAlarmMsg("The VIL(%.1f) is over range[%.1f,%.1f].", dVIL, PIN_LEVEL_MIN, PIN_LEVEL_MAX);
			}
			nRetVal = -2;
			break;
		}
		if (PIN_LEVEL_MIN > dVT || PIN_LEVEL_MAX < dVT)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dVT");
				m_pAlarm->SetAlarmMsg("The VT(%.1f) is over range[%.1f,%.1f].", dVT, PIN_LEVEL_MIN, PIN_LEVEL_MAX);
			}
			nRetVal = -2;
			break;

		}
		if (PIN_LEVEL_MIN > dVOH || PIN_LEVEL_MAX < dVOH)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dVOH");
				m_pAlarm->SetAlarmMsg("The VOH(%.1f) is over range[%.1f,%.1f].", dVOH, PIN_LEVEL_MIN, PIN_LEVEL_MAX);
			}
			nRetVal = -2;
			break;
		}
		if (PIN_LEVEL_MIN > dVOL || PIN_LEVEL_MAX < dVOL)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dVOL");
				m_pAlarm->SetAlarmMsg("The VOL(%.1f) is over range[%.1f,%.1f].", dVOL, PIN_LEVEL_MIN, PIN_LEVEL_MAX);
			}
			nRetVal = -2;
			break;

		}
		if (CLx_LEVEL_MIN > dClampHigh || CLx_LEVEL_MAX < dClampHigh)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dClampHigh");
				m_pAlarm->SetAlarmMsg("The Clamp high(%.1f) is over range[%.1f,%.1f].", dClampHigh, CLx_LEVEL_MIN, CLx_LEVEL_MAX);
			}
			nRetVal = -2;
			break;

		}
		if (CLx_LEVEL_MIN > dClampLow || CLx_LEVEL_MAX < dClampLow)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PIN_LEVEL_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dClampLow");
				m_pAlarm->SetAlarmMsg("The Clamp low(%.1f) is over range[%.1f,%.1f].", dClampHigh, CLx_LEVEL_MIN, CLx_LEVEL_MAX);
			}
			nRetVal = -2;
			break;
		}
		CAL_DATA CalData;

		for (USHORT usChannel : vecChannel)
		{
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}

			mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
			mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
			mapPmuState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));

			CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, CalData);

			ulVIH = (ULONG)((dVIH * CalData.m_fDVHGain[0] + CalData.m_fDVHOffset[0] + 2.5) / 10 * 16383);
			ulVIL = (ULONG)((dVIL * CalData.m_fDVLGain[0] + CalData.m_fDVLOffset[0] + 2.5) / 10 * 16383);
			ulVT = (ULONG)((dVT + 2.5) / 10 * 16383);
			ulVOH = (ULONG)((dVOH + 2.5) / 10 * 16383);
			ulVOL = (ULONG)((dVOL + 2.5) / 10 * 16383);
			ulVCH = (ULONG)((dClampHigh + 2.5) / 10 * 16383);
			ulVCL = (ULONG)((dClampLow + 2.5) / 10 * 16383);

			mapVIH.insert(std::pair<USHORT, ULONG>(usChannel, ulVIH));
			mapVIL.insert(std::pair<USHORT, ULONG>(usChannel, ulVIL));
			mapVT.insert(std::pair<USHORT, ULONG>(usChannel, ulVT));
			mapVOH.insert(std::pair<USHORT, ULONG>(usChannel, ulVOH));
			mapVOL.insert(std::pair<USHORT, ULONG>(usChannel, ulVOL));
			mapVCH.insert(std::pair<USHORT, ULONG>(usChannel, ulVCH));
			mapVCL.insert(std::pair<USHORT, ULONG>(usChannel, ulVCL));
		}
		if (0 != nRetVal)
		{
			break;
		}

		m_Operation.Write305(0x0C, mapPePmu);
		m_Operation.Write305(0x0D, mapChState);
		m_Operation.Write305(0x0E, mapPmuState);
		m_Operation.Write305(0x01, mapVIH);
		m_Operation.Write305(0x02, mapVIL);
		m_Operation.Write305(0x03, mapVT);
		m_Operation.Write305(0x04, mapVOL);
		m_Operation.Write305(0x05, mapVOH);
		m_Operation.Write305(0x06, mapVCH);
		m_Operation.Write305(0x07, mapVCL);
		CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
	} while (0);
	return nRetVal;
}

double CHardwareFunction::GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType)
{
	std::map<USHORT, ULONG> mapRead;
	BYTE byATE305Reg = 0;
	double dPinLevel = 0;

	float fGain = 1;
	float fOffset = 0;
	do
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel) 
		{
			dPinLevel = 0x80000000;
			break;
		}

		if (LEVEL_TYPE::CLAMP_LOW < LevelType)
		{
			dPinLevel = 0x80000001;
			break;
		}
		CAL_DATA CalData;
		CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, CalData);

		mapRead.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
		switch (LevelType)
		{
		case LEVEL_TYPE::VIH:
			byATE305Reg = 0x01;
			fGain = CalData.m_fDVHGain[0];
			fOffset = CalData.m_fDVHOffset[0];
			break;
		case LEVEL_TYPE::VIL:
			byATE305Reg = 0x02;
			fGain = CalData.m_fDVLGain[0];
			fOffset = CalData.m_fDVLOffset[0];
			break;
		case LEVEL_TYPE::VT:
			byATE305Reg = 0x03;
			break;
		case LEVEL_TYPE::VOL:
			byATE305Reg = 0x04;
			break;
		case LEVEL_TYPE::VOH:
			byATE305Reg = 0x05;
			break;
		case LEVEL_TYPE::CLAMP_HIGH:
			byATE305Reg = 0x06;
			break;
		case LEVEL_TYPE::CLAMP_LOW:
			byATE305Reg = 0x07;
			break;
		case LEVEL_TYPE::IOH:
			byATE305Reg = 0x08;
			break;
		case LEVEL_TYPE::IOL:
			byATE305Reg = 0x09;
			break;
		default:
			break;
		}

		auto iterChannel = mapRead.begin();

		m_Operation.Read305(byATE305Reg, mapRead);		

		dPinLevel = (iterChannel->second) * 10.0 / 16383.0 - 2.5;
		dPinLevel -= fOffset;
		dPinLevel /= fGain;
		if (LEVEL_TYPE::IOH == LevelType || LEVEL_TYPE::IOL == LevelType)
		{
			dPinLevel = dPinLevel * 0.012 / 5;
		}			

	} while (0);

	return dPinLevel;
}

int CHardwareFunction::SetPeriod(BYTE byTimesetIndex, double dPeriod)
{
	ULONG ulPeriod = 0;
	int nRetVal = 0;
	do
	{
		if (TIMESET_COUNT <= byTimesetIndex)
		{
			nRetVal = -1;
			break;
		}

		if (MIN_PERIOD - EQUAL_ERROR > dPeriod || MAX_PERIOD + EQUAL_ERROR < dPeriod)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PERIOD_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName("dPeriod");
				m_pAlarm->SetAlarmMsg("The period(%.0fns) is over range[%.0f, %.0f]", dPeriod, MIN_PERIOD, MAX_PERIOD);
			}
			nRetVal = -2;
			break;
		}
		ulPeriod = (ULONG)(dPeriod + EQUAL_ERROR);

		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_PERIOD, byTimesetIndex, 1, &ulPeriod);
		CPeriod::Instance()->SetPeriod(m_bySlotNo, m_byControllerIndex, byTimesetIndex, dPeriod);
		if (CBindInfo::Instance()->IsBind())
		{
			set<BYTE> setSlot;
			set<BYTE> setController;
			CBindInfo::Instance()->GetBindInfo(setSlot, setController);
			for (auto bySlotNo : setSlot)
			{
				for (auto Controller : setController)
				{
					CPeriod::Instance()->SetPeriod(bySlotNo, Controller, byTimesetIndex, dPeriod);
				}
			}
		}

	} while (0);

	return nRetVal;
}

double CHardwareFunction::GetPeriod(BYTE byTimesetIndex)
{
	ULONG ulPeriod = 0;
	double dPeriod = 0;

	do 
	{
		if (TIMESET_COUNT <= byTimesetIndex)
		{
			dPeriod = -1;
			break;
		}

		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_PERIOD, byTimesetIndex, 1, &ulPeriod);

		dPeriod = ulPeriod;
		CPeriod::Instance()->SetPeriod(m_bySlotNo, m_byControllerIndex, byTimesetIndex, dPeriod);
	} while (0);

	return dPeriod;
}

int CHardwareFunction::SetEdge(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, const double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
{
	ULONG ulT1R(0), ulT1F(0), ulIOR(0), ulIOF(0), ulSTBR(0), ulSTBF(0), ulWaveFormat(0);
	USHORT ulStartAddr = 0;

	if (TIMESET_COUNT <= byTimesetIndex)
	{
		return -1;
	}

	switch (WaveFormat)
	{
	case WAVE_FORMAT::NRZ:
		break;
	case WAVE_FORMAT::RZ:
		break;
	case WAVE_FORMAT::RO:
		break;
	case WAVE_FORMAT::SBL:
		break;
	case WAVE_FORMAT::SBH:
		break;
	case WAVE_FORMAT::SBC:
		break;
	default:
		return -2;
		break;
	}
	switch (IOFormat)
	{
	case IO_FORMAT::NRZ:
		break;
	case IO_FORMAT::RO:
		break;
		return -2;
	default:
		break;
	}
	switch (CompareMode)
	{
	case COMPARE_MODE::EDGE:
		break;
	case COMPARE_MODE::WINDOW:
		break;
	default:
		return -2;
		break;
	}
	if (nullptr == pdEdge)
	{
		return -3;
	}

	char lpszParamName[EDGE_COUNT][8] = { "dT1R", "dT1F", "dIOR", "dIOF", "dSTBR", "dSTBF" };
	double dMaxValue = CPeriod::Instance()->GetPeriod(m_bySlotNo, m_byControllerIndex, byTimesetIndex) + EQUAL_ERROR;
	if (0 >= dMaxValue)
	{
		dMaxValue = GetPeriod(byTimesetIndex);
	}
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT; ++byEdgeIndex)
	{
		if (dMaxValue < pdEdge[byEdgeIndex] || -EQUAL_ERROR > pdEdge[byEdgeIndex])
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_EDGE_ERROR);
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetParamName(lpszParamName[byEdgeIndex]);
				m_pAlarm->SetAlarmMsg("The %s(%.0fns) is over range[%.0f,%.0f].", lpszParamName[byEdgeIndex], pdEdge[byEdgeIndex], 0., dMaxValue);
			}
			return -4;
		}
	}

	int nRetVal = 0;
	if (WAVE_FORMAT::NRZ == WaveFormat)
	{
	}
	else if ((WAVE_FORMAT::RZ == WaveFormat) || (WAVE_FORMAT::RO == WaveFormat))
	{
		nRetVal += EdgeCheck(pdEdge[0], pdEdge[1], "T1F");
	}
	else
	{
		nRetVal += EdgeCheck(pdEdge[0], pdEdge[1], "T1F");
		nRetVal += EdgeCheck(pdEdge[2], pdEdge[0], "T1R");
	}

	if (IO_FORMAT::RO == IOFormat)
	{
		nRetVal += EdgeCheck(pdEdge[2], pdEdge[3], "IOF");
	}

	if (COMPARE_MODE::WINDOW == CompareMode)
	{
		nRetVal += EdgeCheck(pdEdge[4], pdEdge[5], "STBF");
	}
	if (0 != nRetVal)
	{
		nRetVal = -4;
	}

	ulT1R = (ULONG)(pdEdge[0] + EQUAL_ERROR);
	ulT1F = (ULONG)(pdEdge[1] + EQUAL_ERROR);
	ulIOR = (ULONG)(pdEdge[2] + EQUAL_ERROR);
	ulIOF = (ULONG)(pdEdge[3] + EQUAL_ERROR);
	ulSTBR = (ULONG)(pdEdge[4] + EQUAL_ERROR);
	ulSTBF = (ULONG)(pdEdge[5] + EQUAL_ERROR);

	ulWaveFormat = (ULONG)IOFormat;
	ulWaveFormat = (ULONG)(WaveFormat)+(ulWaveFormat << 4);
	for (USHORT usChannel : vecChannel)
	{
		// FMT
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulWaveFormat);

		// T1R
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (1 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulT1R);

		// T1F
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (2 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulT1F);

		// IOR
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (3 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulIOR);

		// IOF
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (4 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulIOF);

		// STBR
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (5 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulSTBR);

		// STBF
		ulStartAddr = (usChannel & 0x0F) << 12;
		ulStartAddr |= (6 << 8);
		ulStartAddr |= (byTimesetIndex & 0xFF);
		m_Operation.WriteRAM(COperation::BRAM_TYPE::MEM_TIMING, ulStartAddr, 1, &ulSTBF);
	}

	return nRetVal;
}

int CHardwareFunction::GetEdge(USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	ULONG ulREGValue = 0;
	USHORT usStartAddr = 0;
	int nRetVal = 0;

	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (TIMESET_COUNT <= byTimesetIndex)
	{
		return -2;
	}
	if (nullptr == pdEdge)
	{
		return -3;
	}
	// FMT
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);

	IOFormat = (IO_FORMAT)(ulREGValue >> 4 & 0x03);
	WaveFormat = (WAVE_FORMAT)(ulREGValue & 0x0F);

	// T1R
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (1 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[0] = ulREGValue;

	// T1F
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (2 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[1] = ulREGValue;

	// IOR
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (3 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[2] = ulREGValue;

	// IOF
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (4 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[3] = ulREGValue;

	// STBR
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (5 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[4] = ulREGValue;

	// STBF
	usStartAddr = (usChannel & 0x0F) << 12;
	usStartAddr |= (6 << 8);
	usStartAddr |= (byTimesetIndex & 0xFF);
	m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_TIMING, usStartAddr, 1, &ulREGValue);
	pdEdge[5] = ulREGValue;

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x818);
	if (0 == (ulREGValue >> 2 & 0x01))
	{
		CompareMode = COMPARE_MODE::EDGE;
	}
	else
	{
		CompareMode = COMPARE_MODE::WINDOW;
	}
	return 0;
}

int CHardwareFunction::ReadDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, UINT uStartLine, UINT uReadLineCount, USHORT *pusDataBuff)
{
	BOOL bBRAM = TRUE;
	UINT uMaxPatternCount = DCM_BRAM_PATTERN_LINE_COUNT;
	switch (MemoryType)
	{
	case MEM_TYPE::BRAM:
		break;
	case MEM_TYPE::DRAM:
		uMaxPatternCount = DCM_DRAM_PATTERN_LINE_COUNT;
		bBRAM = FALSE;
		break;
	default:
		return -1;
		break;
	}

	COperation::BRAM_TYPE RAMType = COperation::BRAM_TYPE::FM;
	COperation::DRAM_TYPE DRAMType = COperation::DRAM_TYPE::FM;

	switch (DataType)
	{
	case DATA_TYPE::FM:
		RAMType = COperation::BRAM_TYPE::FM;
		DRAMType = COperation::DRAM_TYPE::FM;
		break;
	case DATA_TYPE::MM:
		RAMType = COperation::BRAM_TYPE::MM;
		DRAMType = COperation::DRAM_TYPE::MM;
		break;
	case DATA_TYPE::IOM:
		RAMType = COperation::BRAM_TYPE::IOM;
		DRAMType = COperation::DRAM_TYPE::IOM;
		break;
	case DATA_TYPE::CMD:
		RAMType = COperation::BRAM_TYPE::IMM1;
		DRAMType = COperation::DRAM_TYPE::CMD;
		break;
	case DATA_TYPE::OPERAND:
		RAMType = COperation::BRAM_TYPE::IMM2;
		if (MEM_TYPE::DRAM == MemoryType)
		{
			return -2;
		}
		break;
	default:
		return -2;
		break;
	}

	if (uMaxPatternCount <= uStartLine)
	{
		return -3;
	}
	else if (uMaxPatternCount < uStartLine + uReadLineCount)
	{
		return -4;
	}
	if (nullptr == pusDataBuff || 0 == uReadLineCount)
	{
		return -5;
	}

	BOOL bStartOdd = FALSE;
	UINT uDRAMStartLine = uStartLine / 2;
	UINT uLineCount = (uReadLineCount + 1) / 2;
	if (0 != uStartLine % 2)
	{
		bStartOdd = TRUE;
		if (0 == uReadLineCount % 2)
		{
			++uLineCount;
		}
	}

	if (!bBRAM)
	{
		WaitDRAMReady();
	}

	ULONG *pulReadData = nullptr;
	try
	{
		pulReadData = new ULONG[uLineCount];
		memset(pulReadData, 0, uLineCount * sizeof(ULONG));
	}
	catch (const std::exception&)
	{
		return -6;
	}

	CBindInfo* pBind = CBindInfo::Instance();
	BOOL bBind = pBind->IsBind();

	set<BYTE> setSlot;
	set<BYTE> setController;
	BYTE byTargetSlot = 0;
	if (bBind)
	{
		byTargetSlot = pBind->GetBindInfo(setSlot, setController);
		pBind->ClearBind();
	}
	if (bBRAM)
	{
		m_Operation.ReadRAM(RAMType, uDRAMStartLine, uLineCount, pulReadData);
	}
	else
	{
		m_Operation.ReadDRAM(DRAMType, uDRAMStartLine, uLineCount, pulReadData);
	}
	
	if (bBind)
	{
		pBind->Bind(setSlot, setController, byTargetSlot);
	}

	BYTE byShifOffset = 0;
	UINT uPatternLineIndex = 0;
	UINT uLineIndex = 0;
	do
	{
		for (int nIndex = 0; nIndex < 2; ++nIndex)
		{
			if (0 == uPatternLineIndex && bStartOdd)
			{
				++nIndex;
			}
			byShifOffset = nIndex * 16;
			if (MEM_TYPE::DRAM == MemoryType)
			{
				byShifOffset = 16 - byShifOffset;
			}
			pusDataBuff[uPatternLineIndex] = (pulReadData[uLineIndex] >> byShifOffset) & 0xFFFF;
			++uPatternLineIndex;
			if (uPatternLineIndex >= uReadLineCount)
			{
				break;
			}
		}
		++uLineIndex;
	} while (uPatternLineIndex < uReadLineCount);
	if (nullptr != pulReadData)
	{
		delete[] pulReadData;
		pulReadData = nullptr;
	}

	return 0;
}

int CHardwareFunction::WriteDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, UINT uStartLine, UINT uWriteLineCount, const USHORT *pusData)
{
	UINT uMaxPatternCount = DCM_BRAM_PATTERN_LINE_COUNT;
	switch (MemoryType)
	{
	case MEM_TYPE::BRAM:
		break;
	case MEM_TYPE::DRAM:
		uMaxPatternCount = DCM_DRAM_PATTERN_LINE_COUNT;
		break;
	default:
		return -1;
		break;
	}

	COperation::BRAM_TYPE RAMType = COperation::BRAM_TYPE::FM;
	COperation::DRAM_TYPE DRAMType = COperation::DRAM_TYPE::FM;

	switch (DataType)
	{
	case DATA_TYPE::FM:
		RAMType = COperation::BRAM_TYPE::FM;
		DRAMType = COperation::DRAM_TYPE::FM;
		break;
	case DATA_TYPE::MM:
		RAMType = COperation::BRAM_TYPE::MM;
		DRAMType = COperation::DRAM_TYPE::MM;
		break;
	case DATA_TYPE::IOM:
		RAMType = COperation::BRAM_TYPE::IOM;
		DRAMType = COperation::DRAM_TYPE::IOM;
		break;
	case DATA_TYPE::CMD:
		RAMType = COperation::BRAM_TYPE::IMM1;
		DRAMType = COperation::DRAM_TYPE::CMD;
		break;
	case DATA_TYPE::OPERAND:
		RAMType = COperation::BRAM_TYPE::IMM2;
		if (MEM_TYPE::DRAM == MemoryType)
		{
			return -2;
		}
		break;
	default:
		return -2;
		break;
	}

	if (uMaxPatternCount <= uStartLine)
	{
		return -3;
	}
	else if (uMaxPatternCount < uStartLine + uWriteLineCount)
	{
		return -4;
	}

	if (nullptr == pusData || 0 == uWriteLineCount)
	{
		return -5;
	}
	int nRetVal = 0;

	BOOL bBind = CBindInfo::Instance()->IsBind();
	USHORT usControllerCount = 0;
	if (bBind)
	{
		usControllerCount = CBindInfo::Instance()->GetBindControllerCount();
	}
	vector<USHORT> vecBindController;
	ULONG* pulOddBindData = nullptr;
	ULONG* pulEvenBindData = nullptr;
	
	BOOL bStartOdd = FALSE;
	BOOL bEndEven = FALSE;
	int nBaseOffset = 0;
	int nLineCountMinus = 0;
	ULONG ulData = 0;
	UINT uRAMStartLine = uStartLine / 2;
	UINT uLineCount = (uWriteLineCount + 1) / 2;

	if (0 != uStartLine % 2)
	{
		bStartOdd = TRUE;
		if (bBind)
		{
			///<Has bind board, clear bind and read data, then rebind
			try
			{
				pulOddBindData = new ULONG[usControllerCount];
				memset(pulOddBindData, 0, usControllerCount * sizeof(ULONG));
			}
			catch (const std::exception&)
			{
				return -6;
			}

			int nDataIndex = 0;
			COperation* pOperation = nullptr;
			BIND_READ_BEGIN(pOperation, m_bySlotNo)
			{
				if (MEM_TYPE::BRAM == MemoryType)
				{
					pOperation->ReadRAM(RAMType, uRAMStartLine, 1, &ulData);
					ulData &= 0x0000FFFF;
				}
				else
				{
					pOperation->ReadDRAM(DRAMType, uRAMStartLine, 1, &ulData);
					ulData = ulData & 0xFFFF0000;
				}
				pulOddBindData[nDataIndex++] = ulData;
			}
			BIND_READ_END

			nBaseOffset = 1;
		}
		else
		{
			if (MEM_TYPE::BRAM == MemoryType)
			{
				m_Operation.ReadRAM(RAMType, uRAMStartLine, 1, &ulData);
				ulData &= 0x0000FFFF;
			}
			else
			{
				m_Operation.ReadDRAM(DRAMType, uRAMStartLine, 1, &ulData);
				ulData = ulData & 0xFFFF0000;
			}
		}
		if (0 == uWriteLineCount % 2)
		{
			++uLineCount;
			bEndEven = TRUE;
		}
	}
	else if (0 != uWriteLineCount % 2)
	{
		bEndEven = TRUE;
	}

	ULONG *pulLineData = nullptr;
	try
	{
		pulLineData = new ULONG[uLineCount];
		memset(pulLineData, 0, uLineCount * sizeof(ULONG));
	}
	catch (const std::exception &e)
	{
		if (nullptr != pulOddBindData)
		{
			delete[] pulOddBindData;
			pulOddBindData = nullptr;
		}
		return -6;
	}

	int nDataOffset = 0;
	BYTE byShifOffset = 0;
	UINT uPatternLineIndex = 0;
	UINT uLineIndex = 0;
	do
	{
		for (int nIndex = 0; nIndex < 2; ++nIndex)
		{
			if (0 == uPatternLineIndex && bStartOdd)
			{
				pulLineData[0] = (pusData[0] << 16) | ulData;
				if (MEM_TYPE::DRAM == MemoryType)
				{
					pulLineData[0] = pusData[0] | ulData;
				}
				++uPatternLineIndex;
				break;
			}
			byShifOffset = nIndex * DCM_CHANNELS_PER_CONTROL;
			
			if (MEM_TYPE::DRAM == MemoryType)
			{
				byShifOffset = DCM_CHANNELS_PER_CONTROL - byShifOffset;
			}
			pulLineData[uLineIndex] |= pusData[uPatternLineIndex] << byShifOffset;

			++uPatternLineIndex;
			if (uPatternLineIndex >= uWriteLineCount)
			{
				break;
			}
		}
		++uLineIndex;
	} while (uPatternLineIndex < uWriteLineCount);

	if (bEndEven)
	{
		UINT uCurLineIndex = uLineIndex - 1;
		if (bBind)
		{
			try
			{
				pulEvenBindData = new ULONG[usControllerCount];
				memset(pulEvenBindData, 0, usControllerCount * sizeof(ULONG));
			}
			catch (const std::exception&)
			{
				if (nullptr != pulOddBindData)
				{
					delete[] pulOddBindData;
					pulOddBindData = nullptr;
				}
				if (nullptr != pulLineData)
				{
					delete[] pulLineData;
					pulLineData = nullptr;
				}
				return -6;
			}

			int nControllerIndex = 0;
			COperation* pOperation = nullptr;
			BIND_READ_BEGIN(pOperation, m_bySlotNo)
			{
				if (MEM_TYPE::BRAM == MemoryType)
				{
					pOperation->ReadRAM(RAMType, uRAMStartLine + uCurLineIndex, 1, &ulData);
					ulData = (pulLineData[uCurLineIndex] & 0x0000FFFF) | (ulData & 0xFFFF0000);
				}
				else
				{
					pOperation->ReadDRAM(DRAMType, uRAMStartLine + uCurLineIndex, 1, &ulData);
					ulData = (pulLineData[uCurLineIndex] & 0xFFFF0000) | (ulData & 0x0000FFFF);
				}
				pulEvenBindData[nControllerIndex++] = ulData;
			}
			BIND_READ_END

			nLineCountMinus = nBaseOffset + 1;
		}
		else
		{
			if (MEM_TYPE::BRAM == MemoryType)
			{
				m_Operation.ReadRAM(RAMType, uRAMStartLine + uCurLineIndex, 1, &ulData);
				pulLineData[uCurLineIndex] = (pulLineData[uCurLineIndex] & 0x0000FFFF) | (ulData & 0xFFFF0000);
			}
			else
			{
				m_Operation.ReadDRAM(DRAMType, uRAMStartLine + uCurLineIndex, 1, &ulData);
				pulLineData[uCurLineIndex] = (pulLineData[uCurLineIndex] & 0xFFFF0000) | (ulData & 0x0000FFFF);
			}
		}
	}

	if (MEM_TYPE::BRAM == MemoryType)
	{
		m_Operation.WriteRAM(RAMType, uRAMStartLine + nBaseOffset, uLineIndex - nLineCountMinus, &pulLineData[nBaseOffset]);
	}
	else
	{
		m_Operation.WriteDRAM(DRAMType, uRAMStartLine + nBaseOffset, uLineIndex - nLineCountMinus, &pulLineData[nBaseOffset]);
	}

	if (bBind)
	{
		///<Has bind, clear bind and write data to each controller binded
		int nControllerIndex = 0;
		COperation* pOperation = nullptr;
		BIND_READ_BEGIN(pOperation, m_bySlotNo)
		{
			if (bStartOdd)
			{
				if (MEM_TYPE::BRAM == MemoryType)
				{
					pulOddBindData[nControllerIndex] |= pusData[0] << DCM_CHANNELS_PER_CONTROL;
					pOperation->WriteRAM(RAMType, uRAMStartLine, 1, &pulOddBindData[nControllerIndex]);
				}
				else
				{
					pulOddBindData[nControllerIndex] |= pusData[0];
					pOperation->WriteDRAM(DRAMType, uRAMStartLine, 1, &pulOddBindData[nControllerIndex]);

				}
			}
			if (bEndEven)
			{
				int nDataIndex = uWriteLineCount - 1;
				if (MEM_TYPE::BRAM == MemoryType)
				{
					pulEvenBindData[nControllerIndex] |= pusData[nDataIndex];
					pOperation->WriteRAM(RAMType, uRAMStartLine + uLineIndex - 1, 1, &pulEvenBindData[nControllerIndex]);
				}
				else
				{
					pulEvenBindData[nControllerIndex] |= pusData[nDataIndex] << DCM_CHANNELS_PER_CONTROL;
					pOperation->WriteDRAM(DRAMType, uRAMStartLine + uLineIndex - 1, 1, &pulEvenBindData[nControllerIndex]);
				}
			}
			++nControllerIndex;
		}
		BIND_READ_END
	}
	if (nullptr != pulOddBindData)
	{
		delete[] pulOddBindData;
		pulOddBindData = nullptr;
	}
	if (nullptr != pulEvenBindData)
	{
		delete[] pulEvenBindData;
		pulEvenBindData = nullptr;
	}
	if (nullptr != pulLineData)
	{
		delete[] pulLineData;
		pulLineData = nullptr;
	}
	return 0;
}

int CHardwareFunction::WriteChannelDataMemory(MEM_TYPE MemoryType, DATA_TYPE DataType, USHORT usChannel, UINT uStartLine, UINT uWriteLineCount, ULONG* pulData)
{
	switch (MemoryType)
	{
	case MEM_TYPE::BRAM:
		break;
	case MEM_TYPE::DRAM:
	default:
		return -1;
		break;
	}
	switch (DataType)
	{
	case DATA_TYPE::FM:
		break;
	case DATA_TYPE::MM:
		break;
	case DATA_TYPE::IOM:
		break;
	case DATA_TYPE::CMD:
	case DATA_TYPE::OPERAND:
	default:
		return -2;
		break;
	}
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -3;
	}
	if (DCM_BRAM_PATTERN_LINE_COUNT <= uStartLine)
	{
		return -4;
	}
	if (DCM_BRAM_PATTERN_LINE_COUNT < uStartLine + uWriteLineCount)
	{
		return -5;
	}

	UINT uDataLeft = uWriteLineCount;
	UINT uCurWriteLine = 0;
	UINT uCurStartLine = uStartLine;
	int nDataIndex = 0;
	while (0 < uDataLeft)
	{
		uCurWriteLine = 32 < uDataLeft ? 32 : uDataLeft;

		ULONG ulMode = uCurStartLine;
		ulMode |= BYTE(DataType) << 18;
		ulMode |= usChannel << 20;
		ulMode |= uCurWriteLine << 24;

		ULONG ulBaseMode = ulMode;
		ulMode |= m_byControllerIndex << 16;

		if (CBindInfo::Instance()->IsBind())
		{
			COperation* pOperation = nullptr;
			BIND_READ_BEGIN(pOperation, m_bySlotNo)
			{
				ulMode = ulBaseMode | (pOperation->GetControllerIndex() << 16);
				pOperation->WriteBoard(0x0101, ulMode);
				pOperation->WriteBoard(0x0102, pulData[nDataIndex]);

				ULONG ulStatus = 0;
				while (0 == ulStatus)
				{
					ulStatus = pOperation->ReadBoard(0x0D);
				}
			}
			BIND_READ_END
		}
		else
		{
			m_Operation.WriteBoard(0x0101, ulMode);
			m_Operation.WriteBoard(0x0102, pulData[nDataIndex]);

			ULONG ulStatus = 0;
			while (0 == ulStatus)
			{
				ulStatus = m_Operation.ReadBoard(0x0D);
			}
		}

		uDataLeft -= uCurWriteLine;
		uCurStartLine += uCurWriteLine;
		++nDataIndex;
	}

	return 0;
}

int CHardwareFunction::SetRunParameter(UINT uStartLineNo, UINT uStopLineNo, BOOL bWithDRAM, UINT uDRAMStartLineNo, BOOL bEnableStart)
{
	ULONG regValue = 0;
	int nRet = 0;
	do 
	{
		if (DCM_BRAM_PATTERN_LINE_COUNT <= uStartLineNo)
		{
			nRet = -1;
			break;
		}

		if (DCM_BRAM_PATTERN_LINE_COUNT <= uStopLineNo)
		{
			nRet = -2;
			break;
		}

		if (uStartLineNo >= uStopLineNo) 
		{
			nRet = -3;
			break;
		}

		if (DCM_DRAM_PATTERN_LINE_COUNT < uDRAMStartLineNo)
		{
			nRet = -4;
			break;
		}

		ULONG ulPatMode = 0x08;///<The value of register PAT_MODE
		if (0x30 < m_ulBoardVer)
		{
			for (auto Channel : m_setUnComparedChannel)
			{
				ulPatMode |= 1 << (Channel + 16);///<Shield channel's comparison
			}
			ulPatMode |= m_bSelectFail << 5;
		}
		m_setUnComparedChannel.clear();

		if (bWithDRAM)
		{
			ulPatMode |= 1 << 7;
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0818, ulPatMode);
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0819, uDRAMStartLineNo / 2);
			
			WaitDRAMReady();
		}
		else
		{
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0818, ulPatMode);
		}
		m_bLatestRanWithDRAM = bWithDRAM;

		///<Enable receive start signal
		if (bEnableStart)
		{
			m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x0017, 0);
		}

		regValue = ((uStopLineNo & 0x1FFF) << 16) + (uStartLineNo & 0x1FFF);
		m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0820, regValue);
	} while (0);

	return nRet;
}

int CHardwareFunction::SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	switch (ChannelStatus)
	{
	case CHANNEL_OUTPUT_STATUS::LOW:
		break;
	case CHANNEL_OUTPUT_STATUS::HIGH:
		break;
	case CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE:
		break;
	default:
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_MODE_ERROR);
			m_pAlarm->SetAlarmMsg("The channel status(%d) is not supported.", (int)ChannelStatus);
		}
		return -1;
		break;
	}
	ULONG ulREGValue = 0;
	ULONG ulEnable = 0;
	for (auto usChannel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			return -2;
		}
		ulEnable |= 1 << usChannel;
		BYTE byBitValue = 0;
		if (CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE == ChannelStatus)
		{
			byBitValue = 1;
		}
		if (0 != byBitValue)
		{
			ulREGValue |= 1 << usChannel;
		}
		
		if (CHANNEL_OUTPUT_STATUS::HIGH == ChannelStatus)
		{
			ulREGValue |= 1 << (DCM_CHANNELS_PER_CONTROL + usChannel);
		}
	}

	SetChannelMCUMode(vecChannel);

	m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0815, ulREGValue);
	m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0816, ulEnable);
	return 0;
}

int CHardwareFunction::DisableChannel(const std::vector<USHORT>& vecChannel)
{
	return CChannelMode::Instance()->SetUnexpectMode(m_Operation, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
}

int CHardwareFunction::GetChannelMode(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	CChannelMode::CHANNEL_MODE ChannelMode = CChannelMode::Instance()->GetChannelMode(m_Operation, usChannel);
	return (BYTE)ChannelMode;
}

void CHardwareFunction::UpdateChannelMode()
{
	CChannelMode::Instance()->UpdateMode(m_Operation);
}

void CHardwareFunction::Run()
{
	m_bBRAMFailMemoryFilled = FALSE;
	m_bDRAMFailMemoryFilled = FALSE;
	m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x000E, 0x00000002);
	if (m_bLatestRanWithDRAM)
	{
		m_Operation.WaitUs(25);
	}
}

void CHardwareFunction::SynRun()
{
	m_bBRAMFailMemoryFilled = FALSE;
	m_bDRAMFailMemoryFilled = FALSE;
	read_dw(0xFC0000);
	write_dw(0xFC0000, 0);
	if (m_bLatestRanWithDRAM)
	{
		DelayUs(25);
	}
}

void CHardwareFunction::EnableStart(BOOL bEnable)
{
	ULONG ulREG = 0x01;
	if (bEnable)
	{
		ulREG = 0x00;
	}
	m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x0017, ulREG);
}

void CHardwareFunction::StopRun()
{
	m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x000E, 0x00000004);
}

int CHardwareFunction::GetRunningStatus()
{
	ULONG ulREGValue = 0;

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);

	if (!(ulREGValue & (1 << 30))) 
	{
		return -1;
	}
	ULONG ulCheckBit = 1 << 24 | 1 << 20;
	if (ulREGValue & ulCheckBit)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CHardwareFunction::GetStopLineNo()
{
	ULONG ulREGValue = 0;
	int nRetVal = 0;

	nRetVal = GetRunningStatus();
	if (-1 == nRetVal) 
	{
		return -1;
	}
	if (0 == nRetVal)
	{
		return -2;
	}

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0814);
	if (ulREGValue & (1 << 24)) 
	{
		return 1;
	}

	ulREGValue &= 0x1FFF;

	return ulREGValue;
}

ULONG CHardwareFunction::GetRunLineCount()
{
	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x800);
}

int CHardwareFunction::GetResult()
{
	ULONG ulREGValue = 0;

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);
	if (!(ulREGValue & (1 << 30))) 
	{
		return -1;
	}

	if (!(ulREGValue & (1 << 24)))
	{
		return -2;
	}

	if (ulREGValue & (1 << 23))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int CHardwareFunction::GetChannelResult(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);
	if (!(ulREGValue & (1 << 30)))
	{
		///<Not ran vector
		return -2;
	}

	if (!(ulREGValue & (1 << 24)))
	{
		///<running
		return -3;
	}
	return ulREGValue >> usChannel & 0x01;
}

int CHardwareFunction::GetMCUResult(const std::vector<USHORT>& vecChannel)
{
	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);
	if (!(ulREGValue & (1 << 30)))
	{
		///<Not ran vector
		return -1;
	}

	if (!(ulREGValue & (1 << 24)))
	{
		///<running
		return -2;
	}
	int nRetVal = 0;
	for (auto Channel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= Channel)
		{
			///<Channel number is over range
			nRetVal = -3;
			break;
		}
		if (0 != (ulREGValue >> Channel & 0x01))
		{
			nRetVal = 1;
			break;
		}
	}
	return nRetVal;
}

int CHardwareFunction::GetChannelFailCount(USHORT usChannel)
{
	ULONG ulREGValue = 0;
	int nRetVal = 0;

	if (DCM_CHANNELS_PER_CONTROL <= usChannel) 
	{
		return -1;
	}

	nRetVal = GetRunningStatus();
	if (-1 == nRetVal) 
	{
		return -2;
	}
	if (0 == nRetVal)
	{
		return -3;
	}

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0804 + usChannel);

	return ulREGValue;
}

int CHardwareFunction::GetFailCount()
{
	int nRetVal = GetRunningStatus();
	if (-1 == nRetVal) 
	{
		return -1;
	}
	if (0 == nRetVal)
	{
		return -2;
	}

	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0801);
}

int CHardwareFunction::GetFailData(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine)
{
	vecBRAMLine.clear();
	vecDRAMLine.clear();

	ULONG ulREGValue(0), ulSVMFailCount(0), ulLVMFailCount(0);
	ULONG ulData[BRAM_MAX_SAVE_FAIL_LINE_COUNT] = { 0 };
	int nRetVal = 0;

	nRetVal = GetRunningStatus();
	if (-1 == nRetVal)
	{
		return -1;
	}
	if (0 == nRetVal)
	{
		return -2;
	}

	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0802);
	ulSVMFailCount = (ulREGValue >> 16) & 0x0FFF;
	ulLVMFailCount = ulREGValue & 0x03FF;

	if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < ulSVMFailCount)
	{
		ulSVMFailCount = BRAM_MAX_SAVE_FAIL_LINE_COUNT;
	}
	CheckFailMemoryFilled(ulSVMFailCount, ulLVMFailCount);
	DATA_RESULT FailData;
	if (0 != ulSVMFailCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x0000, ulSVMFailCount, ulData);
		for (int nLineIndex = 0; nLineIndex < ulSVMFailCount; nLineIndex++)
		{
			FailData.m_nLineNo = (ulData[nLineIndex] >> 16) & 0x1FFF;
			FailData.m_usData = ulData[nLineIndex] & 0xFFFF;

			if (0 == FailData.m_usData)
			{
				return -3;
			}
			vecBRAMLine.push_back(FailData);
		}
	}


	if (0 != ulLVMFailCount)
	{
		ULONG ulAddr[DRAM_MAX_SAVE_FAIL_LINE_COUNT] = { 0 };
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x4000, ulLVMFailCount, ulAddr);
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x6000, ulLVMFailCount, ulData);
		for (int nLineIndex = 0; nLineIndex < ulLVMFailCount; nLineIndex++)
		{
			FailData.m_nLineNo = ulAddr[nLineIndex] - 1;
			FailData.m_usData = ulData[nLineIndex] & 0xFFFF;

			if (0 == FailData.m_usData)
			{
				return -3;
			}
			vecDRAMLine.push_back(FailData);
		}
	}

	return 0;
}

int CHardwareFunction::GetFailMemoryFilled(BOOL& bBRAMFilled, BOOL& bDRAMFilled)
{
	if (!m_bBRAMFailMemoryFilled || !m_bDRAMFailMemoryFilled)
	{
		bBRAMFilled = TRUE;
		bDRAMFilled = TRUE;

		int nRetVal = GetRunningStatus();
		if (-1 == nRetVal)
		{
			return -1;
		}
		if (0 == nRetVal)
		{
			return -2;
		}
		ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0802);
		ULONG ulSVMFailCount = (ulREGValue >> 16) & 0x0FFF;
		ULONG ulLVMFailCount = ulREGValue & 0x03FF;
		if (BRAM_MAX_SAVE_FAIL_LINE_COUNT < ulSVMFailCount)
		{
			ulSVMFailCount = BRAM_MAX_SAVE_FAIL_LINE_COUNT;
		}
		CheckFailMemoryFilled(ulSVMFailCount, ulLVMFailCount);
	}
	bBRAMFilled = m_bBRAMFailMemoryFilled;
	bDRAMFilled = m_bDRAMFailMemoryFilled;
	return 0;
}

int CHardwareFunction::SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode)
{
	std::map<USHORT, ULONG> mapPePmu, mapChState, mapVT;
	ULONG ulVTValue = 0;
	int nRetVal = 0;
	if (-1.5 - EQUAL_ERROR > dVTVoltValue || 6 + EQUAL_ERROR < dVTVoltValue)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VT_VALUE_ERROR);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			m_pAlarm->SetAlarmMsg("The VT voltage(%.1f) is over range[%.1f,%.1f].", dVTVoltValue, -1.5, 6.);
		}
		return -2;
	}
	ulVTValue = (ULONG)((dVTVoltValue + 2.5) / 10 * 16383);
	do
	{
		switch (VTMode)
		{
		case VT_MODE::FORCE:
			break;
		case VT_MODE::REALTIME:
			break;
		case VT_MODE::TRISTATE:
			break;
		case VT_MODE::CLOSE:
			break;
		default:
			return -3;
			break;
		}

		for (USHORT usChannel : vecChannel)
		{
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}
			switch (VTMode)
			{
			case VT_MODE::FORCE:
				mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0002));
				mapVT.insert(std::pair<USHORT, ULONG>(usChannel, ulVTValue));
				break;
			case VT_MODE::REALTIME:
				mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
				mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0001));
				mapVT.insert(std::pair<USHORT, ULONG>(usChannel, ulVTValue));
				break;
			case VT_MODE::TRISTATE:
				mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0001));
				break;
			case VT_MODE::CLOSE:
				mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
				mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
				mapVT.insert(std::pair<USHORT, ULONG>(usChannel, ulVTValue));
				break;
			default:
				break;
			}
		}
		if (0 != nRetVal) 
		{
			break;
		}

		if (0 != mapPePmu.size()) 
		{
			m_Operation.Write305(0x0C, mapPePmu);
			CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
		}

		if (0 != mapChState.size()) 
		{
			m_Operation.Write305(0x0D, mapChState);			
		}

		if (0 != mapVT.size()) 
		{
			m_Operation.Write305(0x03, mapVT);
		}

	} while (0);
	return nRetVal;
}

int CHardwareFunction::GetVTMode(USHORT usChannel, VT_MODE& VTMode)
{
	std::map<USHORT, ULONG> mapRead;
	ULONG ulREGValue0(0), ulREGValue1(0);

	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	mapRead.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));

	m_Operation.Read305(0x0C, mapRead);

	ulREGValue0 = mapRead.begin()->second;

	m_Operation.Read305(0x0D, mapRead);

	ulREGValue1 = mapRead.begin()->second;

	if (0x0002 == ulREGValue0)
	{
		VTMode = VT_MODE::FORCE;
	}
	else if (0x0001 == ulREGValue0)
	{
		VTMode = VT_MODE::TRISTATE;
	}
	else if (0x0000 == ulREGValue0)
	{
		if (0x0001 == ulREGValue1)
		{
			VTMode = VT_MODE::REALTIME;
		}
		else if (0x0000 == ulREGValue1)
		{
			VTMode = VT_MODE::CLOSE;
		}
	}
	return 0;
}

int CHardwareFunction::SetDynamicLoad(std::vector<USHORT> vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow)
{
	std::map<USHORT, ULONG> mapVT, mapIOH, mapIOL, mapVCH, mapVCL;
	std::map<USHORT, ULONG> mapPePmu, mapChState;
	ULONG ulVT(0), ulIOH(0), ulIOL(0), ulVCH(0), ulVCL(0);

	if ((dIOH < IOx_LEVEL_MIN) || (dIOH > IOx_LEVEL_MAX)
		|| (dIOL < IOx_LEVEL_MIN) || (dIOL > IOx_LEVEL_MAX))
	{
		return -2;
	}
	if ((dVTValue < PIN_LEVEL_MIN) || (dVTValue > PIN_LEVEL_MAX))
	{
		return -3;
	}
	if ((dClmapHigh < CLx_LEVEL_MIN) || (dClmapHigh > CLx_LEVEL_MAX)
		|| (dClampLow < CLx_LEVEL_MIN) || (dClampLow > CLx_LEVEL_MAX))
	{
		return -4;
	}
	for (auto usChannel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			return -1;
		}
		ulVT = (ULONG)((dVTValue + 2.5) / 10 * 16383);
		ulIOH = (ULONG)((5 * dIOH / 0.012 + 2.5) / 10 * 16383);
		ulIOL = (ULONG)((5 * dIOL / 0.012 + 2.5) / 10 * 16383);
		ulVCH = (ULONG)((dClmapHigh + 2.5) / 10 * 16383);
		ulVCL = (ULONG)((dClampLow + 2.5) / 10 * 16383);

		mapVT.insert(std::pair<USHORT, ULONG>(usChannel, ulVT));
		mapIOH.insert(std::pair<USHORT, ULONG>(usChannel, ulIOH));
		mapIOL.insert(std::pair<USHORT, ULONG>(usChannel, ulIOL));
		mapVCH.insert(std::pair<USHORT, ULONG>(usChannel, ulVCH));
		mapVCL.insert(std::pair<USHORT, ULONG>(usChannel, ulVCL));

		if (bEnable)
		{
			mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0002));
		}
		else
		{
			mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
		}
		mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
	}
	m_Operation.Write305(0x03, mapVT);
	m_Operation.Write305(0x08, mapIOH);
	m_Operation.Write305(0x09, mapIOL);
	m_Operation.Write305(0x06, mapVCH);
	m_Operation.Write305(0x07, mapVCL);
	m_Operation.Write305(0x0D, mapChState);
	m_Operation.Write305(0x0C, mapPePmu);
	CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
	return 0;
}

int CHardwareFunction::GetDynamicLoad(USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL)
{
	ULONG ulChannelState(0), ulIOH(0), ulIOL(0);
	std::map<USHORT, ULONG> mapRead;

	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	mapRead.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
	m_Operation.Read305(0x0D, mapRead);
	ulChannelState = mapRead.begin()->second;

	if (0 != (ulChannelState >> 1 & 0x01))
	{
		bEnable = TRUE;
	}
	else
	{
		bEnable = FALSE;
	}

	m_Operation.Read305(0x08, mapRead);
	ulIOH = mapRead.begin()->second;
	m_Operation.Read305(0x09, mapRead);
	ulIOL = mapRead.begin()->second;

	dIOH = (ulIOH * 10.0 / 16383.0 - 2.5) * 0.012 / 5;
	dIOL = (ulIOL * 10.0 / 16383.0 - 2.5) * 0.012 / 5;

	return 0;
}

int CHardwareFunction::SetCalibrationRelay(USHORT usChannel, BOOL bConnect)
{
	if (nullptr == m_pRelay)
	{
		m_pRelay = new CRelay(m_Operation);
	}
	return m_pRelay->CalRelay(usChannel, bConnect);
}

int CHardwareFunction::InitPMU(const std::vector<USHORT>& vecChannel)
{
	std::map<USHORT, ULONG> mapPePmu;
	int nRetVal = 0;

	do
	{
		for (USHORT usChannel : vecChannel)
		{
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}

			mapPePmu.insert(make_pair(usChannel, 0x0001));
		}
		if (0 != nRetVal)
		{
			break;
		}

		nRetVal = m_Operation.Write305(0x0C, mapPePmu);
		if (0 != nRetVal)
		{
			nRetVal = -2;
			break;
		}
	} while (0);
	if (0 == nRetVal)
	{
		CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::NEITHER_MODE);
	}
	return nRetVal;
}

int CHardwareFunction::SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow)
{
	double dIRange[5] = { 2E-6, 20E-6, 200E-6, 2E-3, 32E-3 }, dMD(0), dGain(0), dOffset(0);
	ULONG ulVCH(0), ulVCL(0), ulDACCode(0);
	BYTE byRangeCode = 0;
	std::map<USHORT, ULONG> mapWrite[16];
	USHORT usMaxWrite(0), nChannelIndex(0);
	BYTE byMaxStepCount = 0;
	BYTE byRegAddr[16] = { 0 };
	int nRetVal = 0;
	unsigned char ucIRange = 0;
	unsigned char ucMeasureMode = 0;
	unsigned char ucForceMode = 0;

	unsigned char ucChannelIRange = 0;
	do
	{
		if (PMUMode > PMU_MODE::FVMV)
		{
			PMUMode = PMU_MODE::FVMV;
		}

		if (Range > PMU_IRANGE::IRANGE_32MA)
		{
			Range = PMU_IRANGE::IRANGE_32MA;
		}

		if (CLx_LEVEL_MIN - EQUAL_ERROR > dClmapHigh)
		{
			dClmapHigh = CLx_LEVEL_MIN;
		}
		else if(CLx_LEVEL_MAX + EQUAL_ERROR < dClmapHigh)
		{
			dClmapHigh = CLx_LEVEL_MAX;
		}
		if (CLx_LEVEL_MIN - EQUAL_ERROR > dClampLow)
		{
			dClampLow = CLx_LEVEL_MIN;
		}
		else if (CLx_LEVEL_MAX + EQUAL_ERROR < dClampLow)
		{
			dClampLow = CLx_LEVEL_MAX;
		}

		if ((PMU_MODE::FVMV == PMUMode) || (PMU_MODE::FVMI == PMUMode))
		{
			if (PIN_LEVEL_MIN > dSetValue)
			{
				dSetValue = PIN_LEVEL_MIN;
			}
			else if (PIN_LEVEL_MAX < dSetValue)
			{
				dSetValue = PIN_LEVEL_MAX;
			}
		}
		else
		{
			if (-dIRange[(int)Range] - EQUAL_ERROR > dSetValue)
			{
				dSetValue = -dIRange[(int)Range];
			}
			else if(dIRange[(int)Range] + EQUAL_ERROR < dSetValue)
			{
				dSetValue = dIRange[(int)Range];
			}
		}
		CAL_DATA CalData;
		ulVCH = (ULONG)((dClmapHigh + 2.5) / 10 * 16383);
		ulVCL = (ULONG)((dClampLow + 2.5) / 10 * 16383);

		for (USHORT usChannel : vecChannel)
		{
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}
			CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, CalData);
			unsigned char ucCurForceMode = 0;///<0 is FV and 1 is FI
			CPMU::Instance()->GetForceMode(m_bySlotNo, m_byControllerIndex, usChannel, ucCurForceMode, ucChannelIRange);
			switch (Range)
			{
			case PMU_IRANGE::IRANGE_2UA:
				byRangeCode = 0;
				dMD = 1.25e6;
				dGain = CalData.m_fFIGain[0];
				dOffset = CalData.m_fFIOffset[0];
				break;
			case PMU_IRANGE::IRANGE_20UA:
				byRangeCode = 4;
				dMD = 1.25e5;
				dGain = CalData.m_fFIGain[1];
				dOffset = CalData.m_fFIOffset[1];
				break;
			case PMU_IRANGE::IRANGE_200UA:
				byRangeCode = 5;
				dMD = 1.25e4;
				dGain = CalData.m_fFIGain[2];
				dOffset = CalData.m_fFIOffset[2];
				break;
			case PMU_IRANGE::IRANGE_2MA:
				byRangeCode = 6;
				dMD = 1.25e3;
				dGain = CalData.m_fFIGain[3];
				dOffset = CalData.m_fFIOffset[3];
				break;
			case PMU_IRANGE::IRANGE_32MA:
				byRangeCode = 7;
				dMD = 100;
				dGain = CalData.m_fFIGain[4];
				dOffset = CalData.m_fFIOffset[4];
				break;
			default:
				break;
			}

			if ((PMU_MODE::FIMV == PMUMode) || (PMU_MODE::FIMI == PMUMode))
			{
				ulDACCode = (ULONG)(((dSetValue * dGain + dOffset) * dMD + 5) / 10 * 65535);
			}
			else
			{
				ulDACCode = (ULONG)(((dSetValue * CalData.m_fFVGain[0] + CalData.m_fFVOffset[0]) + 2.5) / 10 * 65535);
			}

			byRegAddr[0] = 0x06;
			mapWrite[0].insert(std::pair<USHORT, ULONG>(usChannel, ulVCH));

			byRegAddr[1] = 0x07;
			mapWrite[1].insert(std::pair<USHORT, ULONG>(usChannel, ulVCL));
			if (PMU_MODE::FVMI == PMUMode)
			{
				if (1 == ucCurForceMode)
				{
					///<The latest mode is FI, change to FV first
					byRegAddr[2] = 0x0E;
					mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x00B0 + byRangeCode));
				}

				byRegAddr[3] = 0x0B;
				mapWrite[3].insert(std::pair<USHORT, ULONG>(usChannel, ulDACCode));

				byRegAddr[4] = 0x0E;
				mapWrite[4].insert(std::pair<USHORT, ULONG>(usChannel, 0x02B0 + byRangeCode));

				usMaxWrite = 5;
			}
			else if (PMU_MODE::FIMV == PMUMode)
			{
				BOOL bChangeRange = FALSE;
				if (0 == ucCurForceMode)
				{
					byRegAddr[2] = 0x0E;
					mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x01A8));
				}
				else if(ucChannelIRange != (unsigned char)Range)
				{
					bChangeRange = TRUE;
					if (ucChannelIRange > (unsigned char)Range)
					{
						byRegAddr[2] = 0x0E;
						mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x02A8 + byRangeCode));
					}
					else
					{
						ULONG ulPMUCode = (ULONG)((m_PMUStatus[usChannel].m_ulDACCode - 32767) * (dMD / m_PMUStatus[usChannel].m_ulIRangeMD) + 32767);
						byRegAddr[2] = 0x0B;
						mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, ulPMUCode));
						byRegAddr[3] = 0x0E;
						mapWrite[3].insert(std::pair<USHORT, ULONG>(usChannel, 0x02A8 + byRangeCode));
					}
				}
				byRegAddr[4] = 0x0B;
				mapWrite[4].insert(std::pair<USHORT, ULONG>(usChannel, ulDACCode));
				if (!bChangeRange)
				{
					byRegAddr[5] = 0x0E;
					mapWrite[5].insert(std::pair<USHORT, ULONG>(usChannel, 0x02A8 + byRangeCode));
				}
				usMaxWrite = 6;
			}
			else if (PMU_MODE::FIMI == PMUMode)
			{
				BOOL bChangeRange = FALSE;
				if (0 == ucCurForceMode)
				{
					byRegAddr[2] = 0x0E;
					mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x01A8));
				}
				else if(ucChannelIRange != (unsigned char) Range)
				{
					bChangeRange = TRUE;
					if (ucChannelIRange > (unsigned char)Range)
					{
						byRegAddr[2] = 0x0E;
						mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x02B8 + byRangeCode));
					}
					else
					{
						ULONG ulPMUCode = (ULONG)((m_PMUStatus[usChannel].m_ulDACCode - 32767) * (dMD / m_PMUStatus[usChannel].m_ulIRangeMD) + 32767);
						byRegAddr[2] = 0x0B;
						mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, ulPMUCode));

						byRegAddr[3] = 0x0E;
						mapWrite[3].insert(std::pair<USHORT, ULONG>(usChannel, 0x02B8 + byRangeCode));
					}
				}

				byRegAddr[4] = 0x0B;
				mapWrite[4].insert(std::pair<USHORT, ULONG>(usChannel, ulDACCode));

				if (!bChangeRange)
				{
					byRegAddr[5] = 0x0E;
					mapWrite[5].insert(std::pair<USHORT, ULONG>(usChannel, 0x02B8 + byRangeCode));
				}

				usMaxWrite = 6;
			}
			else
			{
				///<FVMV
				if (1 == ucCurForceMode)
				{
					byRegAddr[2] = 0x0E;
					mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x00A0 + byRangeCode));
				}

				byRegAddr[3] = 0x0B;
				mapWrite[3].insert(std::pair<USHORT, ULONG>(usChannel, ulDACCode));

				byRegAddr[4] = 0x0E;
				mapWrite[4].insert(std::pair<USHORT, ULONG>(usChannel, 0x02A0 + byRangeCode));

				usMaxWrite = 5;
			}

			byRegAddr[usMaxWrite] = 0x0C;
			mapWrite[usMaxWrite++].insert(std::pair<USHORT, ULONG>(usChannel, 0x0005));

			// record history setting
			if ((PMU_MODE::FIMV == PMUMode) || (PMU_MODE::FIMI == PMUMode))
			{
				ucForceMode = 1;
			}
			else 
			{
				ucForceMode = 0;
			}
			if ((PMU_MODE::FIMI == PMUMode) || (PMU_MODE::FVMI == PMUMode))
			{
				ucMeasureMode = 1;
			}
			else
			{
				ucMeasureMode = 0;
			}
			ucIRange = (unsigned char)Range;
			m_PMUStatus[usChannel].m_ulDACCode = ulDACCode;
			m_PMUStatus[usChannel].m_ulIRangeMD = (ULONG)dMD;

			byMaxStepCount = byMaxStepCount > usMaxWrite ? byMaxStepCount : usMaxWrite;
		}

		for (nChannelIndex = 0; nChannelIndex < byMaxStepCount; nChannelIndex++)
		{
			if (0 == mapWrite[nChannelIndex].size())
			{
				continue;
			}
			m_Operation.Write305(byRegAddr[nChannelIndex], mapWrite[nChannelIndex]);			
		}
	} while (0);
	if (0 == nRetVal)
	{
		CPMU::Instance()->SetForceMode(m_bySlotNo, m_byControllerIndex, vecChannel, ucForceMode, ucIRange);
		CPMU::Instance()->SetMeasureMode(m_bySlotNo, m_byControllerIndex, vecChannel, ucMeasureMode);
		CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::PMU_MODE);
	}
	return nRetVal;
}

void CHardwareFunction::GetClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel)
{
	mapClampChannel.clear();
	BOOL bClamp = FALSE;
	map<USHORT, ULONG> mapATEData;
	for (USHORT usChannel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			continue;
		}
		mapATEData.insert(make_pair(usChannel, 0));
	}
	set<BYTE> setBindSlot;
	set<BYTE> setBindController;
	BYTE byTargetSlot = 0;
	BOOL bBind = CBindInfo::Instance()->IsBind();
	if (bBind)
	{
		byTargetSlot = CBindInfo::Instance()->GetBindInfo(setBindSlot, setBindController);
		CBindInfo::Instance()->ClearBind();
	}

	m_Operation.Read305(0x13, mapATEData);
	for (auto& ChannelData : mapATEData)
	{
		if (0 != (ChannelData.second >> 2 & 0x01))
		{
			map<USHORT, ULONG> mapFlagEnable;
			mapFlagEnable.insert(make_pair(ChannelData.first, 0));
			m_Operation.Read305(0x12, mapFlagEnable);
			auto iterEnable = mapFlagEnable.find(ChannelData.first);
			if (mapFlagEnable.end() == iterEnable || 0 != (iterEnable->second >> 1 & 0x01))
			{
				///<Clamp
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_CLAMP);
				}
				unsigned char ucForceMode = 0;
				unsigned char ucIRange = 0;
				CPMU::Instance()->GetForceMode(m_bySlotNo, m_byControllerIndex, ChannelData.first, ucForceMode, ucIRange);
				if (0 == ucForceMode)
				{
					///<FV, current clamp
					mapClampChannel.insert(make_pair(ChannelData.first, 1));
				}
				else
				{
					///<FI, voltage clamp
					mapClampChannel.insert(make_pair(ChannelData.first, 0));
				}
				bClamp = TRUE;
			}
		}
	}
	if (bBind)
	{
		CBindInfo::Instance()->Bind(setBindSlot, setBindController, byTargetSlot);
	}
}

int CHardwareFunction::EnablePMUClampFlag(const std::vector<USHORT>& vecChannel, BOOL bEnable)
{
	ULONG ulREG = 0x01;
	if (bEnable)
	{
		ulREG = 0x03;
	}
	map<USHORT, ULONG> mapATE;
	for (auto Channel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= Channel)
		{
			return -1;
		}
		mapATE.insert(make_pair(Channel, ulREG));
	}
	m_Operation.Write305(0x12, mapATE);
	return 0;
}

double CHardwareFunction::GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange)
{
	std::map<USHORT, ULONG> mapRead;
	double dMD(0), dGain(0), dOffset(0);
	ULONG ulREGValue = 0;
	double dSetValue = 0;

	CAL_DATA CalData;
	do
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			dSetValue = MAX_MEASURE_VALUE;
			break;
		}

		mapRead.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));

		m_Operation.Read305(0x0E, mapRead);

		ulREGValue = mapRead.begin()->second;

		if (0x00 == ((ulREGValue >> 3) & 0x03))
		{
			PMUMode = PMU_MODE::FVMV;
		}
		else if (0x01 == ((ulREGValue >> 3) & 0x03))
		{
			PMUMode = PMU_MODE::FIMV;
		}
		else if (0x02 == ((ulREGValue >> 3) & 0x03))
		{
			PMUMode = PMU_MODE::FVMI;
		}
		else
		{
			PMUMode = PMU_MODE::FIMI;
		}

		CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, CalData);
		if (0x07 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_32MA;
			dMD = 100;
			dGain = CalData.m_fFIGain[4];
			dOffset = CalData.m_fFIOffset[4];
		}
		else if (0x06 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_2MA;
			dMD = 1.25e3;
			dGain = CalData.m_fFIGain[3];
			dOffset = CalData.m_fFIOffset[3];
		}
		else if (0x05 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_200UA;
			dMD = 1.25e4;
			dGain = CalData.m_fFIGain[2];
			dOffset = CalData.m_fFIOffset[2];
		}
		else if (0x04 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_20UA;
			dMD = 1.25 * 1e5;
			dGain = CalData.m_fFIGain[1];
			dOffset = CalData.m_fFIOffset[1];
		}
		else
		{
			PMURange = PMU_IRANGE::IRANGE_2UA;
			dMD = 1.25e6;
			dGain = CalData.m_fFIGain[0];
			dOffset = CalData.m_fFIOffset[0];
		}

		m_Operation.Read305(0x0B, mapRead);

		ulREGValue = mapRead.begin()->second;

		if ((PMUMode == PMU_MODE::FVMV) || (PMUMode == PMU_MODE::FVMI))
		{
			dSetValue = (double)ulREGValue * 10 / 65535 - 2.5;
			dSetValue -= CalData.m_fFVOffset[0];
			dSetValue /= CalData.m_fFVGain[0];
		}
		else
		{
			dSetValue = (double)ulREGValue * 10 / 65535 - 5.0;
			dSetValue /= dMD;
			dSetValue -= dOffset;
			dSetValue /= dGain;
		}
	} while (0);

	return dSetValue;
}

int CHardwareFunction::SetHighMode(const std::vector<USHORT>& vecChannel, BOOL bEnable, double dVoltage)
{
	const BYTE byRegisterCount = 3;
	std::map<USHORT, ULONG> mapWrite[byRegisterCount];
	BYTE byRegAddr[byRegisterCount] = { 0 };
	ULONG ulREGValue = 0;
	int nRetVal = 0;
	set<USHORT> setChannel;
	setChannel.insert(0);
	setChannel.insert(2);
	setChannel.insert(16);
	setChannel.insert(18);
	setChannel.insert(32);
	setChannel.insert(34);
	setChannel.insert(48);
	setChannel.insert(50);

	do
	{
		if (PMU_HIGH_MODE_LEVEL_MIN > dVoltage || PMU_HIGH_MODE_LEVEL_MAX < dVoltage)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmMsg("The voltage(%.1f) is over range[%d,%d].", dVoltage, PMU_HIGH_MODE_LEVEL_MIN, PMU_HIGH_MODE_LEVEL_MAX);
			}
			nRetVal = -2;
			break;
		}

		for (USHORT usChannel : vecChannel)
		{
			if (setChannel.end() == setChannel.find(usChannel))
			{
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is not support PMU high mode.", m_bySlotNo, usChannel);
				}
				nRetVal = -1;
				break;
			}

			byRegAddr[0] = 0x0C;
			mapWrite[0].insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
			dVoltage = dVoltage / 2 - 1;
			ulREGValue = (ULONG)((dVoltage + 2.5) / 10 * 16383);
			byRegAddr[1] = 0x03;
			mapWrite[1].insert(std::pair<USHORT, ULONG>(usChannel, ulREGValue));

			byRegAddr[2] = 0x0D;
			mapWrite[2].insert(std::pair<USHORT, ULONG>(usChannel, 0x0004));

		}
		if (0 != nRetVal)
		{
			break;
		}
		USHORT usChannelOffset = DCM_CHANNELS_PER_CONTROL * m_byControllerIndex;
		for (BYTE byREGIndex = 0; byREGIndex < byRegisterCount; ++byREGIndex)
		{
			m_Operation.Write305(byRegAddr[byREGIndex], mapWrite[byREGIndex]);
		}
		CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
	} while (0);

	return nRetVal;
}

int CHardwareFunction::InitTMU()
{
	ULONG ulREGValue = 0;
	int nWaitTimes = 0;
	for (int nUnitIndex = 0; nUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER;++nUnitIndex)
	{
		CTMU::Instance()->SetChannel(m_bySlotNo, m_byControllerIndex, nUnitIndex, 0);
		CTMU::Instance()->SetMode(m_bySlotNo, m_byControllerIndex, nUnitIndex, (BYTE)TMU_MEAS_MODE::DUTY_PERIOD);
	}

	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0C, 0x0000000C);
	for (nWaitTimes = 0; nWaitTimes < 2000; nWaitTimes++)
	{
		ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C01);
		if (0 == (ulREGValue & 0x02))
		{
			continue;
		}
		ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C11);
		if (0 == (ulREGValue & 0x02))
		{
			continue;
		}
		break;
	}
	if (nWaitTimes >= 2000)
	{
		return -1;
	}

	return 0;
}

int CHardwareFunction::PMUMeasure(const std::vector<USHORT>& vecChannel, int nSampleTimes, double dSamplePeriod)
{
	double dPeriod[7] = { 5.0, 10.0, 20.0, 40.0, 80.0, 160.0, 320.0 };
	for (auto& mapATE : m_mapPMUMeasure)
	{
		mapATE.clear();
	}
	memset(m_byPMUMeasureChip, 0, sizeof(m_byPMUMeasureChip));
	ULONG ulSampSet(0);
	int nRetVal = 0;

	do
	{
		if (dSamplePeriod < (dPeriod[0] + dPeriod[1]) / 2)
		{
			ulSampSet = 0x00;
		}
		else if (dSamplePeriod < (dPeriod[1] + dPeriod[2]) / 2)
		{
			ulSampSet = 0x01;
		}
		else if (dSamplePeriod < (dPeriod[2] + dPeriod[3]) / 2)
		{
			ulSampSet = 0x02;
		}
		else if (dSamplePeriod < (dPeriod[3] + dPeriod[4]) / 2)
		{
			ulSampSet = 0x03;
		}
		else if (dSamplePeriod < (dPeriod[4] + dPeriod[5]) / 2)
		{
			ulSampSet = 0x04;
		}
		else if (dSamplePeriod < (dPeriod[5] + dPeriod[6]) / 2)
		{
			ulSampSet = 0x05;
		}
		else
		{
			ulSampSet = 0x06;
		}
		m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x08FE, ulSampSet);

		for (auto usChannel : vecChannel)
		{
			USHORT usInvalidChannel = 0;
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}

			if (0 == usChannel % 2)
			{
				usInvalidChannel = usChannel + 1;
				m_byPMUMeasureChip[0] |= (1 << (usChannel/2));
				m_mapPMUMeasure[0].insert(make_pair(usChannel, 0x0001)); 
			}
			else 
			{
				usInvalidChannel = usChannel - 1;
				m_byPMUMeasureChip[1] |= (1 << (usChannel / 2));
				m_mapPMUMeasure[1].insert(make_pair(usChannel, 0x0003));
			}
		}

		if ((0 >= nSampleTimes) || (PMU_SAMPLE_DEPTH < nSampleTimes))
		{
			if (0 >= nSampleTimes)
			{
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmMsg("The sample times(%d) is over range[%d,%d] and will be set to %d.", nSampleTimes, 1, PMU_SAMPLE_DEPTH, 1);
				}
				nSampleTimes = 1;
			}
			else
			{
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmMsg("The sample times(%d) is over range[%d,%d] and will be set to %d.", nSampleTimes, 1, PMU_SAMPLE_DEPTH, PMU_SAMPLE_DEPTH);
				}
				nSampleTimes = PMU_SAMPLE_DEPTH;
			}

			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_SAMPLE_TIMES_ERROR);
				m_pAlarm->SetParamName("SampleTimes");
				m_pAlarm->Output(FALSE);
			}
		}

		int nMeasTimes = 1;
		memset(m_byPMUMeasureChipEven, 0, sizeof(m_byPMUMeasureChipEven));
		
		if (m_byPMUMeasureChip[0] & m_byPMUMeasureChip[1])
		{
			///<One chip measure two channels
			nMeasTimes = 2;
			m_byPMUMeasureChipEven[0] = 0xFF;
			m_byPMUMeasureChipEven[1] = 0;
			m_byPMUStatus[0] = 1;
			m_byPMUStatus[1] = 1;
		}
		else
		{
			///<One chip measure one channel
			m_byPMUMeasureChipEven[0] = m_byPMUMeasureChip[0] | (~m_byPMUMeasureChip[1]);
			m_byPMUMeasureChip[0] |= m_byPMUMeasureChip[1];

			for (auto& Channel: m_mapPMUMeasure[1])
			{
				m_mapPMUMeasure[0].insert(make_pair(Channel.first, Channel.second));

			}
			m_mapPMUMeasure[1].clear();

			m_byPMUStatus[0] = 1;
			m_byPMUStatus[1] = 0;
		}
		m_uSampleTimes = nSampleTimes;
		StartPMU();
		if (CBindInfo::Instance()->IsBind())
		{
			///<If the controller is binded, must wait before next measurement
			do 
			{
				nRetVal = WaitPMUFinish();
				if (0 != nRetVal)
				{
					nRetVal = -2;
					break;
				}
				nRetVal = StartPMU();
				if (0 != nRetVal)
				{
					///<No PMU needed to be started
					nRetVal = 0;
					break;
				}
			} while (0 == nRetVal);
		}
		else
		{
			///<Wait stop
			nRetVal = -3;
		}
	} while (0);
	if (0 == nRetVal || -3 == nRetVal)
	{
		CPMU::Instance()->SetSampleSetting(m_bySlotNo, m_byControllerIndex, vecChannel, nSampleTimes, dPeriod[ulSampSet]);
	}

	return nRetVal;
}

double CHardwareFunction::GetPMUMeasureResult(USHORT usChannel, int nSampleTimes)
{
	double dRom(0), dGain(0), dOffset(0);
	ULONG ulPMUBRAM(0), ulBRAMAddr(0);
	SHORT sADCValue(0);
	double dMeas = 0;

	do
	{
		if (AVERAGE_RESULT != nSampleTimes)
		{
			if (DCM_CHANNELS_PER_CONTROL <= usChannel)
			{
				dMeas = MAX_MEASURE_VALUE;
				break;
			}

			UINT uSampleTimes = 0;
			double dSamplePeriod = 0;
			CPMU::Instance()->GetSampleSetting(m_bySlotNo, m_byControllerIndex, usChannel, uSampleTimes, dSamplePeriod);
			if (uSampleTimes <= nSampleTimes)
			{
				dMeas = MAX_MEASURE_VALUE;
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_SAMPLE_TIMES_ERROR);
					m_pAlarm->SetAlarmMsg("The sample times(%d) is over range[0, %d).", nSampleTimes, uSampleTimes);
				}
				break;
			}
			ulBRAMAddr = ((1 - (usChannel % 2)) * DCM_CHANNELS_PER_CONTROL / 2 + usChannel / 2) * PMU_SAMPLE_DEPTH / 2;
			ulBRAMAddr += nSampleTimes / 2;
			m_Operation.ReadRAM(COperation::BRAM_TYPE::PMU_BUF, (USHORT)ulBRAMAddr, 1, &ulPMUBRAM);

			if (0 == (nSampleTimes % 2))
			{
				sADCValue = ulPMUBRAM & 0xFFFF;
			}
			else
			{
				sADCValue = ulPMUBRAM >> 16;
			}
		}
		else
		{
			sADCValue = (SHORT)CPMU::Instance()->GetAverageData(m_bySlotNo, m_byControllerIndex, usChannel);
		}
		CAL_DATA CalData;
		CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, CalData);

		unsigned char ucIRange = 0;
		unsigned char ucMeasureMode = 0;
		unsigned char ucForce = 0;
		CPMU::Instance()->GetMeasureMode(m_bySlotNo, m_byControllerIndex, usChannel, ucMeasureMode);
		CPMU::Instance()->GetForceMode(m_bySlotNo, m_byControllerIndex, usChannel, ucForce, ucIRange);
		if (1 == ucMeasureMode)
		{
			switch ((PMU_IRANGE)ucIRange)
			{
			case PMU_IRANGE::IRANGE_2UA:
				dRom = 250000;
				dGain = CalData.m_fMIGain[0];
				dOffset = CalData.m_fMIOffset[0];
				break;
			case PMU_IRANGE::IRANGE_20UA:
				dRom = 25000;
				dGain = CalData.m_fMIGain[1];
				dOffset = CalData.m_fMIOffset[1];
				break;
			case PMU_IRANGE::IRANGE_200UA:
				dRom = 2500;
				dGain = CalData.m_fMIGain[2];
				dOffset = CalData.m_fMIOffset[2];
				break;
			case PMU_IRANGE::IRANGE_2MA:
				dRom = 250;
				dGain = CalData.m_fMIGain[3];
				dOffset = CalData.m_fMIOffset[3];
				break;
			case PMU_IRANGE::IRANGE_32MA:
				dRom = 15.5;
				dGain = CalData.m_fMIGain[4];
				dOffset = CalData.m_fMIOffset[4];
			default:
				break;
			}

			dMeas = double(sADCValue * 10.0) / 32768.0;
			dMeas = (dMeas - 2.5) / (5 * dRom);
			dMeas = dGain * dMeas + dOffset;
		}
		else
		{
			dGain = CalData.m_fMVGain[0];
			dOffset = CalData.m_fMVOffset[0];
			dMeas = double(sADCValue * 10.0) / 32768.0;
			dMeas = dGain * dMeas + dOffset;
		}
	} while (0);

	return dMeas;
}

int CHardwareFunction::SetTMUUnitChannel(USHORT usChannel, BYTE byUnitIndex)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnitIndex)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_INDEX_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The TMU unit index(%d) is over range[%d.%d].", byUnitIndex, 0, TMU_UNIT_COUNT_PER_CONTROLLER - 1);
		}
		return -2;
	}
	CTMU::Instance()->SetChannel(m_bySlotNo, m_byControllerIndex, byUnitIndex, usChannel);
	ULONG ulSignalEdge = 0;
	ulSignalEdge = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E);
	int nShiftBits = byUnitIndex * 5;
	ulSignalEdge &= ~(0x0F << nShiftBits);
	ulSignalEdge |= usChannel << nShiftBits;

	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E, ulSignalEdge);
	return 0;
}

int CHardwareFunction::GetTMUConnectUnit(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	ULONG ulSignalEdge = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E);

	int nShiftBits = 0;
	for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER;++byUnitIndex)
	{
		if (usChannel == (ulSignalEdge >> nShiftBits & 0x0F))
		{
			return byUnitIndex;
		}
		nShiftBits += 5;
	}
	return -2;
}

int CHardwareFunction::SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, BYTE bySpecifiedUnit)
{
	int nChannelCount = vecChannel.size();
	if (TMU_UNIT_COUNT_PER_CONTROLLER < nChannelCount)
	{
		return -2;
	}
	BOOL bSpecifiedUnit = (BYTE)-1 != bySpecifiedUnit ? TRUE : FALSE;
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= bySpecifiedUnit && (BYTE)-1 != bySpecifiedUnit)
	{
		return -3;
	}
	if (bSpecifiedUnit && 1 != nChannelCount)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_CONNECT_CHANNEL_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("Only one channel should be specified.");
		}
		return -2;
	}
	if (508 < uHoldOffTime)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_HOLDOFF_TMIE_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The hold off time(%d) is over range[%d,%d] and will be set to %d.", uHoldOffTime, 0, 508, 508);
			m_pAlarm->Output(FALSE);
		}
		uHoldOffTime = 508;
	}

	BOOL bCloseHoldoffTime = FALSE;
	if (0 == uHoldOffTime)
	{
		bCloseHoldoffTime = TRUE;
		uHoldOffTime = 0;
	}
	else if (12 < uHoldOffTime)
	{
		uHoldOffTime = (uHoldOffTime - 12) / 8;
	}
	else
	{
		uHoldOffTime = 0;
	}
	USHORT usHoldOffTime = uHoldOffTime * 8 + 12;
	if (bCloseHoldoffTime)
	{
		usHoldOffTime = 0;
	}
	if (2046 < uHoldOffNum)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_HOLDOFF_NUM_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The hold off number(%d) is over range[%d,%d], and will be set to %d.", uHoldOffNum, 0, 2046, 2046);
			m_pAlarm->Output(FALSE);
		}
		uHoldOffNum = 2046;
	}
	BOOL bFindUnit = FALSE;
	ULONG ulSignalEdge = 0;

	bySpecifiedUnit = bSpecifiedUnit ? bySpecifiedUnit : 0;
	ulSignalEdge = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E);
	for (auto usChannel : vecChannel)
	{
		bFindUnit = FALSE;
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			return -1;
		}

		for (BYTE byTMUUnit = bySpecifiedUnit; byTMUUnit < TMU_UNIT_COUNT_PER_CONTROLLER; byTMUUnit++)
		{
			int nChannel = CTMU::Instance()->GetChannel(m_bySlotNo, m_byControllerIndex, byTMUUnit);
			if (usChannel != nChannel)
			{
				if (bSpecifiedUnit)
				{
					break;
				}
				continue;
			}
			bFindUnit = TRUE;
			
			USHORT usREGOffset = byTMUUnit * 0x10;
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C02 + usREGOffset, uHoldOffNum);

			if (bRaiseTriggerEdge)
			{
				ulSignalEdge &= ~(1 << (byTMUUnit + 10));///<Select raise edge
			}
			else
			{
				ulSignalEdge |= (1 << (byTMUUnit + 10));///<Select fall edge
			}

			CTMU::Instance()->SetTriggerEdge(m_bySlotNo, m_byControllerIndex, byTMUUnit, bRaiseTriggerEdge);
			CTMU::Instance()->SetHoldOff(m_bySlotNo, m_byControllerIndex, byTMUUnit, usHoldOffTime, uHoldOffNum);

			ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C00 + usREGOffset);
			ulREGValue &= 0x1F;
			if (bCloseHoldoffTime)
			{
				ulREGValue |= (1 << 11) + 0x02;
			}
			else
			{
				ulREGValue |= (uHoldOffTime << 5) + 0x02;///<The test time is fixed to 1
			}
			ulREGValue |= 1 << 12;
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C00 + usREGOffset, ulREGValue);
			if (bSpecifiedUnit)
			{
				break;
			}
		}
		if (!bFindUnit)
		{
			return -4;
		}
	}
	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E, ulSignalEdge);
	
	return 0;
}

int CHardwareFunction::GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		///<The channel number is over range
		return -1;
	}
	int nUnitIndex = GetTMUConnectUnit(usChannel);
	if (0 > nUnitIndex)
	{
		///<The channel not connect to any TMU unit
		return -2;
	}
	GetTMUUnitParameter(nUnitIndex, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);

	return 0;
}

int CHardwareFunction::GetTMUUnitParameter(BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byTMUUnitIndex)
	{
		return -1;
	}
	USHORT usREGOffset = byTMUUnitIndex * 0x10;
	///<Get the hold off number
	usHoldOffNum = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C02 + usREGOffset);
	///<Get the holf off time
	usHoldOffTime = 0;
	ULONG ulREGValue = 0;
	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C00 + usREGOffset);
	if (0 == (ulREGValue >> 11 & 0x01))
	{
		usHoldOffTime = (ulREGValue >> 5 & 0x3F) * 8 + 12;
	}

	///<Get raise edge
	ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E);

	bRaiseTriggerEdge = 0 == (ulREGValue >> (byTMUUnitIndex + 10) & 0x01);
	return 0;
}

int CHardwareFunction::EnableTMUSelfCheck(BYTE byUnitIndex, BOOL bEnable)
{
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnitIndex)
	{
		return -1;
	}
	USHORT usREGOffset = byUnitIndex * 0x10;

	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C00 + usREGOffset);
	if (bEnable)
	{
		ulREGValue |= 1 << 13;
	}
	else
	{
		ulREGValue &= ~(1 << 13);
	}
	ulREGValue |= 1 << 12;
	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C00 + usREGOffset, ulREGValue);

	return 0;
}

int CHardwareFunction::TMUMeasure(const std::vector<USHORT>& vecChannel, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout)
{
	int nChannelCount = vecChannel.size();
	if (TMU_UNIT_COUNT_PER_CONTROLLER < nChannelCount)
	{
		return -1;
	}
	if (2046 < uSampleNum)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_SAMPLE_NUM_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The sample number(%d) is over range[%d,%d], will be set to %d.", uSampleNum, 1, 2046, 2046);
			m_pAlarm->Output(FALSE);
		}
		uSampleNum = 2046;
	}
	else if (0 == uSampleNum)
	{
		uSampleNum = 1;
	}
	double dTimeoutNs = dTimeout * 1e6;
	if (TMU_MAX_TIMEOUT < dTimeoutNs)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_TIMEOUT_OVER_RANGE);
			m_pAlarm->SetAlarmMsg("The time out number(%d) is over range[%.0f,%.0f], and will be set to %.0f.", dTimeout, 1, TMU_MAX_TIMEOUT * 1e-6, TMU_MAX_TIMEOUT * 1e-6);
			m_pAlarm->Output(FALSE);
		}
		dTimeoutNs = TMU_MAX_TIMEOUT;
	}
	else if (0 == uSampleNum)
	{
		uSampleNum = 1;
	}
	ULONG ulTimeout = dTimeoutNs / 4;

	float fTimeout = ulTimeout * 4 * 1e-6;

	int nUnit[TMU_UNIT_COUNT_PER_CONTROLLER] = { -1,-1 };
	ULONG ulStart = 0;
	ULONG ulStop = 0;
	
	ULONG ulSignalSel = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E);
	int nIndex = 0;
	for (USHORT usChannel : vecChannel)
	{
		if (DCM_CHANNELS_PER_CONTROL <= usChannel)
		{
			return -2;
		}
		BOOL bFindUnit = FALSE;
		int nChannel = 0;
		BYTE byTMUUnit = 0;
		for (byTMUUnit = 0; byTMUUnit < TMU_UNIT_COUNT_PER_CONTROLLER; byTMUUnit++)
		{
			nChannel = CTMU::Instance()->GetChannel(m_bySlotNo, m_byControllerIndex, byTMUUnit);
			if (usChannel != nChannel)
			{
				continue;
			}
			ulStart |= 1 << byTMUUnit;
			ulStop |= 1 << (byTMUUnit + 2);
			bFindUnit = TRUE;
			nUnit[nIndex++] = byTMUUnit;
			switch (MeasMode)
			{
			case TMU_MEAS_MODE::DUTY_PERIOD:
				break;
			case TMU_MEAS_MODE::EDGE_TIME:
				break;
			case TMU_MEAS_MODE::SIGNAL_DELAY:
				break;
				return -3;
			default:
				break;
			}
		
			BYTE byBindTMU = 0;
			switch (MeasMode)
			{
			case TMU_MEAS_MODE::DUTY_PERIOD:
				ulSignalSel &= ~(0x10 << byTMUUnit * 5); // select H signal
				m_byBindTMU[byTMUUnit] = byTMUUnit;
				break;
			case TMU_MEAS_MODE::EDGE_TIME:
				byBindTMU = TMU_UNIT_COUNT_PER_CONTROLLER - byTMUUnit - 1;
				m_byBindTMU[byTMUUnit] = byBindTMU;
				ulSignalSel &= ~(0x10); // select H signal
				ulSignalSel |= (0x10 << 5); // select L signal
				uSampleNum = 0;
				break;
			case TMU_MEAS_MODE::SIGNAL_DELAY:
				byBindTMU = TMU_UNIT_COUNT_PER_CONTROLLER - byTMUUnit - 1;
				m_byBindTMU[byTMUUnit] = byBindTMU;
				ulSignalSel &= ~(0x10); // select H signal
				ulSignalSel &= ~(0x10 << 5); // select H signal
				uSampleNum = 0;
				break;
			default:
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEAS_MODE_ERROR);
					m_pAlarm->SetAlarmMsg("The measurement mode(%d) is not supported.", (BYTE)MeasMode);
				}
				return -6;
				break;
			}
			BYTE byUnitOffSet = byTMUUnit * 0x10;
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C03 + byUnitOffSet, uSampleNum);
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C05 + byUnitOffSet, ulTimeout);
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C06 + byUnitOffSet, ulTimeout);
			m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C07 + byUnitOffSet, 0x04);
			CTMU::Instance()->SetMode(m_bySlotNo, m_byControllerIndex, byTMUUnit, (BYTE)MeasMode);
			CTMU::Instance()->SetSampleNumber(m_bySlotNo, m_byControllerIndex, byTMUUnit, uSampleNum);
			CTMU::Instance()->SetTimeout(m_bySlotNo, m_byControllerIndex, byTMUUnit, fTimeout);
		}
		if (!bFindUnit)
		{
			return -1;
		}
	}
	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0E, ulSignalSel);
	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0C, ulStop);///<Stop the TMU firstly
	m_Operation.WaitUs(1);
	m_Operation.WriteRegister(COperation::REG_TYPE::TMU_REG, 0x0C0C, ulStart);
	return 0;
}

int CHardwareFunction::GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	int nUnitIndex = GetTMUConnectUnit(usChannel);
	if (0 > nUnitIndex)
	{
		///<Not connect to unit
		return -2;
	}
	BYTE byUnitOffSet = nUnitIndex * 0x10;

	uSampleNum = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C03 + byUnitOffSet);
	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C05 + byUnitOffSet);
	dTimeout = ulREGValue / 4. * 1e6;
	///<The measurement mode can't be gotten from board
	CTMU::Instance()->GetMode(m_bySlotNo, usChannel / DCM_CHANNELS_PER_CONTROL, nUnitIndex);
	return 0;
}

double CHardwareFunction::GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nErrorCode)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		nErrorCode = -1;
		return TMU_ERROR;
	}
	BYTE byUnitIndex = 0;
	for (byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
	{
		USHORT usCurChannel = CTMU::Instance()->GetChannel(m_bySlotNo, m_byControllerIndex, byUnitIndex);
		if (usCurChannel == usChannel)
		{
			break;
		}
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnitIndex)
	{
		nErrorCode = -2;
		return TMU_ERROR;
	}

	double dMeasResult = GetTMUUnitMeasureResult(byUnitIndex, MeasType, nErrorCode);
	if (0 != nErrorCode)
	{
		nErrorCode -= 2;
	}
	return dMeasResult;
}

double CHardwareFunction::GetTMUUnitMeasureResult(BYTE byUnitIndex, TMU_MEAS_TYPE MeasType, int& nError)
{
	BYTE byMeasMode = CTMU::Instance()->GetMode(m_bySlotNo, m_byControllerIndex, byUnitIndex);

	switch (MeasType)
	{
	case TMU_MEAS_TYPE::FREQ:
	case TMU_MEAS_TYPE::HIGH_DUTY:
	case TMU_MEAS_TYPE::LOW_DUTY:
		if ((BYTE)TMU_MEAS_MODE::DUTY_PERIOD != byMeasMode)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_MEASURE_ERROR);
				m_pAlarm->SetAlarmMsg("Not measure frequency or duty before.");
			}
			nError = -1;
			return TMU_ERROR;
		}
		break;
	case TMU_MEAS_TYPE::EDGE:
		if ((BYTE)TMU_MEAS_MODE::EDGE_TIME != byMeasMode)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_MEASURE_ERROR);
				m_pAlarm->SetAlarmMsg("Not measure edge time before.");
			}
			nError = -1;
			return TMU_ERROR;
		}
		break;
	case TMU_MEAS_TYPE::DELAY:
		if ((BYTE)TMU_MEAS_MODE::SIGNAL_DELAY != byMeasMode)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_MEASURE_ERROR);
				m_pAlarm->SetAlarmMsg("Not measure delay time before.");
			}
			nError = -1;
			return TMU_ERROR;
		}
		break;
	default:
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEASURE_RESULT_ERROR);
			m_pAlarm->SetAlarmMsg("The measurement result is not supported.");
		}
		nError = -2;
		return TMU_ERROR;
		break;
	}
	USHORT usREGOffset = byUnitIndex * 0x10;

	int nRetVal = WaitTMUStop(m_byBindTMU[byUnitIndex]);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not wait stop
			nRetVal = -3;
			break;
		case -2:
			///<The TMU unit is timeout
			nRetVal = -4;
			break;
		default:
			break;
		}
		nError = nRetVal;
		return TMU_ERROR;
	}

	if ((BYTE)TMU_MEAS_MODE::DUTY_PERIOD != byMeasMode)
	{
		///<Wait the bind unit stop
		nRetVal = WaitTMUStop(m_byBindTMU[byUnitIndex]);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Not wait stop
				nRetVal = -5;
				break;
			case -2:
				///<The TMU unit is timeout
				nRetVal = -6;
				break;
			default:
				break;
			}
			nError = nRetVal;///<Not alarm when TMU is not stop or triggered
			return TMU_ERROR;
		}
	}

	int nSampleCount = CTMU::Instance()->GetSampleNumber(m_bySlotNo, m_byControllerIndex, byUnitIndex);
	if ((BYTE)TMU_MEAS_MODE::DUTY_PERIOD == byMeasMode)
	{
		ULONGLONG ulResult = 0;
		double dPeriod = 0;
		ulResult = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C1D + usREGOffset);
		ulResult <<= 32;
		ulResult += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C1C + usREGOffset);
		ulResult /= (double)nSampleCount + EQUAL_ERROR;
		dPeriod = ulResult / 64.0;///<ns
		if (0 == dPeriod)
		{
			dPeriod = EQUAL_ERROR;
		}
		if (TMU_MEAS_TYPE::FREQ == MeasType)
		{
			return 1 / dPeriod * 1e6;///<kHz
		}

		int nTriggerEdge = CTMU::Instance()->GetTriggerEdge(m_bySlotNo, m_byControllerIndex, byUnitIndex);

		ulResult = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C1F + usREGOffset);
		ulResult <<= 32;
		ulResult += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C1E + usREGOffset);
		ulResult /= (double)nSampleCount + EQUAL_ERROR;
		double dEdge = ulResult / 64.0;///<ns
		double dDuty = dEdge / dPeriod * 100;///<%
		if ((0 == nTriggerEdge && TMU_MEAS_TYPE::LOW_DUTY == MeasType)
			|| (1 == nTriggerEdge && TMU_MEAS_TYPE::HIGH_DUTY == MeasType))
		{
			return dDuty;
		}
		return 100 - dDuty;
	}
	else if ((BYTE)TMU_MEAS_MODE::EDGE_TIME == byMeasMode)
	{
		LONGLONG lTimePoint00(0);
		LONGLONG lTimePoint10(0);

		if (0 != byUnitIndex)
		{
			byUnitIndex = 0;
		}

		ULONG ulRegValue = 0;
		int nRaise = CTMU::Instance()->GetTriggerEdge(m_bySlotNo, m_byControllerIndex, byUnitIndex);

		usREGOffset = byUnitIndex * 0x10;

		lTimePoint00 = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C09 + usREGOffset);
		lTimePoint00 <<= 32;
		lTimePoint00 += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C08 + usREGOffset);


		USHORT usBindREGOffset = m_byBindTMU[byUnitIndex] * 0x10;

		lTimePoint10 = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C09 + usBindREGOffset);
		lTimePoint10 <<= 32;
		lTimePoint10 += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C08 + usBindREGOffset);


		LONGLONG llValue = 0;
		if (nRaise)
		{
			if (lTimePoint00 < lTimePoint10)
			{
				nError = -7;
// 				m_pAlarm->SetAlarmMsg("The TMU measurement error.");
// 				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEASURE_RESULT_ERROR);
				return TMU_ERROR;
			}
			llValue = lTimePoint00 - lTimePoint10;
		}
		else
		{
			if (lTimePoint00 > lTimePoint10)
			{
				nError = -7;
// 				m_pAlarm->SetAlarmMsg("The TMU measurement error.");
// 				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
// 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_MEASURE_RESULT_ERROR);
				return TMU_ERROR;
			}

			llValue = lTimePoint10 - lTimePoint00;
		}
		return llValue / 64.0 / 1000;
	}
	else
	{
		///<Delay
		LONGLONG lTimePoint0(0), lTimePoint1(0);
		usREGOffset = byUnitIndex * 0x10;
		USHORT usBindREGOffset = m_byBindTMU[byUnitIndex] * 0x10;
		lTimePoint0 = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C09 + usREGOffset);
		lTimePoint0 <<= 32;
		lTimePoint0 += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C08 + usREGOffset);

		lTimePoint1 = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C09 + usBindREGOffset);
		lTimePoint1 <<= 32;
		lTimePoint1 += m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C08 + usBindREGOffset);

		return (lTimePoint0 - lTimePoint1) / 64.0 / 1000;
	}
}

int CHardwareFunction::SetIODelay(USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow)
{
    ULONG ulDataDly(0), ulDataEnDly(0), ulHDly(0), ulLDly(0);
    ULONG ulRegValue = 0;

	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	if (0 > dData || 0 > dDataEn || 0 > dHigh || 0 > dLow)
	{
		return -2;
	}

    ulDataDly = (ULONG)(dData + EQUAL_ERROR);
    ulDataEnDly = (ULONG)(dDataEn + EQUAL_ERROR);
    ulHDly = (ULONG)(dHigh / 0.078);
    ulLDly = (ULONG)(dLow / 0.078);

    if (8 < ulDataDly || 8 < ulDataEnDly || 63 < ulHDly || 63 < ulLDly) 
	{
        return -2;
    }

    ulRegValue = (ulDataDly << 24) + (ulDataEnDly << 16) + (ulHDly << 8) + (ulLDly << 0);
    m_Operation.WriteRegister(COperation::REG_TYPE::CAL_REG, 0x1000 + usChannel, ulRegValue);

    return 0;
}

int CHardwareFunction::GetIODelay(USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow)
{
    ULONG ulDataDly(0), ulDataEnDly(0), ulHDly(0), ulLDly(0);
    ULONG ulRegValue = 0;

    if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
        return -1;
    }

    if (nullptr == pdData || nullptr == pdDataEn || nullptr == pdHigh || nullptr == pdLow)
	{
        return -2;
    }

    ulRegValue = m_Operation.ReadRegister(COperation::REG_TYPE::CAL_REG, 0x1000 + usChannel);
    ulDataDly = ulRegValue >> 24;
    ulDataEnDly = (ulRegValue >> 16) & 0xFF;
    ulHDly = (ulRegValue >> 8) & 0xFF;
    ulLDly = ulRegValue & 0xFF;

    *pdData = ulDataDly;
    *pdDataEn = ulDataEnDly;
    *pdHigh = ulHDly * 0.078;
    *pdLow = ulLDly * 0.078;

    ulRegValue = (ulDataDly << 24) + (ulDataEnDly << 16) + (ulHDly << 8) + (ulLDly << 0);

    return 0;
}

int CHardwareFunction::SetTotalStartDelay(double dDelay)
{
    ULONG ulDelay0(0), ulDelay1(0);
    ULONG ulREGValue = 0;

    if (31 * 4.0 + 63 * 0.078 + EQUAL_ERROR < dDelay)
	{
        return -1;
    }

    ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::CAL_REG, 0x1010);
    ulDelay0 = (ULONG)(dDelay);
    ulDelay0 = (ulDelay0 / 4) & 0xFF;

    dDelay = dDelay - (ulDelay0 * 4);
    ulDelay1 = ((ULONG)(dDelay / 0.078)) & 0xFF;

    ulREGValue &= 0xFF00FF00;
    ulREGValue |= ulDelay0 << 16;
    ulREGValue |= ulDelay1 << 0;

    m_Operation.WriteRegister(COperation::REG_TYPE::CAL_REG, 0x1010, ulREGValue);
	return 0;
}

double CHardwareFunction::GetTotalStartDelay()
{
    ULONG ulDelay0(0), ulDelay1(0);
    ULONG ulREGValue = 0;

    ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::CAL_REG, 0x1010);
    ulDelay0 = (ulREGValue >> 16) & 0xFF;
    ulDelay1 = (ulREGValue >> 0) & 0xFF;

    return (ulDelay0 * 4.0 + ulDelay1 * 0.078);
}

int CHardwareFunction::SetTimesetDelay(double dDelay)
{
	ULONG ulREGValue = 0;

	if (31 * 4.0 + EQUAL_ERROR < dDelay)
	{
		return -1;
	}

	ulREGValue = (ULONG)(dDelay / 4);
	ulREGValue &= 0x1F;

	m_Operation.WriteRegister(COperation::REG_TYPE::CAL_REG, 0x1012, ulREGValue);
	return 0;
}

double CHardwareFunction::GetTimesetDelay()
{
    ULONG ulREGValue = 0;

    ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::CAL_REG, 0x1012);
    ulREGValue &= 0x1F;

   return ulREGValue * 4.0;
}

void CHardwareFunction::UpdateDelay()
{
	m_Operation.WriteRegister(COperation::REG_TYPE::CAL_REG, 0x1011, 1);
}

int CHardwareFunction::ReadBRAMMemory(RAM_TYPE BRAMType, UINT uStartLine, UINT uLineCount, ULONG *pulData)
{
	UINT uMaxLineCount(DCM_BRAM_PATTERN_LINE_COUNT / 2), uStartAddr(0);
	int nRetVal = 0;
	BYTE bySubType = 0;
	COperation::BRAM_TYPE RAMType = COperation::BRAM_TYPE::FM;
	switch (BRAMType)
	{
	case CHardwareFunction::RAM_TYPE::IMM1:
	case CHardwareFunction::RAM_TYPE::IMM2:
	case CHardwareFunction::RAM_TYPE::FM:
	case CHardwareFunction::RAM_TYPE::MM:
	case CHardwareFunction::RAM_TYPE::IOM:
	case CHardwareFunction::RAM_TYPE::PMU_BUF:
	case CHardwareFunction::RAM_TYPE::MEM_PERIOD:
		RAMType = (COperation::BRAM_TYPE)BRAMType;
		break;
	case CHardwareFunction::RAM_TYPE::MEM_RSU_SVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_SVM:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR:
		RAMType = (COperation::BRAM_TYPE)((UINT)BRAMType >> 4);
		bySubType = (UINT)BRAMType & 0xF;
		break;
	default:
		return -1;
		break;
	}
	nRetVal = 0;
	do
	{
		uStartAddr = uStartLine;
		if (COperation::BRAM_TYPE::MEM_PERIOD == RAMType)
		{
			uMaxLineCount = 32;
		}
		else if (COperation::BRAM_TYPE::MEM_RSU == RAMType)
		{
			uStartAddr = (bySubType << 13) + uStartLine;
			if (2 > bySubType)
			{
				uMaxLineCount = 4096;
			}
			else
			{
				uMaxLineCount = 1024;
			}
		}
		else if (COperation::BRAM_TYPE::MEM_HIS == RAMType)
		{
			uStartAddr = (bySubType << 14) + uStartAddr;
			if (0 == bySubType)
			{
				uMaxLineCount = 4096;
			}
			else
			{
				uMaxLineCount = 1024;
			}
		}
		else if (COperation::BRAM_TYPE::MEM_TIMING == RAMType)
		{
			uMaxLineCount = 32;
			uStartAddr = (bySubType << 8) + uStartLine;
		}

		if (uStartLine >= uMaxLineCount)
		{
			nRetVal = -2;
			break;
		}

		else if (uMaxLineCount < uStartLine + uLineCount)
		{
			return -3;
		}

		if (nullptr == pulData || 0 == uLineCount)
		{
			return -4;
		}

		m_Operation.ReadRAM(RAMType, uStartAddr, uLineCount, pulData);
	} while (0);

	return nRetVal;
}

int CHardwareFunction::WriteBRAMMemory(RAM_TYPE BRAMType, UINT uStartLine, UINT uLineCount, const ULONG* pulData)
{
	UINT uMaxLineCount(DCM_BRAM_PATTERN_LINE_COUNT / 2), uStartAddr(0);
	int nRetVal = 0;
	BYTE bySubType = 0;
	COperation::BRAM_TYPE RAMType = COperation::BRAM_TYPE::FM;
	switch (BRAMType)
	{
	case CHardwareFunction::RAM_TYPE::IMM1:
	case CHardwareFunction::RAM_TYPE::IMM2:
	case CHardwareFunction::RAM_TYPE::FM:
	case CHardwareFunction::RAM_TYPE::MM:
	case CHardwareFunction::RAM_TYPE::IOM:
	case CHardwareFunction::RAM_TYPE::PMU_BUF:
	case CHardwareFunction::RAM_TYPE::MEM_PERIOD:
		RAMType = (COperation::BRAM_TYPE)BRAMType;
		break;
	case CHardwareFunction::RAM_TYPE::MEM_RSU_SVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_SVM:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR:
		RAMType = (COperation::BRAM_TYPE)((UINT)BRAMType >> 4);
		bySubType = (UINT)BRAMType & 0xF;
		break;
	default:
		return -1;
		break;
	}

	do 
	{
		uStartAddr = uStartLine;
		if (COperation::BRAM_TYPE::MEM_PERIOD == RAMType)
		{
			uMaxLineCount = 32;
		}
		else if (COperation::BRAM_TYPE::MEM_RSU == RAMType)
		{
			uStartAddr = (bySubType << 13) + uStartLine;
			if (2 > bySubType)
			{
				uMaxLineCount = 4096;
			}
			else
			{
				uMaxLineCount = 1024;
			}
		}
		else if (COperation::BRAM_TYPE::MEM_HIS == RAMType)
		{
			uStartAddr = (bySubType << 14) + uStartAddr;
			if (0 == bySubType)
			{
				uMaxLineCount = 4096;
			}
			else
			{
				uMaxLineCount = 1024;
			}
		}
		else if (COperation::BRAM_TYPE::MEM_TIMING == RAMType)
		{
			uMaxLineCount = 32;
			uStartAddr = (bySubType << 8) + uStartLine;
		}

		if (uStartLine >= uMaxLineCount)
		{
			nRetVal = -2;
			break;
		}

		else if (uMaxLineCount < uStartLine + uLineCount)
		{
			return -3;
		}

		if (nullptr == pulData || 0 == uLineCount)
		{
			return -4;
		}

		m_Operation.WriteRAM(RAMType, uStartAddr, uLineCount, pulData);
	} while (0);

	return nRetVal;
}

void CHardwareFunction::DelayUs(ULONG ulUs)
{
	LARGE_INTEGER CurTime, StopTime, Freq;
	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&CurTime);
	StopTime.QuadPart = CurTime.QuadPart + ulUs * Freq.QuadPart * 1e-6;
	while (StopTime.QuadPart > CurTime.QuadPart)
	{
		QueryPerformanceCounter(&CurTime);
	}
}

void CHardwareFunction::DelayMs(ULONG ulMs)
{
	LARGE_INTEGER CurTime, StopTime, Freq;
	QueryPerformanceFrequency(&Freq);
	QueryPerformanceCounter(&CurTime);
	StopTime.QuadPart = CurTime.QuadPart + ulMs * Freq.QuadPart * 1e-3;
	while (StopTime.QuadPart > CurTime.QuadPart)
	{
		QueryPerformanceCounter(&CurTime);
	}
}

int CHardwareFunction::SetCalibrationData(CAL_DATA* pCalibrationData, int nElementCount)
{
	if (nullptr == pCalibrationData)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -2;
	}
	CCalibration::Instance()->SetCalibration(m_bySlotNo, m_byControllerIndex, pCalibrationData);

	int nRetVal = SaveCalibrationData();
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			return -3;
		}
		else
		{
			return -4;
		}
	}
	return 0;
}

int CHardwareFunction::GetCalibrationData(CAL_DATA *pCalibrationData, int nElementCount)
{
	if (nullptr == pCalibrationData || DCM_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -2;
	}
	int nRetVal = ReadCalibrationData();
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			return -3;
		}
		return -4;
	}
	CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, pCalibrationData);
	return 0;
}

int CHardwareFunction::ResetCalibrationData()
{
	return CCalibration::Instance()->ResetCalibrationData(m_bySlotNo, m_byControllerIndex);
}

int CHardwareFunction::ResetCalibrationData(USHORT usChannel)
{
	return CCalibration::Instance()->ResetCalibrationData(m_bySlotNo, m_byControllerIndex, usChannel);
}

int CHardwareFunction::GetCapture(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine)
{
	vecBRAMLine.clear();
	vecDRAMLine.clear();
	ULONG ulREG = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0803);
	ULONG ulBRAMCount = (ulREG >> 16 & 0xFFF);
	ULONG ulDRAMCount = ulREG & 0x3FF;
	if (BRAM_MAX_SAVE_CAPTURE_COUNT <= ulBRAMCount)
	{
		ulBRAMCount = BRAM_MAX_SAVE_CAPTURE_COUNT;
	}
	ULONG ulData[BRAM_MAX_SAVE_CAPTURE_COUNT] = { 0 };
	DATA_RESULT DataResult;
	if (0 != ulBRAMCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0x0000, ulBRAMCount, ulData);
		for (int nLineIndex = 0; nLineIndex < ulBRAMCount; ++nLineIndex)
		{
			DataResult.m_nLineNo = (ulData[nLineIndex] >> 16) & 0xFFFF;
			DataResult.m_usData = ulData[nLineIndex] & 0xFFFF;
			vecBRAMLine.push_back(DataResult);
		}
	}
	if (DRAM_MAX_SAVE_CAPTURE_COUNT <= ulDRAMCount)
	{
		ulBRAMCount = DRAM_MAX_SAVE_CAPTURE_COUNT;
	}
	if (0 != ulDRAMCount)
	{
		ULONG ulAddr[DRAM_MAX_SAVE_CAPTURE_COUNT] = { 0 };
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0x8000, ulDRAMCount, ulAddr);
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0xC000, ulDRAMCount, ulData);
		for (int nLineIndex = 0; nLineIndex < ulDRAMCount; ++nLineIndex)
		{
			DataResult.m_nLineNo = ulAddr[nLineIndex] - 1;
			DataResult.m_usData = ulData[nLineIndex] & 0xFFFF;
			vecDRAMLine.push_back(DataResult);
		}
	}
	return 0;
}

int CHardwareFunction::SetTriggerOut(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	write_dw(16, m_bySlotNo);
	return 0;
}

int CHardwareFunction::GetFailSynType()
{
	if (-1 == m_ulBoardVer)
	{
		GetBoardLogicRevision();
	}
	if (0x30 >= m_ulBoardVer)
	{
		return 0;
	}
	return 1;
}

int CHardwareFunction::SetFailSyn(const std::map<BYTE, BYTE>& mapFailSyn)
{
	if (-1 == m_ulBoardVer)
	{
		GetBoardLogicRevision();
	}
	ULONG ulREGValue = 0;
	if (0x30 >= m_ulBoardVer)
	{
		///<Only support fail sychronous in specific method
		ulREGValue = 0x00111100;
		if (1 < mapFailSyn.size())
		{
			return -1;
		}
		else if (0 != mapFailSyn.size())
		{
			ulREGValue |= mapFailSyn.begin()->second;
		}
		m_Operation.WriteBoard(0x04, ulREGValue);
		return 0;
	}
	for (const auto& Controller : mapFailSyn)
	{
		if (DCM_MAX_CONTROLLERS_PRE_BOARD <= Controller.first)
		{
			return -2;
		}
		if (0 == Controller.second)
		{
			continue;
		}
		ulREGValue |= 0x01 << (24 + Controller.first * 2);
		ulREGValue |= (Controller.second & 0x0F) << Controller.first * 4;
	}
	m_Operation.WriteBoard(0x04, ulREGValue);
	return 0;
}

void CHardwareFunction::EnableSaveSelectedFail(BOOL bEnable)
{
	m_bSelectFail = bEnable;
}

BOOL CHardwareFunction::IsSupportSaveFailSelected()
{
	USHORT usRev = GetControllerLogicRevision();
	if (0x0110 < usRev)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CHardwareFunction::IsSupportFailSyn()
{
	USHORT usBoardRev = GetBoardLogicRevision();
	USHORT usControllerRev = GetControllerLogicRevision();
	if (0x30 < usBoardRev && 0x0110 < usControllerRev)
	{
		return TRUE;
	}
	return FALSE;
}

BYTE CHardwareFunction::GetFlashStatus()
{
	//Write Controller word
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x80000000);
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x20000000);
	//Write flash ID request command
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_READ_STATUS);
	m_Operation.WaitUs(20);
	return (BYTE)m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
}

void CHardwareFunction::SetVectorValid(BOOL bValid)
{
	ULONG ulREGValue = m_Operation.ReadBoard(0xA001);
	if (bValid)
	{
		ulREGValue |= 1 << 4;
	}
	else
	{
		ulREGValue &= ~(1 << 4);
	}
	m_Operation.WriteBoard(0xA001, ulREGValue);
}

BOOL CHardwareFunction::IsVectorValid()
{
	ULONG ulREGValue = m_Operation.ReadBoard(0xA001);
	if ((ulREGValue >> 4) & 0x01)
	{
		return TRUE;
	}
	return FALSE;
}

int CHardwareFunction::SetPatternMode(BOOL bSaveLineOrder, DATA_TYPE CheckDataType, BOOL bSwitch, BOOL bRead)
{
	ULONG ulREG = 0;
	if (bRead)
	{
		ulREG = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0818);
	}
	
	if (bSaveLineOrder)
	{
		ulREG &= ~(1 << 3);
	}
	else
	{
		ulREG |= 1 << 3;
		BYTE byDataType = 0;
		switch (CheckDataType)
		{
		case DATA_TYPE::FM:
			byDataType = 0x01;
			break;
		case DATA_TYPE::MM:
			byDataType = 0x02;
			break;
		case DATA_TYPE::IOM:
			byDataType = 0x03;
			break;
		default:
			return -1;
			break;
		}
		ulREG |= byDataType;
	}
	if (bSwitch)
	{
		ulREG |= 1 << 7;
	}
	else
	{
		ulREG &= ~(1 << 7);
	}
	m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0818, ulREG);
	return 0;
}

UINT CHardwareFunction::GetChannelStatus()
{
	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0817);
}

int CHardwareFunction::GetLineRanOrder(std::vector<UINT>& vecBRAM)
{
	vecBRAM.clear();
	ULONG ulREG = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0803);
	ULONG ulBRAMCount = (ulREG >> 16 & 0xFFF);

	ULONG ulData[0x1000] = { 0 };

	if (0 != ulBRAMCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0, ulBRAMCount, ulData);
		for (UINT uLineIndex = 0; uLineIndex < ulBRAMCount; ++uLineIndex)
		{
			vecBRAM.push_back((ulData[uLineIndex] >> 16) & 0xFFFF);
		}
	}
	return 0;
}

int CHardwareFunction::GetLineOrderCount()
{
	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0803);

	return ulREGValue >> 16 & 0xFFFF;
}

UINT CHardwareFunction::GetRanPatternCount()
{
	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x800);
}

void CHardwareFunction::SoftReset()
{
	m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x0D, 1);
}

int CHardwareFunction::SaveCalibrationData()
{
	if (!CheckFlashID())
	{
		return -1;
	}
	CAL_DATA CalData[DCM_CHANNELS_PER_CONTROL];
	CCalibration::Instance()->GetCalibration(m_bySlotNo, m_byControllerIndex, CalData);
	//The flash can be written.
	BYTE bySectorNo = CAL_DATA_SECTOR_START + m_byControllerIndex;

	UCHAR ucCheckCode[16] = { 0 };
	DWORD dwDataLength = sizeof(CAL_DATA);//The size of calibration data per channel.
	BYTE byCurPageNo = 0;
	DWORD dwDataLeft = dwDataLength;
	DWORD dwSumSize = dwDataLength + sizeof(ucCheckCode);
	BOOL bWriteSuccess = TRUE;
	BYTE *bySize = (BYTE*)(&dwSumSize);
	int nRetVal = 0;//The return value of the write flash function.
	int nRewriteTimes = 0;//The times of write flash.
	BYTE byOffset = 0;//The offset offset to the flash's page.
	BYTE *byWriteFlashData = 0;
	short sCurWriteByte = 0;
	int nStartIndex = 0;
	int nHeadSize = sizeof(ucCheckCode) + sizeof(DWORD);

	do
	{
		bWriteSuccess = TRUE;
		//Erase the sector which the calibration data saved in.
		EraseFlash(bySectorNo);

		//*****************Write calibration data into flash************************************//
		for (int index = 0; index < DCM_CHANNELS_PER_CONTROL; ++index)
		{
			nStartIndex = 0;
			dwDataLeft = dwDataLength;
			byCurPageNo = index * CAL_DATA_PAGE_PER_CH;
			//Write the length of the calibration data per channel.
			nRetVal = WriteFlash(bySectorNo, byCurPageNo, 0, 4, bySize);
			if (0 != nRetVal)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}

			byWriteFlashData = (BYTE*)(CalData[index].m_fDVHGain);

			//Generate MD5 check code.
			STSMD5Context context;
			STSMD5_Init(&context);
			STSMD5_Update(&context, byWriteFlashData, dwDataLength);
			STSMD5_Final(&context, ucCheckCode);
			//Write MD5 check code into flash.
			nRetVal = WriteFlash(bySectorNo, byCurPageNo, 4, sizeof(ucCheckCode), ucCheckCode);
			if (0 != nRetVal)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}
			do
			{
				sCurWriteByte = (short)(dwDataLeft > 256 ? 256 : dwDataLeft);
				byOffset = (0 == (byCurPageNo % CAL_DATA_PAGE_PER_CH)) ? nHeadSize : 0;
				if (256 == sCurWriteByte)
				{
					sCurWriteByte -= byOffset;
				}
				//Write the initialization data into flash.
				nRetVal = WriteFlash(bySectorNo, byCurPageNo, byOffset, sCurWriteByte, &byWriteFlashData[nStartIndex]);
				if (0 != nRetVal)
				{
					++nRewriteTimes;
					bWriteSuccess = FALSE;
					break;
				}
				++byCurPageNo;
				dwDataLeft -= sCurWriteByte;
				nStartIndex += sCurWriteByte;
			} while (0 < dwDataLeft);

			if (!bWriteSuccess)
			{
				//The current write operation is fail, rewrite is again.
				break;
			}
		}
	} while (!bWriteSuccess && REWRITE_TIMES_AFTER_FAIL >= nRewriteTimes);
	if (bWriteSuccess)
	{
		//Save the calibration data to global variable.
		m_bGetCalibrationData = TRUE;
		return 0;
	}
	else
	{
		return -2;
	}
}

int CHardwareFunction::ReadCalibrationData()
{
	if (!CheckFlashID())
	{
		return -1;
	}
	UCHAR cCheckCode[16] = { 0 };
	UCHAR cNewCheckCode[16] = { 0 };

	BYTE bySectorNo = CAL_DATA_SECTOR_START + m_byControllerIndex;//The sector No.of current Controller's calibration.
	DWORD dwCalDataLength = sizeof(CAL_DATA);//The size of calibration data per channel.
	BYTE byCurPageNo = 0;
	DWORD dwDataLeft = dwCalDataLength;
	BOOL bWriteSuccess = TRUE;
	int nRetVal = 0;//The return value of the write flash function.
	int nWriteTimes = 0;//The times of write flash.
	BYTE byOffset = 0;//The offset offset to the flash's page.
	BYTE *byWriteFlashData = 0;
	short sCurReadByte = 0;
	int nStartIndex = 0;
	DWORD dwCurDataLength = 0;
	BYTE *byLength = (BYTE*)(&dwCurDataLength);

	DWORD dwReadDataLength = dwCalDataLength + sizeof(cCheckCode);

	BYTE *pbyReadData = nullptr;
	try
	{
		pbyReadData = new BYTE[dwReadDataLength];
		memset(pbyReadData, 0, dwReadDataLength * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return -2;
	}
	DWORD dwReadStatus = 0x00;
	int nMD5FailTimes = 0;
	BOOL bMD5Success = TRUE;
	DWORD dwMaxDataLength = 256 * CAL_DATA_PAGE_PER_CH;
	do
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			nStartIndex = 0;
			byCurPageNo = usChannel * CAL_DATA_PAGE_PER_CH;
			nRetVal = ReadFlash(bySectorNo, byCurPageNo, 0, 4, byLength);
			if ((DWORD)-1 == dwCurDataLength || 0 == dwCurDataLength || 0 != nRetVal || dwCurDataLength > dwMaxDataLength)
			{
				//Current channel has no calibration data or read fail.
				dwReadStatus |= (0x01 << usChannel);
				continue;
			}
			dwReadDataLength = dwReadDataLength > dwCurDataLength ? dwCurDataLength : dwReadDataLength;
			dwDataLeft = dwReadDataLength;
			do
			{
				sCurReadByte = (short)dwDataLeft > 256 ? 256 : (short)dwDataLeft;
				byOffset = (0 == (byCurPageNo % CAL_DATA_PAGE_PER_CH)) ? 4 : 0;
				if (256 == sCurReadByte)
				{
					sCurReadByte -= byOffset;
				}
				//Read the calibration data from flash.
				nRetVal = ReadFlash(bySectorNo, byCurPageNo, byOffset, sCurReadByte, &pbyReadData[nStartIndex]);
				++byCurPageNo;
				dwDataLeft -= sCurReadByte;
				nStartIndex += sCurReadByte;
			} while (0 < dwDataLeft);

			//Get MD5 check code.
			memcpy(cCheckCode, pbyReadData, sizeof(cCheckCode));
			//Calculate the MD5 check code of the data read.
			STSMD5Context context;
			STSMD5_Init(&context);
			STSMD5_Update(&context, &pbyReadData[sizeof(cCheckCode)], dwCalDataLength);
			STSMD5_Final(&context, cNewCheckCode);
			if (0 == memcmp(cCheckCode, cNewCheckCode, sizeof(cCheckCode)))
			{
				bMD5Success = TRUE;
			}
			else
			{
				bMD5Success = FALSE;
				++nMD5FailTimes;
				break;
			}
			CCalibration::Instance()->SetCalibration(m_bySlotNo, m_byControllerIndex, usChannel, (CAL_DATA*)&pbyReadData[sizeof(cCheckCode)]);
		}

	} while (!bMD5Success && REREAD_TIMES_AFTER_FAIL >= nMD5FailTimes);

	if (nullptr != pbyReadData)
	{
		delete[] pbyReadData;
		pbyReadData = nullptr;
	}
	if (bMD5Success)
	{
		return 0;
	}
	return -3;
}

inline int CHardwareFunction::EdgeCheck(double dSmallerEdge, double dBiggerEdge, const char* lpszBiggerEdgeName)
{
	if (4 - EQUAL_ERROR > dBiggerEdge - dSmallerEdge)
	{
		if (nullptr != lpszBiggerEdgeName)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_EDGE_ERROR);
				m_pAlarm->SetParamName(lpszBiggerEdgeName);
				m_pAlarm->SetAlarmMsg("The %s(%.0fns) is must not less than %.0f.", lpszBiggerEdgeName, dBiggerEdge, dSmallerEdge + 4);
			}
			return -1;
		}
	}
	return 0;
}

inline int CHardwareFunction::WaitDRAMReady()
{
	DWORD dwWaitTimes = 25;
	DWORD dwWaitIndex = 0;
	if (CBindInfo::Instance()->IsBind())
	{
		COperation* pOperation = nullptr;
		BIND_READ_BEGIN(pOperation, m_bySlotNo)
		{
			while (1 != pOperation->IsDRAMReady() && dwWaitTimes > dwWaitIndex++)
			{
				DelayUs(1);
			}
		}
		BIND_READ_END
	}
	else
	{
		while (1 != m_Operation.IsDRAMReady() && dwWaitTimes > dwWaitIndex++)
		{
			DelayUs(1);
		}
	}
	if (dwWaitTimes <= dwWaitIndex)
	{
		return -1;
	}
	return 0;
}

inline int CHardwareFunction::WaitTMUStop(BYTE byUnitIndex)
{
	double dTimeout = CTMU::Instance()->GetTimeout(m_bySlotNo, m_byControllerIndex, byUnitIndex);
	ULONG ulStaus = 0;
	int nWaitTimes = dTimeout * 1000 / 100;
	BOOL bDelay = 0.05 < dTimeout ? TRUE : FALSE;
	int nCheckIndex = 0;
	USHORT usREGOffset = byUnitIndex * 0x10;

	do
	{
		if (0 != nCheckIndex++ && bDelay)
		{
			DelayUs(100);
		}
		ulStaus = m_Operation.ReadRegister(COperation::REG_TYPE::TMU_REG, 0x0C01 + usREGOffset);
	} while (0 != (ulStaus & 0x01) && nWaitTimes > nCheckIndex);
	if (0 != (ulStaus & 0x01))
	{
		///<Not wait stop
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_NOT_STOP);
// 		m_pAlarm->SetAlarmMsg("The TMU unit(%d) in the controller %d of slot %d is not stop after timeout.", byUnitIndex, m_byControllerIndex, m_bySlotNo);
		return -1;
	}
	if (0 != ulStaus >> 2)
	{
		///<The TMU unit is timeout
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_TIMEOUT);
// 		m_pAlarm->SetAlarmMsg("The TMU unit(%d) in the controller %d of slot %d is not trigger in timeout time.", byUnitIndex, m_byControllerIndex, m_bySlotNo);
		return -2;
	}
	return 0;
}

int CHardwareFunction::StartPMU()
{
	BOOL bStartPMU = FALSE;
	int nMeasIndex = 0;
	for (auto& Status : m_byPMUStatus)
	{
		if (1 != Status || 0 == m_mapPMUMeasure[nMeasIndex].size())
		{
			++nMeasIndex;
			continue;
		}

		bStartPMU = TRUE;
		m_Operation.Write305(0x000F, m_mapPMUMeasure[nMeasIndex]);
		DelayUs(85);///<Wait voltage stable
		m_Operation.PMUStart(m_uSampleTimes);
		ULONG ulREGValue = (m_byPMUMeasureChipEven[nMeasIndex] << 20) | ((m_uSampleTimes - 1) << 8) | 0x05;
		m_Operation.Write7606(ulREGValue);
		m_mapPMUMeasure[nMeasIndex].clear();
		Status = 2;
	}
	if (!bStartPMU)
	{
		return -1;
	}
	return 0;
}

int CHardwareFunction::WaitPMUFinish()
{
	BOOL bFinishPMU = FALSE;
	int nMeasIndex = 0;
	int nRetVal = 0;
	for (auto& Status : m_byPMUStatus)
	{
		if (2 != Status)
		{
			++nMeasIndex;
			continue;
		}

		bFinishPMU = TRUE;

		if (CBindInfo::Instance()->IsBind())
		{
			COperation* pOperation = nullptr;
			BIND_READ_BEGIN(pOperation, m_bySlotNo)
			{
				if (0 != nRetVal)
				{
					break;
				}
				nRetVal = CPMU::Instance()->SaveAverageData(*pOperation, m_byPMUMeasureChip[nMeasIndex], m_uSampleTimes, m_byPMUMeasureChipEven[nMeasIndex]);
				if (0 != nRetVal)
				{
					if (nullptr != m_pAlarm)
					{
						m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_MEAS_ERROR);
						m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
						m_pAlarm->SetAlarmMsg("PMU measurement error.");
					}
					nRetVal = -2;
					break;
				}
			}
			BIND_READ_END
		}
		else
		{
			nRetVal = CPMU::Instance()->SaveAverageData(m_Operation, m_byPMUMeasureChip[nMeasIndex], m_uSampleTimes, m_byPMUMeasureChipEven[nMeasIndex]);
			if (0 != nRetVal)
			{
				if (nullptr != m_pAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PMU_MEAS_ERROR);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
					m_pAlarm->SetAlarmMsg("PMU measurement error.");
				}
				nRetVal = -2;
				break;
			}
		}
		Status = 0;
		++nMeasIndex;
	}
	if (!bFinishPMU)
	{
		return -1;
	}
	return nRetVal;
}

void CHardwareFunction::CheckFailMemoryFilled(int nBRAMFailLineCountSaved, int nDRAMFailLineCountSaved)
{
	BOOL bcheckFinish[2] = { FALSE, FALSE };
	nBRAMFailLineCountSaved = FALSE;
	nDRAMFailLineCountSaved = FALSE;
	if (BRAM_MAX_SAVE_FAIL_LINE_COUNT > nBRAMFailLineCountSaved)
	{
		m_bBRAMFailMemoryFilled = FALSE;
		bcheckFinish[0] = TRUE;
	}
	if (DRAM_MAX_SAVE_FAIL_LINE_COUNT > nDRAMFailLineCountSaved)
	{
		m_bDRAMFailMemoryFilled = FALSE;
		bcheckFinish[1] = TRUE;
	}
	if (bcheckFinish[0] && bcheckFinish[1])
	{
		return;
	}
	
	int nFailCount = GetFailCount();
	if (nFailCount > nBRAMFailLineCountSaved + nDRAMFailLineCountSaved)
	{
		if (BRAM_MAX_SAVE_FAIL_LINE_COUNT <= nBRAMFailLineCountSaved)
		{
			nBRAMFailLineCountSaved = TRUE;
		}
		if (DRAM_MAX_SAVE_FAIL_LINE_COUNT <= nDRAMFailLineCountSaved)
		{
			nDRAMFailLineCountSaved = TRUE;
		}
	}
}

int CHardwareFunction::SetChannelMCUMode(const std::vector<USHORT>& vecChannel)
{
	return CChannelMode::Instance()->SetChannelMode(m_Operation, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
}

void CHardwareFunction::SetComparedChannel(const std::set<USHORT>& setChannel)
{
	m_setUnComparedChannel.clear();
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		if (setChannel.end() == setChannel.find(usChannel))
		{
			m_setUnComparedChannel.insert(usChannel);
		}
	}
	if (-1 == m_ulBoardVer)
	{
		GetBoardLogicRevision();
	}
	if (0x30 < m_ulBoardVer)
	{
		return;
	}
	vector<USHORT> vecChannel;
	copy(m_setUnComparedChannel.begin(), m_setUnComparedChannel.end(), back_inserter(vecChannel));
	DisableChannel(vecChannel);
}
