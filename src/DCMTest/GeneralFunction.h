#pragma once
#include <map>
#include <vector>
#include <string>
#include <Windows.h>
#include "SM8213.h"
#define SITE_NUM 16
#define MAX_SITE 32
using namespace std;

inline int GetBoardInfo(std::map<BYTE, USHORT>& mapSlot, const char* lpszVecrotFile, BOOL bCloseFile = FALSE)
{
	mapSlot.clear();
	BYTE byBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 == byBoardCount)
	{
		//No board is inserted.
		return 0;
	}

	BYTE* pbySlotNo = nullptr;
	try
	{
		pbySlotNo = new BYTE[byBoardCount];
		memset(pbySlotNo, 0, byBoardCount * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return -1;
	}
	dcm_GetBoardInfo(pbySlotNo, byBoardCount * sizeof(BYTE));

	char lpszSN[64] = { 0 };
	for (BYTE byBoardIndex = 0; byBoardIndex < byBoardCount; ++byBoardIndex)
	{
		//Save the board SN
		mapSlot.insert(make_pair(pbySlotNo[byBoardIndex], -1));
	}
	if (nullptr != pbySlotNo)
	{
		delete[] pbySlotNo;
		pbySlotNo = nullptr;
	}
	//Load vector.
	int nRetVal = dcm.LoadVectorFile(lpszVecrotFile, FALSE);
	if (0 != nRetVal)
	{
		return -2;
	}
	//Defined pin group G_ALLPIN
	USHORT puSite[MAX_SITE] = { 0 };
	USHORT uSiteCount = 0;
	for (auto& Slot : mapSlot)
	{
		uSiteCount = dcm_GetSlotSite(Slot.first, puSite, MAX_SITE);
		if (0 < uSiteCount)
		{
			USHORT usFirstSite = puSite[0];
			for (USHORT usSiteIndex = 1; usSiteIndex < uSiteCount; ++usSiteIndex)
			{
				if (MAX_SITE <= usSiteIndex)
				{
					break;
				}
				if (usFirstSite > puSite[usSiteIndex])
				{
					usFirstSite = puSite[usSiteIndex];
				}
			}
			Slot.second = usFirstSite;
		}
	}
	if (bCloseFile)
	{
		dcm_CloseFile();
	}
	return 0;
}

inline int CheckParamValidity(CErrorMSG& errMsg, const std::map<BYTE, USHORT>& mapSlot, int nRetVal, 
	const char* lpszPinName, BYTE byPinType, USHORT usSiteID, int nCorrectReturn, BOOL bWithLabel = FALSE, const char* lpszLabel = nullptr,
	BOOL bCheckLabelFirst = TRUE)
{
	USHORT uSiteCount = dcm_GetVectorSiteCount();
	int nExpectValue = 0;

	int nStringType = dcm_GetStringType(lpszPinName);

	if (SITE_ERROR == nRetVal)
	{
		if (usSiteID < uSiteCount)
		{
			nExpectValue = 0;
			errMsg.AddNewError(STRING_ERROR_MSG);
			char lpszDigit[10] = { 0 };
			_itoa_s(usSiteID, lpszDigit, 10, 10);
			errMsg.SetErrorItem("usSiteNo", lpszDigit);
			errMsg.SaveErrorMsg("The site id is in scale, the return value is %d!", nRetVal);
		}
		nExpectValue = SITE_ERROR;
	}
	else if (SITE_INVALID == nRetVal)
	{
		DWORD dwSiteStatus = STSGetsSiteStatus();
		if (0 != (dwSiteStatus >> usSiteID & 0x01))
		{
			nExpectValue = 0;
			errMsg.AddNewError(STRING_ERROR_MSG);
			char lpszDigit[10] = { 0 };
			_itoa_s(usSiteID, lpszDigit, 10, 10);
			errMsg.SetErrorItem("usSiteNo", lpszDigit);
			errMsg.SaveErrorMsg("The site is invalid, but the return value is SITE_INVALID(%d)!", SITE_INVALID);
		}
		nExpectValue = SITE_INVALID;
	}
	else if (0 > nStringType)
	{
		nExpectValue = PIN_NAME_ERROR;
		if (0 != byPinType)
		{
			nExpectValue = PIN_GROUP_ERROR;
		}
		if (nExpectValue != nRetVal)
		{
			//No warning when pin name is not set.
			errMsg.AddNewError(STRING_ERROR_MSG);
			if (0 == byPinType)
			{
				errMsg.SetErrorItem("lpszPinName", lpszPinName);
				errMsg.SaveErrorMsg("No warning when pin name is not defined, the return value is %d!", nRetVal);
			}
			else
			{
				errMsg.SetErrorItem("lpszPinGroup", lpszPinName);
				errMsg.SaveErrorMsg("No warning when pin group is not defined, the return value is %d!", nRetVal);
			}
		}
	}
	else
	{
		nStringType = dcm_GetStringType(lpszLabel);
		if (bWithLabel && 2 != nStringType && bCheckLabelFirst)
		{
			nExpectValue = START_LABEL_ERROR;
			if (START_LABEL_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG, lpszPinName, 1);
				errMsg.SetErrorItem("lpszStartLabel", lpszLabel);
				errMsg.SaveErrorMsg("No warning when Label is not defined!");
			}
			return nExpectValue;
		}
		BOOL bAllBoardCheck = TRUE;
		for (auto& Board : mapSlot)
		{
			if (usSiteID >= Board.second && usSiteID < Board.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
			{
				if (bWithLabel && 2 != nStringType && !bCheckLabelFirst)
				{
					bAllBoardCheck = FALSE;
					nExpectValue = START_LABEL_ERROR;
					if (START_LABEL_ERROR != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG, lpszPinName, 1);
						errMsg.SetErrorItem("lpszStartLabel", lpszLabel);
						errMsg.SaveErrorMsg("No warning when Label is not defined!");
					}
					break;
				}

				nExpectValue = nCorrectReturn;
				if (nCorrectReturn != nRetVal)
				{
					bAllBoardCheck = FALSE;
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The board is inserted, but return (%d).", nRetVal);
				}
				return nExpectValue;
			}
		}
		if (bAllBoardCheck)
		{
			nExpectValue = BOARD_NOT_INSERT_ERROR;
			if (BOARD_NOT_INSERT_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char cDigit[10] = { 0 };
				_itoa_s(usSiteID, cDigit, 10, 10);
				errMsg.SaveErrorMsg("The board is not inserted, but return(%d) value is not BOARD_NOT_INSERTED(%d).", nRetVal, BOARD_NOT_INSERT_ERROR);
			}
		}
	}
	return nExpectValue;
}


inline void GetI2CChannel(const map<BYTE, USHORT>& mapSlot, string& strSCLChannel, string& strSDAChannel, vector<CHANNEL_INFO>& vecSCLChannel, vector<CHANNEL_INFO>& vecSDAChannel)
{
	vecSDAChannel.clear();
	vecSCLChannel.clear();
	USHORT usChannel = 0;
	char lpszChannel[16] = { 0 };
	BYTE bySCLOffset = 0;
	BYTE bySDAOffset = 1;
	USHORT usCurChannel = 0;
	for (auto& Slot : mapSlot)
	{
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			bySCLOffset = 1 - bySCLOffset;
			bySDAOffset = 1 - bySDAOffset;
			for (USHORT usSiteIndex = 0; usSiteIndex < 4; ++usSiteIndex)
			{
				usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usSiteIndex * 4;
				usCurChannel = usChannel + bySCLOffset;
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", Slot.first, usCurChannel);
				strSCLChannel += lpszChannel;
				CHANNEL_INFO Channel;
				Channel.m_bySlotNo = Slot.first;
				Channel.m_usChannel = usCurChannel;
				vecSCLChannel.push_back(Channel);
				
				usCurChannel = usChannel + bySDAOffset;
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", Slot.first, usCurChannel);
				strSDAChannel += lpszChannel;
				Channel.m_bySlotNo = Slot.first;
				Channel.m_usChannel = usCurChannel;

				vecSDAChannel.push_back(Channel);
			}
		}
	}
	strSCLChannel.erase(strSCLChannel.size() - 1);
	strSDAChannel.erase(strSDAChannel.size() - 1);
}

template <typename Report>
inline void SaveBoardSN(Report& TimeReport, const map<BYTE, USHORT>& mapSlot)
{
	char lpszSN[32] = { 0 };
	for (auto& Slot : mapSlot)
	{
		dcm_GetModuleInfoByBoard(Slot.first, lpszSN, sizeof(lpszSN), DCMINFO::MODULE_SN, STS_MOTHER_BOARD);
		TimeReport.SaveBoardSN(Slot.first, lpszSN);
	}
}

inline int GetI2CChannel(const map<BYTE, USHORT>& mapSlot, USHORT uSiteCount, USHORT uMaxcontrollersCount, string& strSCLChannel, string& strSDAChannel, vector<CHANNEL_INFO>* pvecSCLChannel = nullptr, vector<CHANNEL_INFO>* pvecSDAChannel = nullptr)
{
	strSCLChannel.clear();
	strSDAChannel.clear();
	if (nullptr != pvecSCLChannel)
	{
		pvecSCLChannel->clear();
	}
	if (nullptr != pvecSDAChannel)
	{
		pvecSDAChannel->clear();
	}
	if (uSiteCount * 2 > uMaxcontrollersCount* DCM_CHANNELS_PER_CONTROL)
	{
		return -1;
	}
	if (mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD < uMaxcontrollersCount)
	{
		return -2;
	}

	int nSeparatorChannel = (uMaxcontrollersCount * DCM_CHANNELS_PER_CONTROL - uSiteCount * 2) / uSiteCount + 2;
	int nSitePerBoard = DCM_MAX_CHANNELS_PER_BOARD / nSeparatorChannel;
	USHORT uCurChannel = 0;
	USHORT uCurSiteCount = 0;
	int nUsecontrollersCount = 0;
	char lpszChannel[32] = { 0 };
	CHANNEL_INFO Channel;
	for (auto& Slot : mapSlot)
	{
		uCurChannel = 0;
		for (USHORT usSiteIndex = 0; usSiteIndex < nSitePerBoard; ++usSiteIndex)
		{
			if (uSiteCount <= uCurSiteCount)
			{
				break;
			}
			Channel.m_bySlotNo = Slot.first;
			Channel.m_usChannel = uCurChannel;
			if (nullptr != pvecSCLChannel)
			{
				pvecSCLChannel->push_back(Channel);
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", Slot.first, uCurChannel);
			strSCLChannel += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", Slot.first, uCurChannel + 1);
			strSDAChannel += lpszChannel;

			Channel.m_usChannel = uCurChannel + 1;
			if (nullptr != pvecSDAChannel)
			{
				pvecSDAChannel->push_back(Channel);
			}
			++uCurSiteCount;
			uCurChannel += nSeparatorChannel;
		}
	}
	strSCLChannel.erase(strSCLChannel.size() - 1);
	strSDAChannel.erase(strSDAChannel.size() - 1);
	return 0;
}

enum TEST_TYPE
{
	PARAM_VADILITY = 0,
	FUNCTION,
	RUNNING_TIME,
};

inline void GetFunctionName(const char* lpszTestName, string& strFuncName, TEST_TYPE TestType)
{
	strFuncName.clear();
	if (nullptr == lpszTestName)
	{
		return;
	}
	strFuncName = lpszTestName;
	int nPos = strFuncName.find("TestDCM");
	if (-1 != nPos)
	{
		strFuncName.erase(nPos, 7);
	}
	string strEndString;
	switch (TestType)
	{
	case PARAM_VADILITY:
		strEndString = "ParamValidity";
		break;
	case FUNCTION:
		strEndString = "Function";
		break;
	case RUNNING_TIME:
		strEndString = "RunningTime";
		break;
	default:
		break;
	}
	nPos = strFuncName.find(strEndString.c_str());
	if (-1 != nPos)
	{
		strFuncName.erase(nPos);
	}
}

inline int CheckPPMUConnection(CMeasurementFuncReport& Report, const std::map<BYTE, USHORT>& mapSlot)
{
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	dcm.SetPinGroup("G_FIMV", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");
	dcm.SetPinGroup("G_FVMI", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");
	///<Connect PIN 0 and PIN 2 of each controller

	Report.AddTestItem("Check resistance between connected channel");
	Report.SetTestCondition("CH0(FIMV, 1mA) connect CH2(FVMI, 1V) with 1K resistance");
	dcm.Connect("G_ALLPIN");
	dcm.SetPPMU("G_FIMV", DCM_PPMU_FIMV, 1e-3, DCM_PPMUIRANGE_2MA);
	dcm.SetPPMU("G_FVMI", DCM_PPMU_FVMI, 1, DCM_PPMUIRANGE_2MA);
	dcm.PPMUMeasure("G_FIMV", 10, 10);

	double dCheckExpectValue = 2;
	double dMeasResult = 0;
	BOOL bCheckPass = TRUE;
	USHORT usMeasureChannelCount = DCM_CHANNELS_PER_CONTROL / 2;
	char lpszPinName[32] = { 0 };
	BYTE bySlot = 0;
	USHORT usChannel = 0;
	USHORT usSiteNo = 0;
	for (auto& Slot : mapSlot)
	{
		for (BYTE bySiteIndex = 0; bySiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++bySiteIndex)
		{
			usSiteNo = Slot.second + bySiteIndex;
			for (USHORT usChannelIndex = 0; usChannelIndex < usMeasureChannelCount; ++usChannelIndex)
			{
				USHORT usCurChannel = usChannelIndex / 2 * 4 + usChannelIndex % 2;
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usCurChannel);
				dMeasResult = dcm.GetPPMUMeasResult(lpszPinName, usSiteNo);
				if (0.2 < fabs(dMeasResult - dCheckExpectValue))
				{
					string strAdditionMsg;
					if (0.2 > fabs(dMeasResult - CLx_LEVEL_MAX))
					{
						///<Not connect
						strAdditionMsg = "Not connect channel";
					}
					else if (0.2 > fabs(dMeasResult))
					{
						///<Short circuit
						strAdditionMsg = "No 1K resistance";
					}
					else
					{
						///<Resistance is not 1K
						strAdditionMsg = "The resistance is not 1K";
					}
					dcm_GetPinGroupChannel(lpszPinName, usSiteNo, &bySlot, &usChannel, 1);
					Report.AddFailChannel(bySlot, usChannel, dCheckExpectValue, dMeasResult, 1, "V", strAdditionMsg.c_str());
					bCheckPass = FALSE;
				}
			}
		}
	}
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_2UA);
	dcm.Disconnect("G_ALLPIN");
	if (!bCheckPass)
	{
		Report.SetFailInfo("Not continue for resistance check fail");
		return -1;
	}
	return 0;
}

DWORD dwBackupSiteStatus = 0;///<The site status backup
inline void InvalidSite(USHORT usSiteNo)
{
	dwBackupSiteStatus = StsGetsSiteStatus();
	DWORD dwSetSiteStatus = dwBackupSiteStatus;
	dwSetSiteStatus &= ~(1 << usSiteNo);
	StsSetSiteStatus(dwSetSiteStatus);
}

inline void RestoreSite()
{
	STSSetSiteStatus(dwBackupSiteStatus);
}


int ModifyI2CPattern(const char* lpszStartLabel, double dPeriod, USHORT usSiteCount, const CHANNEL_INFO* pSCLMutual, const CHANNEL_INFO* pSDAMutual, USHORT usSiteNo = 0xFFFF, BOOL bAllSiteSame = TRUE)
{
	int nMutualTestVectorBaseLine = dcm_GetLabelLineNo(lpszStartLabel);

	if (0 > nMutualTestVectorBaseLine)
	{
		return -1;
	}

	UINT uStartLine = 0;
	UINT uLineCount = 0;
	BOOL bWithDRAM = FALSE;
	UINT uLineCountBeforeOut = 0;
	UINT uDRAMStartLine = 0;
	UINT uDRAMLineCount = 0;
	dcm_GetLatestI2CMemory(uStartLine, uLineCount, bWithDRAM, uLineCountBeforeOut, uDRAMStartLine, uDRAMLineCount);

	USHORT* pusFM = nullptr;
	USHORT* pusMM = nullptr;
	USHORT* pusIOM = nullptr;
	BYTE* pbyBRAMFMData = nullptr;
	BYTE* pbyBRAMMMData = nullptr;
	BYTE* pbyBRAMIOMData = nullptr;
	BYTE* pbyDRAMFMData = nullptr;
	BYTE* pbyDRAMMMData = nullptr;
	BYTE* pbyDRAMIOMData = nullptr;

	UINT uTotalLineCount = uLineCount + uDRAMLineCount;

	UINT uBRAMByte = (uLineCount + 7) / 8;
	UINT uDRAMByte = (uDRAMLineCount + 7) / 8;
	try
	{
		pusFM = new USHORT[uTotalLineCount];
		pusMM = new USHORT[uTotalLineCount];
		pusIOM = new USHORT[uTotalLineCount];
		pbyBRAMFMData = new BYTE[uBRAMByte];
		pbyBRAMMMData = new BYTE[uBRAMByte];
		pbyBRAMIOMData = new BYTE[uBRAMByte];
		pbyDRAMFMData = new BYTE[uDRAMByte];
		pbyDRAMMMData = new BYTE[uDRAMByte];
		pbyDRAMIOMData = new BYTE[uDRAMByte];


		memset(pusFM, 0, uTotalLineCount * sizeof(USHORT));
		memset(pusMM, 0, uTotalLineCount * sizeof(USHORT));
		memset(pusIOM, 0, uTotalLineCount * sizeof(USHORT));
		memset(pbyBRAMFMData, 0, uBRAMByte * sizeof(BYTE));
		memset(pbyBRAMMMData, 0, uBRAMByte * sizeof(BYTE));
		memset(pbyBRAMIOMData, 0, uBRAMByte * sizeof(BYTE));
		memset(pbyDRAMFMData, 0, uDRAMByte * sizeof(BYTE));
		memset(pbyDRAMMMData, 0, uDRAMByte * sizeof(BYTE));
		memset(pbyDRAMIOMData, 0, uDRAMByte * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return -2;
	}
	USHORT* pusMem[3] = { pusFM,pusMM,pusIOM };
	BYTE** ppbyBRAMMem[3] = { &pbyBRAMFMData, &pbyBRAMMMData,&pbyBRAMIOMData };
	BYTE** ppbyDRAMMem[3] = { &pbyDRAMFMData, &pbyDRAMMMData,&pbyDRAMIOMData };

	UINT uBRAMLineIndex = 0;
	UINT uDRAMLineIndex = 0;
	UINT* puLineIndex = nullptr;
	BYTE*** pppbyMem = nullptr;
	UINT uLatestControllerID = 0;
	BYTE bySlotNo = 0;
	USHORT usChannel = 0;

	USHORT usStartSite = 0;
	USHORT usStopSite = usSiteCount - 1;
	if (0xFFFF != usSiteNo)
	{
		usStartSite = usSiteNo;
		usStopSite = usSiteNo;
	}



	char lpszPattern[100][17] = { 0 };
	dcm_GetPattern(9, 0, TRUE, nMutualTestVectorBaseLine, uLineCount, lpszPattern);

	const CHANNEL_INFO* (pMutual[2]) = { pSCLMutual, pSDAMutual };
	BOOL bSCL = TRUE;
	double dEdge[EDGE_COUNT] = { dPeriod / 4, dPeriod * 3 / 4, 10, 10 + dPeriod / 2, dPeriod * 5 / 8, dPeriod * 3 / 4 };
	for (const auto& Channel : pMutual)
	{
		uLatestControllerID = 0;
		for (USHORT usSiteNo = usStartSite; usSiteNo <= usStopSite; ++usSiteNo)
		{
			bySlotNo = Channel[usSiteNo].m_bySlotNo;
			usChannel = Channel[usSiteNo].m_usChannel;
			if (bSCL)
			{
				if (0 != usChannel % 2)
				{
					usChannel -= 1;
				}
			}
			else if (1 != usChannel % 2)
			{
				usChannel += 1;
			}
			dcm_SetEdgeByIndex(bySlotNo, usChannel, 31, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, dEdge, COMPARE_MODE::EDGE);

			BYTE byController = usChannel / DCM_CHANNELS_PER_CONTROL;
			UINT uID = bySlotNo << 24 | byController;

			if (0 == uLatestControllerID || !bAllSiteSame)
			{
				memset(pbyBRAMFMData, 0, uBRAMByte * sizeof(BYTE));
				memset(pbyBRAMMMData, 0, uBRAMByte * sizeof(BYTE));
				memset(pbyBRAMIOMData, 0, uBRAMByte * sizeof(BYTE));
				memset(pbyDRAMFMData, 0, uDRAMByte * sizeof(BYTE));
				memset(pbyDRAMMMData, 0, uDRAMByte * sizeof(BYTE));
				memset(pbyDRAMIOMData, 0, uDRAMByte * sizeof(BYTE));
				if (uLatestControllerID != uID)
				{
					dcm_GetMemory(bySlotNo, byController, TRUE, DATA_TYPE::FM, nMutualTestVectorBaseLine, uTotalLineCount, pusFM);
					dcm_GetMemory(bySlotNo, byController, TRUE, DATA_TYPE::MM, nMutualTestVectorBaseLine, uTotalLineCount, pusMM);
					dcm_GetMemory(bySlotNo, byController, TRUE, DATA_TYPE::IOM, nMutualTestVectorBaseLine, uTotalLineCount, pusIOM);
					uLatestControllerID = uID;
				}
				UINT uBRAMLineIndex = 0;
				UINT uDRAMLineIndex = 0;

				USHORT usControllerChannel = usChannel % DCM_CHANNELS_PER_CONTROL;
				for (UINT uLineIndex = 0; uLineIndex < uTotalLineCount; ++uLineIndex)
				{
					if (!bWithDRAM || uLineIndex < uLineCountBeforeOut || uLineIndex >= uLineCountBeforeOut + uDRAMLineCount)
					{
						///<BRAM line
						pppbyMem = ppbyBRAMMem;
						puLineIndex = &uBRAMLineIndex;
					}
					else
					{
						///<DRAM line
						pppbyMem = ppbyDRAMMem;
						puLineIndex = &uDRAMLineIndex;
					}

					for (int nDataTypeIndex = 0; nDataTypeIndex < 3; ++nDataTypeIndex)
					{
						if (0 != (pusMem[nDataTypeIndex][uLineIndex] >> usControllerChannel & 0x01))
						{
							(*pppbyMem[nDataTypeIndex])[*puLineIndex / 8] |= 1 << (7 - *puLineIndex % 8);
						}
					}
					++* puLineIndex;
				}

				for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
				{
					BYTE byShiftBits = 0;
					UINT uCurLineCount = 0;
					if (0 == nMemIndex)
					{
						pppbyMem = ppbyBRAMMem;
						puLineIndex = &uBRAMLineIndex;
						uCurLineCount = uLineCount;
					}
					else
					{
						///<DRAM line
						pppbyMem = ppbyDRAMMem;
						puLineIndex = &uDRAMLineIndex;
						uCurLineCount = uDRAMLineCount;
					}
					byShiftBits = uCurLineCount % 8;
					if (0 != byShiftBits)
					{
						UINT uByteIndex = uCurLineCount / 8;
						byShiftBits = 8 - byShiftBits;
						for (int nDataTypeIndex = 0; nDataTypeIndex < 3; ++nDataTypeIndex)
						{
							(*pppbyMem[nDataTypeIndex])[uByteIndex] >>= byShiftBits;
						}
					}
				}
			}

			usChannel = Channel[usSiteNo].m_usChannel;
			dcm_SetMemory(bySlotNo, usChannel, TRUE, DATA_TYPE::FM, uStartLine, uLineCount, pbyBRAMFMData);
			dcm_SetMemory(bySlotNo, usChannel, TRUE, DATA_TYPE::MM, uStartLine, uLineCount, pbyBRAMMMData);
			dcm_SetMemory(bySlotNo, usChannel, TRUE, DATA_TYPE::IOM, uStartLine, uLineCount, pbyBRAMIOMData);

			dcm_GetPattern(9, 0, TRUE, uStartLine, uLineCount, lpszPattern);
			if (!bWithDRAM)
			{
				continue;
			}
			dcm_SetMemory(bySlotNo, usChannel, FALSE, DATA_TYPE::FM, uDRAMStartLine, uDRAMLineCount, pbyDRAMFMData);
			dcm_SetMemory(bySlotNo, usChannel, FALSE, DATA_TYPE::MM, uDRAMStartLine, uDRAMLineCount, pbyDRAMMMData);
			dcm_SetMemory(bySlotNo, usChannel, FALSE, DATA_TYPE::IOM, uDRAMStartLine, uDRAMLineCount, pbyDRAMIOMData);
		}
		bSCL = FALSE;
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

	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			pppbyMem = ppbyBRAMMem;
		}
		else
		{
			///<DRAM line
			pppbyMem = ppbyDRAMMem;
		}
		for (int nDataTypeIndex = 0; nDataTypeIndex < 3; ++nDataTypeIndex)
		{
			if (nullptr != *pppbyMem[nDataTypeIndex])
			{
				delete[] * pppbyMem[nDataTypeIndex];
				*pppbyMem[nDataTypeIndex] = nullptr;

			}
		}
	}
	return 0;
}
