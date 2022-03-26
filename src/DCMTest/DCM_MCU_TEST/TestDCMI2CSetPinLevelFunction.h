#pragma once
/**
 * @file TestDCMI2CSetPinLevelFunction.h
 * @brief Check the function function of I2CSetPinLevel
 * @author Guangyun Wang
 * @date 2021/04/16
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CSetPinLevelFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszI2CVectorFilePath);
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

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();

	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;

	double dValidSitePinLevel[4] = { 3.0, 0, 1.5, 0.8 };
	double dInvalidSitePinLevel[4] = { 5, 0, 3, 1 };
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CSetPinLevel(dInvalidSitePinLevel[0], dInvalidSitePinLevel[1], dInvalidSitePinLevel[2], dInvalidSitePinLevel[3]);
	InvalidSite(usInvalidSite);
	dcm.I2CSetPinLevel(dValidSitePinLevel[0], dValidSitePinLevel[1], dValidSitePinLevel[2], dValidSitePinLevel[3]);
	double dCurPinLevel = 0;
	XT_EXPECT_EQ(nRetVal, 0);

	BYTE bySlotNo = 0;
	USHORT usChannel = 0;
	
	vector<CHANNEL_INFO>* avecChannel[2] = { &vecSCLChannel, &vecSDAChannel };
	double* pdTargetLevel = nullptr;
	CHANNEL_INFO* pChannel = nullptr;
	for (auto Channel : avecChannel)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
		{
			if (usInvalidSite == usSiteNo)
			{
				pdTargetLevel = dInvalidSitePinLevel;
			}
			else
			{
				pdTargetLevel = dValidSitePinLevel;
			}
			pChannel = &Channel->at(usSiteNo);
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIH, dCurPinLevel);
			XT_EXPECT_REAL_EQ(dCurPinLevel, pdTargetLevel[0], 0.01);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[0]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VIL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[1]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOH, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[2]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
			dcm_GetLevelSettingValue(pChannel->m_bySlotNo, pChannel->m_usChannel, DCM_VOL, dCurPinLevel);
			if (0.01 < fabs(dCurPinLevel - pdTargetLevel[3]))
			{
				funcReport.SaveFailChannel(pChannel->m_bySlotNo, pChannel->m_usChannel);
				continue;
			}
		}
	}
	RestoreSite();
	funcReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}