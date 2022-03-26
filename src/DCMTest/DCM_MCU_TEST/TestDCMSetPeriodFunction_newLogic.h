#pragma once
/*!
* @file      TestDCM100SetPeriodFunction.h
*
* Copyright (C) 北京华峰测控技术有限公司
*
* @author    Guangyun Wang
* @date      2017/11/14
* @version   v 1.0.0.0
* @brief     测试SetPeriod功能
* @comment
*/
XT_TEST(FunctionFunctionTest, TestDCM100SetPeriodFunction_newLogic)
{
	CFuncReport funcReport(this->GetName(), "FunctionFunctionTest");

	dcm100.LoadVectorFile(g_cVectorFilePath, FALSE);
	dcm100.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	const int nMutualTestGroupChannelCount = 4;//The count of channel in each mutual test group.
	const int nFailStep = 104;//The line count between two fail section.
	const int nFailSectionCount = 8;//The count of fail section.
	const int nFirstFailSectionStart = 2;//The start line of first fail section.
	const int nFailCountPerSectionPerPin = 2;//The fail count of the channel per section.
	const int nFailCountPerPin = nFailCountPerSectionPerPin*nFailSectionCount;//The sum number of the fail line per channel.
	const ULONG ulExpectFailLineNoPerGroup[nMutualTestGroupChannelCount][nFailCountPerSectionPerPin] = { { 0, 4 },{ 1, 5 },{ 3, 5 },{ 2, 4 } };

	int failCountPerPin[DCM100_CHANNELS_PER_CONTROL] = { 0 };
	for (int ich = 0; ich < DCM100_CHANNELS_PER_CONTROL; ich++)
	{
		failCountPerPin[ich] = nFailCountPerPin + 2 * 5;//loopB
		if ((ich + 1) % 4 == 0)
		{
			failCountPerPin[ich] += 10;//repeat
		}
	}

	ULONG **ulExpectFailLineNo = new ULONG*[nMutualTestGroupChannelCount];//The expect fail line number.
	for (int nIndex = 0; nIndex < nMutualTestGroupChannelCount; ++nIndex)
	{
		ulExpectFailLineNo[nIndex] = new ULONG[nFailSectionCount * nFailCountPerSectionPerPin];
	}

	for (int nSectionIndex = 0; nSectionIndex < nFailSectionCount; ++nSectionIndex)
	{
		ULONG ulBaseOffset = nSectionIndex * nFailStep;
		for (int nFailIndex = 0; nFailIndex < nFailCountPerSectionPerPin; ++nFailIndex)
		{
			for (int nPinNo = 0; nPinNo < nMutualTestGroupChannelCount; ++nPinNo)
			{
				ulExpectFailLineNo[nPinNo][nSectionIndex * nFailCountPerSectionPerPin + nFailIndex] = ulBaseOffset + ulExpectFailLineNoPerGroup[nPinNo][nFailIndex] + nFirstFailSectionStart;
			}
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
			for (int nIndex = 0; nIndex < DCM100_MAX_BOARD_NUM; ++nIndex)
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

	double dSTBRIndex = 10;//The time of STBR
	const double dAddStep = 5;
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

	LARGE_INTEGER timeStart, timeStop, timeFreq;
	double dTimeConsum[2] = { 0 };
	QueryPerformanceFrequency(&timeFreq);

	double dSetOperand = 10000;

	double dPeriod = { 0 };
	const char* cVectorLabel[2] = { "TEST_FAIL_ST", "TEST_FAIL_SP" };
	dPeriod = dcm100_GetTimeSetPeriod(nValidControlStart / 4, 0, 0);
	ULONG ulVectorLineCount = 0;
	dcm100_GetVectorLineCount(cVectorLabel[0], cVectorLabel[1], ulVectorLineCount);

	BOOL bPass = TRUE;

//	for (dSTBRIndex = 3; dSTBRIndex < 8; dSTBRIndex += dAddStep)
	{
		dSTBRIndex = 3;
		dcm100.SetTime("G_ALLPIN", 0, DCM100_DTFT_NRZ, DCM100_IOFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 3 / 4, dPeriod * dSTBRIndex / 8, dPeriod * 3/4, DCM100_EdgeCompare);


		dcm100.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
		bPass = TRUE;
		for (int nChannelNo = 0; nChannelNo < 16; ++nChannelNo)
		{
			sts_sprintf(cPinName, 5, "CH%d", nChannelNo);
			for (int nSiteIndex = 0; nSiteIndex < nValidSiteCount; ++nSiteIndex)
			{
				dcm100.GetFailCount(cPinName, uValidSite[nSiteIndex], ulFailCount);
				if (ulFailCount != failCountPerPin[nChannelNo])
				{
					//Fail count is not equal to the expected.
					bPass = FALSE;
					break;
				}
				dcm100.GetFailLineNo(cPinName, uValidSite[nSiteIndex], nFailCountPerPin, ulFailLineNo);

				if (0 != memcmp(ulExpectFailLineNo[nChannelNo % 4], ulFailLineNo, nFailCountPerPin * sizeof(ULONG)))
				{
					bPass = FALSE;
					break;
				}
			}
			if (!bPass)
			{
				break;
			}
		}
		if (!bPass)
		{
			//Not all channel is pass in current CLK setting.
//			continue;
		}
		else
		{
//			break;
		}
	}

	QueryPerformanceCounter(&timeStart);
	dcm100.RunVectorWithGroup("G_ALLPIN", cVectorLabel[0], cVectorLabel[1]);
	QueryPerformanceCounter(&timeStop);
	dTimeConsum[0] = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart*1e6;

	dcm100.SetPeriod(0, 10 * dPeriod);
	dcm100.SetTime("G_ALLPIN", 1, DCM100_DTFT_NRZ, DCM100_IOFT_NRZ, 10*dPeriod / 8, 10 * dPeriod * 3 / 4, 10 * dPeriod / 8, 10 * dPeriod * 3 / 4, 10 * dPeriod * dSTBRIndex / 8, 10 * dPeriod * 3/4, DCM100_EdgeCompare);

	funcReport.AddTestItem("Running Time Check");
	funcReport.AddClkSetting(10 * dPeriod / 8, 10 * dPeriod * 3 / 4, 10 * dPeriod / 8, 10 * dPeriod * 3 / 4, 10 * dPeriod * dSTBRIndex / 8, 10 * dPeriod * 3 / 4);
	QueryPerformanceCounter(&timeStart);
	dcm100.RunVectorWithGroup("G_ALLPIN", cVectorLabel[0], cVectorLabel[1]);
	QueryPerformanceCounter(&timeStop);
	dTimeConsum[1] = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart*1e6;

	ULONG ulLineCount = 0;
	dcm100_GetVectorLineCount(cVectorLabel[0], cVectorLabel[1], ulLineCount);

	//if ((dTimeConsum[1] - dTimeConsum[0]) < 32 * dPeriod * 10 * dSetOperand / (1000 * dPeriod * 10))
	if ((dTimeConsum[1] - dTimeConsum[0]) < ulLineCount * dPeriod * 9/10 / 1000 * 0.95)
	{

		XT_EXPECT_TRUE(FALSE);

		for (int nSiteIndex = 0; nSiteIndex < nValidSiteCount; ++nSiteIndex)
		{
			for (int nPin = 0; nPin < DCM100_CHANNELS_PER_CONTROL; ++nPin)
			{
				funcReport.SaveFailChannel(uValidSite[nSiteIndex] * DCM100_CHANNELS_PER_CONTROL + nPin);
			}
		}
	}

	for (int nChannelNo = 0; nChannelNo < 16; ++nChannelNo)
	{
		sts_sprintf(cPinName, 5, "CH%d", nChannelNo);
		for (int nSiteIndex = 0; nSiteIndex < nValidSiteCount; ++nSiteIndex)
		{
			dcm100.GetFailCount(cPinName, uValidSite[nSiteIndex], ulFailCount);
			if (ulFailCount != failCountPerPin[nChannelNo])
			{
				//Fail count is not equal to the expected.
				funcReport.SaveFailChannel(uValidSite[nSiteIndex] * DCM100_CHANNELS_PER_CONTROL + nChannelNo);
				continue;
			}
			dcm100.GetFailLineNo(cPinName, uValidSite[nSiteIndex], nFailCountPerPin, ulFailLineNo);

			if (0 != memcmp(ulExpectFailLineNo[nChannelNo % 4], ulFailLineNo, nFailCountPerPin * sizeof(ULONG)))
			{
				funcReport.SaveFailChannel(uValidSite[nSiteIndex] * DCM100_CHANNELS_PER_CONTROL + nChannelNo);
				bPass = FALSE;
				break;
			}
		}
	}


	int nIndex = 0;
	for (nIndex = 0; nIndex < DCM100_CHANNELS_PER_CONTROL; ++nIndex)
	{
		if (TRUE == bFail[nIndex])
		{
			break;
		}
	}
	if (DCM100_CHANNELS_PER_CONTROL <= nIndex)
	{
		bAllFail = FALSE;
	}

	XT_EXPECT_FALSE(bAllFail);
	funcReport.Print(this, g_cReportFilePath);


	for (int nIndex = 0; nIndex < nMutualTestGroupChannelCount; ++nIndex)
	{
		delete[] ulExpectFailLineNo[nIndex];
	}
	delete[] ulExpectFailLineNo;

	dcm100.Disconnect("G_ALLPIN");
	dcm100_CloseFile();
}