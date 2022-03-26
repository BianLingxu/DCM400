#pragma once
/*!
* @file      TestDCMSetVTFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/20
* @version   v 1.0.0.0
* @brief     测试SetVT功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetVTFunction)
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
	dcm.SetPinGroup("G_01PIN", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");
	dcm.SetPinGroup("G_23PIN", "CH2,CH3,CH6,CH7,CH10,CH11,CH14,CH15");

	const char* lpszLabel[4] = { "ALL_PASS_ST","ALL_PASS_SP","TEST_FAIL_ST","TEST_FAIL_SP" };

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	ULONG ulFailCount = 0;

	ULONG ulFailLineNo[100] = { 0 };

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	BOOL bFail[DCM_CHANNELS_PER_CONTROL] = { 0 };
	memset(bFail, 0, sizeof(bFail));
	BOOL bAllFail = TRUE;

	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, DCM_IO_NRZ, 10, dPeriod / 2 + 10, 10, dPeriod/2 + 10, dPeriod * 3 / 4);

	const int nTestCount = 4;
	double dVTValue[nTestCount / 2] = { 3.5,0.5 };
	const char* lpszPinGroup[2] = { "G_01PIN" ,"G_23PIN" };
	const char *lpszStartLabel[nTestCount] = { "TEST_VT_H_01_ST", "TEST_VT_L_01_ST" ,"TEST_VT_H_23_ST", "TEST_VT_L_23_ST" };
	const char *lpszStopLabel[nTestCount] = { "TEST_VT_H_01_SP", "TEST_VT_L_01_SP", "TEST_VT_H_23_SP", "TEST_VT_L_23_SP" };
	char lpszPinName[16] = { 0 };

	double dVtVoltage = 0;

	vector<DWORD> vecCloseAfterForce;
	vector<DWORD> vecCloseAfterRealL;
	vector<DWORD> vecCloseAfterRealH;

	dcm.SetReadClampAlarmMask("CH0", mapSlot.begin()->second);

	auto* pvecFail = &vecCloseAfterRealH;
	for (int nTestIndex = 0; nTestIndex < 2; ++nTestIndex)
	{
		if (0 == nTestIndex)
		{
			funcReport.AddTestItem("Force VT. PIN：0 1. Voltage:%.2f", dVTValue[0]);
		}
		else
		{
			funcReport.AddTestItem("Force VT. PIN：2 3. Voltage:%.2f", dVTValue[0]);
		}
		dcm.SetChannelStatus("G_ALLPIN", DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
		dcm.SetPPMU(lpszPinGroup[1 - nTestIndex], DCM_PPMU_FIMV, 0.000001, DCM_PPMUIRANGE_2UA);
		delay_ms(1);
		dcm.SetVT(lpszPinGroup[nTestIndex], dVTValue[0], DCM_VT_FORCE);
		dcm.PPMUMeasure(lpszPinGroup[1 - nTestIndex], 10, 10);

		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
			{
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					if (0 == nTestIndex)
					{
						if (0 == nPinIndex % 4 || 1 == nPinIndex % 4)
						{
							continue;
						}
					}
					else
					{
						if (2 == nPinIndex % 4 || 3 == nPinIndex % 4)
						{
							continue;
						}
					}
					sts_sprintf(lpszPinName, sizeof(lpszPinName), "CH%d", nPinIndex);
					dVtVoltage = dcm.GetPPMUMeasResult(lpszPinName, iterSlot->second + usSiteIndex);
					XT_EXPECT_REAL_GREATEREQ(fabs(dVtVoltage), fabs(dVTValue[0]), 0.1);
					if (0.1 < fabs(dVtVoltage - dVTValue[0]))
					{
						funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex);
					}
				}
			}
			++iterSlot;
		}
		dcm.SetVT(lpszPinGroup[nTestIndex], dVTValue[0], DCM_VT_CLOSE);
		delay_ms(1);
		dcm.PPMUMeasure(lpszPinGroup[1 - nTestIndex], 10, 10);

		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
			{
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					if (0 == nTestIndex)
					{
						if (0 == nPinIndex % 4 || 1 == nPinIndex % 4)
						{
							continue;
						}
					}
					else
					{
						if (2 == nPinIndex % 4 || 3 == nPinIndex % 4)
						{
							continue;
						}
					}
					///<The pin of VT is high impedance, the PMU FIMV, so PMU clamp 7.2V
					sts_sprintf(lpszPinName, sizeof(lpszPinName), "CH%d", nPinIndex);
					dVtVoltage = dcm.GetPPMUMeasResult(lpszPinName, iterSlot->second + usSiteIndex);
					XT_EXPECT_REAL_LESSEQ(fabs(dVtVoltage), 7.2, 1);
					XT_EXPECT_REAL_LESSEQ(7.2, fabs(dVtVoltage), 1);
					if (1 < fabs(dVtVoltage - 7.2))
					{
						DWORD dwCurChannel = (iterSlot->first << 8) + usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex;
						vecCloseAfterForce.push_back(dwCurChannel);
					}
				}
			}
			++iterSlot;
		}
	}
	BOOL bCheckFailLine = FALSE;
	ULONG ulFirstFailLine = 0;
	BYTE byVTPinFailCount = 0;
	for (int nTestIndex = 0; nTestIndex < nTestCount; ++nTestIndex)
	{
		if (0 == nTestIndex / 2)
		{
			if (0 == nTestIndex % 2)
			{
				funcReport.AddTestItem("RealTime VT. PIN：0 1. VT > VOH");
				byVTPinFailCount = 4;
			}
			else
			{
				funcReport.AddTestItem("RealTime VT. PIN：0 1. VT < VOL");
				byVTPinFailCount = 5;
			}
		}
		else
		{
			if (0 == nTestIndex % 2)
			{
				funcReport.AddTestItem("RealTime VT. PIN：2 3. VT > VOH");
				byVTPinFailCount = 4;
			}
			else
			{
				funcReport.AddTestItem("RealTime VT. PIN：2 3. VT < VOL");
				byVTPinFailCount = 5;
			}
		}
		dcm.SetVT(lpszPinGroup[nTestIndex / 2], dVTValue[nTestIndex % 2], DCM_VT_REALTIME);
		dcm.RunVectorWithGroup("G_ALLPIN", lpszStartLabel[nTestIndex], lpszStopLabel[nTestIndex]);
		//dcm.SaveFailMap(0);
		
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
			{
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					bCheckFailLine = FALSE;
					sts_sprintf(lpszPinName, sizeof(lpszPinName), "CH%d", nPinIndex);
					if (1 == nTestIndex / 2)
					{
						if (0 == nPinIndex % 4 || 1 == nPinIndex % 4)
						{
							bCheckFailLine = TRUE;
						}
						else
						{
							bCheckFailLine = FALSE;
						}
					}
					else
					{
						if (0 == nPinIndex % 4 || 1 == nPinIndex % 4)
						{
							bCheckFailLine = FALSE;
						}
						else
						{
							bCheckFailLine = TRUE;
						}
					}
					if (bCheckFailLine)
					{
						dcm.GetFailCount(lpszPinName, iterSlot->second + usSiteIndex, ulFailCount);
						XT_EXPECT_EQ((int)ulFailCount, 1);
						if (1 != ulFailCount)
						{
							funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex);
							continue;
						}
						dcm.GetFirstFailLineNo(lpszPinName, iterSlot->second + usSiteIndex, ulFirstFailLine);
						XT_EXPECT_EQ((int)ulFirstFailLine, 8);
						if (8 != ulFirstFailLine)
						{
							funcReport.SaveFailChannel(iterSlot->first, usSiteIndex* DCM_CHANNELS_PER_CONTROL + nPinIndex);
						}
					}
					else
					{
						dcm.GetFailCount(lpszPinName, iterSlot->second + usSiteIndex, ulFailCount);
						XT_EXPECT_EQ((int)ulFailCount, byVTPinFailCount);
						if (byVTPinFailCount != ulFailCount)
						{
							funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex);
							continue;
						}
					}
				}
			}
			++iterSlot;
		}
		dcm.SetVT(lpszPinGroup[nTestIndex / 2], dVTValue[nTestIndex % 2], DCM_VT_CLOSE);
		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_VT_CL_ST", "TEST_VT_CL_SP");
		//dcm.SaveFailMap(0);
		if (0 == nTestIndex % 2)
		{
			pvecFail = &vecCloseAfterRealH;
		}
		else
		{
			pvecFail = &vecCloseAfterRealL;
		}
		iterSlot = mapSlot.begin();
		DWORD dwCurChannel;
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
			{
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					sts_sprintf(lpszPinName, sizeof(lpszPinName), "CH%d", nPinIndex);
					dcm.GetFailCount(lpszPinName, iterSlot->second + usSiteIndex, ulFailCount);
					XT_EXPECT_EQ((int)ulFailCount, 2);
					if (2 != ulFailCount)
					{
						dwCurChannel = (iterSlot->first << 8) + usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex;
						pvecFail->push_back(dwCurChannel);
						continue;
					}
					dcm.GetFailLineNo(lpszPinName, iterSlot->second + usSiteIndex, 100, ulFailLineNo);
					XT_EXPECT_EQ((int)ulFailLineNo[0], 3);
					XT_EXPECT_EQ((int)ulFailLineNo[1], 6);
					if (3 != ulFailLineNo[0] || 6 != ulFailLineNo[1])
					{
						dwCurChannel = (iterSlot->first << 8) + usSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex;
						pvecFail->push_back(dwCurChannel);
					}
				}
			}
			++iterSlot;
		}
	}

	USHORT usChannel = 0;
	funcReport.AddTestItem("Close After Force VT");
	for (USHORT usChannel : vecCloseAfterForce)
	{
		funcReport.SaveFailChannel(usChannel >> 8, usChannel & 0xFF);
	}	

	funcReport.AddTestItem("Close After RealTime VT. VT > VOH");
	for (USHORT usChannel : vecCloseAfterRealH)
	{
		funcReport.SaveFailChannel(usChannel >> 8, usChannel & 0xFF);
	}

	funcReport.AddTestItem("Close After RealTime VT. VT < VOL");
	for (USHORT usChannel : vecCloseAfterRealL)
	{
		funcReport.SaveFailChannel(usChannel >> 8, usChannel & 0xFF);
	}
	
	dcm.SetVT("G_ALLPIN", 3.5, DCM_VT_CLOSE);

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;

	double dInvalidSiteVT = 4;
	int nInvalidSiteMode = DCM_VT_REALTIME;
	double dValidSiteVT = 2;
	int nValidSiteMode = DCM_VT_CLOSE;

	nRetVal = dcm.SetVT("CH2", dInvalidSiteVT, DCM_VT_REALTIME);

	InvalidSite(usInvalidSite);
	nRetVal = dcm.SetVT("CH2", dValidSiteVT, DCM_VT_CLOSE);

	double dTargetVT = 0;
	int nTargetMode = 0;
	USHORT usSiteNo = 0;
	BYTE bySlotNo = 0;
	usChannel = 0;
	double dCurVT = 0;
	int nCurMode = 0;
	for (auto& Slot : mapSlot)
	{
		usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex, ++usSiteNo)
		{
			if (usInvalidSite == usSiteNo)
			{
				dTargetVT = dInvalidSiteVT;
				nTargetMode = nInvalidSiteMode;
			}
			else
			{
				dTargetVT = dValidSiteVT;
				nTargetMode = nValidSiteMode;
			}
			dcm_GetPinGroupChannel("CH2", usSiteNo, &bySlotNo, &usChannel, 1);
			dcm_GetLevelSettingValue(bySlotNo, usChannel, DCM_VT, dCurVT);
			dcm_GetVTMode(bySlotNo, usChannel, nCurMode);
			XT_EXPECT_EQ(nCurMode, nTargetMode);
			XT_EXPECT_REAL_EQ(dCurVT, dTargetVT, 0.1);
			if (nCurMode != nTargetMode || 0.1 < fabs(dTargetVT - dCurVT))
			{
				funcReport.SaveFailChannel(bySlotNo, usChannel);
			}
		}
	}
	RestoreSite();
	dcm.SetVT("CH2");

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}