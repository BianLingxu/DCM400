#pragma once
/*!
* @file      TestDCMDisconnectRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/14
* @version   v 1.0.0.0
* @brief     测试Disconnect运行时间
* @comment
*/
#include "..\DCMTestMain.h"
#include "..\PTE.h"

class CDisconnectPTE : public CPTE
{
public:
	/**
	 * @brief Constructor
	 * @param[in] dcm The instance of DCM
	*/
	CDisconnectPTE(DCM& dcm);
	/**
	 * @brief The function whose PTE will be calculated
	*/
	virtual void FuncExecute();
	/**
	 * @brief Reset the status afer function execution
	*/
	virtual void ResetStatus();

private:
	DCM* m_pDCM;///<The point to the instance of DCM
};

CDisconnectPTE::CDisconnectPTE(DCM& dcm)
	: m_pDCM(&dcm)
{
}

void CDisconnectPTE::FuncExecute()
{
	dcm.Disconnect(m_strAllPin.c_str());
}

void CDisconnectPTE::ResetStatus()
{
	dcm.Connect(m_strAllPin.c_str());
}

XT_TEST(FunctionRunningTimeTest, TestDCMDisconnectRunningTime)
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

	int nChannelCount = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD;
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");

	dcm.Connect("G_ALLPIN");

	timeReport.timeStart();
	dcm.Disconnect("G_ALLPIN");
	timeReport.timeStop();
	timeReport.addMsg("Disconnect %d channels ", nChannelCount);

	dcm.Connect("G_ALLPIN");

	timeReport.timeStart();
	dcm.Disconnect("G_ODDPIN");
	timeReport.timeStop();
	timeReport.addMsg("Disconnect %d channels of odd pin ", nChannelCount / 2);

	dcm.Connect("G_ALLPIN");

	timeReport.timeStart();
	dcm.Disconnect("G_EVENPIN");
	timeReport.timeStop();
	timeReport.addMsg("Disconnect %d channels of even pin ", nChannelCount / 2);

	mapSlot.clear();


	timeReport.SetAdditionItemTittle("PTE");
	CDisconnectPTE PTE(dcm);
	///<The PTE for qual sites in one controller
	double dPTE = PTE.GetPTE(1, FALSE, 4, 4, 4);
	timeReport.AdditionItem("Quad Sites in One Controller", dPTE, "%");
	///<The PTE for qual sites in four controller Unparallel
	dPTE = PTE.GetPTE(4, FALSE, 4, 4, 4);
	timeReport.AdditionItem("Quad Sites in Four Controller Unparallel", dPTE, "%");

	///<The PTE for qual sites in four controller with parallel
	dPTE = PTE.GetPTE(4, TRUE, 4, 4, 4);
	timeReport.AdditionItem("Quad Sites in Four Controller in Parallel", dPTE, "%");


	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
