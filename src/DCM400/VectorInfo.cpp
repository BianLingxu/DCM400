#include "pch.h"
#include "VectorInfo.h"
#include "IACVPrjPin.h"
#include "IACVInstance3.h"
#include "IACVLabels.h"
#include <fstream>
using namespace std;
CVectorInfo::CVectorInfo(CDriverAlarm* pAlarm)
	: m_pAlarm(pAlarm)
{
	Reset();
}

CVectorInfo::~CVectorInfo()
{
	Delete(m_mapPattern);
}

int CVectorInfo::OpenFile(const char* lpszFileName, std::map<std::string, CPin*>& mapPin)
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

	IACVPrjPin* pPrjPin = nullptr;
	m_pVector->QueryInterface("IACVPrjPin", (void**)&pPrjPin);
	if (nullptr == pPrjPin)
	{
		return -3;
	}

	USHORT usSiteCount = m_pVector->GetSiteCount(0);
	if (-1 == usSiteCount)
	{
		//Vector file format is wrong.
		return -4;
	}
	m_usPinCount = m_pVector->GetPinCount(0);


	IACVLabels* pVectLabel = nullptr;
	if (0 != m_pVector->QueryInterface("IACVLabels", (void**)&pVectLabel) || nullptr == pVectLabel)
	{
		return -5;
	}

	m_mapLabel.clear();
	const char* lpszLabel = nullptr;
	int nLineNum = 0;
	int nLabelCount = pVectLabel->labelCount();
	for (int nLableIndex = 0; nLableIndex < nLabelCount;++nLableIndex)
	{
		nRetVal = pVectLabel->label(nLableIndex, (const char**)&lpszLabel, nLineNum);
		if (0 != nRetVal || nullptr == lpszLabel)
		{
			break;
		}

		m_mapLabel.insert(make_pair(lpszLabel, nLineNum));
	}

	IACVPrjPin::slot_ch* pSlotChannelBuff = nullptr;
	try
	{
		pSlotChannelBuff = new IACVPrjPin::slot_ch[usSiteCount];
	}
	catch (const std::exception&)
	{
		return -6;
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
		CPin* pSavePin = new CPin(*pCurPin);
		m_mapPinInfo.insert(std::pair<USHORT, CPin*>(uCurPinNo, pSavePin));
	}
	if (nullptr != pSlotChannelBuff)
	{
		delete[] pSlotChannelBuff;
		pSlotChannelBuff = nullptr;
	}

	int nLength = 256;
	m_pVector->GetID(m_lpszID, nLength);
	m_pVector->GetSaveMark(m_lpszSaveMark, nLength);
	m_nLineCount = m_pVector->GetRAMLineCount();
	m_nLineCount += m_pVector->GetSDRAMLineCount();
	m_pVector->QueryInterface("IACVFailTag", (void**)&m_pFailTag);
	return 0;
}

int CVectorInfo::GetTimeSet(std::map<std::string, CSeriesValue*>& mapPinSeries, std::map<std::string, CTimeSet*>& mapTimeSet)
{
	if (m_bFileReadError)
	{
		return -1;
	}
	Delete(mapPinSeries);
	Delete(mapTimeSet);

	///<Set the series index is same with time set index in current
	USHORT uTimeSetCount = m_pVector->GetTimeSetCount(0, 0);
	double dCurPeriod = 0;
	USHORT uCurCLKSettingCount = 0;
	int nArrayLength = 0;
	double dCLKSetting[6] = { 0 };
	UCHAR ucWaveType[3] = { 0 };
	std::string strTimesetName;

	//Get the timeset
	int* pnPinNo = nullptr;
	try
	{
		pnPinNo = new int[m_usPinCount];
	}
	catch (const std::exception&)
	{
		return -5;
	}

	vector<CHANNEL_INFO> vecChannelInfo;

	for (auto& Pin : m_mapPinInfo)
	{
		if (nullptr == Pin.second)
		{
			continue;
		}
		Pin.second->GetAllChannel(vecChannelInfo, TRUE);
	}

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

		///<Set the series index is same with time set index in current
		CTimeSet* pTimeSet = new CTimeSet(nTimeSetIndex);
		BYTE abySeries[EDGE_COUNT] = { 0 };
		for (auto& Series : abySeries)
		{
			Series = nTimeSetIndex;
		}
		CTimeSetSeries CurSeries;

		CurSeries.SetEdgeSeries(abySeries);
		CurSeries.SetFormatSeries(abySeries[0]);
		CurSeries.SetPeriodSeries(abySeries[0]);
		for (auto& Channel : vecChannelInfo)
		{
			pTimeSet->SetChannelSeries(Channel, CurSeries);
		}
		mapTimeSet.insert(make_pair(strTimesetName, pTimeSet));

		CSeriesValue* pSeriesValue = new CSeriesValue();

		for (int nCLKIndex = 0; nCLKIndex < uCurCLKSettingCount; ++nCLKIndex)
		{
			pSeriesValue->SetPeriod(nTimeSetIndex, dCurPeriod);

			dCLKSetting[0] = m_pVector->GetClkEdgeClk1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[1] = m_pVector->GetClkEdgeClk2(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[2] = m_pVector->GetClkEdgeIO1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[3] = m_pVector->GetClkEdgeIO2(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[4] = m_pVector->GetClkEdgeSTB1(0, 0, nTimeSetIndex, nCLKIndex);
			dCLKSetting[5] = m_pVector->GetClkEdgeSTB2(0, 0, nTimeSetIndex, nCLKIndex);
			pSeriesValue->SetEdge(nTimeSetIndex, dCLKSetting);

			ucWaveType[0] = m_pVector->GetDriveType1(0, 0, nTimeSetIndex, nCLKIndex);
			ucWaveType[1] = m_pVector->GetIOWave(0, 0, nTimeSetIndex, nCLKIndex);
			ucWaveType[2] = m_pVector->GetSTBType(0, 0, nTimeSetIndex, nCLKIndex);
			WAVE_FORMAT WaveFormat = WAVE_FORMAT::NRZ;
			IO_FORMAT IOFormat = IO_FORMAT::NRZ;
			COMPARE_MODE CompareMode = COMPARE_MODE::EDGE;
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
				CompareMode = COMPARE_MODE::EDGE;
				break;
			case 1:
				///<Windows mode
				CompareMode = COMPARE_MODE::WINDOW;
				break;
			default:
				break;
			}
			pSeriesValue->SetFormat(nTimeSetIndex, WaveFormat, IOFormat, CompareMode);

			nArrayLength = 0;
			m_pVector->GetTimeSetPinNos(0, 0, nTimeSetIndex, nCLKIndex, nullptr, nArrayLength);
			if (0 < nArrayLength)
			{
				m_pVector->GetTimeSetPinNos(0, 0, nTimeSetIndex, nCLKIndex, pnPinNo, nArrayLength);
			}
			else
			{
				continue;
			}
			BOOL bFirst = TRUE;
			CSeriesValue* pCurSeries = nullptr;
			for (USHORT usChannelIndex = 0; usChannelIndex < nArrayLength; ++usChannelIndex)
			{
				auto iterPin = m_mapPinInfo.find(pnPinNo[usChannelIndex]);
				if (m_mapPinInfo.end() == iterPin)
				{
					continue;
				}
				string strPinName = iterPin->second->GetName();
				if (bFirst)
				{
					bFirst = FALSE;
					pCurSeries = pSeriesValue;
				}
				else
				{
					pCurSeries = new CSeriesValue(*pSeriesValue);
				}
				mapPinSeries.insert(make_pair(strPinName, pCurSeries));
			}
			if (bFirst)
			{
				if (nullptr != pSeriesValue)
				{
					delete pSeriesValue;
					pSeriesValue = nullptr;
				}
			}
		}
	}
	if (nullptr != pnPinNo)
	{
		delete[] pnPinNo;
		pnPinNo = nullptr;
	}
	return 0;
}

int CVectorInfo::GetLabel(std::map<std::string, UINT>& mapLabel)
{
	mapLabel.clear();
	if (m_bFileReadError)
	{
		return -1;
	}
	mapLabel = m_mapLabel;
	return 0;
}

int CVectorInfo::ReadLine(UINT uStartLine, UINT& uReadLineCount)
{
	if (m_bFileReadError)
	{
		return -1;
	}
	if (nullptr == m_pVector)
	{
		return -2;
	}
	int nRetVal = 0;
	USHORT usTimeSet = 0;
	vector<CMD_INFO> vecCMD;
	CLinePattern LinePattern(m_usPinCount);

	UINT uCurLine = uStartLine - 1;
	auto iterPattern = m_mapPattern.begin();
	while (m_mapPattern.end() != iterPattern)
	{
		if (iterPattern->first >= uStartLine)
		{
			break;
		}

		while (m_mapPattern.end() != m_mapPattern.find(++uCurLine))
		{

		}
		m_mapPattern.insert(make_pair(uCurLine, iterPattern->second));
		m_mapPattern.erase(iterPattern);
	}

	auto iterData = m_mapCMDData.begin();
	while (m_mapCMDData.end() != iterData)
	{
		if (iterData->first >= uStartLine)
		{
			break;
		}
		m_mapCMDData.erase(iterData);
		iterData = m_mapCMDData.begin();
	}
	auto iterCMD = m_mapCMD.begin();
	while (m_mapCMD.end() != iterCMD)
	{
		if (iterCMD->first >= uStartLine)
		{
			break;
		}
		m_mapCMD.erase(iterCMD);
		iterCMD = m_mapCMD.begin();
	}

	vector<CMD_INFO> vecTargetCMD;
	USHORT usTargetTimeSet = 0;
	set<UINT> setLineNeed;
	auto CMDHandle = [&](UINT uCurLine, BOOL bSetGlobal)
	{
		nRetVal = ReadLine(uCurLine, usTimeSet, LinePattern, vecCMD);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<Read file error
				nRetVal = -1;
				break;
			case -2:
				///<Allocate memory fail
				nRetVal = -3;
				break;
			case -3:
				///<The vector line number is not existed
				nRetVal = 1;
				break;
			case -4:
				///<The label is not existed
				nRetVal = -5;
				break;
			default:
				break;
			}
			return nRetVal;
		}
		if (bSetGlobal)
		{
			vecTargetCMD = vecCMD;
			usTargetTimeSet = usTimeSet;
		}
		iterPattern = m_mapPattern.find(uCurLine);
		if (m_mapPattern.end() == iterPattern)
		{
			CLinePattern* pPattern = new CLinePattern(m_usPinCount);
			m_mapPattern.insert(make_pair(uCurLine, pPattern));
			iterPattern = m_mapPattern.find(uCurLine);
		}
		*iterPattern->second = LinePattern;
		iterCMD = m_mapCMD.find(uCurLine);
		if (m_mapCMD.end() == iterCMD)
		{
			CLineCMD LineCMD;
			m_mapCMD.insert(make_pair(uCurLine, LineCMD));
			iterCMD = m_mapCMD.find(uCurLine);
		}
		iterCMD->second.SetCMD(usTimeSet, vecCMD);
		return 0;
	};

	for (UINT uLineIndex = 0, uTargetLine = uStartLine; uLineIndex < uReadLineCount; ++uLineIndex, ++uTargetLine)
	{
		UINT uCurLine = uTargetLine;

		nRetVal = CMDHandle(uCurLine, TRUE);
		if (0 > nRetVal)
		{
			break;
		}
		else if(1 == nRetVal)
		{
			///<The last line
			uReadLineCount = uLineIndex;
			nRetVal = 0;
			break;
		}
		iterData = m_mapCMDData.find(uLineIndex);
		if (m_mapCMDData.end() == iterData)
		{
			CPatternCMD CMDValue(m_pAlarm);
			m_mapCMDData.insert(make_pair(uLineIndex, CMDValue));
			iterData = m_mapCMDData.find(uLineIndex);
		}
		int nLastLineCount = 0;
		do
		{
			nLastLineCount = 0;
			nRetVal = iterData->second.SetCommandInfo(uTargetLine, usTargetTimeSet, vecTargetCMD, m_mapCMD, setLineNeed);
			if (0 == nRetVal)
			{
				break;
			}
			else if (-4 != nRetVal)
			{
				break;
			}
			///<Other lines' command needed
			for (auto Line : setLineNeed)
			{
				nRetVal = CMDHandle(Line, FALSE);
				if (0 > nRetVal)
				{
					break;
				}
				else if (1 == nRetVal)
				{
					///<The last line
					nRetVal = 0;
					++nLastLineCount;
				}
			}
			if (setLineNeed.size() == nLastLineCount)
			{
				///<All line needed are all not exited
				break;
			}
			if (0 != nRetVal)
			{
				///<Read line fail
				break;
			}
		} while (TRUE);
		if (0 != nRetVal)
		{
			break;
		}
	}
	return nRetVal;
}

int CVectorInfo::ReadLine(UINT uLineNo, USHORT& usTimeSet, CLinePattern& LinePattern, std::vector<CMD_INFO>& vecCMDInfo)
{
	if (m_bFileReadError)
	{
		return -1;
	}
	vecCMDInfo.clear();
	char* lpszLabel = nullptr;
	char* lpszOperand = nullptr;
	char* lpszComment = nullptr;
	BYTE byTimeSet = 0;
	pPinPatternInfo pvectorInfo = nullptr;
	try
	{
		pvectorInfo = new PinPatternInfo[m_usPinCount];
	}
	catch (const std::exception&)
	{
		return -2;
	}

	IACVProject::InstructionsInfo TMUCode;
	TMUCode.code = 0;

	IACVProject::InstructionsInfo MCUCode;
	MCUCode.code = 0;
	USHORT usInsCode = 0;
	int nParallelCode = 0;
	const char* lpszParallelCode = nullptr;
	bool bCurSaveBRAM = FALSE;
	int nParallelInsCount = 0;
	string strLabel;
	int nRetVal = 0;
	do
	{
		//Read one more line than needed, in order to judge whether the last read line is the last line of current memory
		nRetVal = m_pVector->GetAllVecLine(0, 0, uLineNo, (const char**)&lpszLabel, (const char**)&lpszComment, (const char**)&lpszOperand,
			(IACVProject::PinPatternInfo*)pvectorInfo, byTimeSet, usInsCode, bCurSaveBRAM, TMUCode, MCUCode);
		
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The line is not existed
				nRetVal = -3;
			default:
				///<Read the vector line fail
				nRetVal = -1;
				m_bFileReadError = TRUE;
				break;
			}
			break;
		}
		if (nullptr == lpszLabel || strlen(lpszLabel))
		{
			strLabel.clear();
		}
		else
		{
			strLabel = lpszLabel;
		}
		CMD_INFO CMDInfo;

		int nCode = GetCommand(usInsCode);
		CMDInfo.m_nCode = nCode;
		CMDInfo.m_nOperand = 0;
		if (nullptr != lpszOperand)
		{
			if ('0' > lpszOperand[0] || '9' < lpszOperand[0])
			{
				if (0 != strlen(lpszOperand))
				{
					auto iterLabel = m_mapLabel.find(lpszOperand);
					if (m_mapLabel.end() == iterLabel)
					{
						///<The label is not existed
						nRetVal = -4;
					}
					if (nullptr != m_pAlarm)
					{
						m_pAlarm->SetAlarmID(ALARM_ID::ALARM_START_LABEL_ERROR);
						m_pAlarm->SetAlarmMsg("The label(%s) used in line %d is not existed.", lpszOperand, uLineNo + 1);
					}
				}
			}
			else
			{
				CMDInfo.m_nOperand = atoi(lpszOperand);
			}
		}
		vecCMDInfo.push_back(CMDInfo);

		
		if (nullptr != m_pFailTag)
		{
			m_pFailTag->GetAllVecLineFailTag(uLineNo, nParallelCode, &lpszParallelCode);

			CMDInfo.m_nCode = GetParallelCommand(nParallelCode);
			if (0 <= CMDInfo.m_nCode)
			{
				CMDInfo.m_nOperand = 0;
				vecCMDInfo.push_back(CMDInfo);
			}
		}
		if (0 != MCUCode.code)
		{
			CMDInfo.m_nCode = m_CMDCode.GetCMDCode("CAPTURE");
			CMDInfo.m_nOperand = 0;
			vecCMDInfo.push_back(CMDInfo);
		}
		for (USHORT usPin = 0; usPin < m_usPinCount;++usPin)
		{
			LinePattern.SetPattern(usPin, pvectorInfo[usPin].lpszPattern[0]);
		}
		usTimeSet = byTimeSet;
	} while (FALSE);
	if (nullptr != pvectorInfo)
	{
		delete[] pvectorInfo;
		pvectorInfo = nullptr;
	}
	return nRetVal;
}

int CVectorInfo::GetReadLine(int nLine, const CLinePattern*& pLinePattern, const CPatternCMD*& PatternCMD)
{
	if (m_bFileReadError)
	{
		return -1;
	}
	auto iterLine = m_mapPattern.find(nLine);
	if (m_mapPattern.end() == iterLine)
	{
		return -2;
	}
	pLinePattern = iterLine->second;
	auto iterCMD = m_mapCMDData.find(nLine);
	if (m_mapCMDData.end() == iterCMD)
	{
		return -3;
	}
	PatternCMD = &iterCMD->second;
	return 0;
}

int CVectorInfo::GetLineCount()
{
	return m_nLineCount;
}

void CVectorInfo::CloseFile()
{
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
	m_usPinCount = 0;
	m_nFileVersion = 0;
	m_bFileReadError = FALSE;
	m_pIns = nullptr;
	m_pFailTag = nullptr;
	m_nLineCount = 0;
}

int CVectorInfo::GetParallelCommand(USHORT usParallelCMDCode)
{
	switch (usParallelCMDCode)
	{
	case 1:
		return m_CMDCode.GetCMDCode("FAIL_ON");
		break;
	case 2:
		return m_CMDCode.GetCMDCode("FAIL_OFF");
	default:
		return -1;
		break;
	}
}

int CVectorInfo::GetCommand(USHORT usCMDCode)
{
	switch (usCMDCode)
	{
	case 0:
		return m_CMDCode.GetCMDCode("INC");
		break;
	case 0x07:
		return m_CMDCode.GetCMDCode("HLT");
		break;
	case 0x0800:
		return m_CMDCode.GetCMDCode("CALL");
		break;
	case 0x0180:
		return m_CMDCode.GetCMDCode("END_LOOPA");
		break;
	case 0x0100:
		return m_CMDCode.GetCMDCode("END_LOOPB");
		break;
	case 0x0080:
		return m_CMDCode.GetCMDCode("END_LOOPC");
		break;
	case 0x0700:
		return m_CMDCode.GetCMDCode("FJUMP");
		break;
	case 0x0480:
		return m_CMDCode.GetCMDCode("JUMP");
		break;
	case 0x0481:
		return m_CMDCode.GetCMDCode("MASKF");
		break;
	case 0x0482:
		return m_CMDCode.GetCMDCode("MJUMP");
		break;
	case 0x001:
		return m_CMDCode.GetCMDCode("REPEAT");
		break;
	case 0x3000:
		return m_CMDCode.GetCMDCode("RETURN");
		break;
	case 0x3001:
		return m_CMDCode.GetCMDCode("SET_FLAGA");
		break;
	case 0x3002:
		return m_CMDCode.GetCMDCode("SET_FLAGB");
		break;
	case 0x3003:
		return m_CMDCode.GetCMDCode("SET_GLO");
		break;
	case 0x40:
		return m_CMDCode.GetCMDCode("SET_LOOPA");
		break;
	case 0x38:
		return m_CMDCode.GetCMDCode("SET_LOOPB");
		break;
	case 0x30:
		return m_CMDCode.GetCMDCode("SET_LOOPC");
		break;
	case 0x0781:
		return m_CMDCode.GetCMDCode("MATCH");
		break;
	case 0x03E8:
		return m_CMDCode.GetCMDCode("SET_MCNT");
		break;
	case 0x03E9:
		return m_CMDCode.GetCMDCode("SIG_STOP");
		break;
	case 0x3EA:
		return m_CMDCode.GetCMDCode("TRIG_OUT");
		break;
	default:
		return -1;
		break;
	}
}

CLinePattern::CLinePattern(USHORT usPinCount)
	: m_usPinCount(usPinCount)
	, m_bValid(FALSE)
{
	m_pucPattern = new char[usPinCount];
	memset(m_pucPattern, 0, usPinCount * sizeof(char));
}

CLinePattern::~CLinePattern()
{
	m_usPinCount = 0;
	if (nullptr != m_pucPattern)
	{
		delete m_pucPattern;
		m_pucPattern = nullptr;
	}
}

CLinePattern* CLinePattern::operator=(const CLinePattern& Source)
{
	if (this == &Source)
	{
		return this;
	}
	if (nullptr != Source.m_pucPattern)
	{
		memcpy_s(m_pucPattern, m_usPinCount * sizeof(char), Source.m_pucPattern, Source.m_usPinCount * sizeof(char));
	}
	return this;
}

USHORT CLinePattern::GetPinCount()
{
	return m_usPinCount;
}

int CLinePattern::SetPattern(const char* pcPattern)
{
	if (nullptr == pcPattern)
	{
		return -1;
	}
	memcpy_s(m_pucPattern, m_usPinCount * sizeof(char), pcPattern, m_usPinCount * sizeof(char));
	return 0;
}

int CLinePattern::SetPattern(USHORT usPinIndex, char cPattern)
{
	if (m_usPinCount <= usPinIndex)
	{
		return -1;
	}
	m_pucPattern[usPinIndex] = cPattern;
	return 0;
}

const char* CLinePattern::GetLinePattern() const
{
	return m_pucPattern;
}

void CLinePattern::SetValid(BOOL bValid)
{
	m_bValid = bValid;
	if (!m_bValid)
	{
		memset(m_pucPattern, 0, m_usPinCount * sizeof(char));
	}
}

BOOL CLinePattern::IsValid()
{
	return m_bValid;
}
