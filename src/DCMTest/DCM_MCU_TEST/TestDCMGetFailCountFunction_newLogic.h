/*!
* @file      TestDCM100GetFailCountFunction.h
*
* Copyright (C) 北京华峰测控技术有限公司
*
* @author    Guangyun Wang
* @date      2017/10/30
* @version   v 1.0.0.0
* @brief     测试GetFailCount功能
* @comment
*/


XT_TEST(FunctionFunctionTest, TestDCM100GetFailCountFunction_newLogic)
{
// 	HINSTANCE dcm100Module = LoadLibrary("C:\\AccoTest\\STS8250_A55\\DCM100.dll");
// 	if (NULL != dcm100Module)
// 	{
// 		GetCtrlFailCount = (GETCTLFAILCOUNT)GetProcAddress(dcm100Module, "dcm100_GetCtrlFailCount");
// 	}

	CFuncReport funcReport(this->GetName(), "FunctionFunctionTest");

	int loadRet = dcm100.LoadVectorFile(g_cVectorFilePath, FALSE);
//	dcm100.LoadVectorFile("F:\\HardwareDiver\\STS8250\\trunk\\src\\DCM100Test\\DCM100_TEST.vec");
//	return;
	int pinGpRet = dcm100.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	const int nMutualTestGroupChannelCount = 4;//The count of channel in each mutual test group.
	const int nFailStep = 104;//The line count between two fail section.
	const int nFailSectionCount = 8;//The count of fail section.
	const int nFirstFailSectionStart = 2;//The start line of first fail section.
	const int nFailCountPerSectionPerPin = 2;//The fail count of the channel per section.
	const int nFailCountPerPin = nFailCountPerSectionPerPin*nFailSectionCount;//The sum number of the fail line per channel.
	const ULONG ulExpectFailLineNo[nMutualTestGroupChannelCount][2] = { { 0, 4 }, { 1, 5 }, { 3, 5 }, { 2, 4 } };

	int failCountPerPin[DCM100_CHANNELS_PER_CONTROL] = { 0 };
	for (int ich = 0; ich < DCM100_CHANNELS_PER_CONTROL; ich++)
	{
		failCountPerPin[ich] = nFailCountPerPin + 2 * 5;//loopB
		if ((ich + 1) % 4 == 0)
		{
			failCountPerPin[ich] += 10;//repeat
		}
	}

	int nValidSiteCount = 0;
	USHORT uValidSite[MAX_SITE] = { -1 };
	memset(uValidSite, -1, sizeof(uValidSite));
	DWORD dwValidBoard = 0;//The valid DCM100 board.
	dwValidBoard = dcm100_GetValidBoard();
	if (0 == dwValidBoard)
	{
		//No board is inserted.

		XT_EXPECT_TRUE(FALSE);

		funcReport.SetNoBoardValid();

		funcReport.Print(this, g_cReportFilePath);

		return;
	}

	int nValidControlStart = 0;
	for (int nBoardIndex = 0; nBoardIndex < DCM100_MAX_BOARD_NUM; ++nBoardIndex)
	{
		if (0 != ((dwValidBoard >> nBoardIndex) & 0x01))
		{
			nValidControlStart = nBoardIndex * DCM100_MAX_BOARD_NUM;
			for (int nIndex = 0; nIndex < DCM100_MAX_BOARD_NUM; ++ nIndex)
			{
				uValidSite[nValidSiteCount++] = nValidControlStart + nIndex;
			}
		}
	}

	char cSN[32] = { 0 };
	for (int nBankIndex = 0; nBankIndex < nValidSiteCount; nBankIndex += 4)
	{
		//Save the board SN
		dcm100_GetModuleInfoByBoard(uValidSite[nBankIndex] / 4, cSN, 32, DCM100INFO::MODULE_SN, STS_MOTHER_BOARD);
		funcReport.SaveBoardSN(uValidSite[nBankIndex] / 4, cSN);
	}

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 2.5;
	char cPinName[10] = { 0 };//The pin name.
	ULONG ulFailCount = 0;

	ULONG ulFailLineNo[100] = { 0 };
	int nRetVal = 0;
	BOOL bTestPass = FALSE;
	dcm100.Connect("G_ALLPIN");
	dcm100.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	BOOL bFail[DCM100_CHANNELS_PER_CONTROL] = { 0 };
	memset(bFail, 0, sizeof(bFail));
	BOOL bAllFail = TRUE;
	
	ULONG ulCtrlFailCount[DCM100_MAX_CONTROLLERS_PRE_BOARD * DCM100_MAX_BOARD_NUM] = { 0 };

	funcReport.AddTestItem("Pin Mutual Test");

// 	if (NULL != GetCtrlFailCount)
// 	{
// 		for (int nIndex = 0; nIndex < DCM100_MAX_CONTROLLERS_PRE_BOARD; ++nIndex)
// 		{
// 			nRetVal = GetCtrlFailCount(3, nIndex, ulCtrlFailCount[nIndex + DCM100_MAX_CONTROLLERS_PRE_BOARD * 1]);
// 		}
// 	}


	for (dSTBR = 20; dSTBR < 110; dSTBR += dAddStep)
	{
//		dSTBR = 50;
		
		dcm100.SetTime("G_ALLPIN", 0, DCM100_DTFT_NRZ, DCM100_IOFT_NRZ, 10, 110, 10, 110, dSTBR, 110, DCM100_EdgeCompare);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		delay_ms(10);

		dcm100.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");

// 		if (NULL != GetCtrlFailCount)
// 		{
// 			for (int nIndex = 0;nIndex <DCM100_MAX_CONTROLLERS_PRE_BOARD;++nIndex)
// 			{
// 				nRetVal = GetCtrlFailCount(3, nIndex, ulCtrlFailCount[nIndex + DCM100_MAX_CONTROLLERS_PRE_BOARD * 1]);
// 			}
// 		}

		dcm100.SaveFailMap(0);
		memset(bFail, 0, sizeof(bFail));
		for (int nChannelNo = 0; nChannelNo < 16; ++nChannelNo)
		{
			sts_sprintf(cPinName, 5, "CH%d", nChannelNo); 
			for (int nSiteIndex = 0; nSiteIndex < nValidSiteCount; ++nSiteIndex)
			{
				dcm100.GetFailCount(cPinName, uValidSite[nSiteIndex], ulFailCount);
				if (ulFailCount != failCountPerPin[nChannelNo])
				{
					funcReport.SaveFailChannel(uValidSite[nSiteIndex] * DCM100_CHANNELS_PER_CONTROL + nChannelNo);
					bFail[nChannelNo] = TRUE;
				}
			}
		}
		int nIndex = 0;
		for ( nIndex = 0; nIndex < DCM100_CHANNELS_PER_CONTROL;++ nIndex)
		{
			if (TRUE == bFail[ nIndex])
			{
				break;
			}
		}
		if (DCM100_CHANNELS_PER_CONTROL <= nIndex)
		{
			bAllFail = FALSE;
		}
	}

	XT_EXPECT_FALSE(bAllFail);
	funcReport.Print(this, g_cReportFilePath);

	dcm100.Disconnect("G_ALLPIN");
	dcm100_CloseFile();
}