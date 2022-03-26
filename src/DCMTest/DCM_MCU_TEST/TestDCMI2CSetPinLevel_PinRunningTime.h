#pragma once
/**
 * @file TestDCMI2CSetPinLevel_PinRunningTime.h
 * @brief Check the running time of I2CSetPinLevel
 * @author Guangyun Wang
 * @date 2021/04/16
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMI2CSetPinLevel_PinRunningTime)
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

	string strSCLChannel;
	string strSDAChannel;

	GetI2CChannel(mapSlot, 1, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 1, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SCL);
	timeReport.timeStop();
	timeReport.addMsg("One site SCL");

	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SDA);
	timeReport.timeStop();
	timeReport.addMsg("One site SDA");
	timeReport.timeStart();

	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_BOTH);
	timeReport.timeStop();
	timeReport.addMsg("One site both SCL and SDA");


	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SCL);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SCL in one controller");
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SDA);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SDA in one controller");
	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_BOTH);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites both SCL and SDA in one controller");


	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SCL);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SCL in four controllers");

	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_SDA);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SDA in four controllers");

	timeReport.timeStart();
	dcm.I2CSetPinLevel(3.0, 0.0, 1.5, 0.8, DCM_I2C_BOTH);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites both SCL and SDA in four controllers");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
