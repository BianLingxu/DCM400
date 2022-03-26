/**
 * @file TestDCMSetChannelStatusFuntion
 * @brief The function check of SetChannelStatus
 * @author Guangyun Wang
 * @date 2020/08/18
 * @copyrigt AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#pragma once
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetChannelStatusFunction)
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
		for(auto& Slot : mapSlot)
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

	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	const BYTE byChannelStatusCount = 2;
	CHANNEL_STATUS ChannelStatus[byChannelStatusCount] = { DCM_HIGH, DCM_LOW };
	const char lpszStatusName[byChannelStatusCount][8] = { "HIGH", "LOW" };
	int nStatusIndex = 0;
	BYTE byTargetStatus = 0;
	for (auto& Status : ChannelStatus)
	{
		funcReport.AddTestItem(lpszStatusName[nStatusIndex]);
		dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, Status);
		if (DCM_HIGH == Status)
		{
			byTargetStatus = 3;
		}
		else
		{
			byTargetStatus = 0;
		}
		delay_ms(10);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD;++usChannel)
			{
				int nCurStatus = dcm_GetChannelStatus(Slot.first, usChannel);
				XT_EXPECT_EQ(nCurStatus, byTargetStatus);
				if (byTargetStatus != nCurStatus)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
		++nStatusIndex;
	}
	funcReport.AddTestItem("Check Invalid Site");
	const char* lpszPinName = "CH2";
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.SetChannelStatus(lpszPinName, usInvalidSite, DCM_HIGH_IMPEDANCE);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	RestoreSite();


	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}