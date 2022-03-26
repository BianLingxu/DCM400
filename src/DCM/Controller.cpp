#include "Controller.h"
#include "CalibrationInfo.h"
#include "STSMD5.h"
#include "HardInfo.h"
#include <iterator>
using namespace std;

CController::CController(BYTE bySlotNo, BYTE byIndex, CDriverAlarm* pAlarm)
	: m_HardwareFunction(bySlotNo, pAlarm)
	, m_Pattern(m_HardwareFunction, pAlarm)
	, m_pAlarm(pAlarm)
{
	m_bySlotNo = bySlotNo;
	m_byIndex = byIndex;
	m_Pattern.SetDefaultPattern(CPattern::PATTERN_SIGN::PAT_X);
	m_HardwareFunction.SetControllerIndex(m_byIndex);
	m_bGetCalibraitonInfo = FALSE;
	m_pChannelData = nullptr;
	for (auto& Data : m_arrayDataWritten)
	{
		Data = nullptr;
	}
}

CController::~CController()
{
	if (nullptr != m_pChannelData)
	{
		delete m_pChannelData;
		m_pChannelData = nullptr;
	}
	ClearPreread();
}

BOOL CController::IsExist()
{
	return m_HardwareFunction.IsControllerExist();
}

USHORT CController::GetFPGARevision()
{
	return m_HardwareFunction.GetControllerLogicRevision() & 0xFFFF;
}

int CController::SetTotalStartDelay(double dDelay)
{
	return m_HardwareFunction.SetTotalStartDelay(dDelay);
}

double CController::GetTotalStartDelay()
{
	return m_HardwareFunction.GetTotalStartDelay();
}

int CController::SetTimesetDelay(double dDelay)
{
	return m_HardwareFunction.SetTimesetDelay(dDelay);
}

double CController::GetTimesetDelay()
{
	return m_HardwareFunction.GetTimesetDelay();
}

int CController::SetIODelay(USHORT usChannel, double dData, double dDataEn, double dHigh, double dLow)
{
	return m_HardwareFunction.SetIODelay(usChannel, dData, dDataEn, dHigh,dLow);
}

int CController::GetIODelay(USHORT usChannel, double* pdData, double* pdDataEn, double* pdHigh, double* pdLow)
{
	return m_HardwareFunction.GetIODelay(usChannel, pdData, pdDataEn, pdHigh, pdLow);
}

void CController::UpdateDelay()
{
	m_HardwareFunction.UpdateDelay();
}

int CController::SetVector(USHORT uChannel, BOOL bBRAM, UINT uStartPatternLine, char cPatternSign, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, USHORT usOperand, BOOL bCapture, BOOL bSwitch)
{
	if (DCM_CHANNELS_PER_CONTROL <= uChannel)
	{
		return -1;
	}
	if (bBRAM)
	{
		if (DCM_BRAM_PATTERN_LINE_COUNT <= uStartPatternLine)
		{
			return -2;
		}
	}
	else
	{
		if (DCM_DRAM_PATTERN_LINE_COUNT <= uStartPatternLine)
		{
			return -2;
		}
	}

	int nRetVal = 0;
	nRetVal = m_Pattern.AddChannelPattern(uChannel, bBRAM, uStartPatternLine, cPatternSign, byTimeset, lpszCMD, lpszParallelCMD, usOperand, bCapture, bSwitch);

	if (0 != nRetVal)
	{
		nRetVal = -3;
	}
	return nRetVal;
}

int CController::GetPattern(BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17])
{
	CPattern Pattern(m_HardwareFunction, m_pAlarm);
	int nRetVal = 0;
	UINT uBRAMStartLine = 0;
	UINT uDRAMStartLine = 0;
	UINT uBRAMLineCount = 0;
	UINT uDRAMLineCount = 0;
	UINT* puStartLine = &uBRAMStartLine;
	UINT* puLineCount = &uBRAMLineCount;
	if (!bBRAM)
	{
		puStartLine = &uDRAMStartLine;
		puLineCount = &uDRAMLineCount;
	}
	*puStartLine = uStartLine;
	*puLineCount = uLineCount;
	nRetVal = Pattern.ReadPattern(bBRAM, uStartLine, uLineCount, lpszPattern);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Start line is over range
			nRetVal = -1;
			break;
		case -2:
			///<Line count is over range
			nRetVal = -2;
			break;
		case -3:
			///<The point of pattern is nullptr
			nRetVal = -3;
		case -4:
			///<Allocate memory fail
			nRetVal = -4;
		default:
			break;
		}
		return nRetVal;
	}
	return 0;
}

int CController::GetMemory(BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData)
{
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	if (!bBRAM)
	{
		MemType = MEM_TYPE::DRAM;
	}
	int nRetVal = m_HardwareFunction.ReadDataMemory(MemType, DataType, uStartLine, uLineCount, pusData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The memory type is error, will not happened
			break;
		case -2:
			///<The data type is not supported
			nRetVal = -1;
			break;
		case -3:
			///<The start line is over range
			nRetVal = -2;
			break;
		case -4:
			///<The data count is over range
			nRetVal = -3;
			break;
		case -5:
			///<The point of data is nullptr or the read data count is 0
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CController::SetMemory(USHORT usChannel, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	if (!bBRAM)
	{
		MemType = MEM_TYPE::DRAM;
	}

	if (nullptr == pbyData)
	{
		return -7;
	}

	USHORT* pusData = nullptr;
	try
	{
		pusData = new USHORT[uLineCount];
		memset(pusData, 0, uLineCount * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -3;
	}

	int nRetVal = m_HardwareFunction.ReadDataMemory(MemType, DataType, uStartLine, uLineCount, pusData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Not happened
			break;
		case -2:
			///<Data type is not supported
			nRetVal = -2;
			break;
		case -3:
			///<The start line is over range
			nRetVal = -4;
			break;
		case -4:
			///<The line count is over range
			nRetVal = -5;
			break;
		case -5:
			///<The read line count is 0
			nRetVal = -6;
			break;
		default:
			break;
		}
		if (nullptr != pusData)
		{
			delete[] pusData;
			pusData = nullptr;
		}
		return nRetVal;
	}
	int nCurByteBiteCount = 8;
	for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
	{
		int nCurByteIndex = uLineIndex / 8;

		int nValidBitCount = uLineCount - nCurByteIndex * 8;
		nValidBitCount = nValidBitCount > 8 ? 8 : nValidBitCount;

		BYTE byCurBit = pbyData[nCurByteIndex] >> (nValidBitCount - (uLineIndex % 8) - 1) & 0x01;
		if (0 != byCurBit)
		{
			pusData[uLineIndex] |= 1 << usChannel;
		}
		else
		{
			pusData[uLineIndex] &= ~(1 << usChannel);
		}
	}
	m_HardwareFunction.WriteDataMemory(MemType, DataType, uStartLine, uLineCount, pusData);

	if (nullptr != pusData)
	{
		delete[] pusData;
		pusData = nullptr;
	}

	return 0;
}

int CController::LoadVector()
{
	int nRetVal = m_Pattern.Load();
	return nRetVal;
}

int CController::SetOperand(UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange)
{
	int nRetVal = m_Pattern.SetOperand(uBRAMLineNo, usOperand, bCheckRange);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The line number is over range
			nRetVal = -1;
			break;
		case -2:
			///<The operand is over range
			nRetVal = -2;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	return 0;
}

int CController::SetInstruction(UINT uBRAMLineNo, const char* lpszInstruction, USHORT usOperand)
{
	int nRetVal = m_Pattern.SetInstruction(uBRAMLineNo, lpszInstruction, usOperand);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The line number is over range
			nRetVal = -1;
			break;
		case -2:
			///<The point of the instruction is nullptr
			nRetVal = -2;
			break;
		case -3:
			///<The instruction is not supported
			nRetVal = -3;
			break;
		case -4:
			///<The operand is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CController::GetInstruction(UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize)
{
	return m_Pattern.GetInstruction(uBRAMLineNo, lpszInstruction, nBuffSize);
}

int CController::SetSaveSelectFail(UINT uRAMLineNo, BOOL bStartSave, BOOL bBRAM, BOOL bDelete /*= FALSE*/)
{
	string strParallelInstruction = "FAIL_ON";
	if (!bStartSave)
	{
		strParallelInstruction = "FAIL_OFF";
	}
	else if (bDelete)
	{
		strParallelInstruction = "";
	}
	int nRetVal = m_Pattern.SetParallelInstruction(uRAMLineNo, strParallelInstruction.c_str(), bBRAM);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The line number is over range
			nRetVal = -1;
			break;
		case -2:
			///<The instruction is nullptr, not will happen
			break;
		case -3:
			///<The instruction is not supported, not will heappen
			break;
		default:
			break;
		}
		return nRetVal;
	}
	return 0;
}

int CController::GetOperand(UINT uBRAMLineNo)
{
	return m_Pattern.GetOperand(uBRAMLineNo);
}

int CController::SetPeriod(BYTE byTimesetIndex, double dPeriod)
{
	return m_HardwareFunction.SetPeriod(byTimesetIndex, dPeriod);
}

double CController::GetPeriod(BYTE byTimesetIndex)
{
	return m_HardwareFunction.GetPeriod(byTimesetIndex);
}

int CController::SetEdge(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
{
	return m_HardwareFunction.SetEdge(vecChannel, byTimesetIndex, pdEdge,WaveFormat,IOFormat, CompareMode);
}

int CController::GetEdge(USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	return m_HardwareFunction.GetEdge(usChannel, byTimesetIndex, pdEdge, WaveFormat, IOFormat, CompareMode);
}

int CController::InitMCU(const std::vector<USHORT>& vecChannel)
{
	return m_HardwareFunction.InitMCU(vecChannel);
}

int CController::InitPMU(const std::vector<USHORT>& vecChannel)
{
	return m_HardwareFunction.InitPMU(vecChannel);
}

int CController::SetPinLevel(const std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVOH, double dVOL)
{
	return m_HardwareFunction.SetPinLevel(vecChannel, dVIH, dVIL, (dVIH + dVIL) / 2, dVOH, dVOL, 7.5, -2.5);
}

int CController::SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	return m_HardwareFunction.SetChannelStatus(vecChannel, ChannelStatus);
}

void CController::UpdateChannelMode()
{
	return m_HardwareFunction.UpdateChannelMode();
}

int CController::GetChannelStatus(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	ULONG ulStatus = m_HardwareFunction.GetChannelStatus();
	return ((ulStatus >> (usChannel + 16) & 0x01) << 1) | (ulStatus >> usChannel & 0x01);
}

int CController::SetChannelMCUMode(const std::vector<USHORT>& vecChannel)
{
	return m_HardwareFunction.SetChannelMCUMode(vecChannel);
}

int CController::GetChannelMode(USHORT usChannel)
{
	return m_HardwareFunction.GetChannelMode(usChannel);
}

void CController::SetComparedChannel(const std::vector<USHORT>& vecChannel)
{
	set<USHORT> setChannel;
	copy(vecChannel.begin(), vecChannel.end(), inserter(setChannel, setChannel.begin()));
	m_HardwareFunction.SetComparedChannel(setChannel);
}
int CController::SetRunParam(UINT uStartLineNumber, UINT uStopLineNumber, BOOL bWithDRAM, UINT uDRAMStartLine, BOOL bEnableStart)
{
	m_vecBRAMCapture.clear();
	m_vecDRAMCapture.clear();
	m_vecDRAMFailLine.clear();
	m_vecBRAMFailLine.clear();
	m_HardwareFunction.StopRun();
	return m_HardwareFunction.SetRunParameter(uStartLineNumber, uStopLineNumber, bWithDRAM, uDRAMStartLine, bEnableStart);
}

void CController::SynRun()
{
	m_HardwareFunction.SynRun();
}

void CController::EnableStart(BOOL bEnable)
{
	m_HardwareFunction.EnableStart(bEnable);
}

void CController::Stop()
{
	m_HardwareFunction.StopRun();
}

int CController::GetMCUResult(const std::vector<USHORT>& vecChannel)
{
	return m_HardwareFunction.GetMCUResult(vecChannel);
}

int CController::GetChannelResult(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	int nRetVal = m_HardwareFunction.GetChannelResult(usChannel);
	if (0 > nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			///<Not ran vector
			nRetVal = -2;
			break;
		case -3:
			///<Vector running
			nRetVal = -3;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CController::GetRunningStatus()
{
	return m_HardwareFunction.GetRunningStatus();
}

int CController::GetFailCount(USHORT usChannel)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	return m_HardwareFunction.GetChannelFailCount(usChannel);
}

int CController::GetFailCount()
{
	return m_HardwareFunction.GetFailCount();
}

int CController::GetFailLineNo(USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh)
{
	vecBRAMLineNo.clear();
	vecDRAMLineNo.clear();
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	int nRetVal = 0;
	if (bForceRefresh || ( 0 == m_vecBRAMFailLine.size() && 0 == m_vecDRAMFailLine.size()))
	{
		nRetVal = m_HardwareFunction.GetFailData(m_vecBRAMFailLine, m_vecDRAMFailLine);

		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				nRetVal = -2;
				break;
			case -2:
				nRetVal = -3;
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	vector<int>* pvecLine = nullptr;
	vector<CHardwareFunction::DATA_RESULT>* pvecFailLine = nullptr;
	int nFailLineCount = 0;
	for (int nMemIndex = 0; nMemIndex < 2; nMemIndex++)
	{
		if (0 == nMemIndex)
		{
			pvecLine = &vecBRAMLineNo;
			pvecFailLine = &m_vecBRAMFailLine;
		}
		else
		{
			pvecLine = &vecDRAMLineNo;
			pvecFailLine = &m_vecDRAMFailLine;
		}
		if (0 == pvecFailLine->size())
		{
			continue;
		}
		nFailLineCount = 0;
		for (auto& FailLine : *pvecFailLine)
		{
			if (uGetMaxFailCount <= nFailLineCount)
			{
				break;
			}
			if (0 != (FailLine.m_usData >> usChannel & 0x01))
			{
				pvecLine->push_back(FailLine.m_nLineNo);
				++nFailLineCount;
			}
		}
	}

	return 0;
}

int CController::GetMCUFailLineNo(std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh /*= FALSE*/)
{
	vecBRAMLineNo.clear();
	vecDRAMLineNo.clear();	
	int nRetVal = 0;
	if (bForceRefresh || (0 == m_vecBRAMFailLine.size() && 0 == m_vecDRAMFailLine.size()))
	{
		nRetVal = m_HardwareFunction.GetFailData(m_vecBRAMFailLine, m_vecDRAMFailLine);

		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				nRetVal = -1;
				break;
			case -2:
				nRetVal = -2;
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	for (auto& LineNo : m_vecBRAMFailLine)
	{
		vecBRAMLineNo.push_back(LineNo.m_nLineNo);
	}
	for (auto& LineNo : m_vecDRAMFailLine)
	{
		vecDRAMLineNo.push_back(LineNo.m_nLineNo);
	}
	return 0;
}

void CController::DeleteFailLine(int nBRAMDeleteCount, int nDRAMDeleteCount)
{
	auto DelteFail = [&](BOOL bBRAM)
	{
		int nDeleteCount = bBRAM ? nBRAMDeleteCount : nDRAMDeleteCount;
		auto& vecFailLine = bBRAM ? m_vecBRAMFailLine : m_vecDRAMFailLine;
		auto& bFilter = bBRAM ? m_bBRAMFilter : m_bDRAMFilter;
		int nFailCount = vecFailLine.size();
		int nFailLeft = nDeleteCount;
		for (int nFailIndex = nFailCount - 1; 0 <= nFailIndex && 0 < nFailLeft; --nFailIndex, --nFailLeft)
		{
			vecFailLine.pop_back();
		}
		if (0 != nDeleteCount)
		{
			bFilter = TRUE;
		}
	};
	DelteFail(TRUE);
	DelteFail(FALSE);
}

int CController::GetLastCertainResultLineNo(USHORT usChannel, int& nBRAMLineNo, BOOL& bBRAMLineFail, int& nDRAMLineNo, BOOL& bDRAMLineFail)
{
	nBRAMLineNo = 0;
	nDRAMLineNo = 0;
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		///<Channel number is over range
		return -1;
	}
	int nControllerFailCount = m_HardwareFunction.GetFailCount();
	if (0 == nControllerFailCount)
	{
		bBRAMLineFail = FALSE;
		bDRAMLineFail = FALSE;
		nBRAMLineNo = -1;
		nDRAMLineNo = -1;
		return 0;
	}
	if (0 == m_vecBRAMFailLine.size() && 0 == m_vecDRAMFailLine.size())
	{
		///<Not read fail line number, read it before
		PreloadFailLineNo(-1);
	}
	BOOL bBRAMFailMemoryFilled = FALSE;
	BOOL bDRAMFailMemoryFilled = FALSE;
	m_HardwareFunction.GetFailMemoryFilled(bBRAMFailMemoryFilled, bDRAMFailMemoryFilled);

	if (!m_bBRAMFilter && !bBRAMFailMemoryFilled && BRAM_MAX_SAVE_FAIL_LINE_COUNT > m_vecBRAMFailLine.size())
	{
		bBRAMLineFail = FALSE;
		nBRAMLineNo = -1;
	}
	else
	{
		m_bBRAMFilter = TRUE;
		int nFailCount = m_vecBRAMFailLine.size();
		if (0 != nFailCount)
		{
			bBRAMLineFail = m_vecBRAMFailLine[nFailCount - 1].m_usData >> usChannel & 0x01;
			nBRAMLineNo = m_vecBRAMFailLine[nFailCount - 1].m_nLineNo;
		}
	}

	if (!m_bDRAMFilter && !bDRAMFailMemoryFilled && DRAM_MAX_SAVE_FAIL_LINE_COUNT > m_vecDRAMFailLine.size())
	{
		bBRAMLineFail = FALSE;
		nDRAMLineNo = -1;
	}
	else
	{
		m_bDRAMFilter = TRUE;
		int nFailCount = m_vecDRAMFailLine.size();
		if (0 != nFailCount)
		{
			bDRAMLineFail = m_vecDRAMFailLine[nFailCount - 1].m_usData >> usChannel & 0x01;
			nDRAMLineNo = m_vecDRAMFailLine[nFailCount - 1].m_nLineNo;
	}
}
	return 0;
}

int CController::PreloadFailLineNo(UINT uGetMaxFailLineCount)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("PreloadFailLineNo_%d", m_byIndex);
#endif // RECORD_TIME

	if (0 != m_vecBRAMFailLine.size() || 0 != m_vecDRAMFailLine.size())
	{
#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
#endif // RECORD_TIME

		return 0;
	}
	int nRetVal = 0;
	nRetVal = m_HardwareFunction.GetFailData(m_vecBRAMFailLine, m_vecDRAMFailLine);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			nRetVal = -1;
			break;
		case -2:
			nRetVal = -2;
			break;
		default:
			break;
		}
	}
#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
	return nRetVal;
}

void CController::ClearFailLineNo()
{
	m_vecBRAMFailLine.clear();
	m_vecDRAMFailLine.clear();
}

int CController::GetCapture(USHORT usChannel, std::vector<LINE_DATA>& vecBRAMCapture, std::vector<LINE_DATA>& vecDRAMCapture)
{
	vecBRAMCapture.clear();
	vecDRAMCapture.clear();
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	int nRetVal = 0;
	if (0 == m_vecBRAMCapture.size() && 0 == m_vecDRAMCapture.size())
	{
		nRetVal = m_HardwareFunction.GetCapture(m_vecBRAMCapture, m_vecDRAMCapture);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Not ran vector
				nRetVal = -2;
				break;
			case -2:
				///<Vector running
				nRetVal = -3;
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	LINE_DATA DataResult;

	auto CopyCapture = [&](BOOL bBRAM)
	{
		auto& vecCapture = bBRAM ? m_vecBRAMCapture : m_vecDRAMCapture;
		auto& vecChannelCapture = bBRAM ? vecBRAMCapture : vecDRAMCapture;
		for (auto& Capture : vecCapture)
		{
			DataResult.m_nLineNo = Capture.m_nLineNo;
			DataResult.m_byData = Capture.m_usData >> usChannel & 0x01;
			vecChannelCapture.push_back(DataResult);
		}
	};
	CopyCapture(TRUE);
	CopyCapture(FALSE);

	return 0;
}

int CController::GetStopLineNo()
{
	return m_HardwareFunction.GetStopLineNo();
}

ULONG CController::GetRunLineCount()
{
	return m_HardwareFunction.GetRanPatternCount();
}

int CController::SetCalibrationInfo(STS_CALINFO* pCalInfo, BYTE *pbyChannelStatus, int nArrayCount)
{
	m_bGetCalibraitonInfo = FALSE;
	if(nullptr == pCalInfo || nullptr == pbyChannelStatus)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL > nArrayCount)
	{
		return -2;
	}
	int nUpdateChannelCount = 0;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		if (pbyChannelStatus[usChannel])
		{
			++nUpdateChannelCount;
		}
	}
	CCalibrationInfo CalibrationInfo(m_HardwareFunction);
	if (DCM_CHANNELS_PER_CONTROL != nUpdateChannelCount)
	{
		CalibrationInfo.GetCalibrationInfo(0xFFFF, m_CalibrationInfo);
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			if (pbyChannelStatus[usChannel])
			{
				memcpy_s(&m_CalibrationInfo[usChannel], sizeof(STS_CALINFO), &pCalInfo[usChannel], sizeof(STS_CALINFO));
			}
		}
	}
	else
	{
		memcpy_s(m_CalibrationInfo, sizeof(m_CalibrationInfo), pCalInfo, nArrayCount * sizeof(STS_CALINFO));
	}

	int nFailChannel = CalibrationInfo.SetCalibrationInfo(m_CalibrationInfo);
	if (0 != nFailChannel)
	{
		return -3;
	}
	m_bGetCalibraitonInfo = TRUE;
	return 0;
}

int CController::GetCalibrationInfo(STS_CALINFO* pCalInfo, int nArrayCount)
{
	m_bGetCalibraitonInfo = FALSE;
	if (nullptr == pCalInfo)
	{
		return -1;
	}
	if (DCM_CHANNELS_PER_CONTROL > nArrayCount)
	{
		return -2;
	}
	int nRetVal = 0;
	CCalibrationInfo CalibrationInfo(m_HardwareFunction);
	int nFailChannel = CalibrationInfo.GetCalibrationInfo(0xFFFF, m_CalibrationInfo);
	if (0 != nFailChannel)
	{
		nRetVal = -3;
	}
	else
	{
		m_bGetCalibraitonInfo = TRUE;
	}
	memcpy_s(pCalInfo, nArrayCount * sizeof(STS_CALINFO), m_CalibrationInfo, DCM_CHANNELS_PER_CONTROL * sizeof(STS_CALINFO));
	return nRetVal;
}

int CController::GetCalibrationInfo(USHORT usChannel, STS_CALINFO& CalibrationInfo)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	int nRetVal = 0;
	if (!m_bGetCalibraitonInfo)
	{
		CCalibrationInfo Calibration(m_HardwareFunction);
		int nFailChannel = Calibration.GetCalibrationInfo(0xFFFF, m_CalibrationInfo);
		if (0 == nFailChannel)
		{
			m_bGetCalibraitonInfo = TRUE;
		}
		else if (0 != (nFailChannel >> usChannel & 0x01))
		{
			nRetVal = -2;
		}
	}

	memcpy_s(&CalibrationInfo, sizeof(STS_CALINFO), &m_CalibrationInfo[usChannel], sizeof(STS_CALINFO));

	return nRetVal;
}

int CController::SetCalibrationData(CAL_DATA *pCalibrationData, int nElementCount)
{
	return m_HardwareFunction.SetCalibrationData(pCalibrationData, nElementCount);
}

int CController::ResetCalibrationData()
{
	return m_HardwareFunction.ResetCalibrationData();
}

int CController::GetCalibrationData(CAL_DATA *pCalibrationData, int nElementCount)
{
	return m_HardwareFunction.GetCalibrationData(pCalibrationData, nElementCount);
}

int CController::ReadCalibrationData()
{
	return m_HardwareFunction.ReadCalibrationData();
}

int CController::SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow)
{
	return m_HardwareFunction.SetPMUMode(vecChannel, PMUMode, Range, dSetValue, dClmapHigh, dClampLow);
}

void CController::GetClampChannel(const std::vector<USHORT>& vecChannel, std::map<USHORT, UCHAR>& mapClampChannel)
{
	m_HardwareFunction.GetClampChannel(vecChannel, mapClampChannel);
}

int CController::EnablePMUClampFlag(const std::vector<USHORT>& vecChannel, BOOL bEnable)
{
	return m_HardwareFunction.EnablePMUClampFlag(vecChannel, bEnable);
}

int CController::SetDynamicLoad(std::vector<USHORT> vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow)
{
	return m_HardwareFunction.SetDynamicLoad(vecChannel, bEnable, dIOH, dIOL, dVTValue, dClmapHigh, dClampLow);
}

int CController::GetDynamicLoad(USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL)
{
	return m_HardwareFunction.GetDynamicLoad(usChannel, bEnable, dIOH, dIOL);
}

int CController::PMUMeasure(const std::vector<USHORT>& vecChannel, int nSampleTimes, double dSamplePeriod)
{
	return m_HardwareFunction.PMUMeasure(vecChannel, nSampleTimes, dSamplePeriod);
}

int CController::StartPMU()
{
	return m_HardwareFunction.StartPMU();
}

int CController::WaitPMUFinish()
{
	return m_HardwareFunction.WaitPMUFinish();
}

double CController::GetPMUMeasureResult(USHORT usChannel, int nSampleTimes)
{
	return m_HardwareFunction.GetPMUMeasureResult(usChannel, nSampleTimes);
}

double CController::GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange)
{
	return m_HardwareFunction.GetPMUMode(usChannel, PMUMode, PMURange);
}

int CController::SetPrereadLine(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType)
{
	for (auto& Sector : m_vecPrereadData)
	{
		if (nullptr != Sector && 0 == Sector->SetVectorInfo(uStartLine, uLineCount, MemType))
		{
			return 0;
		}
	}

	if (MAX_PREREAD_SECTOR_COUNT <= m_vecPrereadData.size())
	{
		return -1;
	}

	CChannelData* pChannelData = new CChannelData(m_HardwareFunction);
	int nRetVal = pChannelData->SetVectorInfo(uStartLine, uLineCount, MemType);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			///<The start line is over range
			nRetVal = -2;
			break;
		case -3:
			///<The line count is over range
			nRetVal = -3;
			break;
		case -4:
			///<Allocate memory fail
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	if (0 == nRetVal)
	{
		m_vecPrereadData.push_back(pChannelData);
	}
	else
	{
		delete pChannelData;
		pChannelData = nullptr;
	}

	return nRetVal;
}

int CController::SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode)
{
	return m_HardwareFunction.SetVTMode(vecChannel, dVTVoltValue, VTMode);
}

int CController::GetVTMode(USHORT usChannel, VT_MODE& VTMode)
{
	return m_HardwareFunction.GetVTMode(usChannel, VTMode);
}

CHardwareFunction *CController::GetHardwareFunction()
{
	return &m_HardwareFunction;
}

int CController::SetLineInfo(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType)
{
	int nRetVal = 0;
	CChannelData*& pChannelData = m_arrayDataWritten[(int)MemType];
	for (auto& ChannelData : m_vecPrereadData)
	{
		if (nullptr == ChannelData)
		{
			continue;
		}
		nRetVal = ChannelData->SetLineInfo(uStartLine, uLineCount, MemType);
		if (0 == nRetVal)
		{
			///<The vector has been preread
			pChannelData = ChannelData;
			return 0;
		}
	}
	if (nullptr == m_pChannelData)
	{
		m_pChannelData = new CChannelData(m_HardwareFunction);
	}
	pChannelData = m_pChannelData;
	nRetVal = pChannelData->SetLineInfo(uStartLine, uLineCount, MemType);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -2:
			///<The start line is over range
			nRetVal = -1;
			break;
		case -3:
			///<The line count is over range
			nRetVal = -2;
		case -4:
			///<Allocate memory fail
			nRetVal = -3;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	return 0;
}

int CController::SetWaveData(const std::vector<USHORT>& vecChannel, MEM_TYPE MemType, const BYTE* pbyData)
{
	int nRetVal = 0;
	CChannelData*& pChannelData = m_arrayDataWritten[(int)MemType];
	if (nullptr == pChannelData)
	{
		return -1;
	}
	for (auto usChannel : vecChannel)
	{
		nRetVal = pChannelData->SetChannelData(usChannel, MemType, pbyData);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				nRetVal = -2;
				break;
			case -2:
				nRetVal = -3;
				break;
			default:
				break;
			}
			return nRetVal;
		}
	}
	return 0;
}

int CController::WriteData()
{
	BOOL bWrite = FALSE;
	for (auto& pChannelData : m_arrayDataWritten)
	{
		if (nullptr != pChannelData)
		{
			pChannelData->Write();
			pChannelData = nullptr;
			bWrite = TRUE;
		}
	}
	if (!bWrite)
	{
		return -1;
	}
	return 0;
}

double CController::GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType)
{
	return m_HardwareFunction.GetPinLevel(usChannel, LevelType);
}

int CController::SetTMUUnitChannel(USHORT usChannel, BYTE byUnitIndex)
{
	return m_HardwareFunction.SetTMUUnitChannel(usChannel, byUnitIndex);
}

int CController::GetTMUConnectUnit(USHORT usChannel)
{
	return m_HardwareFunction.GetTMUConnectUnit(usChannel);
}

int CController::SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHolfOffNum, BYTE bySpecifiedUnit)
{
	return m_HardwareFunction.SetTMUParam(vecChannel, bRaiseTriggerEdge, uHoldOffTime, uHolfOffNum, bySpecifiedUnit);
}

int CController::GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	return m_HardwareFunction.GetTMUParameter(usChannel, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
}

int CController::GetTMUUnitParameter(BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
{
	return m_HardwareFunction.GetTMUUnitParameter(byTMUUnitIndex, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
}

int CController::TMUMeasure(const std::vector<USHORT>& vecChannel, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout)
{
	return m_HardwareFunction.TMUMeasure(vecChannel, MeasMode, uSampleNum, dTimeout);
}

int CController::GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout)
{
	return m_HardwareFunction.GetTMUMeasure(usChannel, MeasMode, uSampleNum, dTimeout);
}

double CController::GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nErrorCode)
{
	return m_HardwareFunction.GetTMUMeasureResult(usChannel, MeasType, nErrorCode);
}

int CController::SetTriggerOut(USHORT usChannel)
{
	return m_HardwareFunction.SetTriggerOut(usChannel);
}

int CController::GetFailMemoryFilled(BOOL& bBRAMFilled, BOOL& bDRAMFilled)
{
	return m_HardwareFunction.GetFailMemoryFilled(bBRAMFilled, bDRAMFilled);
}

void CController::EnableSaveSelectedFail(BOOL bEnable)
{
	m_HardwareFunction.EnableSaveSelectedFail(bEnable);
}

int CController::GetFailSynType()
{
	return m_HardwareFunction.GetFailSynType();
}

int CController::SetFailSyn(const std::map<BYTE, BYTE>& mapFailSyn)
{
	return m_HardwareFunction.SetFailSyn(mapFailSyn);
}

int CController::GetInstructionType(const char* lpszInstruction)
{
	return m_Pattern.GetInstructionType(lpszInstruction);
}

void CController::ClearPreread()
{
	for (auto& Sector : m_vecPrereadData)
	{
		if (nullptr != Sector)
		{
			delete Sector;
			Sector = nullptr;
		}
	}
	m_vecPrereadData.clear();
}
