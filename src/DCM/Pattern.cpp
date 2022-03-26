#include "Pattern.h"
#include "Pattern.h"
#include "HardwareInfo.h"
using namespace std;

//#define CHECK_DATA 1

CPattern::CPattern(CHardwareFunction& HardwareFunction, CDriverAlarm* pAlarm)
{
	m_pHardwareFunction = &HardwareFunction;
	m_DefaultPatternSign = PATTERN_SIGN::PAT_X;
	m_pAlarm = pAlarm;
}

CPattern::~CPattern()
{
	Reset();
}

void CPattern::SetDefaultControlData(USHORT usCMD, USHORT usOperand)
{
	m_DefaultControlData.m_usCMD = usCMD;
	m_DefaultControlData.m_usOperand = usOperand;
}

int CPattern::SetDefaultPattern(PATTERN_SIGN DefaultPattern)
{
	switch (DefaultPattern)
	{
	case CPattern::PATTERN_SIGN::PAT_0:
		break;
	case CPattern::PATTERN_SIGN::PAT_1:
		break;
	case CPattern::PATTERN_SIGN::PAT_H:
		break;
	case CPattern::PATTERN_SIGN::PAT_L:
		break;
	case CPattern::PATTERN_SIGN::PAT_X:
		break;
	case CPattern::PATTERN_SIGN::PAT_M:
		break;
	case CPattern::PATTERN_SIGN::PAT_V:
		break;
	case CPattern::PATTERN_SIGN::PAT_S:
		break;
	default:
		return -1;
		break;
	}
	m_DefaultPatternSign = DefaultPattern;
	return 0;
}

UINT CPattern::GetPatternCount()
{
	return m_mapBRAMLine.size() + m_mapDRAMLine.size();
}

int CPattern::AddChannelPattern(USHORT usChannel, BOOL bBRAM, UINT uPatternLineIndex, char cPattern, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, ULONG ulOperand, BOOL bCapture, BOOL bSwitch)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (bBRAM && DCM_BRAM_PATTERN_LINE_COUNT <= uPatternLineIndex)
	{
		return -2;
	}
	else if (!bBRAM && DCM_DRAM_PATTERN_LINE_COUNT <= uPatternLineIndex)
	{
		return -2;
	}
	switch ((CPattern::PATTERN_SIGN)cPattern)
	{
	case CPattern::PATTERN_SIGN::PAT_0:
		break;
	case CPattern::PATTERN_SIGN::PAT_1:
		break;
	case CPattern::PATTERN_SIGN::PAT_H:
		break;
	case CPattern::PATTERN_SIGN::PAT_L:
		break;
	case CPattern::PATTERN_SIGN::PAT_X:
		break;
	case CPattern::PATTERN_SIGN::PAT_M:
		break;
	case CPattern::PATTERN_SIGN::PAT_V:
		break;
	case CPattern::PATTERN_SIGN::PAT_S:
		break;
	default:
		return -3;
		break;
	}
	if (TIMESET_COUNT <= byTimeset)
	{
		return -4;
	}
	if (nullptr == lpszCMD)
	{
		return -5;
	}
	CONTROL_DATA ControlData;
	int nRetVal = GetControlData(bBRAM, byTimeset, lpszCMD, lpszParallelCMD, ulOperand, bCapture, bSwitch, ControlData);
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			return -6;
		}
		else
		{
			return -7;
		}
	}
	
	AddChannelPattern(usChannel, uPatternLineIndex, cPattern, bBRAM, ControlData.m_usCMD, ControlData.m_usOperand);
	
	return 0;
}

int CPattern::AddChannelPattern(USHORT usChannel, UINT uPatternLineIndex, char cPattern, BOOL bBRAM, ULONG ulCMD, ULONG ulOperand)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	auto iterLine = m_mapBRAMLine.begin();
	PATTERN_CODE PatternCode;
	int nDefaultCode = PatternSign2Code(m_DefaultPatternSign);
	if (0 != (nDefaultCode >> 2 & 0x01))
	{
		PatternCode.m_usFM = 0xFFFF;
	}
	if (0 != (nDefaultCode >> 1 & 0x01))
	{
		PatternCode.m_usMM = 0xFFFF;
	}
	if (nDefaultCode & 0x01)
	{
		PatternCode.m_usIOM = 0xFFFF;
	}
	
	if (bBRAM)
	{
		if (DCM_BRAM_PATTERN_LINE_COUNT < uPatternLineIndex)
		{
			return -2;
		}
		iterLine = m_mapBRAMLine.find(uPatternLineIndex);
		if (m_mapBRAMLine.end() == iterLine)
		{
			m_mapBRAMLine.insert(make_pair(uPatternLineIndex, PatternCode));
			iterLine = m_mapBRAMLine.find(uPatternLineIndex);
		}
	}
	else
	{
		if (DCM_DRAM_PATTERN_LINE_COUNT < uPatternLineIndex)
		{
			return -2;
		}
		iterLine = m_mapDRAMLine.find(uPatternLineIndex);
		if (m_mapDRAMLine.end() == iterLine)
		{
			m_mapDRAMLine.insert(make_pair(uPatternLineIndex, PatternCode));
			iterLine = m_mapDRAMLine.find(uPatternLineIndex);
		}
	}

	int nPatternCode = PatternSign2Code(PATTERN_SIGN(cPattern));
	if (0 > nPatternCode)
	{
		return -3;
	}
	auto iterControl = m_mapBRAMControl.begin();
	if (bBRAM)
	{
		iterControl = m_mapBRAMControl.find(uPatternLineIndex);
		if (m_mapBRAMControl.end() == iterControl)
		{
			m_mapBRAMControl.insert(make_pair(uPatternLineIndex, m_DefaultControlData));
			iterControl = m_mapBRAMControl.find(uPatternLineIndex);
		}
	}
	else if (!bBRAM)
	{
		iterControl = m_mapDRAMControl.find(uPatternLineIndex);
		if (m_mapDRAMControl.end() == iterControl)
		{
			m_mapDRAMControl.insert(make_pair(uPatternLineIndex, m_DefaultControlData));
			iterControl = m_mapDRAMControl.find(uPatternLineIndex);
		}
	}
	iterControl->second.m_usCMD = (USHORT)ulCMD;
	iterControl->second.m_usOperand = (USHORT)ulOperand;

	SetValue(iterLine->second.m_usFM, usChannel, (nPatternCode >> 2) & 0x01);
	SetValue(iterLine->second.m_usMM, usChannel, (nPatternCode >> 1) & 0x01);
	SetValue(iterLine->second.m_usIOM, usChannel, nPatternCode & 0x01);

	return 0;
}

int CPattern::ParseCMD(USHORT usCMD, BOOL bBRAM, BYTE* pbyTimeset, BOOL* pbCapture, BOOL* pbSwitch, std::string& strInstr, std::string& strParallelInstr)
{
	if (nullptr == pbyTimeset || nullptr == pbCapture || nullptr == pbSwitch)
	{
		return -1;
	}
	strInstr.clear();
	strParallelInstr.clear();
	USHORT usCMDCode = usCMD & 0x1F;
	int nMaxOperand = Code2Instruction(usCMDCode, strInstr);
	
	*pbSwitch = FALSE;
	if (bBRAM)
	{
		*pbCapture = usCMD >> 10 & 0x01;
		if (0x17 == (usCMD & 0x1F))
		{
			*pbSwitch = TRUE;
		}
	}
	else
	{
		*pbCapture = usCMD >> 3 & 0x01;
		if (0 != (usCMD & 0x01))
		{
			*pbSwitch = TRUE;
		}
	}
	if (0 != (usCMD & 0x4000))
	{
		strParallelInstr = "FAIL_ON";
	}
	else if(0 != (usCMD & 0x8000))
	{
		strParallelInstr = "FAIL_OFF";
	}

	*pbyTimeset = usCMD >> 5 & 0x01F;
	return nMaxOperand;
}

int CPattern::AddPattern(UINT uPatternLineIndex, BOOL bBRAM, const char* lpszPattern, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, USHORT usOperand, BOOL bCapture, BOOL bSwitch)
{
	auto iterLine = m_mapBRAMLine.begin();
	if (bBRAM)
	{
		if (DCM_BRAM_PATTERN_LINE_COUNT < uPatternLineIndex)
		{
			return -1;
		}
		iterLine = m_mapBRAMLine.find(uPatternLineIndex);
		if (m_mapBRAMLine.end() != iterLine)
		{
			return -2;
		}
	}
	else
	{
		if (DCM_DRAM_PATTERN_LINE_COUNT < uPatternLineIndex)
		{
			return -1;
		}
		iterLine = m_mapDRAMLine.find(uPatternLineIndex);
		if (m_mapDRAMLine.end() != iterLine)
		{
			return -2;
		}
	}

	if (nullptr == lpszPattern)
	{
		return -3;
	}
	if (TIMESET_COUNT <= byTimeset)
	{
		return -4;
	}
	if (nullptr == lpszCMD)
	{
		return -5;
	}
	CONTROL_DATA ControlData;
	int nRetVal = GetControlData(bBRAM, byTimeset, lpszCMD, lpszParallelCMD, usOperand, bCapture, bSwitch, ControlData);
	if (0 != nRetVal)
	{
		if (-1 == nRetVal)
		{
			return -6;
		}
		return -7;
	}
	nRetVal = AddPattern(uPatternLineIndex, bBRAM, lpszPattern, ControlData.m_usCMD, ControlData.m_usOperand);
	if (0 != nRetVal)
	{
		return -8;
	}
	return nRetVal;
}

int CPattern::AddPattern(UINT uPatternLineIndex, BOOL bBRAM, const char* lpszPattern, USHORT usCMD, USHORT usOperand)
{
	UINT uMaxLine = DCM_BRAM_PATTERN_LINE_COUNT;
	auto pmapLine = &m_mapBRAMLine;
	auto pmapControl = &m_mapBRAMControl;
	if (!bBRAM)
	{
		uMaxLine = DCM_DRAM_PATTERN_LINE_COUNT;
		pmapLine = &m_mapDRAMLine;
		pmapControl = &m_mapDRAMControl;
	}

	if (uMaxLine <= uPatternLineIndex)
	{
		return -1;
	}
	auto iterLine = pmapLine->find(uPatternLineIndex);
	if (pmapLine->end() != iterLine)
	{
		return -2;
	}
	if (nullptr == lpszPattern)
	{
		return -3;
	}

	int nCurCode = 0;
	PATTERN_CODE PatternCode;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		nCurCode = PatternSign2Code(PATTERN_SIGN(lpszPattern[usChannel]));
		if (0 > nCurCode)
		{
			return -4;
		}
		PatternCode.m_usFM |= ((nCurCode >> 2) & 0x01) << usChannel;
		PatternCode.m_usMM |= ((nCurCode >> 1) & 0x01) << usChannel;
		PatternCode.m_usIOM |= (nCurCode & 0x01) << usChannel;
	}

	auto iterControl = pmapControl->find(uPatternLineIndex);
	if (pmapControl->end() == iterControl)
	{
		pmapControl->insert(make_pair(uPatternLineIndex, m_DefaultControlData));
		iterControl = pmapControl->find(uPatternLineIndex);
	}
	iterControl->second.m_usCMD = usCMD;
	iterControl->second.m_usOperand = usOperand;
		
	pmapLine->insert(make_pair(uPatternLineIndex, PatternCode));

	return 0;
}

int CPattern::Load()
{
	if (0 == m_mapBRAMLine.size() && 0 == m_mapDRAMLine.size())
	{
		return 0;
	}

	USHORT* pusFM = nullptr;
	USHORT* pusMM = nullptr;
	USHORT* pusIOM = nullptr;
	USHORT* pusCMD = nullptr;
	USHORT* pusOperand = nullptr;

	UINT uBRAMWriteBaseLine = 0;
	UINT uBRAMLineCount = 0;
	UINT uDRAMWriteBaseLine = 0;
	UINT uDRAMLineCount = 0;

	auto pmapLine = &m_mapBRAMLine;
	auto pmapControl = &m_mapBRAMControl;
	auto iterControl = m_mapBRAMControl.begin();
	auto riterLine = m_mapBRAMLine.rbegin();
	UINT* puPatternLineCount = &uBRAMLineCount;
	UINT* puBaseLine = &uBRAMWriteBaseLine;
	UINT* puLineCount = &uBRAMLineCount;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			pmapControl = &m_mapBRAMControl;
			pmapLine = &m_mapBRAMLine;
			puPatternLineCount = &uBRAMLineCount;
			puBaseLine = &uBRAMWriteBaseLine;
			puLineCount = &uBRAMLineCount;
		}
		else
		{
			pmapControl = &m_mapDRAMControl;
			pmapLine = &m_mapDRAMLine;
			puPatternLineCount = &uDRAMLineCount;
			puBaseLine = &uDRAMWriteBaseLine;
			puLineCount = &uDRAMLineCount;
		}
		if (0 == pmapLine->size())
		{
			continue;
		}
		*puBaseLine = pmapLine->begin()->first;
		riterLine = pmapLine->rbegin();
		if (pmapLine->rend() != riterLine)
		{
			*puLineCount = riterLine->first - *puBaseLine + 1;
		}
	}

	UINT uMaxLineCount = uBRAMLineCount > uDRAMLineCount ? uBRAMLineCount : uDRAMLineCount;

	try
	{
		pusFM = new USHORT[uMaxLineCount];
		memset(pusFM, 0, uMaxLineCount * sizeof(USHORT));
		pusMM = new USHORT[uMaxLineCount];
		memset(pusMM, 0, uMaxLineCount * sizeof(USHORT));
		pusIOM = new USHORT[uMaxLineCount];
		memset(pusIOM, 0, uMaxLineCount * sizeof(USHORT));

		pusCMD = new USHORT[uMaxLineCount];
		memset(pusCMD, 0, uMaxLineCount * sizeof(USHORT));
		if (0 != uBRAMLineCount)
		{
			pusOperand = new USHORT[uMaxLineCount];
			memset(pusOperand, 0, uMaxLineCount * sizeof(USHORT));
		}
	}
	catch (const std::exception&)
	{
		return -1;
	}

	PATTERN_CODE DefaultCode;
	int nPatternCode = PatternSign2Code(m_DefaultPatternSign);
	for (USHORT uChannel = 0; uChannel < DCM_CHANNELS_PER_CONTROL; ++uChannel)
	{
		DefaultCode.m_usFM |= ((nPatternCode >> 2) & 0x01) << uChannel;
		DefaultCode.m_usMM |= ((nPatternCode >> 1) & 0x01) << uChannel;
		DefaultCode.m_usIOM |= (nPatternCode & 0x01) << uChannel;
	}

	const PATTERN_CODE* pPatternCode = nullptr;
	const CONTROL_DATA* pControlData = nullptr;

	UINT uWriteBaseLine = uBRAMWriteBaseLine;
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	UINT uLineCount = 0;
	auto iterLine = m_mapBRAMLine.begin();
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			pmapControl = &m_mapBRAMControl;
			pmapLine = &m_mapBRAMLine;
			uWriteBaseLine = uBRAMWriteBaseLine;
			MemType = MEM_TYPE::BRAM;
			uLineCount = uBRAMLineCount;
		}
		else
		{
			pmapControl = &m_mapDRAMControl;
			pmapLine = &m_mapDRAMLine;
			uWriteBaseLine = uDRAMWriteBaseLine;
			MemType = MEM_TYPE::DRAM;
			uLineCount = uDRAMLineCount;
		}
		if (0 == uLineCount)
		{
			continue;
		}
		UINT uCurLineNo = 0;
		for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
		{
			uCurLineNo = uLineIndex + uWriteBaseLine;
			iterLine = pmapLine->find(uCurLineNo);
			if (pmapLine->end() == iterLine)
			{
				pPatternCode = &DefaultCode;
			}
			else
			{
				pPatternCode = &iterLine->second;
			}
			pusFM[uLineIndex] = pPatternCode->m_usFM;
			pusMM[uLineIndex] = pPatternCode->m_usMM;
			pusIOM[uLineIndex] = pPatternCode->m_usIOM;

			iterControl = pmapControl->find(uCurLineNo);
			if (pmapControl->end() == iterControl)
			{
				pControlData = &m_DefaultControlData;
			}
			else
			{
				pControlData = &iterControl->second;
			}
			pusCMD[uLineIndex] = pControlData->m_usCMD;
			if (MEM_TYPE::BRAM == MemType)
			{
				pusOperand[uLineIndex] = pControlData->m_usOperand;
			}
		}
		m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::FM, uWriteBaseLine, uLineCount, pusFM);
		m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::MM, uWriteBaseLine, uLineCount, pusMM);
		m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::IOM, uWriteBaseLine, uLineCount, pusIOM);
		m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::CMD, uWriteBaseLine, uLineCount, pusCMD);

#ifdef CHECK_DATA
		USHORT* pusData = nullptr;
		try
		{
			pusData = new USHORT[uLineCount];
			memset(pusData, 0, uLineCount * sizeof(USHORT));
			m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::CMD, uWriteBaseLine, uLineCount, pusData);
			if (0 != memcmp(pusData, pusCMD, uLineCount * sizeof(USHORT)))
			{
				MessageBox(nullptr, "FAIL", "Warning", MB_OK | MB_ICONWARNING);
			}
			delete[] pusData;
			pusData = nullptr;
		}
		catch (const std::exception&)
		{

		}
#endif // CHECK_DATA


		if (MEM_TYPE::BRAM == MemType)
		{
			m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::OPERAND, uWriteBaseLine, uLineCount, pusOperand);
		}
	}
	if (nullptr != pusFM)
	{
		delete[] pusFM;
		pusFM = nullptr;
	}
	if (nullptr != pusMM)
	{
		delete[] pusMM;
		pusMM = nullptr;
	}
	if (nullptr != pusIOM)
	{
		delete[] pusIOM;
		pusIOM = nullptr;
	}
	if (nullptr != pusCMD)
	{
		delete[] pusCMD;
		pusCMD = nullptr;
	}
	if (nullptr != pusOperand)
	{
		delete[] pusOperand;
		pusFM = nullptr;
	}
	m_mapBRAMLine.clear();
	m_mapDRAMLine.clear();
	m_mapBRAMControl.clear();
	m_mapDRAMControl.clear();

	return 0;
}

int CPattern::ReadPattern(BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17])
{
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	UINT uMaxLineCount = DCM_BRAM_PATTERN_LINE_COUNT;
	if (!bBRAM)
	{
		MemType = MEM_TYPE::DRAM;
		uMaxLineCount = DCM_DRAM_PATTERN_LINE_COUNT;
	}
	if (uMaxLineCount <= uStartLine)
	{
		return -1;
	}
	else if (uMaxLineCount < uStartLine + uLineCount)
	{
		return -2;
	}
	if (nullptr == lpszPattern)
	{
		return -3;
	}
	
	USHORT* pusFM = nullptr;
	USHORT* pusMM = nullptr;
	USHORT* pusIOM = nullptr;

	UINT uLeftLine = uLineCount;
	UINT uOffset = 0;
	UINT uCurReadLineCount = 0;
	while (0 < uLeftLine)
	{
		uCurReadLineCount = uLeftLine > 2048 ? 2048 : uLeftLine;
		if (nullptr == pusFM)
		{
			try
			{
				pusFM = new USHORT[uCurReadLineCount];
				pusMM = new USHORT[uCurReadLineCount];
				pusIOM = new USHORT[uCurReadLineCount];
				memset(pusFM, 0, uCurReadLineCount * sizeof(USHORT));
				memset(pusMM, 0, uCurReadLineCount * sizeof(USHORT));
				memset(pusIOM, 0, uCurReadLineCount * sizeof(USHORT));
			}
			catch (const std::exception&)
			{
				return -4;
			}
		}

		m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::FM, uStartLine + uOffset, uCurReadLineCount, pusFM);
		m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::MM, uStartLine + uOffset, uCurReadLineCount, pusMM);
		m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::IOM, uStartLine + uOffset, uCurReadLineCount, pusIOM);
		BYTE byCode = 0;
		UINT uCurLineNo = uOffset;
		for (UINT uLineIndex = 0; uLineIndex < uCurReadLineCount; ++uLineIndex, ++uCurLineNo)
		{
			for (USHORT uChannel = 0; uChannel < DCM_CHANNELS_PER_CONTROL; ++uChannel)
			{
				byCode = 0;
				byCode |= ((pusFM[uLineIndex] >> uChannel) & 0x01) << 2;
				byCode |= ((pusMM[uLineIndex] >> uChannel) & 0x01) << 1;
				byCode |= (pusIOM[uLineIndex] >> uChannel) & 0x01;
				lpszPattern[uCurLineNo][uChannel] = Code2Pattern(byCode);
			}
			lpszPattern[uCurLineNo][DCM_CHANNELS_PER_CONTROL] = 0;
		}
		uOffset += uCurReadLineCount;
		uLeftLine -= uCurReadLineCount;
	}
	if (nullptr != pusFM)
	{
		delete[] pusFM;
		pusFM = nullptr;
	}
	if (nullptr != pusMM)
	{
		delete[] pusMM;
		pusMM = nullptr;
	}
	if (nullptr != pusIOM)
	{
		delete[] pusIOM;
		pusIOM = nullptr;
	}
	return 0;
}

int CPattern::GetPatternCode(BOOL bBRAM, CODE_TYPE CodeType, USHORT* pusPattern, UINT uElementCount)
{
	switch (CodeType)
	{
	case CPattern::CODE_TYPE::FM:
		break;
	case CPattern::CODE_TYPE::MM:
		break;
	case CPattern::CODE_TYPE::IOM:
		break;
	default:
		return -1;
		break;
	}

	if (nullptr == pusPattern || 0 == uElementCount)
	{
		return -2;
	}

	map<UINT, PATTERN_CODE>* pPaternLine = &m_mapBRAMLine;
	if (!bBRAM)
	{
		pPaternLine = &m_mapDRAMLine;
	}
	
	UINT uLineCount = uElementCount > pPaternLine->size() ? pPaternLine->size() : uElementCount;
	auto iterLine = pPaternLine->begin();
	for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
	{
		iterLine = pPaternLine->find(uLineIndex);
		if (pPaternLine->end() == iterLine)
		{
			continue;
		}
		switch (CodeType)
		{
		case CPattern::CODE_TYPE::FM:
			pusPattern[uLineIndex] = iterLine->second.m_usFM;
			break;
		case CPattern::CODE_TYPE::MM:
			pusPattern[uLineIndex] = iterLine->second.m_usMM;
			break;
		case CPattern::CODE_TYPE::IOM:
			pusPattern[uLineIndex] = iterLine->second.m_usIOM;
			break;
		default:
			break;
		}
	}

	return uLineCount;
}

void CPattern::Reset()
{
	m_mapBRAMLine.clear();
	m_mapBRAMControl.clear();
	m_mapDRAMLine.clear();
	m_mapDRAMControl.clear();
}

int CPattern::SetOperand(UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange)
{
	if (DCM_BRAM_PATTERN_LINE_COUNT <= uBRAMLineNo)
	{
		///<Line number over range
		return -1;
	}
	if (bCheckRange)
	{
		USHORT usCMD = 0;
		m_pHardwareFunction->ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, uBRAMLineNo, 1, &usCMD);

		BYTE byTimeset = 0;
		BOOL bCapture = FALSE;
		BOOL bSwitch = FALSE;
		char lpszInstruction[16] = { 0 };
		string strInstruction;
		string strParallelInstruction;
		int nMaxOperand = ParseCMD(usCMD, TRUE, &byTimeset, &bCapture, &bSwitch, strInstruction, strParallelInstruction);
		if (usOperand > nMaxOperand)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
				m_pAlarm->SetAlarmMsg("The operand(%d) is over range[1, %d].", usOperand, nMaxOperand);
			}
			return -2;
		}
	}

	m_pHardwareFunction->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::OPERAND, uBRAMLineNo, 1, &usOperand);
	return 0;
}

USHORT CPattern::GetCommand(BOOL bBRAM, BYTE byTimeset, BOOL bCapture, BOOL bSwitch)
{
	CONTROL_DATA ControlData;
	GetControlData(bBRAM, byTimeset, "INC", "", 0, bCapture, bSwitch, ControlData);
	return ControlData.m_usCMD;
}


int CPattern::SetInstruction(UINT uLineNo, const char* lpszInstruction, USHORT usOperand)
{
	if (DCM_BRAM_PATTERN_LINE_COUNT <= uLineNo)
	{
		///<Line number is over range
		return -1;
	}
	if (nullptr == lpszInstruction)
	{
		///<The instruction is nullptr
		return -2;
	}
	USHORT usCMD = 0;
	m_pHardwareFunction->ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, uLineNo, 1, &usCMD);

	BYTE byTimeset = 0;
	BOOL bCapture = FALSE;
	BOOL bSwitch = FALSE;
	char lpszOriIns[16] = { 0 };
	string strInstruction;
	string strParallelInstruction;
	ParseCMD(usCMD, TRUE, &byTimeset, &bCapture, &bSwitch, strInstruction, strParallelInstruction);

	CONTROL_DATA ControlData;
	int nRetVal = GetControlData(TRUE, byTimeset, lpszInstruction, strParallelInstruction.c_str(), usOperand, bCapture, bSwitch, ControlData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The command is not supported
			nRetVal = -3;
			break;
		case -2:
			///<The operand is over range
			nRetVal = -4;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	m_pHardwareFunction->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, uLineNo, 1, &ControlData.m_usCMD);
	m_pHardwareFunction->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::OPERAND, uLineNo, 1, &ControlData.m_usOperand);
	return 0;
}

int CPattern::GetInstruction(UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize)
{
	if (DCM_BRAM_PATTERN_LINE_COUNT <= uBRAMLineNo)
	{
		///<Line number over range
		return -1;
	}
	if (nullptr == lpszInstruction || 0 >= nBuffSize)
	{
		return -2;
	}
	if (16 > nBuffSize)
	{
		return -3;
	}
	USHORT usCMD = 0;
	m_pHardwareFunction->ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, uBRAMLineNo, 1, &usCMD);

	BYTE byTimeset = 0;
	BOOL bCapture = FALSE;
	BOOL bSwitch = FALSE;
	string strInstruction;
	string strParallelInstruction;
	int nMaxOperand = ParseCMD(usCMD, TRUE, &byTimeset, &bCapture, &bSwitch, strInstruction, strParallelInstruction);

	strcpy_s(lpszInstruction, nBuffSize, strInstruction.c_str());
	return 0;
}

int CPattern::GetOperand(UINT uBRAMLineNo)
{
	if (DCM_BRAM_PATTERN_LINE_COUNT <= uBRAMLineNo)
	{
		return -1;
	}
	USHORT usOperand = 0;
	m_pHardwareFunction->ReadDataMemory(MEM_TYPE::BRAM, DATA_TYPE::OPERAND, uBRAMLineNo, 1, &usOperand);
	return usOperand;
}

int CPattern::SetParallelInstruction(UINT uRAMLineNo, const char* lpszInstruction, BOOL bBRAM)
{
	const int nMaxLineCount = bBRAM ? DCM_BRAM_PATTERN_LINE_COUNT : DCM_DRAM_PATTERN_LINE_COUNT;
	if (nMaxLineCount >= nMaxLineCount)
	{
		return -1;
	}
	if (nullptr == lpszInstruction)
	{
		return -2;
	}
	MEM_TYPE MemType = bBRAM ? MEM_TYPE::BRAM : MEM_TYPE::DRAM;
	USHORT usCMD = 0;
	m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::CMD, uRAMLineNo, 1, &usCMD);

	BYTE byTimeset = 0;
	BOOL bCapture = FALSE;
	BOOL bSwitch = FALSE;
	string strInstruction;
	string strParallelInstruction;
	int nMaxOperand = ParseCMD(usCMD, bBRAM, &byTimeset, &bCapture, &bSwitch, strInstruction, strParallelInstruction);

	CONTROL_DATA ControlData;
	USHORT usOperand = 0;
	int nRetVal = GetControlData(bBRAM, byTimeset, lpszInstruction, strParallelInstruction.c_str(), usOperand, bCapture, bSwitch, ControlData);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The command is not supported, not will happened
			break;
		case -2:
			///<The operand is over range, not will happened
			break;
		case -3:
			///<The parallel instruction is not supported
			return -3;
		default:
			break;
		}
		return nRetVal;
	}
	m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::CMD, uRAMLineNo, 1, &ControlData.m_usCMD);
	return 0;
}

int CPattern::GetInstructionType(const char* lpszInstruction)
{
	int nInstructionType = -1;
	if (nullptr == lpszInstruction)
	{
		return -1;
	}
	USHORT usCode = 0;
	int nRetVal = Instruction2Code(lpszInstruction, usCode, &nInstructionType);
	if (0 <= nRetVal)
	{
		return nInstructionType;
	}
	nRetVal = ParallelInstruction2Code(lpszInstruction, usCode);
	if (0 <= nRetVal)
	{
		nInstructionType = 2;
	}
	else
	{
		nInstructionType = -1;
	}
	return nInstructionType;
}

int CPattern::PatternSign2Code(PATTERN_SIGN PatternSign)
{
	BYTE byCode = 0;
	switch (PatternSign)
	{
	case CPattern::PATTERN_SIGN::PAT_0:
		byCode = 1;
		break;
	case CPattern::PATTERN_SIGN::PAT_1:
		byCode = 7;
		break;
	case CPattern::PATTERN_SIGN::PAT_H:
		byCode = 6;
		break;
	case CPattern::PATTERN_SIGN::PAT_L:
		byCode = 0;
		break;
	case CPattern::PATTERN_SIGN::PAT_X:
		byCode = 5;
		break;
	case CPattern::PATTERN_SIGN::PAT_M:
		byCode = 4;
		break;
	case CPattern::PATTERN_SIGN::PAT_V:
		byCode = 2;
		break;
	case CPattern::PATTERN_SIGN::PAT_S:
		byCode = 3;
		break;
	default:
		return -1;
		break;
	}

	return byCode;
}

int CPattern::Code2Pattern(BYTE byCode)
{
	char cSign = 0;
	switch (byCode & 0x07)
	{
	case 0:
		cSign = 'L';
		break;
	case 1:
		cSign = '0';
		break;
	case 2:
		cSign = 'V';
		break;
	case 3:
		cSign = 'S';
		break;
	case 4:
		cSign = 'M';
		break;
	case 5:
		cSign = 'X';
		break;
	case 6:
		cSign = 'H';
		break;
	case 7:
		cSign = '1';
		break;
	default:
		return -1;
		break;
	}
	return cSign;
}

inline int CPattern::GetControlData(BOOL bBRAM, BYTE byTimeset, const char* lpszInstrction, const char* lpszParallelCMD, ULONG ulOperand, BOOL bCapture, BOOL bSwitch, CONTROL_DATA& ControlData)
{
	USHORT usCMD = 0;
	int nRetVal = Instruction2Code(lpszInstrction, usCMD);
	if (0 > nRetVal)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CMD_NOT_SUPPORTED);
		if (nullptr != lpszInstrction)
		{
			m_pAlarm->SetAlarmMsg("The instruction(%s) is not supported.", lpszInstrction);
		}
		else
		{
			m_pAlarm->SetAlarmMsg("The instruction is not nullptr.", lpszInstrction);
		}
		return -1;
	}
	if (ulOperand > (ULONG)nRetVal)
	{
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
			m_pAlarm->SetAlarmMsg("The operand(%d) is over range[1, %d].", ulOperand, nRetVal);
		}
		return -2;
	}
	ControlData.m_usCMD = usCMD;
	nRetVal = ParallelInstruction2Code(lpszParallelCMD, usCMD);
	if (0 != nRetVal)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CMD_NOT_SUPPORTED);
		if (nullptr != lpszParallelCMD)
		{
			m_pAlarm->SetAlarmMsg("The parallel instruction(%s) is not supported.", lpszParallelCMD);
		}
		else
		{
			m_pAlarm->SetAlarmMsg("The parallel instruction is nullptr.", lpszParallelCMD);
		}
		return -3;
	}

	ControlData.m_usOperand = (USHORT)ulOperand;

	if (bBRAM)
	{
		ControlData.m_usCMD |= usCMD;
		if (bCapture)
		{
			ControlData.m_usCMD |= 1 << 10;
		}
	}
	else
	{
		ControlData.m_usCMD = usCMD;
		if (0 == strcmp(lpszInstrction, "SIG_STOP"))
		{
			ControlData.m_usCMD |= 0x03;
		}
		else if (0 == strcmp(lpszInstrction, "TRIG_OUT"))
		{
			ControlData.m_usCMD |= 0x02;
		}
		if (bCapture)
		{
			ControlData.m_usCMD |= 1 << 3;
		}
	}
	ControlData.m_usCMD |= byTimeset << 5;

	if (bSwitch)
	{
		if (bBRAM)
		{
			ControlData.m_usCMD |= 0x17;
		}
		else
		{
			ControlData.m_usCMD |= 0x01;
		}
	}
	return 0;
}

int CPattern::Instruction2Code(const char* lpszCMDSign, USHORT& usCode, int* pnInstructionType)
{
	int nMaxOperand = 0x7FFFFFFF;
	if (nullptr != pnInstructionType)
	{
		*pnInstructionType = 0;
	}
	if (nullptr == lpszCMDSign)
	{
		return -1;
	}
	if (0 == strcmp(lpszCMDSign, "INC"))
	{
		usCode = 0;
	}
	else if (0 == strcmp(lpszCMDSign, "REPEAT"))
	{
		nMaxOperand = 65535;
		usCode = 0x01;
	}
	else if (0 == strcmp(lpszCMDSign, "JUMP"))
	{
		usCode = 0x02;
	}
	else if (0 == strcmp(lpszCMDSign, "CALL"))
	{
		usCode = 0x03;
	}
	else if (0 == strcmp(lpszCMDSign, "RETURN"))
	{
		usCode = 0x04;
	}
	else if (0 == strcmp(lpszCMDSign, "SET_LOOPA"))
	{
		nMaxOperand = 65535;
		usCode = 0x05;
	}
	else if (0 == strcmp(lpszCMDSign, "END_LOOPA"))
	{
		usCode = 0x06;
	}
	else if (0 == strcmp(lpszCMDSign, "SET_LOOPB"))
	{
		nMaxOperand = 65535;
		usCode = 0x07;
	}
	else if (0 == strcmp(lpszCMDSign, "END_LOOPB"))
	{
		usCode = 0x08;
	}
	else if (0 == strcmp(lpszCMDSign, "SET_LOOPC"))
	{
		nMaxOperand = 65535;
		usCode = 0x09;
	}
	else if (0 == strcmp(lpszCMDSign, "END_LOOPC"))
	{
		usCode = 0x0A;
	}
	else if (0 == strcmp(lpszCMDSign, "SET_MCNT"))
	{
		nMaxOperand = 65535;
		usCode = 0x0B;
	}
	else if (0 == strcmp(lpszCMDSign, "FJUMP"))
	{
		usCode = 0x0C; 
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "MJUMP"))
	{
		usCode = 0x0D;
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "MASKF"))
	{
		usCode = 0x0E;
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "SET_GLO"))
	{
		usCode = 0x0F;
	}
	else if (0 == strcmp(lpszCMDSign, "MATCH"))
	{
		usCode = 0x10;
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "SET_FAIL"))
	{
		usCode = 0x11;
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "CLR_FAIL"))
	{
		usCode = 0x12;
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = 1;
		}
	}
	else if (0 == strcmp(lpszCMDSign, "SET_FLAGA"))
	{
		usCode = 0x13;
	}
	else if (0 == strcmp(lpszCMDSign, "SET_FLAGB"))
	{
		usCode = 0x14;
	}
	else if (0 == strcmp(lpszCMDSign, "TRIG_OUT"))
	{
		usCode = 0x15;
	}
	else if (0 == strcmp(lpszCMDSign, "SIG_STOP"))
	{
		usCode = 0x16;
	}
	else if (0 == strcmp(lpszCMDSign, "RAM_OUT"))
	{
		usCode = 0x17;
	}
	else if (0 == strcmp(lpszCMDSign, "HLT"))
	{
		usCode = 0x1F;
	}
	else
	{
		///<Not supported
		if (nullptr != pnInstructionType)
		{
			*pnInstructionType = -1;
		}
		return -2;
	}
	return nMaxOperand;
}

int CPattern::ParallelInstruction2Code(const char* lpszCMDSign, USHORT& usCode)
{
	usCode = 0;
	if (nullptr == lpszCMDSign)
	{
		return -1;
	}
	if (0 == strlen(lpszCMDSign))
	{
		return 0;
	}
	if (0 == strcmp(lpszCMDSign, "FAIL_ON"))
	{
		usCode = 0x4000;
	}
	else if (0 == strcmp(lpszCMDSign, "FAIL_OFF"))
	{
		usCode = 0x8000;
	}
	else
	{
		return -2;
	}
	return 0;
}

int CPattern::Code2Instruction(USHORT usCode, std::string& strInstruction)
{
	strInstruction.clear();
	int nMaxOperand = DCM_BRAM_PATTERN_LINE_COUNT;
	switch (usCode)
	{
	case 0:
		strInstruction = "INC";
		break;
	case 1:
		strInstruction = "REPEAT";
		break;
	case 2:
		strInstruction = "JUMP";
		break;
	case 3:
		strInstruction = "CALL";
		break;
	case 4:
		strInstruction = "RETURN";
		break;
	case 5:
		strInstruction = "SET_LOOPA";
		nMaxOperand = 65535;
		break;
	case 6:
		strInstruction = "END_LOOPA";
		break;
	case 7:
		strInstruction = "SET_LOOPB";
		nMaxOperand = 65535;
		break;
	case 8:
		strInstruction = "END_LOOPB";
		break;
	case 9:
		strInstruction = "SET_LOOPC";
		nMaxOperand = 65535;
		break;
	case 0x0A:
		strInstruction = "END_LOOPC";
		break;
	case 0x0B:
		strInstruction = "SET_MCNT";
		nMaxOperand = 65535;
		break;
	case 0x0C:
		strInstruction = "FJUMP";
		break;
	case 0x0D:
		strInstruction = "MJUMP";
		break;
	case 0x0E:
		strInstruction = "MASKF";
		break;
	case 0x0F:
		strInstruction = "SET_GLO";
		break;
	case 0x10:
		strInstruction = "MATCH";
		break;
	case 0x11:
		strInstruction = "SET_FAIL";
		break;
	case 0x12:
		strInstruction = "CLR_FAIL";
		break;
	case 0x13:
		strInstruction = "SET_FLAGA";
		break;
	case 0x14:
		strInstruction = "SET_FLAGB";
		break;
	case 0x15:
		strInstruction = "TRIG_OUT";
		break;
	case 0x16:
		strInstruction = "SIG_STOP";
		break;
	case 0x17:
		strInstruction = "RAM_OUT";
		break;
	case 0x1F:
		strInstruction = "HLT";
		break;
	default:
		///<Unknown instruction
		return -2;
		break;
	}
	return nMaxOperand;
}

template<typename T>
inline void CPattern::SetValue(T& Data, int nBitIndex, bool bOne)
{
	if (bOne)
	{
		Data |= 1 << nBitIndex;
	}
	else
	{
		Data &= ~(1 << nBitIndex);
	}
}
