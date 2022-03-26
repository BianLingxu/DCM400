#include "pch.h"
#include "HardwareFunction.h"
#include "STSMD5.h"
#include "STSSP8201.h"
#include "FlashInfo.h"
#include "Bind.h"
#include "PMU.h"
#include "TMU.h"
#include "Period.h"
// #include "ChannelMode.h"

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
	, m_uSampleTimes(0)
	, m_pAlarm(pAlarm)
{
	m_pAlarm = CDriverAlarm::Instance();
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
	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
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

	ULONG ulData = m_Operation.ReadRegister(0x0000, TRUE, 0);
	if (0xFFFFFFFF != ulData)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CHardwareFunction::IsControllerExist()
{
	return TRUE;
}

ULONG CHardwareFunction::GetFlashID()
{
#if 0 // WAVE-TEMP
	DWORD dwFlashID = 0;

	//Write Controller word
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x00008000);
	//Write flash ID request command
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_READ_ID);

	m_Operation.WaitUs(200);
	return m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
#endif
	return 0;
}

BOOL CHardwareFunction::CheckFlashID()
{
#if 0 // WAVE-TEMP
	DWORD dwReadID = GetFlashID();
	if (FLASH_ID == dwReadID)
	{
		return TRUE;
	}
#endif
	return FALSE;
}

int CHardwareFunction::ReadFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usReadByteCount, BYTE *pbyData)
{
#if 0 // WAVE-TEMP
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
#endif
	return 0;
}

int CHardwareFunction::WriteFlash(BYTE bySectorNo, BYTE byPageNo, BYTE byOffset, USHORT usWriteByteCount, BYTE *pbyWriteData)
{
#if 0 // WAVE-TEMP
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
#else
	return 0;
#endif
}

int CHardwareFunction::EraseFlash(BYTE bySectorNo)
{
#if 0 // WAVE-TEMP
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
#endif
	return 0;
}

USHORT CHardwareFunction::GetBoardLogicRevision()
{
	m_Operation.SetControllerIndex(0);
	ULONG ulREGValue = m_Operation.ReadRegister(0x000A, FALSE, 0);
	m_Operation.SetControllerIndex(m_byControllerIndex);

	return ulREGValue & 0xFFFF;
}

USHORT CHardwareFunction::GetControllerLogicRevision()
{
	ULONG ulREGValue = 0;
	switch (m_byControllerIndex)
	{
	case 0:
		ulREGValue = m_Operation.ReadRegister(0x000A, FALSE, 0);
		break;
	case 1:
		m_Operation.SetControllerIndex(0);
		ulREGValue = m_Operation.ReadRegister(0x000A, FALSE, 1);
		m_Operation.SetControllerIndex(1);
		break;
	case 2:
		ulREGValue = m_Operation.ReadRegister(0x000A, FALSE, 2);
		break;
	case 3:
		m_Operation.SetControllerIndex(0);
		ulREGValue = m_Operation.ReadRegister(0x0002, TRUE, 3);
		m_Operation.SetControllerIndex(3);
		break;
	default:
		ulREGValue = 0xFFFFFFFF;
		break;
	}

	return ulREGValue & 0xFFFF;
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

int CHardwareFunction::SetHighVoltageRelay(const std::vector<USHORT>& vecChannel, BOOL bConnect /*= TRUE*/)
{
	if (nullptr == m_pRelay)
	{
		m_pRelay = new CRelay(m_Operation);
	}
	int nRetVal = m_pRelay->SetHighVoltageRelay(vecChannel, bConnect);
	if (0 != nRetVal)
	{
		set<USHORT> setHighChannel;
		m_pRelay->GetHighVoltageChannel(setHighChannel);
		for (auto Channel : vecChannel)
		{
			if (setHighChannel.end() == setHighChannel.end())
			{
				m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
				m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is not support high voltage mode.", m_bySlotNo, Channel);
				break;
			}
		}
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
#if 0 // WAVE-TEMP
	std::map<USHORT, ULONG> mapVIH, mapVIL, mapVT, mapVOH, mapVOL, mapVCH, mapVCL;
	std::map<USHORT, ULONG> mapPePmu, mapChState, mapPmuState;
	ULONG ulVIH(0), ulVIL(0), ulVT(0), ulVOH(0), ulVOL(0), ulVCH(0), ulVCL(0);
	USHORT index = 0;
	int nRetVal = 0;

	nRetVal = 0;

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
		DCM400_CAL_DATA CalData;

		for (USHORT usChannel : vecChannel)
		{
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}

			mapPePmu.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
			mapChState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
			mapPmuState.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));

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
		//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
	} while (0);
	return nRetVal;
#else
	return 0;
#endif
}

double CHardwareFunction::GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType)
{
#if 0 // WAVE-TEMP
	std::map<USHORT, ULONG> mapRead;
	BYTE byATE305Reg = 0;
	double dPinLevel = 0;

	float fGain = 1;
	float fOffset = 0;
	do
	{
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel) 
		{
			dPinLevel = 0x80000000;
			break;
		}

		if (LEVEL_TYPE::CLAMP_LOW < LevelType)
		{
			dPinLevel = 0x80000001;
			break;
		}

		mapRead.insert(std::pair<USHORT, ULONG>(usChannel, 0x0000));
		switch (LevelType)
		{
		case LEVEL_TYPE::VIH:
			byATE305Reg = 0x01;
			break;
		case LEVEL_TYPE::VIL:
			byATE305Reg = 0x02;
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
		if (LEVEL_TYPE::IOH == LevelType || LEVEL_TYPE::IOL == LevelType)
		{
			dPinLevel = dPinLevel * 0.012 / 5;
		}			

	} while (0);

	return dPinLevel;
#else
	return 0;
#endif
}

int CHardwareFunction::SetPeriodSeries(BYTE bySeries, double dPeriod)
{
	ULONG ulPeriod = 0;
	int nRetVal = 0;

	do
	{
		if (TIME_SERIES_MAX_COUNT <= bySeries)
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
		ulPeriod = (ULONG)(dPeriod + EQUAL_ERROR) * 2;

		m_Operation.WriteBRAM(COperation::BRAM_TYPE::T0_RAM, bySeries, 1, &ulPeriod);
		CPeriod::Instance()->SetPeriod(m_bySlotNo, m_byControllerIndex, bySeries, dPeriod);
	} while (0);

	return nRetVal;
}

double CHardwareFunction::GetPeriodSeries(BYTE bySeries)
{
	ULONG ulPeriod = 0;
	double dPeriod = 0;

	do 
	{
		if (TIME_SERIES_MAX_COUNT <= bySeries)
		{
			dPeriod = -1;
			break;
		}

		m_Operation.ReadBRAM(COperation::BRAM_TYPE::T0_RAM, bySeries, 1, &ulPeriod);

		dPeriod = ulPeriod / 2;
	} while (0);

	return dPeriod;
}

int CHardwareFunction::SetEdgeSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, const double* pdEdge)
{
	if (TIME_SERIES_MAX_COUNT <= bySeries)
	{
		return -1;
	}
	if (nullptr == pdEdge)
	{
		return -2;
	}
	ULONG ulStartAddr(0), ulEdgeValue(0);
	ULONG aulStartAddr[EDGE_COUNT] = { 0x0200,0x0300, 0x0100 ,0x0400,0x0500 ,0x0600 };
		
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT; ++byEdgeIndex)
	{
		ulStartAddr = aulStartAddr[byEdgeIndex] + bySeries; 
		ulEdgeValue = (ULONG)(pdEdge[byEdgeIndex] + EQUAL_ERROR) * 2;
		for (USHORT usChannel : vecChannel)
		{
			m_Operation.WriteBRAM(COperation::BRAM_TYPE::MEM_TIMMING, ulStartAddr | (usChannel << 12), 1, &ulEdgeValue);
		}
	}

	return 0;
}

double CHardwareFunction::GetEdgeSeries(USHORT usChannel, BYTE bySeries, EDGE_TYPE EdgeType)
{
	ULONG ulStartAddr, ulEdgeValue;

	if (TIME_SERIES_MAX_COUNT <= bySeries)
	{
		return -1;
	}

	switch (EdgeType)
	{
	case EDGE_TYPE::T1R:
		ulStartAddr = 0x0200;
		break;
	case EDGE_TYPE::T1F:
		ulStartAddr = 0x0300;
		break;
	case EDGE_TYPE::IOR:
		ulStartAddr = 0x0100;
		break;
	case EDGE_TYPE::IOF:
		ulStartAddr = 0x0400;
		break;
	case EDGE_TYPE::STBR:
		ulStartAddr = 0x0500;
		break;
	case EDGE_TYPE::STBF:
		ulStartAddr = 0x0600;
		break;
	default:
		return -2;
		break;
	}
	ulStartAddr += bySeries;

	m_Operation.ReadBRAM(COperation::BRAM_TYPE::MEM_TIMMING, ulStartAddr | (usChannel << 12), 1, &ulEdgeValue);

	return (double)(ulEdgeValue / 2);
}

int CHardwareFunction::SetFormatSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
{
	ULONG ulWaveFormat;

	if (TIME_SERIES_MAX_COUNT <= bySeries)
	{
		return -1;
	}

	switch (WaveFormat)
	{
	case WAVE_FORMAT::NRZ:
	case WAVE_FORMAT::RZ:
	case WAVE_FORMAT::RO:
	case WAVE_FORMAT::SBC:
	case WAVE_FORMAT::SBH:
	case WAVE_FORMAT::SBL:
	case WAVE_FORMAT::STAY:
	case WAVE_FORMAT::FH:
	case WAVE_FORMAT::FL:
		break;
	default:
		return -2;
		break;
	}

	switch (IOFormat)
	{
	case IO_FORMAT::NRZ:
	case IO_FORMAT::RO:
	case IO_FORMAT::OFF:
		break;
	default:
		return -2;
		break;
	}

	switch (CompareMode)
	{
	case COMPARE_MODE::EDGE:
	case COMPARE_MODE::WINDOW:
	case COMPARE_MODE::OFF:
		break;
	default:
		return -2;
		break;
	}

	ulWaveFormat = (((ULONG)CompareMode) << 6) + (((ULONG)IOFormat) << 4) + (ULONG)WaveFormat;
	for (USHORT usChannel : vecChannel)
	{
		m_Operation.WriteBRAM(COperation::BRAM_TYPE::MEM_TIMMING, bySeries | (usChannel << 12), 1, &ulWaveFormat);
	}

	return 0;
}

int CHardwareFunction::GetFormatSeries(USHORT usChannel, BYTE bySeries, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	ULONG ulWaveFormat;

	if (TIME_SERIES_MAX_COUNT <= bySeries)
	{
		return -1;
	}

	m_Operation.ReadBRAM(COperation::BRAM_TYPE::MEM_TIMMING, bySeries | (usChannel << 12), 1, &ulWaveFormat);
	WaveFormat = (WAVE_FORMAT)(ulWaveFormat & 0x0F);
	IOFormat = (IO_FORMAT)((ulWaveFormat >> 4) & 0x03);
	CompareMode = (COMPARE_MODE)((ulWaveFormat >> 6) & 0x03);

	return 0;
}

int CHardwareFunction::GetTimeSet(USHORT usChannel, USHORT usTimeSetIndex, BYTE& byPeriodSeries, BYTE* pbyEdgeSeriesIndex, BYTE& byFormatSeries)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (TIME_SET_MAX_COUNT <= usTimeSetIndex)
	{
		return -2;
	}
	if (nullptr == pbyEdgeSeriesIndex)
	{
		return -3;
	}
	ULONG ulREGValue[3] = { 0 };
	///<Get the period series
	m_Operation.ReadBRAM(COperation::BRAM_TYPE::GLO_T0_SET, usTimeSetIndex, 1, ulREGValue);
	byPeriodSeries = ulREGValue[0] & 0x1F;

	BYTE byREGIndex = usChannel / 6;
	BYTE byShiftCount = usChannel % 6 * 5;
	///<The Edge value
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT;++byEdgeIndex)
	{
		m_Operation.ReadBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, (byEdgeIndex + 1) << 13, 3, ulREGValue);
		pbyEdgeSeriesIndex[byEdgeIndex] = ulREGValue[byREGIndex] >> byShiftCount & 0x1F;
	}
	///<Get format
	m_Operation.ReadBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, 7 << 13, 3, ulREGValue);
	byFormatSeries = ulREGValue[byREGIndex] >> byShiftCount & 0x1F;
	return 0;
}

int CHardwareFunction::SetTimeSet(const vector<USHORT>& vecChannel, USHORT usTimeSetIndex, BYTE byPeriodSeries, const BYTE* pbyEdgeSeries, BYTE byFormatSeries)
{
	if (TIME_SET_MAX_COUNT <= usTimeSetIndex)
	{
		return -1;
	}
	if (TIME_SERIES_MAX_COUNT <= byPeriodSeries)
	{
		return -2;
	}
	if (nullptr == pbyEdgeSeries)
	{
		return -3;
	}

	auto SetData = [](ULONG& ulTarget, BYTE byData, BYTE byTargetStartBit)
	{
		for (BYTE byBit = 0; byBit < 5; ++byBit)
		{
			if (byData >> byBit)
			{
				ulTarget |= 1 << byTargetStartBit;
			}
			else
			{
				ulTarget &= ~(1 << byTargetStartBit);
			}
			++byTargetStartBit;
		}
	};

	ULONG aulEdgeREG[EDGE_COUNT][3] = { 0 };
	ULONG aulFormatREG[3] = { 0 };
	ULONG ulPeriod[EDGE_COUNT][3] = { 0 };
	if (DCM400_CHANNELS_PER_CONTROL != vecChannel.size())
	{
		for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT;++byEdgeIndex)
		{
			m_Operation.ReadBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, (byEdgeIndex + 1) << 13, 3, aulEdgeREG[byEdgeIndex]);
		}
		m_Operation.ReadBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, 7 << 13, 3, aulFormatREG);
	}
	WAVE_FORMAT WaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT IOFormat = IO_FORMAT::NRZ;
	COMPARE_MODE CompareMode = COMPARE_MODE::EDGE;
	double dEdge[EDGE_COUNT] = { 0 };
	for (auto& Edge : dEdge)
	{
		Edge = -1;
	}
	int nRetVal = 0;
	double dPeriod = CPeriod::Instance()->GetPeriod(m_bySlotNo, m_byControllerIndex, byPeriodSeries);
	for (auto Channel : vecChannel)
	{
		if (DCM400_CHANNELS_PER_CONTROL <= Channel)
		{
			///<Not will happen
			return -4;
		}
		
		for (int nEdgeIdnex = 0; nEdgeIdnex < EDGE_COUNT;++nEdgeIdnex)
		{
			if (TIME_SERIES_MAX_COUNT <= pbyEdgeSeries[nEdgeIdnex])
			{
				nRetVal = -5;
				break;
			}
		}
		if (0 != nRetVal)
		{
			break;
		}
		if (TIME_SERIES_MAX_COUNT <= byFormatSeries)
		{
			nRetVal = -6;
		}
		GetFormatSeries(Channel, byFormatSeries, WaveFormat, IOFormat, CompareMode);
		dEdge[0] = GetEdgeSeries(Channel, pbyEdgeSeries[0], EDGE_TYPE::T1R);
		dEdge[2] = GetEdgeSeries(Channel, pbyEdgeSeries[2], EDGE_TYPE::IOR);
		dEdge[4] = GetEdgeSeries(Channel, pbyEdgeSeries[4], EDGE_TYPE::STBR);
		double dMaxValue = dPeriod - 0.5 - EQUAL_ERROR;
		if (dEdge[0] > dMaxValue)
		{
			m_pAlarm->SetAlarmMsg("The edge T1R(%.1f) of channel(S%d_%d) is over range[0, %.1f].", dEdge[0], m_bySlotNo, Channel, dMaxValue);
			nRetVal = -7;
			break;
		}
		if (dEdge[2] > dMaxValue)
		{
			m_pAlarm->SetAlarmMsg("The edge IOR(%.1f) of channel(S%d_%d) is over range[0, %.1f].", dEdge[2], m_bySlotNo, Channel, dMaxValue);
			nRetVal = -7;
			break;
		}
		if (dEdge[4] > dMaxValue)
		{
			m_pAlarm->SetAlarmMsg("The edge STBR(%.1f) of channel(S%d_%d) is over range[0, %.1f].", dEdge[4], m_bySlotNo, Channel, dMaxValue);
			nRetVal = -7;
			break;
		}
		switch (WaveFormat)
		{
		case WAVE_FORMAT::NRZ:
			break;
		case WAVE_FORMAT::RZ:
		case WAVE_FORMAT::RO:
			dEdge[1] = GetEdgeSeries(Channel, pbyEdgeSeries[1], EDGE_TYPE::T1F);
			dMaxValue = dEdge[1] - 0.5 - EQUAL_ERROR;
			if (dEdge[0] > dEdge[1] - 0.5 - EQUAL_ERROR)
			{
				m_pAlarm->SetAlarmMsg("The edge T1R(%.1f) of channel(S%d_%d) is over range[0, %.1f(T1F)].", dEdge[0], m_bySlotNo, Channel, dMaxValue);
				nRetVal = -7;
			}
			break;
		case WAVE_FORMAT::SBC:
		case WAVE_FORMAT::SBH:
		case WAVE_FORMAT::SBL:
			dEdge[1] = GetEdgeSeries(Channel, pbyEdgeSeries[1], EDGE_TYPE::T1F);
			dMaxValue = dEdge[1] - 0.5 - EQUAL_ERROR;
			if (dEdge[0] > dMaxValue)
			{
				m_pAlarm->SetAlarmMsg("The edge T1R(%.1f) of channel(S%d_%d) is over range[0, %.1f(T1F)].", dEdge[0], m_bySlotNo, Channel, dMaxValue);
				nRetVal = -7;
			}
			dMaxValue = dEdge[0] - 0.5 - EQUAL_ERROR;
			if (dEdge[2] > dMaxValue)
			{
				m_pAlarm->SetAlarmMsg("The edge IOR(%.1f) of channel(S%d_%d) is over range[0, %.1f(T1R)].", dEdge[2], m_bySlotNo, Channel, dMaxValue);
				nRetVal = -7;
			}
			break;
		case WAVE_FORMAT::STAY:
			break;
		case WAVE_FORMAT::FH:
			break;
		case WAVE_FORMAT::FL:
			break;
		default:
			break;
		}
		if (0 != nRetVal)
		{
			break;
		}

		switch (IOFormat)
		{
		case IO_FORMAT::NRZ:
			break;
		case IO_FORMAT::RO:
			if (0 > dEdge[3] - EQUAL_ERROR)
			{
				dEdge[3] = GetEdgeSeries(Channel, pbyEdgeSeries[3], EDGE_TYPE::IOF);
			}
			dMaxValue = dEdge[3] - 0.5 - EQUAL_ERROR; 
			if (dEdge[2] > dMaxValue)
			{
				m_pAlarm->SetAlarmMsg("The edge IOR(%.1f) of channel(S%d_%d) is over range[0, %.1f(IOF)].", dEdge[2], m_bySlotNo, Channel, dMaxValue);
				nRetVal = -7;
			}
			break;
		case IO_FORMAT::OFF:
			break;
		default:
			break;
		}
		if (0 != nRetVal)
		{
			break;
		}
		switch (CompareMode)
		{
		case COMPARE_MODE::EDGE:
			break;
		case COMPARE_MODE::WINDOW:
			dEdge[5] = GetEdgeSeries(Channel, pbyEdgeSeries[5], EDGE_TYPE::STBF);
			dMaxValue = dEdge[5] - 0.5 - EQUAL_ERROR;
			if (dEdge[4] > dMaxValue)
			{
				m_pAlarm->SetAlarmMsg("The edge IOR(%.1f) of channel(S%d_%d) is over range[0, %.1f(STBF)].", dEdge[4], m_bySlotNo, Channel, dMaxValue);
				return -7;
			}
			break;
		case COMPARE_MODE::OFF:
			break;
		default:
			break;
		}
		if (0 != nRetVal)
		{
			break;
		}
			

		///<Save Edge value
		for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT; ++byEdgeIndex)
		{
			SetData(aulEdgeREG[byEdgeIndex][Channel / 6], pbyEdgeSeries[byEdgeIndex], Channel % 6 * 5);
		}
		///<Save Format
		SetData(aulFormatREG[Channel / 6], byFormatSeries, Channel % 6 * 5);

	}
	
	if (0 != nRetVal)
	{
		if (-7 == nRetVal)
		{
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_EDGE_ERROR);
			return nRetVal;
		}
		return nRetVal;
	}
	///<Write Period
	ULONG ulREG = byPeriodSeries & 0x1F;
	m_Operation.WriteBRAM(COperation::BRAM_TYPE::GLO_T0_SET, usTimeSetIndex, 1, &ulREG);
	///<Write Edge
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT;++byEdgeIndex)
	{
		m_Operation.WriteBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, (byEdgeIndex + 1) << 13, 3, aulEdgeREG[byEdgeIndex]);
	}	
	///<Write Format
	m_Operation.WriteBRAM(COperation::BRAM_TYPE::GLO_TIMINGSET, 7<< 13, 3, aulFormatREG);
	return 0;
}

int CHardwareFunction::WritePatternMemory(UINT uStartLineNo, UINT uLineCount, const ULONG* pulData)
{
	return WriteDataMemory(TRUE, uStartLineNo, uLineCount, pulData);
}

int CHardwareFunction::ReadPatternMemory(UINT uStartLineNo, UINT uLineCount, ULONG* pulData)
{
	return ReadDataMemory(TRUE, uStartLineNo, uLineCount, pulData);
}

int CHardwareFunction::WriteCMDMemory(UINT uStartLineNo, UINT uLineCount, const ULONG* pulData)
{
	return WriteDataMemory(FALSE, uStartLineNo, uLineCount, pulData);
}


int CHardwareFunction::ReadCMDMemory(UINT uStartLineNo, UINT uLineCount, ULONG* pulData)
{
	
	return ReadDataMemory(FALSE, uStartLineNo, uLineCount, pulData);
}

int CHardwareFunction::SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNLE_MODE ChannelMode, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	return 0;
}

int CHardwareFunction::GetChannelMode(USHORT usChannel)
{
	return 0;
}

void CHardwareFunction::EnableStart(BOOL bEnable)
{
	ULONG regData = 0x01;

	if (bEnable)
	{
		regData = 0x00;
	}
	m_Operation.WriteRegister(0x000C, regData, TRUE, 0);
}

int CHardwareFunction::SetRunParameter(UINT uStartLineNo, UINT uStopLineNo, BOOL bWithDRAM, UINT uDRAMStartLineNo, BOOL bEnableStart)
{
#if 0 // WAVE-TEMP
	ULONG regValue = 0;
	int nRet = 0;
	do 
	{
		if (DCM400_BRAM_PATTERN_LINE_COUNT <= uStartLineNo)
		{
			nRet = -1;
			break;
		}

		if (DCM400_BRAM_PATTERN_LINE_COUNT <= uStopLineNo)
		{
			nRet = -2;
			break;
		}

		if (uStartLineNo >= uStopLineNo) 
		{
			nRet = -3;
			break;
		}

		if (DCM400_DRAM_PATTERN_LINE_COUNT < uDRAMStartLineNo)
		{
			nRet = -4;
			break;
		}

		if (bWithDRAM)
		{
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0818, 0x88);
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0819, uDRAMStartLineNo / 2);
			
			WaitDRAMReady();
		}
		else
		{
			m_Operation.WriteRegister(COperation::REG_TYPE::FUNC_REG, 0x0818, 0x08);
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
#else
	return 0;
#endif
}

void CHardwareFunction::Run()
{
	m_bBRAMFailMemoryFilled = FALSE;
	m_bDRAMFailMemoryFilled = FALSE;
	m_Operation.WriteRegister(0x0008, 0x00000001, FALSE, 0);
	if (m_bLatestRanWithDRAM)
	{
		m_Operation.WaitUs(25);
	}
}

void CHardwareFunction::SynRun()
{
#if 0 // WAVE-TEMP
	m_bBRAMFailMemoryFilled = FALSE;
	m_bDRAMFailMemoryFilled = FALSE;
	read_dw(0xFC0000);
	write_dw(0xFC0000, 0);
	if (m_bLatestRanWithDRAM)
	{
		DelayUs(25);
	}
#endif
}

void CHardwareFunction::ForceStopRun()
{
	m_Operation.WriteRegister(0x0008, 0x00000002, FALSE, 0);
}

int CHardwareFunction::GetRunningStatus()
{
	ULONG ulREGValue = 0;

	//ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);

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

	//ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0814);
	if (ulREGValue & (1 << 24)) 
	{
		return 1;
	}

	ulREGValue &= 0x1FFF;

	return ulREGValue;
}

ULONG CHardwareFunction::GetRunLineCount()
{
	//return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x800);
	return 0;
}

int CHardwareFunction::GetResult()
{
	ULONG ulREGValue = 0;

	//ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0821);
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
	int nRetVal = 0;

	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	nRetVal = GetChannelFailCount(usChannel);
	if (nRetVal < 0)
	{
		return nRetVal;
	}

	if (0 == nRetVal) 
	{
		return 0;
	}
	else 
	{
		return 1;
	}
}

int CHardwareFunction::GetChannelFailCount(USHORT usChannel)
{
#if 0
	ULONG ulREGValue = 0;
	int nRetVal = 0;

	if (DCM400_CHANNELS_PER_CONTROL <= usChannel) 
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
#else
	return 0;
#endif
}

int CHardwareFunction::GetFailCount()
{
#if 0
	int nRetVal = GetRunningStatus();
	if (-1 == nRetVal) 
	{
		return -2;
	}
	if (0 == nRetVal)
	{
		return -3;
	}

	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0801);
#else
	return 0;
#endif
}

int CHardwareFunction::GetFailData(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine)
{
#if 0 // WAVE-TEMP
	vecBRAMLine.clear();
	vecDRAMLine.clear();

	ULONG ulREGValue(0), ulSVMFailCount(0), ulLVMFailCount(0);
	ULONG ulRamRsu[BRAM_MAX_SAVE_FAIL_LINE_COUNT] = { 0 };
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

	m_bBRAMFailMemoryFilled = FALSE;
	if (BRAM_MAX_SAVE_FAIL_LINE_COUNT == ulSVMFailCount)
	{
		m_bBRAMFailMemoryFilled = TRUE;
	}
	DATA_RESULT FailData;
	if (0 != ulSVMFailCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x0000, ulSVMFailCount, ulRamRsu);
		for (int nLineIndex = 0; nLineIndex < ulSVMFailCount; nLineIndex++)
		{
			FailData.m_nLineNo = (ulRamRsu[nLineIndex] >> 16) & 0x1FFF;
			FailData.m_usData = ulRamRsu[nLineIndex] & 0xFFFF;

			if (0 == FailData.m_usData)
			{
				return -3;
			}
			vecBRAMLine.push_back(FailData);
		}
	}

	m_bDRAMFailMemoryFilled = FALSE;

	if (0 != ulLVMFailCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x4000, ulLVMFailCount, ulRamRsu);
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_RSU, 0x6000, ulLVMFailCount, ulRamRsu + 2048);
		for (int nLineIndex = 0; nLineIndex < ulLVMFailCount; nLineIndex++)
		{
			FailData.m_nLineNo = ulRamRsu[nLineIndex] - 1;
			FailData.m_usData = ulRamRsu[2048 + nLineIndex] & 0xFFFF;

			if (0 == FailData.m_usData)
			{
				return -3;
			}
			vecDRAMLine.push_back(FailData);
		}
		if (DRAM_MAX_SAVE_FAIL_LINE_COUNT == vecDRAMLine.size())
		{
			m_bDRAMFailMemoryFilled = TRUE;
		}
	}
#endif
	return 0;
}

int CHardwareFunction::GetFailMemoryFilled(BOOL& bBRAMFilled, BOOL& bDRAMFilled)
{
#if 0 // WAVE-TEMP
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
		if (BRAM_MAX_SAVE_FAIL_LINE_COUNT == ulSVMFailCount)
		{
			m_bBRAMFailMemoryFilled = TRUE;
		}
		if (DRAM_MAX_SAVE_FAIL_LINE_COUNT == ulLVMFailCount)
		{
			m_bDRAMFailMemoryFilled = TRUE;
		}
	}
	bBRAMFilled = m_bBRAMFailMemoryFilled;
	bDRAMFilled = m_bDRAMFailMemoryFilled;
#endif
	return 0;
}

int CHardwareFunction::SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode)
{
#if 0 // WAVE-TEMP
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
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
			//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
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
#else
	return 0;
#endif
}

int CHardwareFunction::GetVTMode(USHORT usChannel, VT_MODE& VTMode)
{
#if 0 // WAVE-TEMP
	std::map<USHORT, ULONG> mapRead;
	ULONG ulREGValue0(0), ulREGValue1(0);

	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return 0;
}

int CHardwareFunction::SetDynamicLoad(std::vector<USHORT> vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow)
{
#if 0 // WAVE-TEMP
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
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
	//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
#endif
	return 0;
}

int CHardwareFunction::GetDynamicLoad(USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL)
{
#if 0 // WAVE-TEMP
	ULONG ulChannelState(0), ulIOH(0), ulIOL(0);
	std::map<USHORT, ULONG> mapRead;

	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return 0;
}

int CHardwareFunction::SetCalibrationRelay(USHORT usChannel, BOOL bConnect)
{
#if 0
	if (nullptr == m_pRelay)
	{
		m_pRelay = new CRelay(m_Operation);
	}
	return m_pRelay->CalRelay(usChannel, bConnect);
#else
	return 0;
#endif
}

int CHardwareFunction::InitPMU(const std::vector<USHORT>& vecChannel)
{
#if 0 // WAVE-TEMP
	std::map<USHORT, ULONG> mapPePmu;
	int nRetVal = 0;

	do
	{
		for (USHORT usChannel : vecChannel)
		{
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
		//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::NEITHER_MODE);
	}
	return nRetVal;
#else
	return 0;
#endif
}

int CHardwareFunction::SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow)
{
#if 0 // WAVE-TEMP
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
		ulVCH = (ULONG)((dClmapHigh + 2.5) / 10 * 16383);
		ulVCL = (ULONG)((dClampLow + 2.5) / 10 * 16383);

		for (USHORT usChannel : vecChannel)
		{
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
			{
				nRetVal = -1;
				break;
			}
			unsigned char ucCurForceMode = 0;///<0 is FV and 1 is FI
			CPMU::Instance()->GetForceMode(m_bySlotNo, m_byControllerIndex, usChannel, ucCurForceMode, ucChannelIRange);
			switch (Range)
			{
			case PMU_IRANGE::IRANGE_2UA:
				byRangeCode = 0;
				dMD = 1.25e6;
				break;
			case PMU_IRANGE::IRANGE_20UA:
				byRangeCode = 4;
				dMD = 1.25e5;
				break;
			case PMU_IRANGE::IRANGE_200UA:
				byRangeCode = 5;
				dMD = 1.25e4;
				break;
			case PMU_IRANGE::IRANGE_2MA:
				byRangeCode = 6;
				dMD = 1.25e3;
				break;
			case PMU_IRANGE::IRANGE_32MA:
				byRangeCode = 7;
				dMD = 100;
				break;
			default:
				break;
			}

			if ((PMU_MODE::FIMV == PMUMode) || (PMU_MODE::FIMI == PMUMode))
			{
				ulDACCode = (ULONG)((dSetValue * dMD + 5) / 10 * 65535);
			}
			else
			{
				ulDACCode = (ULONG)((dSetValue + 2.5) / 10 * 65535);
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
		//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::PMU_MODE);
	}
	return nRetVal;
#else
return 0;
#endif
}

void CHardwareFunction::GetClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel)
{
#if 0 // WAVE-TEMP
	mapClampChannel.clear();
	BOOL bClamp = FALSE;
	map<USHORT, ULONG> mapATEData;
	for (USHORT usChannel : vecChannel)
	{
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
}

int CHardwareFunction::EnablePMUClampFlag(const std::vector<USHORT>& vecChannel, BOOL bEnable)
{
#if 0 // WAVE-TEMP
	ULONG ulREG = 0x01;
	if (bEnable)
	{
		ulREG = 0x03;
	}
	map<USHORT, ULONG> mapATE;
	for (auto Channel : vecChannel)
	{
		if (DCM400_CHANNELS_PER_CONTROL <= Channel)
		{
			return -1;
		}
		mapATE.insert(make_pair(Channel, ulREG));
	}
	m_Operation.Write305(0x12, mapATE);
#endif
	return 0;
}

double CHardwareFunction::GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange)
{
#if 0 // WAVE-TEMP
	std::map<USHORT, ULONG> mapRead;
	double dMD(0), dGain(0), dOffset(0);
	ULONG ulREGValue = 0;
	double dSetValue = 0;

	do
	{
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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

		if (0x07 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_32MA;
			dMD = 100;
		}
		else if (0x06 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_2MA;
			dMD = 1.25e3;
		}
		else if (0x05 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_200UA;
			dMD = 1.25e4;
		}
		else if (0x04 == (ulREGValue & 0x07))
		{
			PMURange = PMU_IRANGE::IRANGE_20UA;
			dMD = 1.25 * 1e5;
		}
		else
		{
			PMURange = PMU_IRANGE::IRANGE_2UA;
			dMD = 1.25e6;
		}

		m_Operation.Read305(0x0B, mapRead);

		ulREGValue = mapRead.begin()->second;

		if ((PMUMode == PMU_MODE::FVMV) || (PMUMode == PMU_MODE::FVMI))
		{
			dSetValue = (double)ulREGValue * 10 / 65535 - 2.5;
		}
		else
		{
			dSetValue = (double)ulREGValue * 10 / 65535 - 5.0;
			dSetValue /= dMD;
		}
	} while (0);

	return dSetValue;
#else
	return 0;
#endif
}

int CHardwareFunction::SetHighMode(const std::vector<USHORT>& vecChannel, BOOL bEnable, double dVoltage)
{
#if 0 // WAVE-TEMP
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
		USHORT usChannelOffset = DCM400_CHANNELS_PER_CONTROL * m_byControllerIndex;
		for (BYTE byREGIndex = 0; byREGIndex < byRegisterCount; ++byREGIndex)
		{
			m_Operation.Write305(byRegAddr[byREGIndex], mapWrite[byREGIndex]);
		}
		//CChannelMode::Instance()->SaveChannelMode(m_bySlotNo, m_byControllerIndex, vecChannel, CChannelMode::CHANNEL_MODE::MCU_MODE);
	} while (0);

	return nRetVal;
#else
	return 0;
#endif
}

int CHardwareFunction::InitTMU()
{
#if 0 // WAVE-TEMP
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
#endif
	return 0;
}

int CHardwareFunction::PMUMeasure(const std::vector<USHORT>& vecChannel, int nSampleTimes, double dSamplePeriod)
{
#if 0 // WAVE-TEMP
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
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#else
return 0;
#endif
}

double CHardwareFunction::GetPMUMeasureResult(USHORT usChannel, int nSampleTimes)
{
#if 0 // WAVE-TEMP
	double dRom(0), dGain(0), dOffset(0);
	ULONG ulPMUBRAM(0), ulBRAMAddr(0);
	SHORT sADCValue(0);
	double dMeas = 0;

	do
	{
		if (AVERAGE_RESULT != nSampleTimes)
		{
			if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
			ulBRAMAddr = ((1 - (usChannel % 2)) * DCM400_CHANNELS_PER_CONTROL / 2 + usChannel / 2) * PMU_SAMPLE_DEPTH / 2;
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
				break;
			case PMU_IRANGE::IRANGE_20UA:
				dRom = 25000;
				break;
			case PMU_IRANGE::IRANGE_200UA:
				dRom = 2500;
				break;
			case PMU_IRANGE::IRANGE_2MA:
				dRom = 250;
				break;
			case PMU_IRANGE::IRANGE_32MA:
				dRom = 15.5;
			default:
				break;
			}

			dMeas = double(sADCValue * 10.0) / 32768.0;
			dMeas = (dMeas - 2.5) / (5 * dRom);
		}
		else
		{
			dMeas = double(sADCValue * 10.0) / 32768.0;
		}
	} while (0);

	return dMeas;
#else
	return 0;
#endif
}

int CHardwareFunction::SetTMUUnitChannel(USHORT usChannel, BYTE byUnitIndex)
{
#if 0 // WAVE-TEMP
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return 0;
}

int CHardwareFunction::GetTMUConnectUnit(USHORT usChannel)
{
#if 0 // WAVE-TEMP
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return -2;
}

int CHardwareFunction::SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, BYTE bySpecifiedUnit)
{
#if 0
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
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return 0;
}

int CHardwareFunction::GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#if 0
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
#endif
	return 0;
}

int CHardwareFunction::EnableTMUSelfCheck(BYTE byUnitIndex, BOOL bEnable)
{
#if 0
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
#endif
	return 0;
}

int CHardwareFunction::TMUMeasure(const std::vector<USHORT>& vecChannel, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout)
{
#if 0 // WAVE-TEMP
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
		if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#endif
	return 0;
}

int CHardwareFunction::GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout)
{
#if 0
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
	CTMU::Instance()->GetMode(m_bySlotNo, usChannel / DCM400_CHANNELS_PER_CONTROL, nUnitIndex);
#endif
	return 0;
}

double CHardwareFunction::GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nErrorCode)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
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
#if 0 // WAVE-TEMP
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
		nError = 0;
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
#else
return 0;
#endif
}

int CHardwareFunction::ReadBRAMMemory(RAM_TYPE BRAMType, UINT uStartLine, UINT uLineCount, ULONG *pulData)
{
#if 0 // WAVE-TEMP
	UINT uMaxLineCount(DCM400_BRAM_PATTERN_LINE_COUNT / 2), uStartAddr(0);
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
	case CHardwareFunction::RAM_TYPE::MEM_RSU_SVM2:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM3:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_SVM:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBF:
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
#else
	return 0;
#endif
}

int CHardwareFunction::WriteBRAMMemory(RAM_TYPE BRAMType, UINT uStartLine, UINT uLineCount, const ULONG* pulData)
{
#if 0
	UINT uMaxLineCount(DCM400_BRAM_PATTERN_LINE_COUNT / 2), uStartAddr(0);
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
	case CHardwareFunction::RAM_TYPE::MEM_RSU_SVM2:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM3:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_SVM:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1:
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR:
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBF:
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
#else
	return 0;
#endif
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

int CHardwareFunction::SetCalibrationData(DCM400_CAL_DATA* pCalibrationData, int nElementCount)
{
	if (nullptr == pCalibrationData)
	{
		return -1;
	}
	if (DCM400_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -2;
	}
	int nRetVal = SaveCalibrationData(pCalibrationData, nElementCount);
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

int CHardwareFunction::GetCalibrationData(DCM400_CAL_DATA*pCalibrationData, int nElementCount)
{
	if (nullptr == pCalibrationData || DCM400_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -1;
	}
	if (DCM400_CHANNELS_PER_CONTROL > nElementCount)
	{
		return -2;
	}
	int nRetVal = ReadCalibrationData(pCalibrationData, nElementCount);
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			return -3;
		}
		return -4;
	}
	return 0;
}

int CHardwareFunction::GetCapture(std::vector<DATA_RESULT>& vecBRAMLine, std::vector<DATA_RESULT>& vecDRAMLine)
{
#if 0 // WAVE-TEMP
	vecBRAMLine.clear();
	vecDRAMLine.clear();
	ULONG ulREG = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0803);
	ULONG ulBRAMCount = (ulREG >> 16 & 0xFFF);
	ULONG ulDRAMCount = ulREG & 0x3FF;

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
	if (0 != ulDRAMCount)
	{
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0x8000, ulDRAMCount, ulData);
		m_Operation.ReadRAM(COperation::BRAM_TYPE::MEM_HIS, 0xC000, ulDRAMCount, ulData + 2048);
		for (int nLineIndex = 0; nLineIndex < ulDRAMCount; ++nLineIndex)
		{

			DataResult.m_nLineNo = ulData[nLineIndex] - 1;
			DataResult.m_usData = ulData[2048 + nLineIndex] & 0xFFFF;
			vecDRAMLine.push_back(DataResult);
		}
	}
#endif
	return 0;
}

int CHardwareFunction::SetTriggerOut(USHORT usChannel)
{
	if (DCM400_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}

	write_dw(16, m_bySlotNo);
	return 0;
}

int CHardwareFunction::SetFailSyn(std::vector<BYTE>& vecController)
{
	ULONG ulData = 0x00111100;
	for (auto Controller : vecController)
	{
		if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= Controller)
		{
			return -1;
		}
		ulData |= 1 << Controller;
	}
	//m_Operation.WriteBoard(0x04, ulData);
	return 0;
}

BYTE CHardwareFunction::GetFlashStatus()
{
#if 0 // WAVE-TEMP
	//Write Controller word
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x80000000);
	m_Operation.WriteBoard(FLASH_CTRL_ADDR, 0x20000000);
	//Write flash ID request command
	m_Operation.WriteBoard(FLASH_CMD_ADDR, FLASH_CMD_READ_STATUS);
	m_Operation.WaitUs(20);
	return (BYTE)m_Operation.ReadBoard(FLASH_READ_DATA_ADDR);
#else
	return 0;
#endif
}

void CHardwareFunction::SetVectorValid(BOOL bValid)
{
#if 0 // WAVE-TEMP
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
#endif
}


BOOL CHardwareFunction::IsVectorValid()
{
#if 0 // WAVE-TEMP
	ULONG ulREGValue = m_Operation.ReadBoard(0xA001);
	if ((ulREGValue >> 4) & 0x01)
	{
		return TRUE;
	}
#endif
	return FALSE;
}

UINT CHardwareFunction::GetChannelStatus()
{
#if 0
	return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0817);
#else
	return 0;
#endif
}

int CHardwareFunction::GetLineRanOrder(std::vector<UINT>& vecBRAM)
{
#if 0 // WAVE-TEMP
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
#endif
	return 0;
}

int CHardwareFunction::GetLineOrderCount()
{
#if 0 // WAVE-TEMP
	ULONG ulREGValue = m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x0803);

	return ulREGValue >> 16 & 0xFFFF;
#else
	return 0;
#endif
}

UINT CHardwareFunction::GetRanPatternCount()
{
	//return m_Operation.ReadRegister(COperation::REG_TYPE::FUNC_REG, 0x800);
	return 0;
}

void CHardwareFunction::SoftReset()
{
	//m_Operation.WriteRegister(COperation::REG_TYPE::SYS_REG, 0x0D, 1);
}

int CHardwareFunction::SaveCalibrationData(const DCM400_CAL_DATA* pCalibrationData, USHORT usChannelCount)
{
	if (!CheckFlashID())
	{
		return -1;
	}
	BYTE bySectorNo = CAL_DATA_SECTOR_START + m_byControllerIndex;

	UCHAR ucCheckCode[16] = { 0 };
	DWORD dwDataLength = sizeof(DCM400_CAL_DATA);//The size of calibration data per channel.
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
		for (USHORT usChannel = 0; usChannel < DCM400_CHANNELS_PER_CONTROL; ++usChannel)
		{
			nStartIndex = 0;
			dwDataLeft = dwDataLength;
			byCurPageNo = usChannel * CAL_DATA_PAGE_PER_CH;
			//Write the length of the calibration data per channel.
			nRetVal = WriteFlash(bySectorNo, byCurPageNo, 0, 4, bySize);
			if (0 != nRetVal)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}

			byWriteFlashData = (BYTE*)(pCalibrationData[usChannel].m_fDVHGain);

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

int CHardwareFunction::ReadCalibrationData(DCM400_CAL_DATA* pCalibrationData, int nElementCount)
{
	if (!CheckFlashID())
	{
		return -1;
	}
	UCHAR cCheckCode[16] = { 0 };
	UCHAR cNewCheckCode[16] = { 0 };

	BYTE bySectorNo = CAL_DATA_SECTOR_START + m_byControllerIndex;//The sector No.of current Controller's calibration.
	DWORD dwCalDataLength = sizeof(DCM400_CAL_DATA);//The size of calibration data per channel.
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
		for (USHORT usChannel = 0; usChannel < DCM400_CHANNELS_PER_CONTROL; ++usChannel)
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
			if (usChannel < nElementCount)
			{
				memcpy_s(&pCalibrationData[usChannel], sizeof(DCM400_CAL_DATA), &pbyReadData[sizeof(cCheckCode)], sizeof(DCM400_CAL_DATA));
			}
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
	if (0.5 - EQUAL_ERROR > dBiggerEdge - dSmallerEdge)
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
#if 0
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
#endif
	return 0;
}

inline int CHardwareFunction::WaitTMUStop(BYTE byUnitIndex)
{
#if 0
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
#endif
	return 0;
}

int CHardwareFunction::WriteDataMemory(BOOL bPattern, UINT uStartLineNo, UINT uLineCount, const ULONG* pulData)
{
	if (DCM400_MAX_PATTERN_COUNT <= uStartLineNo)
	{
		///<The start line number is over range
		return -1;
	}
	if (DCM400_MAX_PATTERN_COUNT < uStartLineNo + uLineCount)
	{
		///<The line count is over range
		return -2;
	}
	if (nullptr == pulData)
	{
		return -3;
	}
	const int nMultipleTimes = bPattern ? 8 : 2;
	const int nElementTimes = bPattern ? 2 : 8;
	const int nSizePerLine = bPattern ? 8 : 32;
	UINT uTargetLineCount = uLineCount;
	ULONG* pulTargetData = nullptr;
	UINT uHeadAddLineCount = uStartLineNo % nMultipleTimes;///<Data memory only can write from times of 8
	uTargetLineCount += uHeadAddLineCount;
	///<Data memory only can write times of 8 lines
	UINT uTailAddLineCount = uTargetLineCount % nMultipleTimes == 0 ? 0 : (nMultipleTimes - uTargetLineCount % nMultipleTimes);
	uTargetLineCount += uTargetLineCount;
	int nElementCount = uTargetLineCount * nElementTimes;
	UINT uTargetStartLineNo = uStartLineNo - uHeadAddLineCount;
	if (uTargetLineCount != uLineCount)
	{
		try
		{
			pulTargetData = new ULONG[nElementCount];
		}
		catch (const std::exception&)
		{
			///<Allocate memory fail
			return -4;
		}
		memset(pulTargetData, 0, nElementCount * sizeof(ULONG));
		if (0 != uHeadAddLineCount)
		{
			m_Operation.ReadDRAM(COperation::DRAM_TYPE::PAT_DRAM0, (uStartLineNo - uHeadAddLineCount) * nSizePerLine, uHeadAddLineCount * nElementTimes, pulTargetData);
		}
		if (0 != uTailAddLineCount)
		{
			UINT uTailStartLineNo = uTargetLineCount - uHeadAddLineCount - uLineCount;
			m_Operation.ReadDRAM(COperation::DRAM_TYPE::PAT_DRAM0, uTailStartLineNo * nSizePerLine, uTailAddLineCount * nElementTimes, &pulTargetData[uTailStartLineNo * nElementTimes]);
		}
		memcpy_s(&pulTargetData[uHeadAddLineCount * nElementTimes], uLineCount * nElementTimes * sizeof(ULONG), pulData, uLineCount * nElementTimes * sizeof(ULONG));
}
	else
	{
		pulTargetData = (ULONG*)pulData;
	}

	int nRetVal = m_Operation.WriteDRAM(COperation::DRAM_TYPE::PAT_DRAM0, uTargetStartLineNo * nSizePerLine, uTargetLineCount * nElementTimes, pulTargetData);
	if (uTargetLineCount != uLineCount)
	{
		if (nullptr != pulTargetData)
		{
			delete[] pulTargetData;
			pulTargetData = nullptr;
		}
	}
	if (0 != nRetVal)
	{
		return -5;
	}
	return 0;
}

int CHardwareFunction::ReadDataMemory(BOOL bPattern, UINT uStartLineNo, UINT uLineCount, ULONG* pulData)
{
	if (DCM400_MAX_PATTERN_COUNT <= uStartLineNo)
	{
		///<The start line number is over range
		return -1;
	}
	if (DCM400_MAX_PATTERN_COUNT < uStartLineNo + uLineCount)
	{
		///<The line count is over range
		return -2;
	}
	if (nullptr == pulData)
	{
		return -3;
	}

	const int nMultipleTimes = bPattern ? 8 : 2;
	const int nElementTimes = bPattern ? 2 : 8;
	const int nSizePerLine = bPattern ? 8 : 32;
	UINT uTargetLineCount = uLineCount;
	ULONG* pulTargetData = nullptr;
	UINT uHeadAddLineCount = uStartLineNo % nMultipleTimes;///<Data memory only can write from times of 8
	uTargetLineCount += uHeadAddLineCount;
	///<Data memory only can write times of 8 lines
	UINT uTailAddLineCount = uTargetLineCount % nMultipleTimes == 0 ? 0 : (nMultipleTimes - uTargetLineCount % nMultipleTimes);
	uTargetLineCount += uTargetLineCount;
	int nElementCount = uTargetLineCount * nElementTimes;
	UINT uTargetStartLineNo = uStartLineNo - uHeadAddLineCount;
	if (uTargetLineCount != uLineCount)
	{
		try
		{
			pulTargetData = new ULONG[nElementCount];
		}
		catch (const std::exception&)
		{
			///<Allocate memory fail
			return -4;
		}
		memset(pulTargetData, 0, nElementCount * sizeof(ULONG));
}
	else
	{
		pulTargetData = (ULONG*)pulData;
	}

	int nRetVal = m_Operation.ReadDRAM(COperation::DRAM_TYPE::PAT_DRAM0, uTargetStartLineNo * nSizePerLine, nElementCount, pulTargetData);
	if (0 != nRetVal)
	{
		if (uTargetLineCount != uLineCount)
		{
			if (nullptr != pulTargetData)
			{
				delete[] pulTargetData;
				pulTargetData = nullptr;
			}
		}
		return -5;
	}
	if (uTargetLineCount != uLineCount)
	{
		memcpy_s(pulData, uLineCount * nElementTimes * sizeof(ULONG), &pulTargetData[uHeadAddLineCount * nElementTimes], uLineCount * nElementTimes * sizeof(ULONG));
		if (nullptr != pulTargetData)
		{
			delete[] pulTargetData;
			pulTargetData = nullptr;
		}
	}
	return 0;
}

int CHardwareFunction::StartPMU()
{
#if 0 // WAVE-TEMP
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
#endif
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
