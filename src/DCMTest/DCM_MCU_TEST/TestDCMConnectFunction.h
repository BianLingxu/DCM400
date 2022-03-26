#pragma once
/*!
* @file      TestDCMConnectFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/23
* @version   v 1.0.0.0
* @brief     测试Connect功能
* @comment
*/
#include "..\DCMTestMain.h"
#define SITE_PER_BOARD 4
XT_TEST(FunctionFunctionTest, TestDCMConnectFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");//Error message.

	map<BYTE, USHORT> mapSlot;

	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);

	//Load vector.
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	int nPinGroupTestCount = 17;
	char lpszPinGroup[16] = { 0 };//Save name of all pin group

	funcReport.AddTestItem("Function Relay Test");
	BYTE bySlotNo[DCM_CHANNELS_PER_CONTROL] = { 0 };
	USHORT usChannel[DCM_CHANNELS_PER_CONTROL] = { 0 };
	BYTE byConnectChannel[DCM_MAX_CHANNELS_PER_BOARD] = { 0 };
	int nPinCount = 0;
	int nRelayStatus = 0;
	BOOL bErrorExit = FALSE;
	dcm.Disconnect("G_ALLPIN");
	for (int nPinGroupTestIndex = 0; nPinGroupTestIndex < nPinGroupTestCount; ++nPinGroupTestIndex)
	{
		if (DCM_CHANNELS_PER_CONTROL > nPinGroupTestIndex)
		{
			sprintf_s(lpszPinGroup, sizeof(lpszPinGroup), "CH%d", nPinGroupTestIndex);
		}
		else
		{
			strcpy_s(lpszPinGroup, sizeof(lpszPinGroup), "G_ALLPIN");
		}
		dcm.Connect(lpszPinGroup);
		memset(byConnectChannel, 0, sizeof(byConnectChannel));
		for (auto& Slot : mapSlot)
		{
			for (USHORT usSiteIndex = 0; usSiteIndex < SITE_PER_BOARD; ++usSiteIndex)
			{
				nPinCount = dcm_GetPinGroupChannel(lpszPinGroup, Slot.second + usSiteIndex, bySlotNo, usChannel, DCM_CHANNELS_PER_CONTROL);
				if (DCM_CHANNELS_PER_CONTROL < nPinCount)
				{
					funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
					MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
					bErrorExit = TRUE;
					break;
				}
				for (int nPinIndex = 0; nPinIndex < nPinCount; ++nPinIndex)
				{
					if (Slot.first != bySlotNo[nPinIndex])
					{
						funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
						MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
						bErrorExit = TRUE;
						break;
					}
					if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel[nPinIndex])
					{
						funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
						MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
						bErrorExit = TRUE;
						break;
					}
					byConnectChannel[usChannel[nPinIndex]] = 1;
					if (bErrorExit)
					{
						break;
					}
				}
			}
			if (bErrorExit)
			{
				break;
			}
			for (USHORT usChannelIndex = 0; usChannelIndex < DCM_MAX_CHANNELS_PER_BOARD; ++usChannelIndex)
			{
				nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannelIndex, 0);
				XT_EXPECT_EQ((int)byConnectChannel[usChannelIndex], nRelayStatus);
				if (nRelayStatus != byConnectChannel[usChannelIndex])
				{
					funcReport.SaveFailChannel(Slot.first, usChannelIndex);
				}
			}
		}
		dcm.Disconnect(lpszPinGroup);
	}
	XT_EXPECT_EQ(FALSE, bErrorExit);
	if (bErrorExit)
	{
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}

	funcReport.AddTestItem("Check Invalid Site");


	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	const char* lpszPin = "CH5";
	dcm.Connect(lpszPin);
	USHORT usSiteNo = 0;
	for (auto& Slot : mapSlot)
	{
		memset(bySlotNo, 0, sizeof(bySlotNo));
		memset(usChannel, 0, sizeof(usChannel));
		usSiteNo = Slot.second;
		for (USHORT usControllerIndex = 0; usControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usControllerIndex, ++ usSiteNo)
		{
			nPinCount = dcm_GetPinGroupChannel(lpszPin, usSiteNo, bySlotNo, usChannel, DCM_CHANNELS_PER_CONTROL);
			if (DCM_CHANNELS_PER_CONTROL < nPinCount)
			{
				funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
				MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
				bErrorExit = TRUE;
				break;
			}
			for (int nPinIndex = 0; nPinIndex < nPinCount; ++nPinIndex)
			{
				if (Slot.first != bySlotNo[nPinIndex])
				{
					funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
					MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
					bErrorExit = TRUE;
					break;
				}
				if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel[nPinIndex])
				{
					funcReport.SaveAddtionMsg("The vector file(%d) is not right.", g_lpszVectorFilePath);
					MessageBox(nullptr, "The vector file is not right.", "Error", MB_OK | MB_ICONWARNING);
					bErrorExit = TRUE;
					break;
				}
				BYTE byTargetStatus = 1;
				if (usSiteNo == usInvalidSite)
				{
					byTargetStatus = 0;
				}

				nRelayStatus = dcm_GetRelayStatus(Slot.first, usChannel[nPinIndex], 0);
				if (nRelayStatus != byTargetStatus)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel[nPinIndex]);
				}
			}
		}
	}
	dcm.Disconnect(lpszPin);
	RestoreSite();

	XT_EXPECT_EQ(FALSE, bErrorExit);
	if (bErrorExit)
	{
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
	}

	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}