#pragma once
/**
 * @file TestDCMSetDynamicLoad_GroupFunction.h
 * @brief Check the function of SetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/08/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetDynamicLoad_GroupFunction)
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

	nRetVal = dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
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

	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	vector<BYTE> vecFirsTwotPin;
	vector<BYTE> vecLastTwoPin;
	vector<BYTE>* pvecPin;
	string strFirstTwoPinList;
	string strLastTwoPinList;
	string* pstrPinList = nullptr;
	char lpszPin[32] = { 0 };

	auto GetPinName = [&](BYTE byPinNo)->const char*
	{
		sprintf_s(lpszPin, sizeof(lpszPin), "CH%d", byPinNo);
		return lpszPin;
	};

	for (USHORT usPin = 0; usPin < DCM_CHANNELS_PER_CONTROL; ++usPin)
	{
		if (2 > usPin % 4)
		{
			pstrPinList = &strFirstTwoPinList;
			pvecPin = &vecFirsTwotPin;
		}
		else
		{
			pstrPinList = &strLastTwoPinList;
			pvecPin = &vecLastTwoPin;
		}
		*pstrPinList += GetPinName(usPin);
		*pstrPinList += ",";
		pvecPin->push_back(usPin);
	}
	strFirstTwoPinList.erase(strFirstTwoPinList.size() - 1);
	strLastTwoPinList.erase(strLastTwoPinList.size() - 1);
	dcm.SetPinGroup("G_FIRSTTWO", strFirstTwoPinList.c_str());
	dcm.SetPinGroup("G_LASTTWO", strLastTwoPinList.c_str());

	USHORT puSite[MAX_SITE] = { 0 };
	USHORT uSiteCount = 0;
	for (auto& Slot : mapSlot)
	{
		uSiteCount = dcm_GetSlotSite(Slot.first, puSite, MAX_SITE);
		if (0 < uSiteCount)
		{
			USHORT uFirstSite = puSite[0];
			for (USHORT usSiteIndex = 1; usSiteIndex < uSiteCount; ++usSiteIndex)
			{
				if (MAX_SITE <= usSiteIndex)
				{
					break;
				}
				if (uFirstSite > puSite[usSiteIndex])
				{
					uFirstSite = puSite[usSiteIndex];
				}
			}
			Slot.second = uFirstSite;
		}
	}

	double dIOH = 2e-3;
	double dIOL = 2e-3;

	vector<BYTE>* pvecDynamic = nullptr;
	vector<BYTE>* pvecPMU = nullptr;
	string strPinGroup[2] = { "G_FIRSTTWO", "G_LASTTWO" };
	auto CheckCurrent = [&](BOOL bDynamicFirstTwo, BOOL bIOL)
	{
		pvecPMU = bDynamicFirstTwo ? &vecLastTwoPin : &vecFirsTwotPin;
		pvecDynamic = bDynamicFirstTwo ? &vecFirsTwotPin : &vecLastTwoPin;
		double dExpected = bIOL ? -dIOL : dIOH;
		string* pstrPinGroup = bDynamicFirstTwo ? &strPinGroup[1] : &strPinGroup[0];
		dcm.PPMUMeasure(pstrPinGroup->c_str(), 10, 10);
		int nPinCount = pvecDynamic->size();
		USHORT usSiteNo = 0;
		double dReal = 0;
		for (auto& Slot : mapSlot)
		{
			usSiteNo = Slot.second;
			for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController, ++usSiteNo)
			{
				for (BYTE byPinNo = 0; byPinNo < nPinCount; ++byPinNo)
				{
					dReal = dcm.GetPPMUMeasResult(GetPinName(pvecPMU->at(byPinNo)), usSiteNo);
					XT_EXPECT_REAL_EQ(dReal, dExpected, 0.5e-3);
					if (0.5e-3 < fabs(dReal - dExpected))
					{
						funcReport.SaveFailChannel(Slot.first, byController * DCM_CHANNELS_PER_CONTROL + pvecDynamic->at(byPinNo));
					}
				}
			}
		}
	};
	dcm.Connect("G_ALLPIN");
	///<Check IOL
	funcReport.AddTestItem("Check IOL");
	dcm.SetPPMU(strPinGroup[0].c_str(), DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(strPinGroup[1].c_str(), TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strPinGroup[1].c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(FALSE, TRUE);
	dcm.SetDynamicLoad(strPinGroup[1].c_str(), FALSE, dIOH, dIOL, 3);

	dcm.SetPPMU(strPinGroup[1].c_str(), DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(strPinGroup[0].c_str(), TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strPinGroup[0].c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(TRUE, TRUE);
	dcm.SetDynamicLoad(strPinGroup[0].c_str(), FALSE, dIOH, dIOL, 3);


	// 	///<Check IOH
	funcReport.AddTestItem("Check IOH");
	dcm.SetPPMU(strPinGroup[0].c_str(), DCM_PPMU_FVMI, 3.5, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(strPinGroup[1].c_str(), TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strPinGroup[1].c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(FALSE, FALSE);
	dcm.SetDynamicLoad(strPinGroup[1].c_str(), FALSE, dIOH, dIOL, 3);

	dcm.SetPPMU(strPinGroup[1].c_str(), DCM_PPMU_FVMI, 3.5, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(strPinGroup[0].c_str(), TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strPinGroup[0].c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(TRUE, FALSE);
	dcm.SetDynamicLoad(strPinGroup[0].c_str(), FALSE, dIOH, dIOL, 3);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
	funcReport.Print(this, g_lpszReportFilePath);
}