#pragma once
/**
 * @file TestDCMI2CSetDynamicLoadRunningTime.h
 * @brief Check the running time of I2CSetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/08/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMI2CSetDynamicLoadRunningTime)
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
	dcm.I2CSetDynamicLoad(DCM_I2C_SCL, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("One site SCL");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);


	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_SDA, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("One site SDA");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("One site both SCL and SDA");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);


	GetI2CChannel(mapSlot, 4, 1, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_SCL, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SCL in one controller");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_SDA, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SDA in one controller");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites both SCL and SDA in one controller");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);


	GetI2CChannel(mapSlot, 4, 4, strSCLChannel, strSDAChannel);
	dcm.I2CSet(500, 4, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_SCL, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SCL in four controllers");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_SDA, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites SDA in four controllers");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	timeReport.timeStart();
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, TRUE);
	timeReport.timeStop();
	timeReport.addMsg("Quad sites both SCL and SDA in four controllers");
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
	dcm_I2CDeleteMemory();
}
