#pragma once
/**
 * @file TestDCMSetPinLevelFunction.h
 * @brief Check the parameter validity of SetPinLevel
 * @author Guangyun Wang
 * @date 2020/11/27
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetPinLevelFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	//Load vector.
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(iterSlot->first, usChannel);
			}
			++iterSlot;
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;

	double dValidSitePinLevel[4] = { 3.0, 0, 1.5, 0.8 };
	double dInvalidSitePinLevel[4] = { 5, 0, 3, 1 };
	USHORT usTestChannel = 5;
	char lpszPinName[32] = { 0 };
	sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usTestChannel);

	dcm.SetPinLevel(lpszPinName, dInvalidSitePinLevel[0], dInvalidSitePinLevel[1], dInvalidSitePinLevel[2], dInvalidSitePinLevel[3]);
	InvalidSite(usInvalidSite);
	nRetVal = dcm.SetPinLevel(lpszPinName, dValidSitePinLevel[0], dValidSitePinLevel[1], dValidSitePinLevel[2], dValidSitePinLevel[3]);
	double dCurPinLevel = 0;
	XT_EXPECT_EQ(nRetVal, 0);
	
	BYTE bySlotNo = 0;
	USHORT usChannel = 0;
	
	double* pdTargetLevel = nullptr;
	for (auto& Slot : mapSlot)
	{
		USHORT usBaseSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
		{
			USHORT usSiteNo = usBaseSiteNo + byControllerIndex;
			if (usInvalidSite == usSiteNo)
			{
				pdTargetLevel = dInvalidSitePinLevel;
			}
			else
			{
				pdTargetLevel = dValidSitePinLevel;
			}
			dcm_GetPinGroupChannel(lpszPinName, usSiteNo, &bySlotNo, &usChannel, 1);
			dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_VIH, dCurPinLevel);
			XT_EXPECT_REAL_EQ(dCurPinLevel, pdTargetLevel[0], 0.01);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[0]))
			{
				funcReport.SaveFailChannel(bySlotNo, usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_VIL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[1]))
			{
				funcReport.SaveFailChannel(bySlotNo, usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_VOH, dCurPinLevel); 
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[2]))
			{
				funcReport.SaveFailChannel(bySlotNo, usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_VOL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[3]))
			{
				funcReport.SaveFailChannel(bySlotNo, usChannel);
				continue;
			}
		}
	}
	RestoreSite();


	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}