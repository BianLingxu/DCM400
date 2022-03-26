#include "VectorInfo.h"
#include "IACVPrjPin.h"
#include "IACVInstance3.h"
#include "Controller.h"
#include <fstream>
#include <set>
using namespace std;
CVectorInfo::CVectorInfo()
{
	Reset();
}

CVectorInfo::~CVectorInfo()
{
	for (auto& VectorLine : m_mapVectorLine)
	{
		if (nullptr != VectorLine.second)
		{
			delete VectorLine.second;
			VectorLine.second = nullptr;
		}
	}
	m_mapVectorLine.clear();
}

int CVectorInfo::OpenFile(const char* lpszFileName, std::map<std::string, CPin*>& mapPin, std::map<BYTE, CTimeset*>& mapTimeset)
{
	Reset();

	if (nullptr == lpszFileName)
	{
		return -1;
	}

	int nRetVal = CheckFile(lpszFileName);
	if (0 != nRetVal)
	{
		return nRetVal;
	}
	m_usPinCount = m_pVector->GetPinCount(0);

	USHORT usSiteCount = m_pVector->GetSiteCount(0);
	if (-1 == usSiteCount)
	{
		//Vector file format is wrong.
		return -4;
	}

	IACVPrjPin::slot_ch* pSlotChannelBuff = nullptr;
	try
	{
		pSlotChannelBuff = new IACVPrjPin::slot_ch[usSiteCount];
	}
	catch (const std::exception&)
	{
		return -5;
	}

	IACVPrjPin* pPrjPin = nullptr;
	m_pVector->QueryInterface("IACVPrjPin", (void**)&pPrjPin);
	if (nullptr == pPrjPin)
	{
		return -3;
	}
	//CPin::PIN_INFO PinInfo;
	std::map<USHORT, std::string> mapPinNo;
	auto iterPinNo = mapPinNo.begin();
	char cPinProperty = 0;
	CHANNEL_INFO ChannelInfo;
	USHORT uCurPinNo = 0;
	char* lpszPinName = { 0 };
	for (int nPinIndex = 0; nPinIndex < m_usPinCount; ++nPinIndex)
	{
		uCurPinNo = m_pVector->GetPinNo(0, nPinIndex);
		m_pVector->GetPinName(0, nPinIndex, (const char**)&lpszPinName);
		cPinProperty = m_pVector->GetPinProperty(0, nPinIndex);
		CPin* pCurPin = new CPin(lpszPinName, uCurPinNo);

		pPrjPin->GetPinCh_site(nPinIndex, pSlotChannelBuff, usSiteCount);
		for (USHORT uSiteIndex = 0; uSiteIndex < usSiteCount; ++uSiteIndex)
		{
			ChannelInfo.m_bySlotNo = pSlotChannelBuff[uSiteIndex].slot;
			ChannelInfo.m_usChannel = pSlotChannelBuff[uSiteIndex].channel;
			ChannelInfo.m_usChannelID = pSlotChannelBuff[uSiteIndex].chindex;
			pCurPin->AddChannel(ChannelInfo);
		}
		mapPin.insert(std::pair<std::string, CPin*>(lpszPinName, pCurPin));
		mapPinNo.insert(std::pair<USHORT, std::string>(uCurPinNo, lpszPinName));
	}
	if (nullptr != pSlotChannelBuff)
	{
		delete[] pSlotChannelBuff;
		pSlotChannelBuff = nullptr;
	}

	int nLength = 256;
	m_pVector->GetID(m_lpszID, nLength);
	m_pVector->GetSaveMark(m_lpszSaveMark, nLength);

	//Get the timeset
	int* pusPinNo = nullptr;
	try
	{
		pusPinNo = new int[m_usPinCount];
	}
	catch (const std::exception&)
	{
		return -5;
	}
	memset(pusPinNo, 0, m_usPinCount * sizeof(int));
	USHORT uTimeSetCount = m_pVector->GetTimeSetCount(0, 0);
	double dCurPeriod = 0;
	USHORT uCurCLKSettingCount = 0;

	int nArrayLength = 0;
	double dCLKSetting[6] = { 0 };
	UCHAR ucWaveType[3] = { 0 };
	std::string strTimesetName;
	std::vector<std::string> vecPinNo;
	for (int nTimeSetIndex = 0; nTimeSetIndex < uTimeSetCount; ++nTimeSetIndex)
	{
		dCurPeriod = m_pVector->GetTimeSetRate2(0, 0, nTimeSetIndex);
		uCurCLKSettingCount = m_pVector->GetClkEdgeCount(0, 0, nTimeSetIndex);
		const char* lpszTimset = nullptr;
		if (0 == m_pVector->GetTimeSetName(0, 0, nTimeSetIndex, &lpszTimset))
		{
			strTimesetName = lpszTimset;
		}
		else
		{
			strTimesetName = "";
		}
		CTimeset* pCurTimeSet = new CTimeset(lpszTimset, nTimeSetIndex, dCurPeriod);
		CEdge Edge;
		for (int nCLKIndex = 0; nCLKIndex < uCurCLKSettingCount; ++nCLKIndex)
		{
			dCLKSetting[0] = m_pVector->GetClkEdgeClk1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[1] = m_pVector->GetClkEdgeClk2(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[2] = m_pVector->GetClkEdgeIO1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[3] = m_pVector->GetClkEdgeIO2(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[4] = m_pVector->GetClkEdgeSTB1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[5] = m_pVector->GetClkEdgeSTB2(0, 0, nTimeSetIndex, nCLKIndex);
			Edge.SetEdge(dCLKSetting);
			ucWaveType[0] = m_pVector->GetDriveType1(0, 0, nTimeSetIndex, nCLKIndex);
			ucWaveType[1] = m_pVector->GetIOWave(0, 0, nTimeSetIndex, nCLKIndex);
			ucWaveType[2] = m_pVector->GetSTBType(0, 0, nTimeSetIndex, nCLKIndex);
			WAVE_FORMAT WaveFormat = WAVE_FORMAT::NRZ;
			IO_FORMAT IOFormat =IO_FORMAT::NRZ;
			COMPARE_MODE CompareFormat = COMPARE_MODE::EDGE;
			switch (ucWaveType[0])
			{
			case 0:
				///<NRZ
				WaveFormat = WAVE_FORMAT::NRZ;
				break;
			case 1:
				///<RZ
				WaveFormat = WAVE_FORMAT::RZ;
				break;
			case 2:
				///<RO
				WaveFormat = WAVE_FORMAT::RO;
				break;
			case 8:
				///<SBL
				WaveFormat = WAVE_FORMAT::SBL;
				break;
			case 9:
				///<SBH
				WaveFormat = WAVE_FORMAT::SBH;
				break;
			case 10:
				///<SBC
				WaveFormat = WAVE_FORMAT::SBC;
				break;
			default:
				break;
			}

			switch (ucWaveType[1])
			{
			case 0:
				///<NRZ
				IOFormat = IO_FORMAT::NRZ;
				break;
			case 1:
				///<RO
				IOFormat = IO_FORMAT::RO;
				break;
			default:
				break;
			}

			switch (ucWaveType[2])
			{
			case 0:
				///<Edge mode
				CompareFormat = COMPARE_MODE::EDGE;
				break;
			case 1:
				///<Windows mode
				CompareFormat = COMPARE_MODE::WINDOW;
				break;
			default:
				break;
			}

			Edge.SetFormat(WaveFormat, IOFormat, CompareFormat);
			nArrayLength = 0;
			m_pVector->GetTimeSetPinNos(0, 0, nTimeSetIndex, nCLKIndex, nullptr, nArrayLength);
			if (0 < nArrayLength)
			{
				m_pVector->GetTimeSetPinNos(0, 0, nTimeSetIndex, nCLKIndex, pusPinNo, nArrayLength);
			}
			else
			{
				continue;
			}
			for (USHORT usChannelIndex = 0; usChannelIndex < nArrayLength; ++usChannelIndex)
			{
				iterPinNo = mapPinNo.find(pusPinNo[usChannelIndex]);
				if (mapPinNo.end() == iterPinNo)
				{
					continue;
				}

				vecPinNo.push_back(iterPinNo->second);
			}
			Edge.SetPin(vecPinNo);
			pCurTimeSet->AddEdge(Edge);
			vecPinNo.clear();
		}
		mapTimeset.insert(std::pair<BYTE, CTimeset*>(nTimeSetIndex, pCurTimeSet));
	}
	if (nullptr != pusPinNo)
	{
		delete[] pusPinNo;
		pusPinNo = nullptr;
	}
	m_nBRAMLineCount = m_pVector->GetRAMLineCount();
	m_nDRAMLineCount = m_pVector->GetSDRAMLineCount();
	m_pVector->QueryInterface("IACVFailTag", (void**)&m_pFailTag);
	return 0;
}

int CVectorInfo::ReadLine(int nStartLine, int& nReadLineCount, BOOL& bSaveBRAM, BOOL* bLastVectorCurMemory)
{
	if (m_bFileReadError)
	{
		return -1;
	}
	if (nullptr == m_pVector)
	{
		return -2;
	}
	*bLastVectorCurMemory = FALSE;
	char* lpszLabel = nullptr;
	char* lpszOperand = nullptr;
	char* lpszComment = nullptr;
	BYTE byTimeSet = 0;
	pPinPatternInfo vectorInfo = nullptr;
	try
	{
		vectorInfo = new PinPatternInfo[m_usPinCount];
	}
	catch (const std::exception&)
	{
		return -1;
	}

	IACVProject::InstructionsInfo tmuCode;
	tmuCode.code = 0;

	IACVProject::InstructionsInfo mcuCode;
	mcuCode.code = 0;
	USHORT usInsCode = 0;
	int nParallelCode = 0;
	const char* lpszParallelCode = nullptr;
	bool bCurSaveBRAM = FALSE;
	int nIndex = 0;
	DWORD dwCurLineIndex = 0;
	std::map<USHORT, CVectorLine*>::iterator iterVectorLine;
	int nParallelInsCount = 0;
	int nLineCount = m_nBRAMLineCount + m_nDRAMLineCount;
	BOOL bCapture = FALSE;
	for (nIndex = 0; nIndex < nReadLineCount + 1; ++nIndex)
	{
		//Read one more line than needed, in order to judge whether the last read line is the last line of current memory.
		bCapture = FALSE;
		dwCurLineIndex = nIndex + nStartLine;
		nParallelInsCount = 0;
		tmuCode.code = 0;
		mcuCode.code = 0;
		int nRetVal = m_pVector->GetAllVecLine(0, 0, dwCurLineIndex, (const char**)&lpszLabel, (const char**)&lpszComment, (const char**)&lpszOperand, 
			(IACVProject::PinPatternInfo*)vectorInfo, byTimeSet, usInsCode, bCurSaveBRAM, tmuCode, mcuCode);

		if (-1 == nRetVal && dwCurLineIndex == nLineCount)
		{
			//The line next to last line of the vector.
			nReadLineCount = nIndex;
			break;
		}
		else if (0 != nRetVal)
		{
			//Read the vector line fail.
			m_bFileReadError = TRUE;
			nReadLineCount = 0;
			break;
		}
		if (nullptr != m_pFailTag)
		{
			m_pFailTag->GetAllVecLineFailTag(dwCurLineIndex, nParallelCode, &lpszParallelCode);
		}
		if (0 != mcuCode.code)
		{
			bCapture = TRUE;
		}

		if (nIndex == 0)
		{
			bSaveBRAM = bCurSaveBRAM;
		}
		else if ((BOOL)bCurSaveBRAM != bSaveBRAM)
		{
			nReadLineCount = nIndex;
			*bLastVectorCurMemory = TRUE;
			break;
		}
		else if (nIndex == nReadLineCount)
		{
			break;
		}
		iterVectorLine = m_mapVectorLine.find(nIndex);
		if (m_mapVectorLine.end() != iterVectorLine)
		{
			iterVectorLine->second->SetVectorLine(vectorInfo, byTimeSet, usInsCode, nParallelCode, lpszOperand, lpszLabel, bCapture);
		}
		else
		{
			CVectorLine* vectorLine = new CVectorLine(m_usPinCount);
			vectorLine->SetVectorLine(vectorInfo, byTimeSet, usInsCode, nParallelCode, lpszOperand, lpszLabel, bCapture);
			m_mapVectorLine.insert(std::pair<USHORT, CVectorLine*>(nIndex, vectorLine));
		}
		if (0 != lpszLabel[0])
		{
			if (!bCurSaveBRAM)
			{
				//The label line can't be saved in DRAM
				m_bFileReadError = TRUE;
				break;
			}
			string strLabel = lpszLabel;
			m_mapLabel.insert(make_pair(strLabel, m_nCurBRAMIndex));
		}
		if (0 != usInsCode && 0 != lpszOperand[0] && (lpszOperand[0] < '0' || lpszOperand[0] >'9'))
		{
			INS_LABEL LabelIns;
			LabelIns.m_nLineNo = m_nCurBRAMIndex;
			LabelIns.m_strInsLabel = lpszOperand;
			m_vecInsInVector.push_back(LabelIns);
		}
		if (bSaveBRAM)
		{
			++m_nCurBRAMIndex;
		}
	}
	if (nullptr != vectorInfo)
	{
		delete[] vectorInfo;
		vectorInfo = nullptr;
	}
	if (m_bFileReadError)
	{
		return -1;
	}
	m_struBlockMsg.m_nLineCount += nReadLineCount;
	if (0 == nStartLine)
	{
		m_struBlockMsg.m_nStartLineNo = 0;
		m_struBlockMsg.m_nGlobalLineNo = 0;
	}

	if (*bLastVectorCurMemory)
	{
		auto iterBlock = m_mapBRAMBlock.begin();
		if (bSaveBRAM)
		{
			//Save BRAM block.
			m_mapBRAMBlock.insert(make_pair(m_nDRAMBlockCount, m_struBlockMsg));
			if (0 != m_nDRAMBlockCount)
			{
				//Find the start line of next DRAM block.
				iterBlock = m_mapDRAMBlock.find(m_nDRAMBlockCount - 1);
				m_struBlockMsg.m_nStartLineNo = iterBlock->second.m_nStartLineNo + iterBlock->second.m_nLineCount;
			}
		}
		else
		{
			//The previous block is in DRAM
			if (0 == m_nDRAMBlockCount)
			{
				m_struBlockMsg.m_nStartLineNo = 0;
			}
			m_mapDRAMBlock.insert(make_pair(m_nDRAMBlockCount, m_struBlockMsg));

			//Find the start line of next BRAM block.
			iterBlock = m_mapBRAMBlock.find(m_nDRAMBlockCount);
			m_struBlockMsg.m_nStartLineNo = iterBlock->second.m_nStartLineNo + iterBlock->second.m_nLineCount;

			++m_nDRAMBlockCount;
		}
		m_struBlockMsg.m_nGlobalLineNo = nReadLineCount + nStartLine;
		m_struBlockMsg.m_nLineCount = 0;
	}

	if (nStartLine + nReadLineCount == nLineCount)
	{
		//The last line of the vector.
		m_mapBRAMBlock.insert(make_pair(m_nDRAMBlockCount, m_struBlockMsg));
	}

	return 0;
}

int CVectorInfo::GetReadLine(int nLineIndex, pPinPatternInfo& pPinPattern, BYTE& byTimeSet, const char*& lpszCMD, const char*& lpszParallelCMD, const char*& cOperand, const char*& lpszLinelabel, BOOL& bCapture)
{
	map<USHORT, CVectorLine*>::iterator iterVectorLine = m_mapVectorLine.find(nLineIndex);
	if (m_mapVectorLine.end() != iterVectorLine)
	{
		USHORT usCMD = 0;
		USHORT usParallelCMD = 0;
		iterVectorLine->second->GetVectLine(pPinPattern, byTimeSet, usCMD, usParallelCMD, cOperand, lpszLinelabel, bCapture);
		lpszCMD = GetCommand(usCMD);
		lpszParallelCMD = GetParallelCommand(usParallelCMD);
	}
	else
	{
		return -1;
	}
	return 0;
}

void CVectorInfo::CloseFile()
{
	for (auto& VectorLine : m_mapVectorLine)
	{
		if (nullptr != VectorLine.second)
		{
			delete VectorLine.second;
			VectorLine.second = nullptr;
		}
	}
	m_mapVectorLine.clear();
	if (nullptr != m_pIns)
	{
		m_pIns->CloseProect(m_pVector);
	}
	m_pIns = nullptr;
	m_pVector = nullptr;
	if (nullptr != m_hDll)
	{
		FreeLibrary(m_hDll);
		m_hDll = nullptr;
	}
}

int CVectorInfo::GetBRAMLineCount()
{
	return m_nBRAMLineCount;
}

int CVectorInfo::GetDRAMLineCount()
{
	return m_nDRAMLineCount;
}

BOOL CVectorInfo::IsDebugMode()
{
	if (nullptr == m_pIns)
	{
		return FALSE;
	}
	IACVInstance3* pVectorInstance3 = nullptr;
	if (0 != m_pIns->QueryInterface("IACVInstance3", (void**)&pVectorInstance3))
	{
		return FALSE;
	}
	if (nullptr == pVectorInstance3)
	{
		return FALSE;
	}
	bool bDebugMode = false;
	pVectorInstance3->IsDebugModel(bDebugMode);
	if (!bDebugMode)
	{
		return FALSE;
	}
	return TRUE;
}

int CVectorInfo::GetDRAMBlockCount(UINT uBRAMStartLine, UINT nBRAMStopLine)
{
	if (0 == uBRAMStartLine && -1 == nBRAMStopLine)
	{
		return m_mapDRAMBlock.size();
	}
	int nBRAMStartBlockIndex = -1;
	int nBRAMStopBlockIndex = -1;
	ULONG ulCurStartLine = 0;
	ULONG ulCurStopLine = 0;
	for (auto& BRAMBlock : m_mapBRAMBlock)
	{
		ulCurStartLine = BRAMBlock.second.m_nStartLineNo;
		ulCurStopLine = BRAMBlock.second.m_nStartLineNo + BRAMBlock.second.m_nLineCount;
		if (-1 == nBRAMStartBlockIndex)
		{
			if (uBRAMStartLine >= ulCurStartLine && uBRAMStartLine <= ulCurStopLine)
			{
				nBRAMStartBlockIndex = BRAMBlock.first;
			}
		}
		if (-1 == nBRAMStopBlockIndex)
		{
			if (nBRAMStopLine >= ulCurStartLine && nBRAMStopLine <= ulCurStopLine)
			{
				nBRAMStopBlockIndex = BRAMBlock.first;
			}
		}

		if (-1 != nBRAMStartBlockIndex && -1 != nBRAMStopBlockIndex)
		{
			break;
		}
	}
	return nBRAMStopBlockIndex - nBRAMStartBlockIndex;
}

BOOL CVectorInfo::GetBRAMBlock(int nBlockIndex, UINT& uStartLineNo, UINT& uGlobalLineNo, UINT& uBlockLineCount)
{
	auto iterBlock = m_mapBRAMBlock.find(nBlockIndex);
	if (m_mapBRAMBlock.end() != iterBlock)
	{
		uStartLineNo = iterBlock->second.m_nStartLineNo;
		uGlobalLineNo = iterBlock->second.m_nGlobalLineNo;
		uBlockLineCount = iterBlock->second.m_nLineCount;
		return TRUE;
	}
	else
	{
		uStartLineNo = 0;
		uGlobalLineNo = 0;
		uBlockLineCount = 0;
		return FALSE;
	}
}

int CVectorInfo::GetLabelCount()
{
	return m_mapLabel.size();
}

BOOL CVectorInfo::GetDRAMBlock(int nBlockIndex, UINT& uStartLineNo, UINT& uGlobalLineNo, UINT& uBlockLineCount)
{
	auto iterBlock = m_mapDRAMBlock.find(nBlockIndex);
	if (m_mapDRAMBlock.end() != iterBlock)
	{
		uStartLineNo = iterBlock->second.m_nStartLineNo;
		uGlobalLineNo = iterBlock->second.m_nGlobalLineNo;
		uBlockLineCount = iterBlock->second.m_nLineCount;
		return TRUE;
	}
	else
	{
		uStartLineNo = 0;
		uGlobalLineNo = 0;
		uBlockLineCount = 0;
		return FALSE;
	}
}

int CVectorInfo::GetLabelNameWithLabelIndex(UINT uLabelIndex, std::string& strLabelName)
{
	if (uLabelIndex >= m_mapLabel.size())
	{
		return -1;
	}
	auto iterLabel = m_mapLabel.begin();
	for (int nIndex = 0; nIndex < uLabelIndex; ++nIndex)
	{
		++iterLabel;
	}
	strLabelName = iterLabel->first;
	return 0;
}

int CVectorInfo::GetLabelLineNo(const char* lpszLabel, BOOL bGlobalLine)
{
	if (nullptr == lpszLabel)
	{
		return -1;
	}
	auto iterLabel = m_mapLabel.find(lpszLabel);
	if (m_mapLabel.end() == iterLabel)
	{
		return -2;
	}
	int uLabelLineNo = iterLabel->second;
	if (bGlobalLine)
	{
		uLabelLineNo = GetGlobalLineNo(uLabelLineNo, TRUE);
	}
	return uLabelLineNo;
}

int CVectorInfo::GetGlobalLineNo(UINT uRAMLineNo, BOOL bBRAMLine)
{
	if (0 == m_nDRAMBlockCount)
	{
		return uRAMLineNo;
		return 0;
	}
	auto pmapBlock = &m_mapBRAMBlock;
	auto iterBlock = pmapBlock->begin();
	if (bBRAMLine)
	{
		pmapBlock = &m_mapBRAMBlock;
		iterBlock = m_mapBRAMBlock.begin();
	}
	else
	{
		pmapBlock = &m_mapDRAMBlock;
		iterBlock = m_mapDRAMBlock.begin();
	}

	while (pmapBlock->end() != iterBlock)
	{
		if (uRAMLineNo < iterBlock->second.m_nStartLineNo + iterBlock->second.m_nLineCount)
		{
			break;
		}
		++iterBlock;
	}
	if (pmapBlock->end() == iterBlock)
	{
		return -1;
	}
	return iterBlock->second.m_nGlobalLineNo + uRAMLineNo - iterBlock->second.m_nStartLineNo;
}

int CVectorInfo::GetDRAMOffsetGlobalLineNo(int nStartBRAMLine, int nStopBRAMLine, int nDRAMOffsetLine)
{
	int nStartLine[2] = { 0 };
	int nLineCount[2] = { 0 };
	std::map<int, LINE_BLOCK> mapLineInfo;
	int uGolbalStartLine = GetLineInfo(nStartBRAMLine, nStopBRAMLine, nStartLine, nLineCount, mapLineInfo);
	if (0 > uGolbalStartLine)
	{
		switch (uGolbalStartLine)
		{
		case -1:
			uGolbalStartLine = -1;
			break;
		case -2:
			uGolbalStartLine = -2;
			break;
		case -3:
			///<Not happen
		default:
			break;
		}
		return uGolbalStartLine;
	}

	UINT uDRAMOffset = 0;
	for (auto& LineInfo : mapLineInfo)
	{
		if (LineInfo.second.m_bBRAM)
		{
			uGolbalStartLine += LineInfo.second.m_nLineCount;
			continue;
		}
		if (nDRAMOffsetLine < LineInfo.second.m_nLineCount + uDRAMOffset)
		{
			uGolbalStartLine += nDRAMOffsetLine - uDRAMOffset;
			break;
		}
		uGolbalStartLine += LineInfo.second.m_nLineCount;
		uDRAMOffset += LineInfo.second.m_nLineCount;
	}
	return uGolbalStartLine;
}

int CVectorInfo::GetLineNo(UINT uGlobalLineNo, UINT& uLineNo)
{
	if (m_nDRAMLineCount + m_nBRAMLineCount <= uGlobalLineNo)
	{
		return -1;
	}
	if (0 == m_nDRAMBlockCount)
	{
		///<In BRAM
		uLineNo = uGlobalLineNo;
		return 1;
	}
	BOOL bBRAM = TRUE;
	
	auto iterBlock = m_mapBRAMBlock.begin();
	while (m_mapBRAMBlock.end() != iterBlock)
	{
		if (iterBlock->second.m_nGlobalLineNo > uGlobalLineNo)
		{
			bBRAM = FALSE;
			iterBlock = m_mapDRAMBlock.find(iterBlock->first - 1);
			break;
		}
		else if (iterBlock->second.m_nGlobalLineNo < uGlobalLineNo && iterBlock->second.m_nGlobalLineNo + iterBlock->second.m_nLineCount > uGlobalLineNo)
		{
			break;
		}
		++iterBlock;
	}
	if ((bBRAM && m_mapBRAMBlock.end() == iterBlock) ||(!bBRAM && m_mapDRAMBlock.end() == iterBlock))
	{
		return -1;
	}
	uLineNo = iterBlock->second.m_nStartLineNo + uGlobalLineNo - iterBlock->second.m_nGlobalLineNo;
	return bBRAM;
}

int CVectorInfo::GetLabelName(int nLabelLineNo, std::string& strLabelName)
{
	for (auto& Label : m_mapLabel)
	{
		if (Label.second == nLabelLineNo)
		{
			strLabelName = Label.first;
			return 0;
		}
	}
	return -1;
}

BOOL CVectorInfo::GetDRAMRunStartLine(UINT uStartLine, UINT uStopLine, UINT& uDRAMStartLine, int& nDRAMBlockIndex)
{
	if (0 == m_mapDRAMBlock.size())
	{
		return FALSE;
	}
	ULONG ulCurStartLine = 0;
	for (auto& BRAMBlock : m_mapBRAMBlock)
	{
		ulCurStartLine = BRAMBlock.second.m_nStartLineNo;
		if (ulCurStartLine > uStartLine && ulCurStartLine < uStopLine)
		{
			nDRAMBlockIndex = BRAMBlock.first - 1;

			uDRAMStartLine = m_mapDRAMBlock[BRAMBlock.first - 1].m_nStartLineNo;
			return TRUE;
		}
	}
	return FALSE;
}

int CVectorInfo::GetLineInfo(int nStartLine, int nStopLine, int* pnStartLine, int* pnLineCount, std::map<int, LINE_BLOCK>& mapLineInfo, BOOL bBRAMLine)
{
	mapLineInfo.clear();

	ULONG ulLineCount = m_nBRAMLineCount + m_nDRAMLineCount;

	if (nStartLine >= ulLineCount || nStopLine > ulLineCount)
	{
		return -2;
	}

	if (nullptr == pnStartLine || nullptr == pnLineCount)
	{
		return -3;
	}
	if (bBRAMLine)
	{
		pnStartLine[0] = nStartLine;
		pnLineCount[0] = nStopLine - nStartLine + 1;
		pnStartLine[1] = -1;
		pnLineCount[1] = 0;
		LINE_BLOCK CurLineBlock;
		int nGlobalStartLineNo = -1;
		for (auto& BRAMBlock : m_mapBRAMBlock)
		{
			CurLineBlock.m_nLineCount = BRAMBlock.second.m_nLineCount;
			if (nStartLine > BRAMBlock.second.m_nStartLineNo + BRAMBlock.second.m_nLineCount)
			{
				///<The stat line number is not in this block
				continue;
			}
			else if (-1 == nGlobalStartLineNo)
			{
				CurLineBlock.m_nLineCount = BRAMBlock.second.m_nLineCount - nStartLine + BRAMBlock.second.m_nStartLineNo;
				nGlobalStartLineNo = BRAMBlock.second.m_nGlobalLineNo + nStartLine - BRAMBlock.second.m_nStartLineNo;
			}
			CurLineBlock.m_bBRAM = TRUE;
			mapLineInfo.insert(make_pair(mapLineInfo.size(), CurLineBlock));

			if (nStopLine <= BRAMBlock.second.m_nStartLineNo + BRAMBlock.second.m_nLineCount)
			{
				auto riter = mapLineInfo.rbegin();
				riter->second.m_nLineCount -= BRAMBlock.second.m_nStartLineNo + BRAMBlock.second.m_nLineCount - 1 - nStopLine;
				break;
			}

			auto iterDRAMBlock = m_mapDRAMBlock.find(BRAMBlock.first);
			if ((UINT)-1 == pnStartLine[1])
			{
				pnStartLine[1] = iterDRAMBlock->second.m_nStartLineNo;
			}
			pnLineCount[1] += iterDRAMBlock->second.m_nLineCount;

			CurLineBlock.m_bBRAM = FALSE;
			CurLineBlock.m_nLineCount = iterDRAMBlock->second.m_nLineCount;
			mapLineInfo.insert(make_pair(mapLineInfo.size(), CurLineBlock));
		}

		return nGlobalStartLineNo;
	}
	memset(pnStartLine, 0, 2 * sizeof(int));
	memset(pnLineCount, 0, 2 * sizeof(int));

	BOOL bBRAMBlock = FALSE;
	auto iterSRMBlock = m_mapBRAMBlock.begin();
	auto iterDRAMBlock = m_mapDRAMBlock.find(0);
	while (m_mapBRAMBlock.end() != iterSRMBlock)
	{
		if (nStartLine >= iterSRMBlock->second.m_nGlobalLineNo && nStartLine < iterSRMBlock->second.m_nGlobalLineNo + iterSRMBlock->second.m_nLineCount)
		{
			pnStartLine[0] = iterSRMBlock->second.m_nStartLineNo + nStartLine - iterSRMBlock->second.m_nGlobalLineNo;
			iterDRAMBlock = m_mapDRAMBlock.find(iterSRMBlock->first);
			if (m_mapDRAMBlock.end() != iterDRAMBlock)
			{
				pnStartLine[1] = iterDRAMBlock->second.m_nStartLineNo;
			}
			bBRAMBlock = TRUE;
			break;
		}
		else if (nStartLine < iterSRMBlock->second.m_nGlobalLineNo)
		{
			bBRAMBlock = FALSE;
			iterDRAMBlock = m_mapDRAMBlock.find(iterSRMBlock->first - 1);
			//--iterSRMBlock;

			pnStartLine[0] = iterSRMBlock->second.m_nStartLineNo;

			pnStartLine[1] = iterDRAMBlock->second.m_nStartLineNo + nStartLine - iterDRAMBlock->second.m_nGlobalLineNo;

			break;
		}
		++iterSRMBlock;
	}

	auto piterLine = &iterSRMBlock;
	int nCurLineNo = nStartLine;
	pnLineCount[0] = 0;
	pnLineCount[1] = 0;
	int nCurBlockLine = 0;
	LINE_BLOCK LineBlock;
	while (nCurLineNo <= nStopLine)
	{
		if (bBRAMBlock)
		{
			piterLine = &iterSRMBlock;
		}
		else
		{
			piterLine = &iterDRAMBlock;
		}

		if ((*piterLine)->second.m_nGlobalLineNo >= nStartLine)
		{
			nCurBlockLine = (*piterLine)->second.m_nLineCount;
		}
		else
		{
			nCurBlockLine = (*piterLine)->second.m_nLineCount - nStartLine + (*piterLine)->second.m_nGlobalLineNo;
		}

		if (nCurBlockLine + nCurLineNo > nStopLine)
		{
			nCurBlockLine = nStopLine - nCurLineNo + 1;
		}

		LineBlock.m_nLineCount = nCurBlockLine;

		if (bBRAMBlock)
		{
			pnLineCount[0] += nCurBlockLine;
		}
		else
		{
			pnLineCount[1] += nCurBlockLine;
		}
		LineBlock.m_bBRAM = bBRAMBlock;
		mapLineInfo.insert(make_pair(mapLineInfo.size(), LineBlock));
		nCurLineNo += nCurBlockLine;
		bBRAMBlock = !bBRAMBlock;
		++(*piterLine);
	}
	return nStartLine;
}

int CVectorInfo::CombineLine(int nBRAMStartLine, int nBRAMStopLine, std::vector<LINE_DATA>& vecBRAMLine, const std::vector<LINE_DATA>& vecDRAMLine)
{
	int nStartLine[2] = { 0 };
	int nLineCount[2] = { 0 };
	std::map<int, LINE_BLOCK> mapLineInfo;
	int nStartGlobalLine = 0;
	nStartGlobalLine = GetLineInfo(nBRAMStartLine, nBRAMStopLine, nStartLine, nLineCount, mapLineInfo);
	if (0 > nStartGlobalLine)
	{
		switch (nStartGlobalLine)
		{
		case -1:
			nStartGlobalLine = -1;
			break;
		case -2:
			nStartGlobalLine = -2;
			break;
		case -3:
			///<Not happen
		default:
			break;
		}
		return nStartGlobalLine;
	}


	vector<LINE_DATA> vecTempBRAM;
	vector<LINE_DATA> vecTempDRAM;
	auto pvecTemp = &vecTempBRAM;
	LINE_DATA LineData;
	BOOL bBRAM = 0;
	UINT uTotalOffset = nStartGlobalLine;
	std::map<int, BYTE> mapCapture;
	const auto* pvecLine = &vecBRAMLine;

	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		uTotalOffset = nStartGlobalLine;
		UINT uTypeOffSet = 0;
		if (0 == nMemIndex)
		{
			pvecLine = &vecBRAMLine;
			pvecTemp = &vecTempBRAM;
			uTypeOffSet = nStartLine[0];///<The capture line of BRAM is real line number of BRAM
			bBRAM = TRUE;
		}
		else
		{
			///<The capture line of DRAM is the offset to first DRAM line after the ran
			pvecLine = &vecDRAMLine;
			pvecTemp = &vecTempDRAM;
			bBRAM = FALSE;
		}
		auto iterLineInfo = mapLineInfo.begin();
		for (auto& Line : *pvecLine)
		{
			while (mapLineInfo.end() != iterLineInfo)
			{
				if (bBRAM == iterLineInfo->second.m_bBRAM)
				{
					if (Line.m_nLineNo < iterLineInfo->second.m_nLineCount + uTypeOffSet)
					{
						LineData = Line;
						LineData.m_nLineNo += uTotalOffset - uTypeOffSet;
						pvecTemp->push_back(LineData);
						break;
					}
					uTypeOffSet += iterLineInfo->second.m_nLineCount;
				}
				uTotalOffset += iterLineInfo->second.m_nLineCount;
				++iterLineInfo;
			}
		}
	}
	if (0 == vecTempBRAM.size() || 0 == vecTempDRAM.size())
	{
		vecBRAMLine = 0 == vecTempBRAM.size() ? vecTempDRAM : vecTempBRAM;
		return 0;
	}
	vecBRAMLine.clear();

	int nDRAMLineCount = vecTempDRAM.size();
	int nLineIndex = 0;
	int nCurLineNo = 0;
	for (auto Line : vecTempBRAM)
	{
		for (; nLineIndex < nDRAMLineCount; ++nLineIndex)
		{
			nCurLineNo = vecTempDRAM[nLineIndex].m_nLineNo;
			if (Line.m_nLineNo < nCurLineNo)
			{
				break;
			}
			vecBRAMLine.push_back(vecTempDRAM[nLineIndex]);
		}
		vecBRAMLine.push_back(Line);
	}

	for (; nLineIndex < nDRAMLineCount; ++nLineIndex)
	{
		vecBRAMLine.push_back(vecTempDRAM[nLineIndex]);
	}


	return 0;
}

int CVectorInfo::CombineLine(int nBRAMStartLine, int nBRAMStopLine, const std::vector<int>& vecBRAMLine, const std::vector<int>& vecDRAMOffsetLine, std::vector<int>& vecOffsetLine)
{
	vecOffsetLine.clear();
	int nStartLine[2] = { 0 };
	int nLineCount[2] = { 0 };
	std::map<int, LINE_BLOCK> mapLineInfo;
	int nGlobalStartLine = 0;
	nGlobalStartLine = GetLineInfo(nBRAMStartLine, nBRAMStopLine, nStartLine, nLineCount, mapLineInfo);
	if (0 > nGlobalStartLine)
	{
		if (0 != nGlobalStartLine)
		{
			switch (nGlobalStartLine)
			{
			case -1:
				nGlobalStartLine = -1;
				break;
			case -2:
				nGlobalStartLine = -2;
				break;
			case -3:
				///<Not happen
			default:
				break;
			}
			return nGlobalStartLine;
		}
	}

	vector<int> vecTempBRAM;
	vector<int> vecTempDRAM;
	auto pvecTemp = &vecTempBRAM;

	BOOL bBRAM = TRUE;
	int nTypeOffSet = nStartLine[0];///<The BRAM line number is the real line of the  BRAM
	const auto* pvecLine = &vecBRAMLine;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		UINT uTotalOffset = 0;
		if (0 != nMemIndex)
		{
			nTypeOffSet = 0;///<The DRAM line number is the offset line number offset to the first DRAM line of latest ran
			pvecLine = &vecDRAMOffsetLine;
			pvecTemp = &vecTempDRAM;
			bBRAM = FALSE;
		}
		auto iterLineInfo = mapLineInfo.begin();
		for (auto& Line : *pvecLine)
		{
			while (mapLineInfo.end() != iterLineInfo)
			{
				if (bBRAM == iterLineInfo->second.m_bBRAM)
				{
					if (Line < iterLineInfo->second.m_nLineCount + nTypeOffSet)
					{
						pvecTemp->push_back(Line + uTotalOffset - nTypeOffSet);
						break;
					}
					nTypeOffSet += iterLineInfo->second.m_nLineCount;
				}
				uTotalOffset += iterLineInfo->second.m_nLineCount;
				++iterLineInfo;
			}
		}
	}
	if (0 == vecTempBRAM.size() || 0 == vecTempDRAM.size())
	{
		vecOffsetLine = 0 == vecTempBRAM.size() ? vecTempDRAM : vecTempBRAM;
		return 0;
	}

	int nDRAMLineCount = vecTempDRAM.size();
	int nLineIndex = 0;
	int nCurLineNo = 0;
	for (auto Line : vecTempBRAM)
	{
		for (; nLineIndex < nDRAMLineCount; ++nLineIndex)
		{
			nCurLineNo = vecTempDRAM[nLineIndex];
			if (Line < nCurLineNo)
			{
				break;
			}
			vecOffsetLine.push_back(nCurLineNo);
		}
		vecOffsetLine.push_back(Line);
	}

	for (; nLineIndex < nDRAMLineCount; ++nLineIndex)
	{
		vecOffsetLine.push_back(vecTempDRAM[nLineIndex]);
	}

	return 0;
}

int CVectorInfo::CombineLine(int nBRAMStartLine, int nBRAMStopLine, const std::vector<int>& vecBRAMLine, const std::vector<int>& vecDRAMOffsetLine, std::vector<LINE_INFO>& vecLineNo)
{
	vecLineNo.clear();
	int nStartLine[2] = { 0 };
	int nLineCount[2] = { 0 };
	std::map<int, LINE_BLOCK> mapLineInfo;
	int nGlobalStartLine = 0;
	nGlobalStartLine = GetLineInfo(nBRAMStartLine, nBRAMStopLine, nStartLine, nLineCount, mapLineInfo);
	if (0 > nGlobalStartLine)
	{
		if (0 != nGlobalStartLine)
		{
			switch (nGlobalStartLine)
			{
			case -1:
				nGlobalStartLine = -1;
				break;
			case -2:
				nGlobalStartLine = -2;
				break;
			case -3:
				///<Not happen
			default:
				break;
			}
			return nGlobalStartLine;
		}
	}

	vector<LINE_INFO> vecTempBRAM;
	vector<LINE_INFO> vecTempDRAM;
	auto pvecTemp = &vecTempBRAM;


	LINE_INFO LineInfo;
	BOOL bBRAM = TRUE;
	int nTypeOffSet = nStartLine[0];///<The BRAM line number is the real line of the  BRAM
	set<int> setFailLineNo;
	const auto* pvecLine = &vecBRAMLine;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		UINT uTotalOffset = 0;
		if (0 != nMemIndex)
		{
			nTypeOffSet = 0;///<The DRAM line number is the offset line number offset to the first DRAM line of latest ran
			pvecLine = &vecDRAMOffsetLine;
			pvecTemp = &vecTempDRAM;
			bBRAM = FALSE;
		}
		auto iterLineInfo = mapLineInfo.begin();
		for (auto& Line : *pvecLine)
		{
			while (mapLineInfo.end() != iterLineInfo)
			{
				if (bBRAM == iterLineInfo->second.m_bBRAM)
				{
					if (Line < iterLineInfo->second.m_nLineCount + nTypeOffSet)
					{
						OFFSET_LINE_INFO OffsetLineInfo;
						OffsetLineInfo.m_bBRAM = bBRAM;
						OffsetLineInfo.m_nOffsetLineNo = Line;
						
						LineInfo.m_nTotalLineNo = Line + uTotalOffset - nTypeOffSet;
						LineInfo.m_bBRAM = bBRAM;
						LineInfo.m_nLineNo = Line;
						pvecTemp->push_back(LineInfo);
						break;
					}
					nTypeOffSet += iterLineInfo->second.m_nLineCount;
				}
				uTotalOffset += iterLineInfo->second.m_nLineCount;
				++iterLineInfo;
			}
		}
	}
	if (0 == vecTempBRAM.size() || 0 == vecTempDRAM.size())
	{
		vecLineNo = 0 == vecTempBRAM.size() ? vecTempDRAM : vecTempBRAM;
		return 0;
	}

	int nDRAMLineCount = vecTempDRAM.size();
	int nLineIndex = 0;
	int nCurLineNo = 0;
	for (auto Line : vecTempBRAM)
	{
		for (; nLineIndex < nDRAMLineCount; ++nLineIndex)
		{
			nCurLineNo = vecTempDRAM[nLineIndex].m_nTotalLineNo;
			if (Line.m_nTotalLineNo < nCurLineNo)
			{
				break;
			}
			vecLineNo.push_back(vecTempDRAM[nLineIndex]);
		}
		vecLineNo.push_back(Line);
	}

	return 0;
}

int CVectorInfo::SplitData(const std::map<int, LINE_BLOCK>& mapLineInfo, const BYTE* pbyData, int nLineCount, BYTE* pbyBRAMData, int nBRAMDataSize, BYTE* pbyDRAMData, int nDRAMDataSize)
{
	if (0 == mapLineInfo.size())
	{
		return -1;
	}
	if (nullptr == pbyData)
	{
		return -2;
	}

	UINT uBRAMOffset = 0;
	UINT uDRAMOffset = 0;
	UINT uOffset = 0;
	UINT* puOffset = &uBRAMOffset;
	const BYTE* pbyPointData = pbyData;
	BYTE* puPointData = pbyBRAMData;
	auto iterBlock = mapLineInfo.begin();
	UINT uDataSize = 0;
	int nRetVal = 0;
	while (mapLineInfo.end() != iterBlock)
	{
		if (iterBlock->second.m_bBRAM)
		{
			puOffset = &uBRAMOffset;
			puPointData = pbyBRAMData;
			uDataSize = nBRAMDataSize;
			if (nullptr == pbyBRAMData)
			{
				nRetVal = -3;
				break;
			}
		}
		else
		{
			puOffset = &uDRAMOffset;
			puPointData = pbyDRAMData;
			uDataSize = nDRAMDataSize;
			if (nullptr == pbyDRAMData)
			{
				nRetVal = -4;
				break;
			}
		}
		if (uDataSize < iterBlock->second.m_nLineCount + *puOffset)
		{
			if (iterBlock->second.m_bBRAM)
			{
				nRetVal = -5;
			}
			else
			{
				nRetVal = -6;
			}
			break;
		}

		CopyDataBit(puPointData, *puOffset, pbyData, nLineCount, uOffset, iterBlock->second.m_nLineCount);
		*puOffset += iterBlock->second.m_nLineCount;
		uOffset += iterBlock->second.m_nLineCount;
		++iterBlock;
	}
	ShiftLastByteData(pbyBRAMData, uBRAMOffset);
	ShiftLastByteData(pbyDRAMData, uDRAMOffset);

	return nRetVal;
}

void CVectorInfo::AddLabel(const string& strLabelName, UINT uLabelLineNo)
{
	m_mapLabel.insert(make_pair(strLabelName, uLabelLineNo));
}

void CVectorInfo::AddBRAMBlock(UINT uStartLineNo, UINT uGlobalLineNo, UINT uBlockLineCount)
{
	BLOCK_MSG BlockMsg;
	BlockMsg.m_nStartLineNo = uStartLineNo;
	BlockMsg.m_nGlobalLineNo = uGlobalLineNo;
	BlockMsg.m_nLineCount = uBlockLineCount;
	int nCurBlockCount = m_mapBRAMBlock.size();
	m_mapBRAMBlock.insert(make_pair(nCurBlockCount++, BlockMsg));
}

void CVectorInfo::AddDRAMBlock(UINT uStartLineNo, UINT uGlobalLineNo, UINT uBlockLineCount)
{
	BLOCK_MSG BlockMsg;
	BlockMsg.m_nStartLineNo = uStartLineNo;
	BlockMsg.m_nGlobalLineNo = uGlobalLineNo;
	BlockMsg.m_nLineCount = uBlockLineCount;
	int nCurBlockCount = m_mapDRAMBlock.size();
	m_mapDRAMBlock.insert(make_pair(nCurBlockCount++, BlockMsg));
	++m_nDRAMBlockCount;
}

int CVectorInfo::SetLineCount(UINT uBRAMLineCount, UINT uDRAMLineCount)
{
	m_nBRAMLineCount = uBRAMLineCount;
	m_nDRAMLineCount = uDRAMLineCount;
	return 0;
}

int CVectorInfo::GetInsWithLabelCount()
{
	return m_vecInsInVector.size();
}

int CVectorInfo::GetInsWithLabelLineNo(int nInsIndex, int& nInsLabelNo)
{
	if (nInsIndex >= m_vecInsInVector.size())
	{
		return -1;
	}
	INS_LABEL LabelIns = m_vecInsInVector.at(nInsIndex);
	nInsLabelNo = LabelIns.m_nInsLineNo;
	if (-1 == nInsLabelNo)
	{
		auto iterLabel = m_mapLabel.find(LabelIns.m_strInsLabel);
		if (m_mapLabel.end() == iterLabel)
		{
			return -2;
		}
		LabelIns.m_nInsLineNo = iterLabel->second;
		nInsLabelNo = iterLabel->second;
	}
	return LabelIns.m_nLineNo;
}

const char* CVectorInfo::GetVectorFileName()
{
	return m_strFileName.c_str();
}

void CVectorInfo::GetID(char* lpszID, int nIDLengh)
{
	if (nullptr == lpszID)
	{
		return;
	}
	strcpy_s(lpszID, nIDLengh, m_lpszID);
}

void CVectorInfo::GetSaveMark(char* lpszSaveMark, int nMarkLength)
{
	if (nullptr == lpszSaveMark)
	{
		return;
	}
	strcpy_s(lpszSaveMark, nMarkLength, m_lpszSaveMark);
}

int CVectorInfo::CheckFile(const char* lpszFileName)
{
	if (nullptr == lpszFileName)
	{
		return -1;
	}
	ifstream VectorFile(lpszFileName);
	if (!VectorFile.is_open())
	{
		return -2;
	}
	VectorFile.close();


	char lpszModuleName[MAX_PATH] = { 0 };

	///<Get the full path of the VectorEditor module
	HMODULE hModule = GetModuleHandle("DCM.dll");
	GetModuleFileName(hModule, lpszModuleName, sizeof(lpszModuleName));
	string strFile = lpszModuleName;
	int nPos = strFile.rfind("\\");
	if (-1 != nPos)
	{
		strFile.erase(nPos + 1);
	}
	sprintf_s(lpszModuleName, sizeof(lpszModuleName), "%sACVectorModel.dll", strFile.c_str());
		
	m_hDll = LoadLibrary(lpszModuleName);
	if (nullptr == m_hDll)
	{
		return -3;
	}

	m_strFileName = lpszFileName;

	GETPROJECT GetProject = nullptr;
	GetProject = (GETPROJECT)GetProcAddress(m_hDll, "acvector_instance");
	if (nullptr == GetProject)
	{
		FreeLibrary(m_hDll);
		return -3;
	}
	m_pIns = GetProject();
	if (nullptr == m_pIns)
	{
		FreeLibrary(m_hDll);
		return -3;
	}
	int nSplitVersion = 0;
	bool bFormat = m_pIns->IsVectorFile(lpszFileName, m_nFileVersion, nSplitVersion);
	if (!bFormat)
	{
		m_pIns = nullptr;
		FreeLibrary(m_hDll);
		m_hDll = nullptr;
		return -4;
	}

	m_pVector = m_pIns->OpenProject(lpszFileName);
	if (nullptr == m_pVector)
	{
		FreeLibrary(m_hDll);
		m_pIns = nullptr;
		return -4;
	}
	return 0;
}

void CVectorInfo::Reset()
{
	m_strFileName.clear();
	m_nCurBRAMIndex = 0;
	m_usPinCount = 0;
	m_nFileVersion = 0;
	m_nBRAMLineCount = 0;
	m_nDRAMLineCount = 0;
	m_bFileReadError = FALSE;
	m_nDRAMBlockCount = 0;
	m_pIns = nullptr;
	m_pFailTag = nullptr;
	for (auto& VectorLine : m_mapVectorLine)
	{
		if (nullptr != VectorLine.second)
		{
			delete VectorLine.second;
			VectorLine.second = nullptr;
		}
	}
	m_mapVectorLine.clear();

	if (0 != m_mapBRAMBlock.size())
	{
		m_mapBRAMBlock.clear();
	}
	if (0 != m_mapLabel.size())
	{
		m_mapLabel.clear();
	}
	if (0 != m_mapDRAMBlock.size())
	{
		m_mapDRAMBlock.clear();
	}
	if (0 != m_vecInsInVector.size())
	{
		m_vecInsInVector.clear();
	}
	m_struBlockMsg.m_nGlobalLineNo = 0;
	m_struBlockMsg.m_nLineCount = 0;
	m_struBlockMsg.m_nStartLineNo = 0;
}

const char* CVectorInfo::GetCommand(USHORT usCMDCode)
{
	switch (usCMDCode)
	{
	case 0:
		return "INC";
		break;
	case 0x07:
		return "HLT";
		break;
	case 0x0800:
		return "CALL";
		break;
	case 0x0180:
		return "END_LOOPA";
		break;
	case 0x0100:
		return "END_LOOPB";
		break;
	case 0x0080:
		return "END_LOOPC";
		break;
	case 0x0700:
		return "FJUMP";
		break;
	case 0x0480:
		return "JUMP";
		break;
	case 0x0481:
		return "MASKF";
		break;
	case 0x0482:
		return "MJUMP";
		break;
	case 0x001:
		return "REPEAT";
		break;
	case 0x3000:
		return "RETURN";
		break;
	case 0x3001:
		return "SET_FLAGA";
		break;
	case 0x3002:
		return "SET_FLAGB";
		break;
	case 0x3003:
		return "SET_GLO";
		break;
	case 0x40:
		return "SET_LOOPA";
		break;
	case 0x38:
		return "SET_LOOPB";
		break;
	case 0x30:
		return "SET_LOOPC";
		break;
	case 0x0781:
		return "MATCH";
		break;
	case 0x03E8:
		return "SET_MCNT";
		break;
	case 0x03E9:
		return "SIG_STOP";
		break;
	case 0x3EA:
		return "TRIG_OUT";
		break;
	default:
		return nullptr;
		break;
	}
}

const char* CVectorInfo::GetParallelCommand(USHORT usParallelCMDCode)
{
	switch (usParallelCMDCode)
	{
	case 1:
		return "FAIL_ON";
		break;
	case 2:
		return "FAIL_OFF";
	default:
		return "";
		break;
	}
}

inline int CVectorInfo::CopyDataBit(BYTE* pbyDest, int nDestStartBit, const BYTE* pbySource, int nSourceBitCount, int nCopyStartBit, int nCopyBitCount)
{
	if (nullptr == pbyDest || nullptr == pbySource)
	{
		return -1;
	}
	if (nSourceBitCount < nCopyStartBit + nCopyBitCount)
	{
		return -2;
	}
	int nSourceLastByteIndex = nSourceBitCount / 8;
	BOOL bFullByte = 0 == nSourceBitCount % 8 ? TRUE : FALSE;

	int nCurByte = 0;
	int nCurBit = 0;
	for (int nBitIndex = 0; nBitIndex < nCopyBitCount; ++nBitIndex)
	{
		nCurByte = GetByteBitIndex(nCopyStartBit + nBitIndex, nCurBit, bFullByte, nSourceBitCount);
		BYTE byCurBit = pbySource[nCurByte] >> nCurBit & 0x01;

		nCurByte = GetByteBitIndex(nDestStartBit + nBitIndex, nCurBit);
		if (0 != byCurBit)
		{
			pbyDest[nCurByte] |= byCurBit << nCurBit;
		}
		else
		{
			pbyDest[nCurByte] &= ~(byCurBit << nCurBit);
		}
	}

	return 0;
}

inline int CVectorInfo::GetByteBitIndex(int nCurBit, int& nByteBit, BOOL bFullByte, int nBitCount)
{
	int nByte = nCurBit / 8;
	nByteBit = 7 - nCurBit % 8;
	if (bFullByte || -1 == nBitCount)
	{
		return nByte;
	}
	int nTotalByte = (nBitCount + 7) / 8;
	int nLastByteBitCount = nBitCount % 8;
	if (0 != nLastByteBitCount && nByte + 1 == nTotalByte)
	{
		nByteBit = nLastByteBitCount - 1 - nCurBit % 8;
	}

	return nByte;
}

inline int CVectorInfo::ShiftLastByteData(BYTE* pbyData, UINT uBitCount)
{
	if (nullptr == pbyData)
	{
		return -1;
	}
	int nLastByteBitCount = uBitCount % 8;
	if (0 == nLastByteBitCount)
	{
		return 0;
	}
	pbyData[uBitCount / 8] >>= (8 - nLastByteBitCount);
	return 0;
}

CVectorLine::CVectorLine(USHORT uPinCount)
{
	m_uPinCount = uPinCount;
	m_usCMDMSG = 0;
	m_pPinPattern = new PinPatternInfo[uPinCount];
	m_strOperand.clear();
	m_strLineLabel.clear();
	m_byTimeSet = 0;
	m_bCapture = FALSE;
}

CVectorLine::~CVectorLine()
{
	if (nullptr != m_pPinPattern)
	{
		delete m_pPinPattern;
		m_pPinPattern = nullptr;
	}
}

int CVectorLine::SetVectorLine(pPinPatternInfo pPinPattern, BYTE byTimeSet, USHORT usCMD, USHORT usParallelCMD, char* lpszOperand, char* lpszLineLabel, BOOL bCapture)
{
	m_byTimeSet = byTimeSet;
	m_usCMDMSG = usCMD;
	memcpy(m_pPinPattern, pPinPattern, m_uPinCount * sizeof(PinPatternInfo));
	m_strOperand = lpszOperand;
	m_strLineLabel = lpszLineLabel;
	m_bCapture = bCapture;
	m_usParallelCMD = usParallelCMD;
	return 0;
}

void CVectorLine::GetVectLine(pPinPatternInfo& pPinPattern, BYTE& byTimeSet, USHORT& usCMD, USHORT& usParallelCMD, const char*& cOperand, const char*& lpszLineLabel, BOOL& bCapture)
{
	pPinPattern = m_pPinPattern;
	byTimeSet = m_byTimeSet;
	usCMD = m_usCMDMSG;
	usParallelCMD = m_usParallelCMD;
	cOperand = m_strOperand.c_str();
	lpszLineLabel = m_strLineLabel.c_str();
	byTimeSet = m_byTimeSet;
	bCapture = m_bCapture;
}
