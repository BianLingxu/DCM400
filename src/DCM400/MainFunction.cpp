#include "pch.h"
#include "MainFunction.h"
#include "VectorInfo.h"
#include "ConfigFile.h"
#include <fstream>
using namespace std;

//#define _BIND 1

#define BOARD_CHANNEL_BEGIN(vecChannel) {\
set<BYTE> setBoard;\
m_ClassifyBoard.GetBoard(setBoard);\
for(auto Slot : setBoard){\
	auto iterBoard = m_mapBoard.find(Slot);\
	if (m_mapBoard.end() != iterBoard){\
		m_ClassifyBoard.GetBoardChannel(Slot, vecChannel);

#define BOARD_CHANNEL_END }\
}\
}


CMainFunction::CMainFunction(CDriverAlarm* pAlarm)
	: m_pAlarm(pAlarm)
	, m_bLoadVector(FALSE)
{

}

void CMainFunction::CopyBoard(const CMainFunction& Source)
{
	if (this == &Source)
	{
		return;
	}
	for (auto& Board : m_mapBoard)
	{
		AddBoard(Board.first, Board.second->GetChannelCount(FALSE));
	}
}

void CMainFunction::SetDriverAlarm(CDriverAlarm* pAlarm)
{
	m_pAlarm = pAlarm;
}

int CMainFunction::AddBoard(BYTE bySlotNo, USHORT usChannelCount /*= 0*/)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard)
	{
		CBoard* pBoard = new CBoard(bySlotNo, m_pAlarm);
		if (0 == usChannelCount && pBoard->IsExisted())
		{
			delete pBoard;
			pBoard = nullptr;
			return -1;
		}
		m_mapBoard.insert(make_pair(bySlotNo, pBoard));

	}
	return 0;
}

int CMainFunction::GetChannelCount(BYTE bySlotNo)
{
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() == iterBoard)
	{
		return -1;
	}

	int nRetVal = iterBoard->second->GetChannelCount(FALSE);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<The board is not existed
			nRetVal = -1;
			break;
		case -2:
			///<The channel count not recorded
			nRetVal = -2;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	return nRetVal;
}

void CMainFunction::Reset()
{
	for (auto& Board : m_mapBoard)
	{
		if (nullptr != Board.second)
		{
			delete Board.second;
			Board.second = nullptr;
		}
	}
	m_mapBoard.clear();
}

void CMainFunction::GetBoardExisted(std::vector<BYTE>& vecBoard)
{
	vecBoard.clear();
	for (auto& Board : m_mapBoard)
	{
		vecBoard.push_back(Board.first);
	}
}

int CMainFunction::LoadVectorFile(const char* lpszVectorFile, BOOL bReload)
{
	ResetVector();
	if (nullptr == lpszVectorFile)
	{
		///<The point of vector file name is nullptr
		m_pAlarm->SetParamName("lpszVectorFile");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
		m_pAlarm->SetAlarmMsg("The point pointed to vector file is nullptr.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		return -1;
	}
	if (0 == m_mapBoard.size())
	{
		//No board exist;
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		m_pAlarm->SetAlarmMsg("No DCM board existed.");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		return -2;
	}
	CVectorInfo VectorInfo(m_pAlarm);
	int nRetVal = VectorInfo.OpenFile(lpszVectorFile, m_mapPin);
	if (0 != nRetVal)
	{
		m_pAlarm->SetParamName("lpszVectorFile");
		switch (nRetVal)
		{
		case -1:
			///<The point of vector file name is nullptr, checked before
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
			m_pAlarm->SetAlarmMsg("The point pointed to vector file is nullptr.");
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			nRetVal = -1;
			break;
		case -2:
			//VectorEditor can't open the file.
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The vector file(%s) is not existed.", lpszVectorFile);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OCCURALARM);
			nRetVal = -3;
			break;
		case -3:
			//Can't find VectorEditor module.
			m_pAlarm->SetAlarmMsg("Can't find the module of ACVectorModel.dll");
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			nRetVal = -4;
			break;
		case -4:
			//The format of vector file is wrong.
			m_pAlarm->SetParamName("lpszVectorFile");
			m_pAlarm->SetAlarmMsg("The format of vector file(%s) is wrong.", lpszVectorFile);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
			nRetVal = -5;
			break;
		case -5:
			///<Can't get the label of vector
			m_pAlarm->SetParamName(lpszVectorFile);
			m_pAlarm->SetAlarmMsg("Can't get the label information of the file");
			nRetVal = -6;
			break;
		case -6:
			//Allocate memory fail.
			m_pAlarm->AllocateMemoryError();
			nRetVal = -7;
			break;
		default:
			break;
		}
		return nRetVal;
	}
	if (0 == m_mapPin.size())
	{
		///<No pin
		ResetVector();
		m_pAlarm->SetParamName("lpszVectorFile");
		m_pAlarm->SetAlarmMsg("No pin defined in vector file.");
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
		return -8;
	}
	VectorInfo.GetLabel(m_mapLabel);
	map<std::string, CSeriesValue*> mapPinSeries;
	VectorInfo.GetTimeSet(mapPinSeries, m_mapTimeSet);

	nRetVal = InitSite();
	if (0 != nRetVal)
	{
		DeleteMemory(mapPinSeries);
		return -9;
	}
	vector<BYTE> vecUseSlot;
	m_Site.GetUseBoard(vecUseSlot);
	USHORT usUserSlot = vecUseSlot.size();

	set<string> setUsePin;
	for (auto& Pin : m_mapPin)
	{
		setUsePin.insert(Pin.first);
	}
	vector<USHORT> vecSite;
	BYTE byBindSlot = 0;
	BYTE byBindController = 0;
	UINT uBindSite = 0;
	set<USHORT> setBindChannel;
	USHORT usSiteCount = m_Site.GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		setBindChannel.insert(usSiteNo);
		vecSite.push_back(usSiteNo);
	}

	m_bVectorBind = FALSE;

#ifdef _BIND
	nRetVal = Bind(setUsePin, setBindChannel);
	if (0 <= nRetVal)
	{
		uBindSite = nRetVal;
		vecSite.clear();
		vecSite.push_back(uBindSite);
		m_bVectorBind = TRUE;
	}
#endif // _BIND

	///<Stop vector and disable receive total start
	map<BYTE, vector<USHORT>> mapChannel;
	auto iterBoard = m_mapBoard.begin();
	vector<string> vecPin;
	copy(setUsePin.begin(), setUsePin.end(), back_inserter(vecPin));
	GetBoardChannel(vecPin, vecSite, mapChannel);

	for (auto& Channel : mapChannel)
	{
		iterBoard = m_mapBoard.find(Channel.first);
		if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
		{
			iterBoard->second->StopVector(Channel.second);
			iterBoard->second->EnableStart(Channel.second, FALSE);
		}
	}
	mapChannel.clear();

	nRetVal = LoadVectorFileTimeset(vecSite, mapPinSeries);
	if (0 != nRetVal)
	{
		if (m_bVectorBind)
		{
			ClearBind();
		}
		DeleteMemory(mapPinSeries);
		return -10;
	}

	set<string> setFailSynPin;
	///<Load pattern

	///<Clear pin group information
// 	auto ClearPinGroupSection = [&]()
// 	{
// 		string strFile;
// 		string strSection;
// 		GetPinGroupInfoFile(strFile);
// 		GetPinGroupSection(strSection);
// 		WritePrivateProfileString(strSection.c_str(), nullptr, nullptr, strFile.c_str());
// 	};


	DeleteMemory(mapPinSeries);

	BOOL bVectorValid = FALSE;
	if (!bReload)
	{
		bVectorValid = IsVectorValid(lpszVectorFile);

		if (bVectorValid)
		{
			if (m_bVectorBind)
			{
				ClearBind();
			}
			m_bLoadVector = TRUE;
			m_strVectorFile = lpszVectorFile;
// 			ClearPinGroupSection();
// 			SetFailSyn(setFailSynPin);
			return 0;
		}
	}

	m_nVectorLineCout = VectorInfo.GetLineCount();

	if (DCM400_MAX_PATTERN_COUNT < m_nVectorLineCout)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
		m_pAlarm->SetParamName("lpszVectorFile");
		m_pAlarm->SetAlarmMsg("The vector line count(%d) in is over range[0, %d].", m_nVectorLineCout, DCM400_MAX_PATTERN_COUNT);
		m_nVectorLineCout = 0;
		return -1;
	}

	int nLeftLine = m_nVectorLineCout;
	if (VectorInfo.IsDebugMode())
	{
		if (m_bVectorBind)
		{
			ClearBind();
		}
		nLeftLine = 0;
	}
	UINT uCurReadLine = 0;
	BOOL bLoadFail = FALSE;
	CHANNEL_INFO ChannelInfo;
	int nCurPatternLine = 0;
	set<BYTE> setLoadBoard;
	BOOL bFirstLoad = TRUE;
	UINT uLineOffset = 0;

#ifdef _CHECK_VECTOR
	map<ULONG, char> mapVector;
#endif // _CHECK_VECTOR
	BOOL bNoBoard = TRUE;

	if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
	{
		m_pProgressInfo("Load vector", m_nVectorLineCout);
	}
	int nMemLineCount = 0;
	set<CMD_INFO> setCMD;

	string strLabel;
	CCMDCode CMDCode;
	set<int> setFailSynCode;
	const CLinePattern* pLinePattern = nullptr;
	const CPatternCMD* pPatternCMD = nullptr;
	CMDCode.GetConditionalCode(setFailSynCode);
	while (0 < nLeftLine)
	{
		uCurReadLine = 2048 < nLeftLine ? 2048 : nLeftLine;
		nRetVal = VectorInfo.ReadLine(uLineOffset, uCurReadLine);
		if (0 != nRetVal)
		{
			bLoadFail = TRUE;
			break;
		}

		for (UINT uPatternIndex = 0; uPatternIndex < uCurReadLine; ++uPatternIndex)
		{
			BOOL bFailSyn = FALSE;
			int nTMUOperand = -1;
			nCurPatternLine += uPatternIndex;
			VectorInfo.GetReadLine(nCurPatternLine, pLinePattern, pPatternCMD);


			for (auto& Pin : m_mapPin)
			{
				if (bFailSyn)
				{
					setFailSynPin.insert(Pin.first);
				}
				for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
				{
					if (m_bVectorBind)
					{
						usSiteNo = uBindSite;
					}
					int nID = Pin.second->GetID();
					//for (int nSerialIndex = 0; nSerialIndex < pPinPattern[nID].ucSerial; ++nSerialIndex)
					{
						Pin.second->GetChannel(usSiteNo, ChannelInfo);
						iterBoard = m_mapBoard.find(ChannelInfo.m_bySlotNo);
						if (m_mapBoard.end() == iterBoard)
						{
							continue;
						}
						bNoBoard = FALSE;
						nRetVal = iterBoard->second->SetVector(ChannelInfo.m_usChannel, nCurPatternLine, pLinePattern->GetLinePattern()[nID], *pPatternCMD);
						if (0 != nRetVal)
						{
							if (-2 == nRetVal)
							{
								///<The controller is not existed
								m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
								m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
								m_pAlarm->SetAlarmMsg("The board(S%d) is existed, but the channel(S%d_%d) is not existed.", iterBoard->first, ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel);
								m_pAlarm->Output(FALSE);
								continue;
							}
							bLoadFail = TRUE;

							switch (nRetVal)
							{
							case -1:
								///<The channel number is over range
								m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
								m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
								m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is not existed.", ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel);
								m_pAlarm->Output(FALSE);
								break;
							case -3:
								///<The pattern number is over range
								m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
								m_pAlarm->SetAlarmMsg("The pattern number(%d) is over range[1,%d].", nCurPatternLine, DCM400_MAX_PATTERN_COUNT);
								break;
							case -4:
								m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
								m_pAlarm->SetAlarmMsg("Allocate memory fail.");
								break;
							default:
								break;
							}
							break;
						}
						if (bFirstLoad)
						{
							setLoadBoard.insert(iterBoard->first);
							iterBoard->second->SetVectorValid(FALSE);
							auto iterChannel = mapChannel.find(iterBoard->first);
							if (mapChannel.end() == iterChannel)
							{
								vector<USHORT> vecChannel;
								mapChannel.insert(make_pair(iterBoard->first, vecChannel));
								iterChannel = mapChannel.find(iterBoard->first);
							}
							iterChannel->second.push_back(ChannelInfo.m_usChannel);
						}
#ifdef _CHECK_VECTOR
						UINT uID = GetChannelID(ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel);
						auto iterVector = mapVector.find(uID);
						if (mapVector.end() == iterVector)
						{
							mapVector.insert(make_pair(uID, 'X'));
							iterVector = mapVector.find(uID);
						}
						iterVector->second = pPinPattern[iterPin->second->GetID()].lpszPattern[nSerialIndex];
#endif // _CHECK_VECTOR

					}

					if (bLoadFail)
					{
						break;
					}
					if (m_bVectorBind)
					{
						break;
					}
				}

				if (bLoadFail)
				{
					break;
				}
			}
			bFirstLoad = FALSE;
			if (bLoadFail)
			{
				break;
			}

#ifdef _CHECK_VECTOR
			fstream VectorSave;
			string strPath = "D:\\";
			if (bSaveBRAM)
			{
				strPath += "VectorBRAM.csv";
			}
			else
			{
				strPath += "VectorDRAM.csv";
			}

			VectorSave.open(strPath.c_str(), ios::in);
			if (!VectorSave.is_open())
			{
				VectorSave.open(strPath.c_str(), ios::out | ios::app);
				VectorSave << "RAM Index,";
				for (auto& Vector : mapVector)
				{
					BYTE bySlotNo = 0;
					USHORT usChannel = ID2Channel(Vector.first, bySlotNo);
					char lpszMsg[32] = { 0 };
					sprintf_s(lpszMsg, sizeof(lpszMsg), "S%d_%d,", bySlotNo, usChannel);
					VectorSave << lpszMsg;
				}
				VectorSave << endl;
			}
			else
			{
				VectorSave.close();
				VectorSave.open(strPath.c_str(), ios::out | ios::app);
			}

			VectorSave << nCurPatternLine << ",";
			char lpszDigit[16] = { 0 };
			_itoa_s(nCurPatternLine, lpszDigit, sizeof(lpszDigit), 10);
			for (auto& Vector : mapVector)
			{
				VectorSave << Vector.second << ",";
				++iterVector;
			}
			VectorSave << endl;
			VectorSave.close();
#endif // _CHECK_VECTOR

		}
		if (bNoBoard)
		{
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
			m_pAlarm->SetParamName("lpszVectorFileName");
			m_pAlarm->SetAlarmMsg("No valid board used in vector file is existed.");
		}
		if (bLoadFail)
		{
			break;
		}

		uLineOffset += uCurReadLine;
		nLeftLine -= uCurReadLine;
		nMemLineCount += uCurReadLine;
		if (2048 <= nMemLineCount || 0 == nLeftLine)
		{
			for (auto LoadBoard : setLoadBoard)
			{
				iterBoard = m_mapBoard.find(LoadBoard);
				if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
				{
					nRetVal = iterBoard->second->LoadVector();
					if (0 != nRetVal)
					{
						bLoadFail = TRUE;
						break;
					}
				}
			}
			nMemLineCount = 0;
		}

		if (bLoadFail)
		{
			break;
		}

		if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
		{
			m_pProgressStep(uLineOffset);
		}
	}

	for (auto UserBoard : setLoadBoard)
	{
		iterBoard = m_mapBoard.find(UserBoard);
		if (m_mapBoard.end() != iterBoard && nullptr != iterBoard->second)
		{
			iterBoard->second->SetVectorValid(!bLoadFail);
		}
	}

	VectorInfo.CloseFile();

	if (m_bVectorBind)
	{
		ClearBind();
	}
	if (bLoadFail)
	{
		//The vector file is load fail.
		m_bLoadVector = FALSE;

		if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
		{
			m_pProgressStep(m_nVectorLineCout);
		}
		m_nVectorLineCout = 0;
		return -10;
	}
	m_bLoadVector = TRUE;


	m_strVectorFile = lpszVectorFile;
	SaveVectorInformation(setFailSynPin);

	///<Set the fail synchronous
	SetFailSyn(setFailSynPin);

#ifdef _CHECK_VECTOR
	UINT uMaxLineCountPerRead = 1024;
	char** lpszPattern = nullptr;

	fstream VectorSave;
	string strPath[2];
	strPath[0] = "D:\\BoardBRAM.csv";
	strPath[1] = "D:\\BoardDRAM.csv";

	try
	{
		USHORT usChannelCount = mapVector.size();
		lpszPattern = new char* [usChannelCount];
		for (USHORT usChannel = 0; usChannel < usChannelCount; ++usChannel)
		{
			lpszPattern[usChannel] = new char[uMaxLineCountPerRead];
			memset(lpszPattern[usChannel], 0, uMaxLineCountPerRead * sizeof(char));
		}
	}
	catch (const std::exception&)
	{
		return 0;
	}
	string* pstrPath = &strPath[0];
	USHORT usChannel = 0;
	BYTE bySlotNo = 0;
	BOOL bBRAM = TRUE;
	UINT* puLineCount = &m_nBRAMLeftStartLine;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			puLineCount = &m_nBRAMLeftStartLine;
			pstrPath = &strPath[0];
			bBRAM = TRUE;
		}
		else
		{
			puLineCount = &m_nDRAMLeftStartLine;
			pstrPath = &strPath[1];
			bBRAM = FALSE;
		}
		VectorSave.open(pstrPath->c_str(), ios::out | ios::out);
		VectorSave << "RAM Index,";
		for (auto& Vector : mapVector)
		{
			BYTE bySlotNo = 0;
			USHORT usChannel = ID2Channel(Vector.first, bySlotNo);
			char lpszMsg[32] = { 0 };
			sprintf_s(lpszMsg, sizeof(lpszMsg), "S%d_%d,", bySlotNo, usChannel);
			VectorSave << lpszMsg;
			++iterVector;
		}
		VectorSave << endl;

		UINT uCurReadLineCount = 0;
		UINT uVectorLefLine = *puLineCount;
		UINT uOffset = 0;
		while (0 < uVectorLefLine)
		{
			uCurReadLineCount = uVectorLefLine > uMaxLineCountPerRead ? uMaxLineCountPerRead : uVectorLefLine;

			int nIndex = 0;
			for (auto Vector : mapVector)
			{
				usChannel = ID2Channel(Vector.first, bySlotNo);
				iterBoard = m_mapBoard.find(bySlotNo);
				if (m_mapBoard.end() == iterBoard || nullptr == iterBoard->second)
				{
					lpszPattern[nIndex][uOffset] = '-';
					memset(lpszPattern[nIndex++], '-', uCurReadLineCount * sizeof(char));
					continue;
				}
				iterBoard->second->GetVector(usChannel, bBRAM, uOffset, uCurReadLineCount, lpszPattern[nIndex++]);
			}

			for (UINT uLineIndex = 0; uLineIndex < uCurReadLineCount; ++uLineIndex)
			{
				char lpszDigit[8] = { 0 };
				_itoa_s(uOffset + uLineIndex, lpszDigit, sizeof(lpszDigit), 10);
				VectorSave << lpszDigit << ",";
				nIndex = 0;
				iterVector = mapVector.begin();
				for (auto& Vector : mapVector)
				{
					usChannel = ID2Channel(Vector.first, bySlotNo);
					VectorSave << lpszPattern[nIndex][uLineIndex] << ",";
					++nIndex;
				}
				VectorSave << endl;
			}
			uOffset += uCurReadLineCount;
			uVectorLefLine -= uCurReadLineCount;
		}
		VectorSave.close();
	}
#endif // _CHECK_VECTOR

	if (nullptr != m_pProgressInfo && nullptr != m_pProgressStep)
	{
		m_pProgressStep(m_nVectorLineCout);
	}
	//ClearPinGroupSection();
	return 0;
}

void CMainFunction::ResetVector()
{
	m_bLoadVector = FALSE;
	m_mapTimeSet.clear();
	DeleteMemory(m_mapPin);
	DeleteMemory(m_mapPinGroup);
	DeleteMemory(m_mapTimeSet);
}

int CMainFunction::LoadVectorFileTimeset(const std::vector<USHORT>& vecSite, std::map<std::string, CSeriesValue*>& mapPinSeries)
{
	int nRetVal = 0;

	WAVE_FORMAT WaveFormat;
	IO_FORMAT IOFormat;
	COMPARE_MODE CompareMode;
	BOOL bFail = FALSE;
	BOOL bNoBoard = TRUE;

	///<Load series value
	vector<CHANNEL_INFO> vecChannelInfo;
	vector<USHORT> vecChannel;
	double adSeriesEdge[EDGE_COUNT] = { 0 };
	for (auto& Series : mapPinSeries)
	{
		auto iterPin = m_mapPin.find(Series.first);
		if (m_mapPin.end() == iterPin)
		{
			continue;
		}
		iterPin->second->GetAllChannel(vecChannelInfo);
		
		int nSeriesCount = Series.second->GetSeriesCount();

		BOARD_CHANNEL_BEGIN(vecChannel)
		{
			bNoBoard = FALSE;
			for (USHORT usSeriesIndex = 0; usSeriesIndex < nSeriesCount;++usSeriesIndex)
			{
				iterBoard->second->SetPeriodSeries(vecChannel, usSeriesIndex, Series.second->GetPeriod(usSeriesIndex));
				Series.second->GetEdge(usSeriesIndex, adSeriesEdge);
				iterBoard->second->SetEdgeSeries(vecChannel, usSeriesIndex, adSeriesEdge);
				Series.second->GetFormat(usSeriesIndex, WaveFormat, IOFormat, CompareMode);
				iterBoard->second->SetFormatSeries(vecChannel, usSeriesIndex, WaveFormat, IOFormat, CompareMode);
			}
		}
		BOARD_CHANNEL_END
	}

	BYTE abySeries[EDGE_COUNT] = { 0 };
	CTimeSetSeries TimeSetSeries;
	int nSameSeriesCount = 0;
	for (auto& TimeSet : m_mapTimeSet)
	{
		nSameSeriesCount = TimeSet.second->GetSameSeriesCount();
		for (int nSameSeriesIndex = 0; nSameSeriesIndex < nSameSeriesCount; ++nSameSeriesIndex)
		{
			TimeSet.second->GetSameSeries(nSameSeriesIndex, vecChannelInfo, TimeSetSeries);
			m_ClassifyBoard.SetChannel(vecChannelInfo);
			BOARD_CHANNEL_BEGIN(vecChannel)
			{
				bNoBoard = FALSE;
				iterBoard->second->SetTimeSet(vecChannel, TimeSet.second->Index(), TimeSetSeries.GetPeriodSeries(), TimeSetSeries.GetEdgeSeries(),
					TimeSetSeries.GetFormatSeries());
			}
			BOARD_CHANNEL_END
		}
	}
	
	if (bNoBoard)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
		m_pAlarm->SetAlarmMsg("No valid board used in vector file existed.");
		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
		nRetVal = -1;
	}
	return nRetVal;
}

int CMainFunction::GetBoardChannel(const std::vector<std::string>& vecPin, const std::vector<USHORT>& vecSite, std::map<BYTE, std::vector<USHORT>>& mapBoardChannel)
{
	for (auto& Channel : mapBoardChannel)
	{
		Channel.second.clear();
	}
	mapBoardChannel.clear();
	if (0 == m_mapPin.size())
	{
		return -1;
	}
	CHANNEL_INFO ChannelInfo;
	auto iterPin = m_mapPin.begin();
	USHORT usPinCount = vecPin.size();
	USHORT usSiteCount = vecSite.size();
	BOOL bAllPinExist = TRUE;
	for (auto& PinName : vecPin)
	{
		iterPin = m_mapPin.find(PinName);
		if (m_mapPin.end() == iterPin)
		{
			bAllPinExist = FALSE;
			break;
		}
		for (auto usSiteNo : vecSite)
		{
			iterPin->second->GetChannel(usSiteNo, ChannelInfo);
			auto iterChannel = mapBoardChannel.find(ChannelInfo.m_bySlotNo);
			if (mapBoardChannel.end() == iterChannel)
			{
				vector<USHORT> vecChannel;
				mapBoardChannel.insert(make_pair(ChannelInfo.m_bySlotNo, vecChannel));
				iterChannel = mapBoardChannel.find(ChannelInfo.m_bySlotNo);
			}
			iterChannel->second.push_back(ChannelInfo.m_usChannel);
		}
	}
	if (!bAllPinExist)
	{
		return -1;
	}
	return 0;
}

inline int CMainFunction::InitSite(BOOL bAddAlarm)
{
	//Create site class.
	auto iterPin = m_mapPin.begin();
	USHORT uSiteCount = iterPin->second->GetSiteCount();
	for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
	{
		CHANNEL_INFO ChannelInfo;
		for (auto& Pin : m_mapPin)
		{
			Pin.second->GetChannel(usSiteNo, ChannelInfo);
			if (ALL_SITE == ChannelInfo.m_usChannel)
			{
				if (bAddAlarm)
				{
					m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
					m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
					m_pAlarm->SetAlarmMsg("The channel(S%d_%d) of pin(%s) in SITE_%d is over range.",
						ChannelInfo.m_bySlotNo, ChannelInfo.m_usChannel, Pin.first.c_str(), usSiteNo + 1);
					m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
				}
				return -1;
				break;
			}
			m_Site.AddChannel(Pin.first, usSiteNo, ChannelInfo);
		}
	}
	return 0;
}

void CMainFunction::ClearBind()
{

}
inline void CMainFunction::GetVectorInfoFile(std::string& strVectorInfoFile)
{
	HMODULE hModule = GetModuleHandle("DCM.dll");
	char lpszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, lpszFileName, sizeof(lpszFileName));
	strVectorInfoFile = lpszFileName;
	int nPos = strVectorInfoFile.rfind("\\");
	if (-1 != nPos)
	{
		strVectorInfoFile.erase(nPos + 1);
	}
	strVectorInfoFile += "DCM\\";
	CreateDirectory(strVectorInfoFile.c_str(), nullptr);
	string strFileName = m_strVectorFile;
	nPos = strFileName.rfind("\\");
	if (-1 != nPos)
	{
		strFileName.erase(0, nPos + 1);
		nPos = strFileName.rfind(".");
		if (-1 != nPos)
		{
			strFileName.erase(nPos);
		}
	}
	strVectorInfoFile += strFileName;
	strVectorInfoFile += ".ini";
}

void CMainFunction::SaveVectorInformation(const std::set<std::string>& setFailSynPin)
{
	string strVectorInfoFile;
	GetVectorInfoFile(strVectorInfoFile);
	//Get the modified time of the vector file.
	WIN32_FIND_DATA FileAttrib;
	HANDLE hFile = FindFirstFile(m_strVectorFile.c_str(), &FileAttrib);
	SYSTEMTIME SystemTime;
	FileTimeToSystemTime(&(FileAttrib.ftLastWriteTime), &SystemTime);
	CConfigFile ConfigFile(strVectorInfoFile.c_str());

	ConfigFile.SetValue("Vector", "File", m_strVectorFile.c_str());
	ConfigFile.SetValue("Vector", "Time", "%4d%02d%02d%02d%02d%02d%03d", SystemTime.wYear, SystemTime.wMonth, SystemTime.wDay,
		SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);

	char lpszOneValue[32] = { 0 };
	auto NumString = [&](UINT uNum)
	{
		sprintf_s(lpszOneValue, "%d", uNum);
		return lpszOneValue;
	};
	ConfigFile.ClearSection("LineInfo");///<Delete all item under app LineInfo
	ConfigFile.SetValue("LineInfo", "LineCount", "%d,", m_nVectorLineCout);

	ConfigFile.ClearSection("Label");///<Delete all item under app LineInfo
	ConfigFile.SetValue("Label", "Count", "%d", m_mapLabel.size());

	int nLabelIndex = 0;
	for (auto& Label : m_mapLabel)
	{
		ConfigFile.SetValue("Label", NumString(nLabelIndex++), "%s,%d", Label.first.c_str(), Label.second);
	}
	if (0 != setFailSynPin.size())
	{
		string strPinList;
		for (const auto& Pin : setFailSynPin)
		{
			strPinList += Pin;
			strPinList += ",";
		}
		ConfigFile.SetValue("FailSyn", "Pin", strPinList.c_str());
	}

	ConfigFile.Save();
}

int CMainFunction::LoadVectorInfo(const std::string& strInfoFile)
{
	///<Get the modified time of the vector file.
	WIN32_FIND_DATA FileAttrib;
	HANDLE hFile = FindFirstFile(m_strVectorFile.c_str(), &FileAttrib);
	SYSTEMTIME SystemTime;
	FileTimeToSystemTime(&(FileAttrib.ftLastWriteTime), &SystemTime);
	CConfigFile ConfigFile(strInfoFile.c_str());

	string strData = ConfigFile.GetValue("Vector", "File");
	if (0 != strData.compare(m_strVectorFile))
	{
		///<The vector file is not equal
		return -1;
	}
	SYSTEMTIME ValidFileTime;
	sscanf_s(strData.c_str(), "%4d%02d%02d%02d%02d%02d%03d", &ValidFileTime.wYear, &ValidFileTime.wMonth, &ValidFileTime.wDay,
		&ValidFileTime.wHour, &ValidFileTime.wMinute, &ValidFileTime.wSecond, &ValidFileTime.wMilliseconds);
	if (0 != memcmp(&SystemTime, &ValidFileTime, sizeof(ValidFileTime)))
	{
		///<The vector file has been modified
		return -2;
	}

	strData = ConfigFile.GetValue("LineInfo", "LineCount");
	m_nVectorLineCout = atoi(strData.c_str());

	int nPos = 0;
	strData = ConfigFile.GetValue("Label", "Count");
	int nLabelCount = atoi(strData.c_str());
	char lpszKey[32] = { 0 };
	for (int nLabelIndex = 0; nLabelIndex < nLabelCount; ++nLabelIndex)
	{
		_itoa_s(nLabelIndex, lpszKey, sizeof(lpszKey), 10);
		strData = ConfigFile.GetValue("Label", lpszKey);
		nPos = strData.find(",");
		if (-1 == nPos)
		{
			///<The config file have been modified
			return -3;
		}
		m_mapLabel.insert(make_pair(strData.substr(0, nPos), atoi(strData.substr(nPos + 1).c_str())));
	}
	strData = ConfigFile.GetValue("FailSyn", "Pin");
	nPos = 0;
	set<string> setFailSynPin;
	do 
	{
		nPos = strData.find(",");
		if (-1 == nPos && 0 != strData.size())
		{
			if (0 != strData.size())
			{
				setFailSynPin.insert(strData);
				strData.clear();
			}
		}
		else
		{
			setFailSynPin.insert(strData.substr(0, nPos));
			strData.erase(0, nPos + 1);
		}
	} while (-1 != nPos);
	
	SetFailSyn(setFailSynPin);
	return 0;
}

void CMainFunction::SetFailSyn(const std::set<std::string>& setFailSynPin)
{

}

inline void CMainFunction::GetPinGroupInfoFile(std::string& strFile)
{
	HMODULE hModule = GetModuleHandle("DCM400.dll");
	char lpszFileName[MAX_PATH] = { 0 };
	GetModuleFileName(hModule, lpszFileName, sizeof(lpszFileName));
	strFile = lpszFileName;
	int nPos = strFile.rfind("\\");
	if (-1 != nPos)
	{
		strFile.erase(nPos + 1);
	}
	strFile += "DCM\\";
	CreateDirectory(strFile.c_str(), nullptr);
	string strFileName = m_strVectorFile;

	nPos = strFileName.rfind("\\");
	if (-1 != nPos)
	{
		strFileName.erase(0, nPos + 1);
		nPos = strFileName.rfind(".");
		if (-1 != nPos)
		{
			strFileName.erase(nPos);
		}
	}
	strFile += strFileName;
	strFile += "_PinGroup.ini";
}

BOOL CMainFunction::IsVectorValid(const char* lpszVectorFile)
{
	if (nullptr == lpszVectorFile)
	{
		return -1;
	}
	vector<CHANNEL_INFO> vecAllChannel;
	for (auto& Pin : m_mapPin)
	{
		Pin.second->GetAllChannel(vecAllChannel, TRUE);
	}
	m_ClassifyBoard.SetChannel(vecAllChannel);
	vector<USHORT> vecBoardChannel;
	BOOL bValid = TRUE;
	BOARD_CHANNEL_BEGIN(vecBoardChannel)
	{
		bValid = iterBoard->second->IsVectorValid(vecBoardChannel);
		if (!bValid)
		{
			return FALSE;
		}
	}
	BOARD_CHANNEL_END
		
	string strVectorInfoFile;
	GetVectorInfoFile(strVectorInfoFile);
	set<string> setFailSynPin;
	int nRetVal = 0;
	nRetVal = LoadVectorInfo(strVectorInfoFile);
	if (0 != nRetVal)
	{
		return FALSE;
	}
	return bValid;
}

