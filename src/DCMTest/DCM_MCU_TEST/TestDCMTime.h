
void inline SDRAMTypeToString(SDRAM_TYPE sdramType, char * cSdramType, int iArrayLength)
{
	switch (sdramType)
	{
	case DCM100_F_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "F");
		break;
	case DCM100_M_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "M");
		break;
	case DCM100_IO_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "IO");
		break;
	case DCM100_C_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "C");
		break;
	case DCM100_CB_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "CB");
		break;
	case DCM100_S_SDRAM:
		sts_strcpy(cSdramType, iArrayLength, "S");
		break;
	default:
		break;
	}
}


XT_TEST(FunctionRunningTimeTest, TestDCM100Time)
{
	LARGE_INTEGER timeStart, timeStop, timeFreq;
	QueryPerformanceFrequency(&timeFreq);

	dcm100.LoadVectorFile(g_cVectorFilePath);
	ULONG ulReadData[100] = { 0 };
	double dTimeConsumption = 0;
	double dSumTimeConsumption[6] = { 0 };

	for (int iIndex = 0; iIndex < 1000; ++iIndex)
	{
		for (int i = 0; i < 6; ++i)
		{
			QueryPerformanceCounter(&timeStart);
			dcm100_sdram_read_data(8, 0, (SDRAM_TYPE)i, 100, 3, ulReadData);
			QueryPerformanceCounter(&timeStop);
			dTimeConsumption = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart * 1e6;

			QueryPerformanceCounter(&timeStart);
			dcm100_sdram_read_data(8, 0, (SDRAM_TYPE)i, 100, 2, ulReadData);
			QueryPerformanceCounter(&timeStop);
			dTimeConsumption -= (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart * 1e6;
			dSumTimeConsumption[i] += dTimeConsumption;

		}
	}


	XT_PRINTFA("<=============Read data time consumption=================>\n");

	for (int iIndex = 0; iIndex < 6;++iIndex)
	{
		dSumTimeConsumption[iIndex] /= 1000;

		char cSDRAMType[10] = { 0 };
		SDRAMTypeToString((SDRAM_TYPE)iIndex, cSDRAMType, 10);
		XT_PRINTFA("The time consumption of read 1 line SDRAM %2s Data: %.3fus\n", cSDRAMType, dSumTimeConsumption[iIndex]);

	}

	for (int iIndex = 0; iIndex < 1000; ++iIndex)
	{
		for (int i = 0; i < 6; ++i)
		{
			QueryPerformanceCounter(&timeStart);
			dcm100_sdram_write_data(8, 0, (SDRAM_TYPE)i, 100, 3, ulReadData);
			QueryPerformanceCounter(&timeStop);
			dTimeConsumption = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart * 1e6;

			QueryPerformanceCounter(&timeStart);
			dcm100_sdram_write_data(8, 0, (SDRAM_TYPE)i, 100, 2, ulReadData);
			QueryPerformanceCounter(&timeStop);
			dTimeConsumption -= (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart * 1e6;
			dSumTimeConsumption[i] += dTimeConsumption;

		}
	}


	XT_PRINTFA("\n<=============Write data time consumption=================>\n");

	for (int iIndex = 0; iIndex < 6; ++iIndex)
	{
		dSumTimeConsumption[iIndex] /= 1000;

		char cSDRAMType[10] = { 0 };
		SDRAMTypeToString((SDRAM_TYPE)iIndex, cSDRAMType, 10);
		XT_PRINTFA("The time consumption of write 1 line SDRAM %2s Data: %.3fus\n", cSDRAMType, dSumTimeConsumption[iIndex]);

	}
}