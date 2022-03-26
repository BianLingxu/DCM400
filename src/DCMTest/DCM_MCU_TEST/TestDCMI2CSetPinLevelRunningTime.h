#pragma once
/*!
* @file      TestDCMI2CSetPinLevelRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/16
* @version   v 1.0.0.0
* @brief     测试I2CSetPinLevel运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMI2CSetPinLevelRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport timeReport(strFuncName.c_str(), "FunctionRunningTimeTest");

	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		timeReport.SetNoBoardValid();
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		timeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(timeReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	string strSCLChannel;
	string strSDAChannel;

	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	timeReport.timeStop();
	timeReport.addMsg("One site");


	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites in one controller");


	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites in four controllers");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
