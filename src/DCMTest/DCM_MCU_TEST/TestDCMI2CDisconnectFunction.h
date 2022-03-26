#pragma once
/*!
* @file      TestDCMI2CDisconnectFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/03
* @version   v 1.0.0.0
* @brief     测试I2CDisconnect功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CDisconnectFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");//Error message.
	int nRetVal = 0;

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
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH1,CH4,CH5,CH8,CH9,CH12,CH13");

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT uSiteCount = vecSCLChannel.size();
	//Load vector.
	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());


	dcm.Connect("G_ALLPIN");


	funcReport.AddTestItem("Check function relay status");

	dcm.I2CDisconnect();

	BOOL bConnect = FALSE;
	char lpszPinName[32] = { 0 };
	USHORT usChannel = 0;
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
		{
			for (USHORT usPinIndex = 0; usPinIndex < DCM_CHANNELS_PER_CONTROL; ++usPinIndex)
			{
				usChannel = usSiteIndex * DCM_CHANNELS_PER_CONTROL + usPinIndex;
				bConnect = TRUE;
				if (IsChannelExisted(strSCLChannel, iterSlot->first, usChannel) || IsChannelExisted(strSDAChannel, iterSlot->first, usChannel))
				{
					bConnect = FALSE;
				}
				int nRelayStatus = dcm_GetRelayStatus(iterSlot->first, usChannel, 0);
				XT_EXPECT_EQ(bConnect, nRelayStatus);
				if (nRelayStatus != bConnect)
				{
					funcReport.SaveFailChannel(iterSlot->first, usChannel);
				}
			}
		}
		++iterSlot;
	}

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;

	dcm.I2CConnect();

	InvalidSite(usInvalidSite);
	nRetVal = dcm.I2CDisconnect();
	BYTE byTargetRelayStatus = 0;
	USHORT usSiteCount = vecSCLChannel.size();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		byTargetRelayStatus = 0;
		if (usInvalidSite == usSiteNo)
		{
			byTargetRelayStatus = 1;
		}
		int nRelayStatus = dcm_GetRelayStatus(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel, 0);
		XT_EXPECT_EQ(nRelayStatus, byTargetRelayStatus);
		if (nRelayStatus != byTargetRelayStatus)
		{
			funcReport.SaveFailChannel(vecSCLChannel[usSiteNo].m_bySlotNo, vecSCLChannel[usSiteNo].m_usChannel);
		}

		nRelayStatus = dcm_GetRelayStatus(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel, 0);
		XT_EXPECT_EQ(nRelayStatus, byTargetRelayStatus);
		if (nRelayStatus != byTargetRelayStatus)
		{
			funcReport.SaveFailChannel(vecSDAChannel[usSiteNo].m_bySlotNo, vecSDAChannel[usSiteNo].m_usChannel);
		}
	}
	RestoreSite();


	dcm.Disconnect("G_ALLPIN");
	vecSCLChannel.clear();
	vecSDAChannel.clear();

	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}