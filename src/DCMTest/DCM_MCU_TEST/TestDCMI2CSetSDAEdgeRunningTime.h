#pragma once
/*!
* @file      TestDCMI2CSetSDAEdgeRunningTime.h
*
* Copyright (C) ���������ؼ����ɷ����޹�˾
*
* @author    Guangyun Wang
* @date      2020/02/04
* @version   v1.0.0.0
* @brief     ����I2CSetSDAEdge����ʱ��
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMI2CSetSDAEdgeRunningTime)
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
	dcm.I2CSet(200, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetSDAEdge(10, 110, 10, 50);
	timeReport.timeStop();
	timeReport.addMsg("%d sites in %d controllers", 1, 1);


	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetSDAEdge(10, 110, 10, 50);
	timeReport.timeStop();
	timeReport.addMsg("%d sites in %d controllers", 4, 1);


	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(200, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetSDAEdge(10, 110, 10, 50);
	timeReport.timeStop();
	timeReport.addMsg("%d sites in %d controllers", 4, 4);

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
