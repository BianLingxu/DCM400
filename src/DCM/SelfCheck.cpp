#include "SelfCheck.h"
#include "DCM.h"
#include <set>
#include "Pattern.h"
#include "Calibration.h"
#define FAIL_SAVE_COUNT 10
using namespace std;

#define	SELFCHECK_FILE_DASH_NUM	80

CSelfCheck::CSelfCheck(CDCM& DCM)
	: m_pDCM(&DCM)
	, m_byRecordControllerCount(0)
	, m_bySlotNo(0)
	, m_byValidControllerCount(0)
	
{
}

CSelfCheck::~CSelfCheck()
{
	for (auto& Controller : m_mapHardware)
	{
		if (nullptr != Controller.second)
		{
			delete Controller.second;
			Controller.second = nullptr;
		}
	}
	m_mapHardware.clear();
}

int CSelfCheck::Check(const char* lpszLogFileName, BYTE bySlotNo, int* pnCheckResult)
{
	m_bySlotNo = bySlotNo;
	if (nullptr != lpszLogFileName)
	{
		m_strFileName = lpszLogFileName;
	}
	m_CheckLog.SetLogFile(m_strFileName);

	int nChannelCount = m_pDCM->GetChannelCount(bySlotNo, TRUE);
	if (0 >= nChannelCount)
	{
		///<No board or no channels
		SaveBoardInfo();
		switch (nChannelCount)
		{
		case -1:
			NoBoardExist(pnCheckResult, "The board inserted in slot %d is not existed", m_bySlotNo);
			break;
		case -2:
			NoBoardExist(pnCheckResult, "Read flash error");
			break;
		case -3:
			NoBoardExist(pnCheckResult, "Allocate memory fail");
			break;
		case -4:
			NoBoardExist(pnCheckResult, "The data in flash is error");
			break;
		default:
			break;
		}
		set<BYTE> setFailController;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
		{
			setFailController.insert(byControllerIndex);
		}
		SetChannelCheckResult(GetCheckResut(setFailController), pnCheckResult);
		return 0;
	}
	set<BYTE> setCheckController;
	vector<USHORT> vecBoardChannel;
	m_byRecordControllerCount = nChannelCount / DCM_CHANNELS_PER_CONTROL;
	for (BYTE byControllerIndex = 0; byControllerIndex < m_byRecordControllerCount;++byControllerIndex)
	{
		CHardwareFunction* pHardware = new CHardwareFunction(m_bySlotNo);
		pHardware->SetControllerIndex(byControllerIndex);
		if (!pHardware->IsControllerExist())
		{
			delete pHardware;
			pHardware = nullptr;
			continue;
		}
		setCheckController.insert(byControllerIndex);
		pHardware->SetVectorValid(FALSE);
		pHardware->StopRun();///<Stop vector before self check
		m_mapHardware.insert(make_pair(byControllerIndex, pHardware));
		USHORT usBoardChannelOffset = byControllerIndex * DCM_CHANNELS_PER_CONTROL;
		for (USHORT usChannel= 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
		{
			vecBoardChannel.push_back(usBoardChannelOffset + usChannel);
		}
	}
	m_CheckLog.SetCheckController(setCheckController);
	SaveBoardInfo();
	m_byValidControllerCount = m_mapHardware.size();
	if (0 == m_byValidControllerCount)
	{
		NoBoardExist(pnCheckResult, "No valid controller existed, the logic revision of controller is too low if the controller existed.");
		set<BYTE> setFailController;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			setFailController.insert(byControllerIndex);
		}
		SetChannelCheckResult(GetCheckResut(setFailController), pnCheckResult);
		return 0;
	}

	m_mapHardware.begin()->second->SetFunctionRelay(vecBoardChannel, FALSE);

	int nTotalCheckResult = 0xFF;
	int nItemCheckResult = 0;
	///<BRAM check
	nItemCheckResult = BRAMCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<DRAM check
	nItemCheckResult = DRAMCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<Instruction check
	nItemCheckResult = InstructionCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<High speed memory check
	nItemCheckResult = HighSpeedMemoryCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<PMU check
	nItemCheckResult = PMUCheck(pnCheckResult);
	nTotalCheckResult &= nItemCheckResult;

	///<TMU check
	nItemCheckResult = TMUCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<Transfer from serial to parallel
	nItemCheckResult = SerialParallelTransferCheck();
	nTotalCheckResult &= nItemCheckResult;

	///<Check authorized
	BOOL bCheckResult = FALSE;
 	nItemCheckResult = CheckAuthorization(bCheckResult);
 	nTotalCheckResult &= nItemCheckResult;

	PrintWarning();

	SaveCalibrationInfo();

	nTotalCheckResult = SetChannelCheckResult(nTotalCheckResult, pnCheckResult);
	InitializeChannelStatus();

	if (nTotalCheckResult && bCheckResult)
	{
		///<Check PASS
		return 1;
	}
	return 0;
}

void CSelfCheck::SaveBoardInfo()
{
	//显示--模块相关信息
	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	for (int nIndex = 0; nIndex < SELFCHECK_FILE_DASH_NUM; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");

	STS_HARDINFO HardInfo;
	m_pDCM->GetHardInfo(m_bySlotNo, &HardInfo, 1);

	// 1、板卡的名称
	fprintf_s(pFileLog, "  Module Type : DCM (SM8213) Self Check\n");
	// 3、板卡的槽位号
	fprintf_s(pFileLog, "  Slot No.    : #%02d\n", m_bySlotNo);
	// 4、自检版本
	fprintf_s(pFileLog, "  Version     : 2.00\n");
	// 5、板卡的SN号
	fprintf_s(pFileLog, "  Serial No.  : %s\n", HardInfo.moduleInfo.moduleSN);
	// 6、板卡的硬件版本号
	fprintf_s(pFileLog, "  Hard Rev    : %s\n", HardInfo.moduleInfo.moduleHardRev);
	// 7、板卡的FPGA版本信息

	string strLog;
	char lpszLog[128] = { 0 };
	sprintf_s(lpszLog, sizeof(lpszLog), "  FPGA Rev    : SM8213->0x%04x", m_pDCM->GetFPGARevision(m_bySlotNo));
	strLog += lpszLog;
	BYTE byControllerIndex = 0;
	for (auto& Controller : m_mapHardware)
	{
		byControllerIndex = Controller.first;
		sprintf_s(lpszLog, sizeof(lpszLog), "   Controller %d->0x%04x", byControllerIndex, m_pDCM->GetFPGARevision(m_bySlotNo, byControllerIndex));
		strLog += lpszLog;
	}
	strLog += "\n";
	fprintf_s(pFileLog, strLog.c_str());

	for (int nIndex = 0; nIndex < SELFCHECK_FILE_DASH_NUM; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");
	fclose(pFileLog);
}

void CSelfCheck::SaveCalibrationInfo()
{
	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	if (nullptr == pFileLog)
	{
		fopen_s(&pFileLog, m_strFileName.c_str(), "wa+");
	}
	fprintf_s(pFileLog, "\n---------------------------------------------------------------------------------------------------------\n");
	fprintf_s(pFileLog, "Calibration Information\n");
	fprintf_s(pFileLog, "Channel | Result | Date                | Slot | Temperature | Humidity  | Instrument     | CalBoardSN\n");

	// 获取ini文件中的表的信息
	map<int, string> mapCalibrationMeterType;
	GetCalibrationMeter(mapCalibrationMeterType);
	USHORT usChannelNo = 0;
	USHORT usChannelOffset = 0;
	for (BYTE byControllerIndex = 0; byControllerIndex < m_byRecordControllerCount; byControllerIndex++)
	{
		STS_CALINFO CalibrationInfo[DCM_CHANNELS_PER_CONTROL];
		m_pDCM->GetCalibrationInfo(m_bySlotNo, byControllerIndex, CalibrationInfo, DCM_CHANNELS_PER_CONTROL);
		usChannelOffset = byControllerIndex * DCM_CHANNELS_PER_CONTROL;
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; usChannel++)
		{
			usChannelNo = usChannel + usChannelOffset;
			fprintf_s(pFileLog, "S%02d_%02d  | ", m_bySlotNo, usChannelNo);

			// 校准结果
			if (0 == CalibrationInfo[usChannel].calResult)
			{
				fprintf_s(pFileLog, "PASS   | ");
			}
			else if (1 == CalibrationInfo[usChannel].calResult)
			{
				fprintf_s(pFileLog, "FAIL   | ");
			}
			else
			{
				fprintf_s(pFileLog, "N/A    | ");
			}

			// 校准日期
			char str_time[30] = { 0 };
			if (CalibrationInfo[usChannel].calDate > 0)
			{
				struct tm tTime;
				localtime_s(&tTime, &CalibrationInfo[usChannel].calDate);
				strftime(str_time, sizeof(str_time), "%Y-%m-%d %H:%M:%S", &tTime);
				fprintf_s(pFileLog, "%s | ", str_time);
			}
			else
			{
				fprintf_s(pFileLog, "N/A                 | ");
			}

			// 校准时所处的slotID[1~36]
			if ((CalibrationInfo[usChannel].calSlotID < 37) && (CalibrationInfo[usChannel].calSlotID > 0))
			{
				fprintf_s(pFileLog, "%4d | ", CalibrationInfo[usChannel].calSlotID);
			}
			else
			{
				fprintf(pFileLog, "N/A  | ");
			}

			// 校准温度
			if ((CalibrationInfo[usChannel].temperature < 1000) && (CalibrationInfo[usChannel].temperature > -1000))
			{
				fprintf_s(pFileLog, "%8.2f(C) | ", CalibrationInfo[usChannel].temperature);
			}
			else
			{
				fprintf(pFileLog, "N/A         | ");
			}

			// 校准湿度
			if ((CalibrationInfo[usChannel].humidity < 1000) && (CalibrationInfo[usChannel].humidity > -1000))
			{
				fprintf(pFileLog, "%6.2f(%%) | ", CalibrationInfo[usChannel].humidity);
			}
			else
			{
				fprintf(pFileLog, "N/A       | ");
			}

			// 校准表类型
			auto iterType = mapCalibrationMeterType.find(CalibrationInfo[usChannel].meterType);
			if (mapCalibrationMeterType.end() != iterType)
			{
				if (iterType->second.size())
				{
					fprintf(pFileLog, "%-15s", iterType->second.c_str());
					fprintf(pFileLog, "| ");
				}
				else
				{
					fprintf(pFileLog, "N/A            | ");
				}
			}
			else
			{
				fprintf(pFileLog, "N/A            | ");
			}

			// 校准表的SN号
			if ('\0' != CalibrationInfo[usChannel].calBdSn[0])
			{
				fprintf(pFileLog, "%s", CalibrationInfo[usChannel].calBdSn);
			}
			else
			{
				fprintf(pFileLog, "N/A            ");
			}


			fprintf(pFileLog, "\n");
		}
	}
	fprintf(pFileLog, "---------------------------------------------------------------------------------------------------------\n");
	fclose(pFileLog);
	return;
}

void CSelfCheck::GetCalibrationMeter(std::map<int, std::string>& mapCalMeter)
{
	mapCalMeter.clear();

	string moduleName = "DCM.dll";

	HMODULE hModule = GetModuleHandle(moduleName.c_str());
	if (hModule != nullptr)
	{
		char modulePath[255] = { 0 };
		int strLength = 0;
		strLength = GetModuleFileName(hModule, modulePath, 255);
		string strIniPath(modulePath);
		strIniPath = strIniPath.substr(0, strIniPath.length() - moduleName.length());
		strIniPath += "digit_multimeter.ini";

		// 取所有支持的表名称
		set<string> setMeterName;
		char cName[256] = { 0 };
		char temp[256] = { 0 };
		int nMeterNum = 0;
		nMeterNum = GetPrivateProfileInt("instrument", "count", 0, strIniPath.c_str());
		for (int i = 1; i < nMeterNum + 1; ++i)
		{
			_itoa_s(i, temp, 256, 10);
			GetPrivateProfileString("instrument", temp, "", cName, 256, strIniPath.c_str());
			string strName(cName);
			if (strName.size())
			{
				setMeterName.insert(strName);
			}
		}

		set<string>::iterator iter = setMeterName.begin();
		for (iter = setMeterName.begin(); iter != setMeterName.end(); iter++)
		{
			int nTypeId = GetPrivateProfileInt((*iter).c_str(), "TypeId", 0, strIniPath.c_str());

			if (nTypeId <= 18)
			{
				mapCalMeter.insert(make_pair(nTypeId - 16, *iter));
			}
			else
			{
				mapCalMeter.insert(make_pair(nTypeId, *iter));
			}
		}
	}
}

int CSelfCheck::BRAMCheck()
{
	m_CheckLog.SetTestItem("BRAM Check");
	m_CheckLog.SetSubItem("RANDOM");

	const int nMaxMemDepth = 0x1000;

	ULONG* pulWriteBuff = nullptr;
	ULONG* pulReadBuff = nullptr;

	try
	{
		pulWriteBuff = new ULONG[nMaxMemDepth];
		pulReadBuff = new ULONG[nMaxMemDepth];
		memset(pulWriteBuff, 0, nMaxMemDepth * sizeof(ULONG));
		memset(pulReadBuff, 0, nMaxMemDepth * sizeof(ULONG));
	}
	catch (const std::exception&)
	{
		FILE* pFileLog = nullptr;
		fprintf_s(pFileLog, "Allocate %d byte memory fail", nMaxMemDepth * sizeof(ULONG) * 2);
		fclose(pFileLog);
		return -1;
	}

	const BYTE byDataTypeCount = 18;
	const CHardwareFunction::RAM_TYPE RAMType[byDataTypeCount] = { CHardwareFunction::RAM_TYPE::IMM1, 
		CHardwareFunction::RAM_TYPE::IMM2, 
		CHardwareFunction::RAM_TYPE::FM,
		CHardwareFunction::RAM_TYPE::MM,
		CHardwareFunction::RAM_TYPE::IOM, 
		CHardwareFunction::RAM_TYPE::MEM_PERIOD, 
		CHardwareFunction::RAM_TYPE::MEM_RSU_SVM1,
		CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1,
		CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2,
		CHardwareFunction::RAM_TYPE::MEM_HIS_SVM, 
		CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1, 
		CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2, 
		CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT,
		CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R, 
		CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F, 
		CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR, 
		CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF,
		CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR};
	const UINT uDataCount[byDataTypeCount] = { 0x1000,0x1000,0x1000,0x1000,0x1000, 0x20,0x0400,0x0400,0x0400, 0x0400,0x0400,0x400,0x20,0x20,0x20,0x20,0x20,0x20};
	const UINT uValidBit[byDataTypeCount] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x003FFFFF, 0x1FFFFFFF, 0xFFFFFFFF, 0x0000FFFF,
		0x1FFFFFFF,	0xFFFFFFFF, 0x0000FFFF, 0x0000003F, 0x003FFFFF, 0x003FFFFF, 0x003FFFFF, 0x003FFFFF, 0x003FFFFF};
	const char lpszTypeName[byDataTypeCount][32] = { "IMM1", "IMM2", "FM","MM","IOM", "MEM_PERIOD", "MEM_RSU_SVM1", "MEM_RSU_LVM1", "MEM_RSU_LVM2",
		"MEM_HIS_SVM", "MEM_HIS_LVM1", "MEM_HIS_LVM2", "MEM_TIMING_FMT","MEM_TIMING_T1R", "MEM_TIMING_T1F", "MEM_TIMING_IOR", "MEM_TIMING_IOF",	"MEM_TIMING_STBR"};

	for (int nDataIndex = 0; nDataIndex < nMaxMemDepth; ++nDataIndex)
	{
		pulWriteBuff[nDataIndex] = rand() & 0xFF;
		pulWriteBuff[nDataIndex] |= (rand() & 0xFF) << 8;
		pulWriteBuff[nDataIndex] |= (rand() & 0xFF) << 16;
		pulWriteBuff[nDataIndex] |= (rand() & 0xFF) << 24;
	}

	Bind();
	///<Write memory
	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;
	for (int nMemTypeIndex = 0; nMemTypeIndex < byDataTypeCount; ++nMemTypeIndex)
	{
		if (CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT >= RAMType[nMemTypeIndex])
		{
			pBindHardware->WriteBRAMMemory(RAMType[nMemTypeIndex], 0, uDataCount[nMemTypeIndex], pulWriteBuff);
		}
		else
		{
			for (USHORT usChannelIndex = 0; usChannelIndex < DCM_CHANNELS_PER_CONTROL; ++usChannelIndex)
			{
				pBindHardware->WriteBRAMMemory(RAMType[nMemTypeIndex], usChannelIndex << 12, uDataCount[nMemTypeIndex], pulWriteBuff);
			}
		}
	}

	ClearBind();
	for (int nMemTypeIndex = 0; nMemTypeIndex < byDataTypeCount; ++nMemTypeIndex)
	{
		m_CheckLog.SetCheckDataName(lpszTypeName[nMemTypeIndex]);
		for (auto& Controller : m_mapHardware)
		{
			memset(pulReadBuff, 0xBB, uDataCount[nMemTypeIndex] * sizeof(ULONG));
			if (CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT >= RAMType[nMemTypeIndex])
			{
				Controller.second->ReadBRAMMemory(RAMType[nMemTypeIndex], 0, uDataCount[nMemTypeIndex], pulReadBuff);
			}
			else
			{
				for (USHORT usChannelIndex = 0; usChannelIndex < DCM_CHANNELS_PER_CONTROL; ++usChannelIndex)
				{
					Controller.second->ReadBRAMMemory(RAMType[nMemTypeIndex], usChannelIndex << 12, uDataCount[nMemTypeIndex], pulReadBuff);
				}
			}
			int nFailCount = 0;
			if (0xFFFFFFFF != uValidBit[nMemTypeIndex] || 0 != memcmp(pulReadBuff, pulWriteBuff, uDataCount[nMemTypeIndex] * sizeof(ULONG)))
			{
				for (UINT uDataIndex = 0; uDataIndex < uDataCount[nMemTypeIndex]; ++uDataIndex)
				{
					ULONG ulExpectValue = pulWriteBuff[uDataIndex] & uValidBit[nMemTypeIndex];
					if (pulReadBuff[uDataIndex] != ulExpectValue)
					{
						m_CheckLog.AddFailData(Controller.first, "0x%08X|0x%08X(0x%04X)", pulReadBuff[uDataIndex], ulExpectValue, uDataIndex);
				
						if (FAIL_SAVE_COUNT < nFailCount++)
						{
							break;
						}
					}
				}
			}
		}
	}
	
	if (nullptr != pulReadBuff)
	{
		delete[] pulReadBuff;
		pulReadBuff = nullptr;
	}
	if (nullptr != pulWriteBuff)
	{
		delete[] pulWriteBuff;
		pulWriteBuff = nullptr;
	}
	m_CheckLog.SaveLog();
	set<BYTE> setFailController;
	m_CheckLog.GetFailController(setFailController);
	return GetCheckResut(setFailController);
}

int CSelfCheck::DRAMCheck()
{
	m_CheckLog.SetTestItem("DRAM Check");
	m_CheckLog.SetSubItem("RANDOM");
	const int nMemDepth = 0x1000;

	USHORT* pusWriteBuff = nullptr;
	USHORT* pusReadBuff = nullptr;

	try
	{
		pusWriteBuff = new USHORT[nMemDepth];
		pusReadBuff = new USHORT[nMemDepth];
		memset(pusWriteBuff, 0, nMemDepth * sizeof(USHORT));
		memset(pusReadBuff, 0, nMemDepth * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		FILE* pFileLog = nullptr;
		fprintf_s(pFileLog, "Allocate %d byte memory fail", nMemDepth * sizeof(ULONG) * 2);
		fclose(pFileLog);
		return -1;
	}

	const int nDataTypeCount = 4;
	const DATA_TYPE DataType[nDataTypeCount] = { DATA_TYPE::FM, DATA_TYPE::MM, DATA_TYPE::IOM, DATA_TYPE::CMD };
	const char lpszDataTypeName[nDataTypeCount][8] = { "FM","MM","IOM", "CMD" };


	///<Random number
	for (int nDataIndex = 0; nDataIndex < nMemDepth; ++nDataIndex)
	{
		pusWriteBuff[nDataIndex] = rand() & 0xFF;
		pusWriteBuff[nDataIndex] |= (rand() & 0xFF) << 8;
		pusWriteBuff[nDataIndex] |= (rand() & 0xFF) << 16;
		pusWriteBuff[nDataIndex] |= (rand() & 0xFF) << 24;
	}

	Bind();
	///<Write memory
	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;
	for (int nMemTypeIndex = 0; nMemTypeIndex < nDataTypeCount; ++nMemTypeIndex)
	{
		pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DataType[nMemTypeIndex], 0, nMemDepth, pusWriteBuff);
	}
	ClearBind();

	for (int nMemTypeIndex = 0; nMemTypeIndex < nDataTypeCount; ++nMemTypeIndex)
	{
		m_CheckLog.SetCheckDataName(lpszDataTypeName[nMemTypeIndex]);
		for (auto& Controller : m_mapHardware)
		{
			memset(pusReadBuff, 0xBB, nMemDepth * sizeof(USHORT));

			Controller.second->ReadDataMemory(MEM_TYPE::DRAM, DataType[nMemTypeIndex], 0, nMemDepth, pusReadBuff);
			int nFailCount = 0;
			if (0 != memcmp(pusReadBuff, pusWriteBuff, nMemDepth * sizeof(USHORT)))
			{
				for (int nDataIndex = 0; nDataIndex < nMemDepth; ++nDataIndex)
				{
					if (pusReadBuff[nDataIndex] != pusWriteBuff[nDataIndex])
					{
						if (FAIL_SAVE_COUNT < nFailCount++)
						{
							break;
						}
						m_CheckLog.AddFailData(Controller.first, "0x%04X|0x%04X(0x%08X)", pusReadBuff[nDataIndex], pusWriteBuff[nDataIndex], nDataIndex);
					}
				}
			}
		}
	}

	if (nullptr != pusReadBuff)
	{
		delete[] pusReadBuff;
		pusReadBuff = nullptr;
	}
	if (nullptr != pusWriteBuff)
	{
		delete[] pusWriteBuff;
		pusWriteBuff = nullptr;
	}
	m_CheckLog.SaveLog();
	set<BYTE> setFailController;
	m_CheckLog.GetFailController(setFailController);
	return GetCheckResut(setFailController);
}

int CSelfCheck::InstructionCheck()
{
	FILE* pFileLog = nullptr;
	SaveTestItem(pFileLog, "Instruction Check");

	const int nLineOrderCount = 156;
	ULONG ulExpectLineOrder[nLineOrderCount] =
	{
		0x00000000, 0x00010000, 0x00020000, 0x00050000, 0x00060000, 0x00060000, 0x00060000, 0x00060000,
		0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000,
		0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000, 0x00060000,
		0x00060000, 0x00070000, 0x000A0000, 0x000B0000, 0x000C0000, 0x000D0000, 0x002D0000, 0x002E0000,
		0x002F0000, 0x000E0000, 0x000F0000, 0x00100000, 0x00110000, 0x00120000, 0x00130000, 0x00140000,
		0x00150000, 0x00160000, 0x00140000, 0x00150000, 0x00160000, 0x00140000, 0x00150000, 0x00160000,
		0x00140000, 0x00150000, 0x00160000, 0x00140000, 0x00150000, 0x00160000, 0x00140000, 0x00150000,
		0x00160000, 0x00170000, 0x00180000, 0x00190000, 0x001A0000, 0x00180000, 0x00190000, 0x001A0000,
		0x00180000, 0x00190000, 0x001A0000, 0x00180000, 0x00190000, 0x001A0000, 0x00180000, 0x00190000,
		0x001A0000, 0x00180000, 0x00190000, 0x001A0000, 0x001B0000, 0x001C0000, 0x001D0000, 0x001E0000,
		0x001C0000, 0x001D0000, 0x001E0000, 0x001C0000, 0x001D0000, 0x001E0000, 0x001C0000, 0x001D0000,
		0x001E0000, 0x001C0000, 0x001D0000, 0x001E0000, 0x001C0000, 0x001D0000, 0x001E0000, 0x001F0000,
		0x00200000, 0x00210000, 0x00220000, 0x00230000, 0x00240000, 0x00250000, 0x00260000, 0x00270000,
		0x00280000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000, 0x00290000,
		0x00290000, 0x00290000, 0x00290000, 0x00290000,
	};

	Bind();

	set<BYTE> setFailController;
	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;


	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; usChannel++)
	{
		vecChannel.push_back(usChannel);
	}
	pBindHardware->SetPeriod(0, 12);
	pBindHardware->SetPeriod(1, 500);

	double dEdge[EDGE_COUNT] = { 0,4,0,4,0,4 };
	pBindHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ);
	dEdge[4] = 150;
	dEdge[5] = 160;
	pBindHardware->SetEdge(vecChannel, 1, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ);

	CPattern Pattern(*pBindHardware, nullptr);
	UINT uLineIndex = 0;
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<0
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<1
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "JUMP", "", 5, FALSE, 0);           ///<2
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<3
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<4
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_FAIL", "", 23, FALSE, 0);      ///<5
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "REPEAT", "", 20, FALSE, 0);        ///<6
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 1, "FJUMP", "", 10, FALSE, 0);         ///<7
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<8
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);            ///<9
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "CLR_FAIL", "", 0, FALSE, 0);       ///<10
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "CLR_FAIL", "", 0, FALSE, 0);       ///<11
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 1, "INC", "", 43, FALSE, 0);           ///<12
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "CALL", "", 45, FALSE, 0);       	  ///<13
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<14
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<15
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<16
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<17
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<18
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_LOOPA", "", 5, FALSE, 0);	  ///<19
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<20
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<21
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "END_LOOPA", "", 20, FALSE, 0);	  ///<22
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_LOOPB", "", 5, FALSE, 0);	  ///<23
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<24
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<25
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "END_LOOPB", "", 24, FALSE, 0);	  ///<26
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_LOOPC", "", 5, FALSE, 0);	  ///<27
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<28
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<29
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "END_LOOPC", "", 28, FALSE, 0);	  ///<30
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_FAIL", "", 70, FALSE, 0);	  ///<31
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "SET_MCNT", "", 50, FALSE, 0);	  ///<32
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<33
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<34
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<35
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<36
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<37
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<38
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<39
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<40
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 1, "MJUMP", "", 0, FALSE, 0);		  ///<41
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<42
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, 0);			  ///<43
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<44
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<45
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<46
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "RETURN", "", 0, FALSE, 0);		  ///<47
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<48
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, 0);			  ///<49
	Pattern.AddPattern(uLineIndex++, TRUE, "0000000000000000", 0, "HLT", "", 0, FALSE, 0);			  ///<50

	Pattern.Load();

	pBindHardware->SetRunParameter(0, uLineIndex - 1, 0, 0);
	pBindHardware->SetPatternMode(TRUE);
	ClearBind();

	UINT uWaitTimes = uLineIndex * 500 / 10;
	UINT uCurTimes = 0;
	int nRetVal = 0;
	BOOL bNotFirstCheck = FALSE;
	CHardwareFunction* pCurController = nullptr;
	for (auto& Controller : m_mapHardware)
	{
		pCurController = Controller.second;

		pCurController->Run();
		bNotFirstCheck = FALSE;
		do
		{
			if (bNotFirstCheck)
			{
				pCurController->DelayUs(10);
				bNotFirstCheck = TRUE;
			}
			++uCurTimes;
			nRetVal = pCurController->GetRunningStatus();

		} while (1 != nRetVal && uWaitTimes > uCurTimes);
		if (uCurTimes >= uWaitTimes)
		{
			break;
		}
	}

	const int nOrderBuffCount = nLineOrderCount + 10;
	ULONG ulRealLineOrder[nOrderBuffCount] = { 0 };
	BOOL bPass = TRUE;
	for (auto& Controller : m_mapHardware)
	{
		pCurController = Controller.second;
		BYTE byControllerIndex = Controller.first;

		BOOL bControllerPass = TRUE;
		int nRealLineOrderCount = pCurController->GetLineOrderCount();
		if (nRealLineOrderCount != nLineOrderCount)
		{
			SaveSubTestItem(pFileLog, "Controller %d", byControllerIndex);
			fprintf_s(pFileLog, "Line order count: %d|%d\n", nLineOrderCount, nRealLineOrderCount);
			bPass = FALSE;
			bControllerPass = FALSE;
			nRealLineOrderCount = nRealLineOrderCount > nOrderBuffCount ? nOrderBuffCount : nRealLineOrderCount;
		}
		pCurController->ReadBRAMMemory(CHardwareFunction::RAM_TYPE::MEM_HIS_SVM, 0, nRealLineOrderCount, ulRealLineOrder);

		int nCheckStart = 0;
		if (0 == memcmp(ulRealLineOrder, ulExpectLineOrder, nLineOrderCount * sizeof(ULONG)))
		{
			if (bControllerPass)
			{
				fprintf_s(pFileLog, "Controller %d              PASS\n", byControllerIndex);
				continue;
			}
			nCheckStart = nLineOrderCount;
		}
		setFailController.insert(byControllerIndex);
		bPass = FALSE;
		bControllerPass = FALSE;
		string strLog;
		if (0 != nRealLineOrderCount)
		{
			strLog = FormatLog(10, 0, ' ', "Line Index");
			strLog += FormatLog(15, 1, ' ', "Real Order");
			strLog += FormatLog(15, 1, ' ', "Expect Order");
			strLog += '\n';
			fprintf_s(pFileLog, strLog.c_str());
		}
		for (int nLineIndex = nCheckStart; nLineIndex < nRealLineOrderCount; ++nLineIndex)
		{
			if (nLineIndex < nLineOrderCount)
			{
				if (ulExpectLineOrder[nLineIndex] == ulRealLineOrder[nLineIndex])
				{
					continue;
				}

				strLog = FormatLog(10, 0, ' ', "%d", nLineIndex);
				strLog += FormatLog(15, 1, ' ', "0x%03X", ulRealLineOrder[nLineIndex]);
				strLog += FormatLog(15, 1, ' ', "0x%03X", ulExpectLineOrder[nLineIndex]);
				strLog += '\n';
				fprintf_s(pFileLog, strLog.c_str());
				continue;
			}
			strLog = FormatLog(10, 0, ' ', "%d", nLineIndex);
			strLog += FormatLog(15, 1, ' ', "0x%03X", ulRealLineOrder[nLineIndex]);
			strLog += FormatLog(15, 1, ' ', "-");
			strLog += '\n';
			fprintf_s(pFileLog, strLog.c_str());
		}
		fprintf_s(pFileLog, "Controller %d              FAIL\n", byControllerIndex);
	}
	fprintf_s(pFileLog, "\n");
	fclose(pFileLog);

	return GetCheckResut(setFailController);
}

int CSelfCheck::HighSpeedMemoryCheck()
{
	m_CheckLog.SetTestItem("High speed memory Check");
	m_CheckLog.SetSubItem("Data Check");

	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;

	const BYTE byDataTypeCount = 3;
	USHORT usSpecialData[byDataTypeCount] = { 0xBBCC, 0xAACC, 0xABBA };
	const char lpszDataType[byDataTypeCount][4] = {"FM", "MM", "IOM"};
	CHardwareFunction::RAM_TYPE RAMType[byDataTypeCount] = { CHardwareFunction::RAM_TYPE::FM, CHardwareFunction::RAM_TYPE::MM, CHardwareFunction::RAM_TYPE::IOM };
	DATA_TYPE DataType[byDataTypeCount] = {DATA_TYPE::FM, DATA_TYPE::MM, DATA_TYPE::IOM};


	USHORT* pusBRAMExpectData = nullptr;
	UINT uBRAMCheckDataCount = BRAM_MAX_SAVE_FAIL_LINE_COUNT;
	UINT uDRAMCheckDataCount = DRAM_MAX_SAVE_FAIL_LINE_COUNT;
	map<int, USHORT> mapBRAMData;
	map<int, USHORT> mapDRAMData;
	try
	{
		pusBRAMExpectData = new USHORT[uBRAMCheckDataCount];
		memset(pusBRAMExpectData, 0, uBRAMCheckDataCount * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -1;
	}

	Bind();
	set<BYTE> setFailController;

	pBindHardware->SetPeriod(0, 16);
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	double dEdge[EDGE_COUNT] = { 0,4,0,4,0,4 };
	pBindHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);

	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::OPERAND, 0, uBRAMCheckDataCount, pusBRAMExpectData);
	pusBRAMExpectData[uBRAMCheckDataCount / 2] = 0x17;
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::CMD, 0, uBRAMCheckDataCount, pusBRAMExpectData);

	pusBRAMExpectData[uBRAMCheckDataCount / 2] = 0;
	pusBRAMExpectData[uDRAMCheckDataCount - 1] = 0x01;
	pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::CMD, 0, uDRAMCheckDataCount, pusBRAMExpectData);
// 	pusBRAMExpectData[uDRAMCheckDataCount - 1] = 0x00;
// 	pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::OPERAND, 0, uDRAMCheckDataCount, pusBRAMExpectData);


	for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount;++uDataIndex)
	{
		pusBRAMExpectData[uDataIndex] = uDataIndex | (uDataIndex << 10) | (uDataIndex << 20) | (uDataIndex << 30);
		if (0 != pusBRAMExpectData[uDataIndex])
		{
			mapBRAMData.insert(make_pair(uDataIndex, pusBRAMExpectData[uDataIndex]));
		}
		if (uDataIndex < uDRAMCheckDataCount)
		{
			mapDRAMData.insert(make_pair(uDataIndex, pusBRAMExpectData[uDataIndex]));
		}
	}
	auto iterData = mapBRAMData.begin();
	for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount; uDataIndex += 512)
	{
		pusBRAMExpectData[uDataIndex] = usSpecialData[0];		
	}
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::FM, 0, uBRAMCheckDataCount, pusBRAMExpectData);
	pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::FM, 0, uDRAMCheckDataCount, pusBRAMExpectData);

	for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount; uDataIndex += 512)
	{
		pusBRAMExpectData[uDataIndex] = usSpecialData[1];
	}
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::MM, 0, uBRAMCheckDataCount, pusBRAMExpectData);
	pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::MM, 0, uDRAMCheckDataCount, pusBRAMExpectData);
	for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount; uDataIndex += 512)
	{
		pusBRAMExpectData[uDataIndex] = usSpecialData[2];
	}
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::IOM, 0, uBRAMCheckDataCount, pusBRAMExpectData);
	pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DATA_TYPE::IOM, 0, uDRAMCheckDataCount, pusBRAMExpectData);

	for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount; uDataIndex += 512)
	{
		pusBRAMExpectData[uDataIndex] = uDataIndex | (uDataIndex << 10) | (uDataIndex << 20) | (uDataIndex << 30);
	}

	ClearBind();

	int nRetVal = 0;

	UINT uWaitTimes = (UINT)10;
	UINT uCurrentTimes = 0;

	map<UINT, int> mapRunningStatus;
	auto iterStatus = mapRunningStatus.begin();

	for (BYTE byDataTypeIndex = 0; byDataTypeIndex < byDataTypeCount; ++byDataTypeIndex)
	{
		m_CheckLog.SetCheckDataName(lpszDataType[byDataTypeIndex]);

		Bind();
		pBindHardware->SetRunParameter(0, uBRAMCheckDataCount - 1, TRUE, 0);
		pBindHardware->SetPatternMode(FALSE, DataType[byDataTypeIndex], TRUE, FALSE);
		pBindHardware->SynRun();
		pBindHardware->EnableStart(FALSE);
		ClearBind();

		map<UINT, USHORT> mapAddData;
		map<UINT, USHORT> mapBackupData;

		for (UINT uDataIndex = 0; uDataIndex < uBRAMCheckDataCount; uDataIndex += 512)
		{
			mapBackupData.insert(make_pair(uDataIndex, pusBRAMExpectData[uDataIndex]));

			iterData = mapBRAMData.find(uDataIndex);
			if (mapBRAMData.end() != iterData)
			{
				iterData->second = usSpecialData[byDataTypeIndex];
			}
			else
			{
				mapBRAMData.insert(make_pair(uDataIndex, usSpecialData[byDataTypeIndex]));
			}
			if (uDataIndex >= uDRAMCheckDataCount)
			{
				continue;
			}
			iterData = mapDRAMData.find(uDataIndex);
			if (mapDRAMData.end() != iterData)
			{
				iterData->second = usSpecialData[byDataTypeIndex];
			}
			else
			{
				mapDRAMData.insert(make_pair(uDataIndex, usSpecialData[byDataTypeIndex]));
			}
		}
		BYTE byControllerIndex = 0;
		uCurrentTimes = 0;
		for (auto& Controller : m_mapHardware)
		{
			byControllerIndex = Controller.first;

			do
			{
				if (0 != uCurrentTimes++)
				{
					Controller.second->DelayUs(10);
				}
				nRetVal = Controller.second->GetRunningStatus();
			} while (0 == nRetVal && uWaitTimes > uCurrentTimes);

			///<Save the running status
			iterStatus = mapRunningStatus.find(Controller.first);
			if (mapRunningStatus.end() == iterStatus)
			{
				mapRunningStatus.insert(make_pair(Controller.first, nRetVal));
			}
			else
			{
				if (1 != nRetVal)
				{
					iterStatus->second = nRetVal;
				}
			}
			int nFailIndex = 0;
			map<int, USHORT> mapCurBRAMData;
			map<int, USHORT> mapCurDRAMData;
			vector<CHardwareFunction::DATA_RESULT> vecBRAMFail;
			vector<CHardwareFunction::DATA_RESULT> vecDRAMFail;
			Controller.second->GetFailData(vecBRAMFail, vecDRAMFail);

			auto CopyMap = [&](BOOL bBRAM)
			{
				auto& vecFail = bBRAM ? vecBRAMFail : vecDRAMFail;
				auto& mapFail = bBRAM ? mapCurBRAMData : mapCurDRAMData;
				for (auto& Data : vecFail)
				{
					mapFail.insert(make_pair(Data.m_nLineNo, Data.m_usData));
				}
			};
			CopyMap(TRUE);
			CopyMap(FALSE);

			auto CheckResult = [&](BOOL bBRAM, map<int, USHORT>& mapReal, map<int, USHORT>& mapExpected)
			{
				if (mapReal == mapExpected)
				{
					return;
				}
				char cType = bBRAM ? 'B' : 'D';
				iterData = mapExpected.begin();
				for (auto& Data : mapReal)
				{
					if (mapExpected.end() != iterData)
					{
						if (iterData->first != Data.first || iterData->second != Data.second)
						{
							m_CheckLog.AddFailData(Controller.first, "%c:0x%08X(0x%08X)|0x%08X(0x%08X),%d",
								cType, Data.second, Data.first, iterData->second, iterData->first, nFailIndex);
						}
						++iterData;
						++nFailIndex;
					}
					else
					{
						m_CheckLog.AddFailData(Controller.first, "%c:0x%08X(0x%08X)|-,%d",
							cType, Data.second, Data.first, nFailIndex);
					}

					if (FAIL_SAVE_COUNT < nFailIndex++)
					{
						break;
					}
				}
				while (FAIL_SAVE_COUNT > nFailIndex++ && mapExpected.end() != iterData)
				{
					m_CheckLog.AddFailData(Controller.first, "%c:-|0x%08X(0x%08X),%d",
						cType, iterData->second, iterData->first, nFailIndex++);
					++iterData;
				}
			};
			CheckResult(TRUE, mapCurBRAMData, mapBRAMData);
			CheckResult(FALSE, mapCurDRAMData, mapDRAMData);
		}


		for (auto& BackupData : mapBackupData)
		{
			iterData = mapBRAMData.find(BackupData.first);
			if (0 == BackupData.second)
			{
				if (mapBRAMData.end() != iterData)
				{
					mapBRAMData.erase(iterData);
				}
			}
			else
			{
				if (mapBRAMData.end() != iterData)
				{
					iterData->second = BackupData.second;
				}
			}
			iterData = mapDRAMData.find(BackupData.first);
			if (0 == BackupData.second)
			{
				if (mapDRAMData.end() != iterData)
				{
					mapDRAMData.erase(iterData);
				}
			}
			else
			{
				if (mapDRAMData.end() != iterData)
				{
					iterData->second = BackupData.second;
				}
			}
		}
	}
	if (nullptr != pusBRAMExpectData)
	{
		delete[] pusBRAMExpectData;
		pusBRAMExpectData = nullptr;
	}
	m_CheckLog.SaveLog();
	m_CheckLog.GetFailController(setFailController);

	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	if (nullptr == pFileLog)
	{
		fopen_s(&pFileLog, m_strFileName.c_str(), "wa+");
	}
	SaveSubTestItem(pFileLog, "Running Status Check");
	string strHead = FormatLog(15, 0, ' ', "");
	strHead += FormatLog(10, 1, ' ', "Status");
	strHead += FormatLog(10, 1, ' ', "Result");
	strHead += "\n";
	fprintf_s(pFileLog, strHead.c_str());

	string strLog;
	for (auto& Controller : m_mapHardware)
	{
		strLog = FormatLog(15, 0, ' ', "Controller %d", Controller.first);
		iterStatus = mapRunningStatus.find(Controller.first);
		if (mapRunningStatus.end() == iterStatus)
		{
			///<Not will happen
			continue;
		}
		strLog += FormatLog(10, 1, ' ',  "%d", iterStatus->second);
		if (1 != iterStatus->second)
		{
			strLog += FormatLog(10, 1, ' ', "FAIL");
			if (setFailController.end() == setFailController.find(Controller.first))
			{
				setFailController.insert(Controller.first);
			}
		}
		else
		{
			strLog += FormatLog(10, 1, ' ', "PASS");
		}
		strLog += "\n";
		fprintf_s(pFileLog, strLog.c_str());
	}

	fclose(pFileLog);

	return GetCheckResut(setFailController);
}

int CSelfCheck::PMUCheck(int* pnChannelResult)
{
	memset(pnChannelResult, 0, DCM_MAX_CHANNELS_PER_BOARD * sizeof(int));
	
	set<USHORT> setNotUseCalibrationChannel;///<The channel whose calibration data is not be used

	FILE* pFileLog = nullptr;
	SaveTestItem(pFileLog, "PMU Check");
	const int nVoltageCount = 8;
	double dVoltage[nVoltageCount] = { -1.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 };
	double dError = 0.2;
	int nTypeByteCount = 10;
	string strHead = FormatLog(5, 0, ' ', "No");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Target");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Min");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Max");
	strHead += FormatLog(5, 1, ' ', "Unit");
	vector<USHORT> vecControllerChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecControllerChannel.push_back(usChannel);
	}
	BOOL bPass = TRUE;
	double dMeasVoltage = 0;
	string strLog;
	set<BYTE> setFailController;
	BYTE byControllerIndex = 0;
	for (auto& Controller : m_mapHardware)
	{
		byControllerIndex = Controller.first;
		SaveSubTestItem(pFileLog, "Controller %d", byControllerIndex);
		
		///<Save head
		strLog = strHead;
		USHORT ucCurChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL;

		BOOL bChannelResult[DCM_CHANNELS_PER_CONTROL] = { 0 };
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
		{
			strLog += FormatLog(nTypeByteCount, 1, ' ', "CH%02d", ucCurChannel++);
			bChannelResult[usChannel] = TRUE;
		}
		strLog += "\n";
		fprintf_s(pFileLog, strLog.c_str());
		BOOL bUseCalibration[DCM_CHANNELS_PER_CONTROL] = { 0 };
		memset(bUseCalibration, 1, sizeof(bUseCalibration));
		BOOL bContinue = FALSE;

		for (int nVoltageIndex = 0; nVoltageIndex < nVoltageCount; ++nVoltageIndex)
		{
			do
			{
				bContinue = FALSE;
				Controller.second->SetPMUMode(vecControllerChannel, PMU_MODE::FVMV, PMU_IRANGE::IRANGE_2UA, dVoltage[nVoltageIndex], 7.5, -2.5);
				Controller.second->DelayMs(1);
				int nRetVal = Controller.second->PMUMeasure(vecControllerChannel, 10, 10);
				if (0 != nRetVal)
				{
					do 
					{
						nRetVal = Controller.second->WaitPMUFinish();
						nRetVal = Controller.second->StartPMU();
						if (0 != nRetVal)
						{
							break;
						}
					} while (0 == nRetVal);
				}

				double dMaxVoltage = dVoltage[nVoltageIndex] + dError;
				double dMinVoltage = dVoltage[nVoltageIndex] - dError;

				strLog = FormatLog(5, 0, ' ', "%d", nVoltageIndex);
				strLog += FormatLog(nTypeByteCount, 1, ' ', "%.3f", dVoltage[nVoltageIndex]);
				strLog += FormatLog(nTypeByteCount, 1, ' ', "%.3f", dMinVoltage);
				strLog += FormatLog(nTypeByteCount, 1, ' ', "%.3f", dMaxVoltage);
				strLog += FormatLog(5, 1, ' ', "V");

				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
				{
					dMeasVoltage = Controller.second->GetPMUMeasureResult(usChannel, -1);

					char cSign = ' ';
					if (dMinVoltage - 1e-6 > dMeasVoltage || dMaxVoltage + 1e-6 < dMeasVoltage)
					{
						if (bUseCalibration[usChannel])
						{
							///<Break when fail channel use calibration data
							CCalibration::Instance()->ResetCalibrationData(m_bySlotNo, byControllerIndex, usChannel);
							bUseCalibration[usChannel] = FALSE;
							bContinue = TRUE;
							setNotUseCalibrationChannel.insert(Controller.first * DCM_CHANNELS_PER_CONTROL + usChannel);
							continue;
						}
						cSign = '*';
						bPass = FALSE;
						bChannelResult[usChannel] = FALSE;
					}
					strLog += FormatLog(nTypeByteCount, 1, ' ', "%.3f%c", dMeasVoltage, cSign);
				}
			} while (bContinue);

			strLog += "\n";
			fprintf_s(pFileLog, strLog.c_str());
		}
		strLog.clear();
		strLog.append(nTypeByteCount * 4, ' ');
		
		USHORT usChannelOffset = byControllerIndex * DCM_CHANNELS_PER_CONTROL;
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			string strResult = "PASS";
			if (!bChannelResult[usChannel])
			{
				setFailController.insert(byControllerIndex);
				strResult = "FAIL";
			}
			if (bPass)
			{
				pnChannelResult[usChannel + usChannelOffset] = 1;
			}
			strLog += FormatLog(nTypeByteCount, 1, ' ', strResult.c_str());
		}
		strLog += "\n";
		fprintf_s(pFileLog, strLog.c_str());
	}
	fclose(pFileLog);

	if (0 != setNotUseCalibrationChannel.size())
	{
		string strWaring = "PMU(Not using calibration data): ";
		char lpszMsg[32] = { 0 };
		for (auto Channel : setNotUseCalibrationChannel)
		{
			sprintf_s(lpszMsg, sizeof(lpszMsg), "%d,", Channel);
			strWaring += lpszMsg;
		}
		strWaring.erase(strWaring.size() - 1);
		m_vecWarning.push_back(strWaring);
	}


	return GetCheckResut(setFailController);
}

int CSelfCheck::TMUCheck()
{
	FILE* pFileLog = nullptr;
	SaveTestItem(pFileLog, "TMU Check");

	double dError = 0.2;
	int nTypeByteCount = 10;
	string strHead = FormatLog(nTypeByteCount, 0, ' ', "Type");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Target");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Min");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "Max");
	strHead += FormatLog(5, 1, ' ', "Unit");

	for (auto& Controller : m_mapHardware)
	{
		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			strHead += FormatLog(nTypeByteCount, 1, ' ', "Unit %d", Controller.first * TMU_UNIT_COUNT_PER_CONTROLLER + byUnitIndex);
		}
	}
	strHead += "\n";
	fprintf_s(pFileLog, strHead.c_str());


	vector<USHORT> vecControllerChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecControllerChannel.push_back(usChannel);
	}

	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;

	string strLog;
	set<BYTE> setFailUnit;
	set<BYTE> setFailController;
	vector<USHORT> vecChannel;
	vecChannel.push_back(0);
	for (auto& Controller : m_mapHardware)
	{
		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			Controller.second->SetTMUUnitChannel(0, byUnitIndex);
		}
		Controller.second->SetTMUParam(vecChannel, TRUE, 300, 0);

		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			Controller.second->EnableTMUSelfCheck(byUnitIndex, TRUE);
		}
		Controller.second->TMUMeasure(vecChannel, TMU_MEAS_MODE::DUTY_PERIOD, 15, 1);
	}

	const BYTE byTypeCount = 2;
	const char lpszTypeName[byTypeCount][12] = {"Period", "Duty"};
	const char lpszUnits[byTypeCount][4] = {"ns", "%"};
	double dTarget[byTypeCount] = {40, 50};
	double dResolution[byTypeCount] = { 1.5,5 };
	TMU_MEAS_TYPE MeasType[byTypeCount] = {TMU_MEAS_TYPE::FREQ, TMU_MEAS_TYPE::HIGH_DUTY};

	for (int nTypeIndex = 0; nTypeIndex < byTypeCount; ++nTypeIndex)
	{
		strLog = FormatLog(nTypeByteCount, 0, ' ', "%s", lpszTypeName[nTypeIndex]);
		strLog += FormatLog(nTypeByteCount, 1, ' ', "%.2f", dTarget[nTypeIndex]);
		strLog += FormatLog(nTypeByteCount, 1, ' ', "%.2f", dTarget[nTypeIndex] - dResolution[nTypeIndex]);
		strLog += FormatLog(nTypeByteCount, 1, ' ', "%.2f", dTarget[nTypeIndex] + dResolution[nTypeIndex]);
		strLog += FormatLog(5, 1, ' ', "%s", lpszUnits[nTypeIndex]);

		for (auto& Controller : m_mapHardware)
		{
			double dMeasResult = 0;
			for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
			{
				int nTMUErrorCode = 0;
				dMeasResult = Controller.second->GetTMUUnitMeasureResult(byUnitIndex, MeasType[nTypeIndex], nTMUErrorCode);
				if (-EQUAL_ERROR < dMeasResult)
				{
					if (0 == nTypeIndex)
					{
						dMeasResult = 1 / dMeasResult * 1e6;
					}
				}
				char cSign = ' ';
				if (dResolution[nTypeIndex] < fabs(dMeasResult - dTarget[nTypeIndex]))
				{
					cSign = '*';

					BYTE byCurUnit = Controller.first * TMU_UNIT_COUNT_PER_CONTROLLER + byUnitIndex;
					if (setFailUnit.end() == setFailUnit.find(byCurUnit))
					{
						setFailUnit.insert(byCurUnit);
					}
					if (setFailController.end() == setFailController.find(Controller.first))
					{
						setFailController.insert(Controller.first);
					}
				}
				strLog += FormatLog(nTypeByteCount, 1, ' ', "%.2f%c", dMeasResult, cSign);
			}
		}
		strLog += "\n";
		fputs(strLog.c_str(), pFileLog);
	}


	for (auto& Controller : m_mapHardware)
	{
		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			Controller.second->SetTMUUnitChannel(0, byUnitIndex);
		}
		Controller.second->SetTMUParam(vecChannel, TRUE, 300, 0);

		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			Controller.second->EnableTMUSelfCheck(byUnitIndex, TRUE);
		}
		Controller.second->TMUMeasure(vecChannel, TMU_MEAS_MODE::DUTY_PERIOD, 15, 1);
	}

	strHead = FormatLog(nTypeByteCount, 1, ' ', "");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "");
	strHead += FormatLog(nTypeByteCount, 1, ' ', "");
	strHead += FormatLog(5, 1, ' ', " ");

	for (auto& Controller : m_mapHardware)
	{
		for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnitIndex)
		{
			BYTE byCurUnit = Controller.first * TMU_UNIT_COUNT_PER_CONTROLLER + byUnitIndex;
			if (setFailUnit.end() != setFailUnit.find(byCurUnit))
			{
				strHead += FormatLog(nTypeByteCount, 1, ' ', "FAIL");
			}
			else
			{
				strHead += FormatLog(nTypeByteCount, 1, ' ', "PASS");
			}
			Controller.second->EnableTMUSelfCheck(byUnitIndex, TRUE);
		}
	}
	strHead += "\n";
	fprintf_s(pFileLog, strHead.c_str());
	fclose(pFileLog);
	return GetCheckResut(setFailController);
}

int CSelfCheck::SerialParallelTransferCheck()
{
	FILE* pFileLog = nullptr;
	m_CheckLog.SetTestItem("Serial Parallel transformation Check");
	

	const BYTE byDataTypeCount = 3;
	char lpszDataType[byDataTypeCount][8] = { "FM", "MM", "IOM" };


	CHardwareFunction* pBindHardware = m_mapHardware.begin()->second;

	int nCheckLineCount = 1024;
	int nDWCountPerWrite = (nCheckLineCount + 31) / 32;
	USHORT* pusWriteData = nullptr;
	USHORT* pusReadData = nullptr;
	ULONG* pulChannelData = nullptr;

	try
	{
		pusWriteData = new USHORT[nCheckLineCount];
		memset(pusWriteData, 0xFF, nCheckLineCount * sizeof(USHORT));
		pusReadData = new USHORT[nCheckLineCount];
		memset(pusReadData, 0, nCheckLineCount * sizeof(USHORT));
		pulChannelData = new ULONG[nDWCountPerWrite];
		memset(pulChannelData, 0xA5, nDWCountPerWrite * sizeof(ULONG));
	}
	catch (const std::exception&)
	{
		fprintf_s(pFileLog, "Allocate memory FAIL\n");
		return 0;
	}

	USHORT usCheckChannel = 0;
	Bind();
	int nStartAddress = 0;
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::FM, 0, nCheckLineCount, pusWriteData);
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::MM, 0, nCheckLineCount, pusWriteData);
	pBindHardware->WriteDataMemory(MEM_TYPE::BRAM, DATA_TYPE::IOM, 0, nCheckLineCount, pusWriteData);

	pBindHardware->WriteChannelDataMemory(MEM_TYPE::BRAM, DATA_TYPE::FM, usCheckChannel, 0, nCheckLineCount, pulChannelData);
	pBindHardware->WriteChannelDataMemory(MEM_TYPE::BRAM, DATA_TYPE::MM, usCheckChannel, 0, nCheckLineCount, pulChannelData);
	pBindHardware->WriteChannelDataMemory(MEM_TYPE::BRAM, DATA_TYPE::IOM, usCheckChannel, 0, nCheckLineCount, pulChannelData);

	ClearBind();


	for (int nLineIndex = 0; nLineIndex < nCheckLineCount; ++nLineIndex)
	{
		if (0 == (pulChannelData[nLineIndex / 32] >> nLineIndex & 0x01))
		{
			pusWriteData[nLineIndex] &= ~(1 << usCheckChannel);
		}
		else
		{
			pusWriteData[nLineIndex] |= 1 << usCheckChannel;
		}
	}
	DATA_TYPE DataType[byDataTypeCount] = { DATA_TYPE::FM, DATA_TYPE::MM, DATA_TYPE::IOM };

	for (BYTE byDataTypeIndex = 0; byDataTypeIndex < byDataTypeCount; ++byDataTypeIndex)
	{
		m_CheckLog.SetCheckDataName(lpszDataType[byDataTypeIndex]);
		int nMaxFailCount = 0;
		for (auto& Controller : m_mapHardware)
		{
			int nFailCount = 0;
			pBindHardware->ReadDataMemory(MEM_TYPE::BRAM, DataType[byDataTypeIndex], 0, nCheckLineCount, pusReadData);
			if (0 != memcmp(pusReadData, pusWriteData, nCheckLineCount * sizeof(USHORT)))
			{
				for (UINT uDataIndex = 0; uDataIndex < nCheckLineCount; ++uDataIndex)
				{
					if (pusReadData[uDataIndex] != pusWriteData[uDataIndex])
					{
						if (FAIL_SAVE_COUNT < nFailCount++)
						{
							break;
						}
						m_CheckLog.AddFailData(Controller.first, "0x%04X|0x04X(%d)", pusReadData[uDataIndex], pusWriteData[uDataIndex], uDataIndex);
					}
				}

			}
		}
	}
	if (nullptr != pulChannelData)
	{
		delete[] pulChannelData;
		pulChannelData = nullptr;
	}
	if (nullptr == pusWriteData)
	{
		delete[] pusWriteData;
		pulChannelData = nullptr;
	}
	if (nullptr == pusReadData)
	{
		delete[] pusReadData;
		pusReadData = nullptr;
	}
	m_CheckLog.SaveLog();
	set<BYTE> setFailController;
	m_CheckLog.GetFailController(setFailController);
	return GetCheckResut(setFailController);
}

void CSelfCheck::Bind()
{
	if (0 >= m_mapHardware.size() || 0 >= m_byValidControllerCount)
	{
		return;
	}
	set<BYTE> setSlot;
	setSlot.insert(m_bySlotNo);
	set<BYTE> setController;
	
	for (auto& Controller : m_mapHardware)
	{
		setController.insert(Controller.first);
	}
	CBindInfo::Instance()->Bind(setSlot, setController, m_bySlotNo);
}

void CSelfCheck::ClearBind()
{
	if (0 >= m_mapHardware.size())
	{
		return;
	}
	CBindInfo::Instance()->ClearBind();
}

inline void CSelfCheck::SaveLog(FILE* const pFileLog, const char* lpszFormat, ...)
{
	if (nullptr == pFileLog)
	{
		return;
	}
	if (nullptr == lpszFormat)
	{
		return;
	}

	va_list ap;
	va_start(ap, lpszFormat);
	vfprintf_s(pFileLog, lpszFormat, ap);
	va_end(ap);
}

inline string& CSelfCheck::FormatLog(int nMaxByteCount, BYTE bAlignType, char cAddCharacter, const char* lpszFormat, ...)
{
	m_strLogMsg.clear();
	if (0 == nMaxByteCount)
	{
		return m_strLogMsg;
	}
	char lpszLog[128] = { 0 };
	if (nullptr != lpszFormat)
	{
		va_list ap;
		va_start(ap, lpszFormat);
		vsprintf_s(lpszLog, sizeof(lpszLog), lpszFormat, ap);
		va_end(ap);
	}
	m_strLogMsg = lpszLog;
	int nAppendBlank = nMaxByteCount - m_strLogMsg.size();
	if (0 <= nAppendBlank)
	{
		int nLeftAppend = 0;
		int nRightAppend = nAppendBlank;
		switch (bAlignType)
		{
		case 0:
			///<Left alignment
			break;
		case 1:
			///<Middle alignment
			nLeftAppend = nAppendBlank / 2;
			nRightAppend = nAppendBlank - nLeftAppend;
			break;
		case 2:
			///<Right alignment
			nLeftAppend = nAppendBlank;
			nRightAppend = 0;
			break;
		default:
			break;
		}
		for (int nIndex = 0; nIndex < nLeftAppend; ++nIndex)
		{
			m_strLogMsg = cAddCharacter + m_strLogMsg;
		}
		for (int nIndex = 0; nIndex < nRightAppend; ++nIndex)
		{
			m_strLogMsg += cAddCharacter;
		}
	}
	else
	{
		m_strLogMsg.erase(nMaxByteCount - 1, -nAppendBlank);
	}
	return m_strLogMsg;
}

inline void CSelfCheck::SaveTestItem(FILE*& pFileLog, const char* lpszItemName)
{
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	if (nullptr == pFileLog)
	{
		fopen_s(&pFileLog, m_strFileName.c_str(), "wa+");
	}
	string strFormat = "\n";
	strFormat += FormatLog(60, 1, '-', lpszItemName);
	strFormat += "\n";
	fprintf_s(pFileLog, strFormat.c_str());
}

inline void CSelfCheck::SaveSubTestItem(FILE* const pFileLog, const char* lpszItemName, ...)
{
	if (nullptr == pFileLog || nullptr == lpszItemName)
	{
		return;
	}
	fprintf_s(pFileLog, "\n----------");

	char lpszMsg[64] = { 0 };
	va_list ap;
	va_start(ap, lpszItemName);
	vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszItemName, ap);
	va_end(ap);

	fprintf_s(pFileLog, lpszMsg);
	fprintf_s(pFileLog, "----------\n");	
}

void CSelfCheck::NoBoardExist(int* pnCheckResult, const char* lpszFormat, ...)
{
	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	for (int nIndex = 0; nIndex < SELFCHECK_FILE_DASH_NUM; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");

	if (nullptr != lpszFormat)
	{
		va_list ap;
		va_start(ap, lpszFormat);

		char lpszMsg[128] = { 0 };
		vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, ap);
		va_end(ap);

		fprintf_s(pFileLog, lpszMsg);
		fprintf_s(pFileLog, "\n");
	}
	string strLog = FormatLog(20, 0, ' ', "Check result");
	strLog += FormatLog(10, 2, ' ', "FAIL");
	strLog += "\n";
	fprintf_s(pFileLog, strLog.c_str());
	fclose(pFileLog);

	memset(pnCheckResult, 0, DCM_MAX_CHANNELS_PER_BOARD * sizeof(int));
}

int CSelfCheck::CheckAuthorization(BOOL& bCheckPass)
{
	bCheckPass = TRUE;
	FILE* pFileLog = nullptr;
	SaveTestItem(pFileLog, "Controller Authorized Check");

	string strLog = FormatLog(15, 0, ' ', " ");
	for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD;++byController)
	{
		strLog += FormatLog(15, 1, ' ', "Controller %d", byController);
	}
	strLog += "\n";
	fprintf_s(pFileLog, strLog.c_str());

	strLog = FormatLog(15, 0, ' ', "Authorized");
	int nSignIndex = 0;
	char lpszSign[3][12] = { "Y","N", '?' };
	for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController)
	{
		if (m_byRecordControllerCount <= byController)
		{
			nSignIndex = 1;
		}
		else
		{
			nSignIndex = 0;
		}

		strLog += FormatLog(15, 1, ' ', lpszSign[nSignIndex]);
	}
	strLog += "\n";
	fprintf_s(pFileLog, strLog.c_str());
	int nFailSynType = -1;
	map<BYTE, BYTE> mapFailSyn;
	set<BYTE> setFailController;
	vector<BYTE> vecFailSyn;
	vector<string> vecMsg;
	char lpszMsg[128] = { 0 };
	CHardwareFunction* pHardware = nullptr;
	strLog = FormatLog(15, 0, ' ', "Existed");
	for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController)
	{
		auto iterHardware = m_mapHardware.find(byController);
		if (m_mapHardware.end() == iterHardware)
		{
			///<The controller is not existed when check
			CHardwareFunction Hardware(m_bySlotNo);
			Hardware.SetControllerIndex(byController);
			if (Hardware.IsControllerExist())
			{
				if (m_byRecordControllerCount <= byController)
				{
					nSignIndex = 0;
				}
				else
				{
					nSignIndex = 2;
				}
			}
			else
			{
				nSignIndex = 1;
			}
		}
		else
		{
			if (nullptr == pHardware)
			{
				pHardware = iterHardware->second;
			}
			if (!iterHardware->second->IsControllerExist())
			{
				///<Controller not existed
				nSignIndex = 1;
			}
			else
			{
				///<Controller existed
				nSignIndex = 0;
			}

			if (-1 == nFailSynType)
			{
				///<Get the fail sychronized type
				nFailSynType = pHardware->GetFailSynType();
			}

			///<Fail synchronous type
			if (0 == nFailSynType)
			{
				///<Only support sychronize first serval controller's fail signal
				if (0 != mapFailSyn.size())
				{
					mapFailSyn.begin()->second |= 1 << byController;
				}
				else
				{
					mapFailSyn.insert(make_pair(0, 1 << byController));
				}
			}
			else if (mapFailSyn.end() == mapFailSyn.find(byController))
			{
				///<Synchronize its own fail signal
				mapFailSyn.insert(make_pair(byController, 0));
			}
		}
		strLog += FormatLog(15, 1, ' ', lpszSign[nSignIndex]);
		
		if (1 == nSignIndex && m_byRecordControllerCount > byController)
		{
			setFailController.insert(byController);

			bCheckPass = FALSE;
			sprintf_s(lpszMsg, sizeof(lpszMsg), "The controller %d is authorized, but it is not existed", byController);
			vecMsg.push_back(lpszMsg);
		}
		else if (0 == nSignIndex && m_byRecordControllerCount <= byController)
		{
			setFailController.insert(byController);
			bCheckPass = FALSE;
			sprintf_s(lpszMsg, sizeof(lpszMsg), "The controller %d is unauthorized, but it is existed", byController);
			vecMsg.push_back(lpszMsg);
		}
		else if (2 == nSignIndex)
		{
			setFailController.insert(byController);
			bCheckPass = FALSE;
			sprintf_s(lpszMsg, sizeof(lpszMsg), "The controller %d is not existed when checking", byController);
			vecMsg.push_back(lpszMsg);
		}
	}
	pHardware->SetFailSyn(mapFailSyn);
	strLog += "\n\n";
	fprintf_s(pFileLog, strLog.c_str());

	if (bCheckPass)
	{
		strLog = FormatLog(20, 0, ' ', "Authorized Check");
		strLog += FormatLog(10, 2, ' ', "PASS");
		strLog += "\n";
		fprintf_s(pFileLog, strLog.c_str());
	}
	else
	{
		strLog = FormatLog(20, 0, ' ', "Authorized Check");
		strLog += FormatLog(10, 2, ' ', "FAIL");
		strLog += "\n";
		fprintf_s(pFileLog, strLog.c_str());
		SaveSubTestItem(pFileLog, "Detail fail information");
		int nMsgCount = vecMsg.size();
		for (int nIndex = 0; nIndex < nMsgCount; ++nIndex)
		{
			fprintf_s(pFileLog, vecMsg[nIndex].c_str());
			fprintf_s(pFileLog, "\n");
		}
	}

	fclose(pFileLog);

	return GetCheckResut(setFailController);
}

int CSelfCheck::SetChannelCheckResult(BYTE byControlCheckResult, int* pnCheckResult)
{
	int nCheckResult = 1;
	memset(pnCheckResult, 0, DCM_CHANNELS_PER_CONTROL * sizeof(int));

	for (auto& Controller : m_mapHardware)
	{
		BYTE byControllerIndex = Controller.first;
		if (0 != (byControlCheckResult >> byControllerIndex & 0x01))
		{
			USHORT usChannelOffset = byControllerIndex * DCM_CHANNELS_PER_CONTROL;
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				pnCheckResult[usChannel + usChannelOffset] = 1;
			}
		}
		else
		{
			nCheckResult = 0;
		}
	}
	return nCheckResult;
}

inline int CSelfCheck::GetCheckResut(std::set<BYTE>& setFailController)
{
	int nCheckResult = 0;
	for (auto& Controller : m_mapHardware)
	{
		if (setFailController.end() == setFailController.find(Controller.first))
		{
			nCheckResult |= 1 << Controller.first;
		}
	}
	return nCheckResult;
}

void CSelfCheck::InitializeChannelStatus()
{
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	for (auto& Controller : m_mapHardware)
	{
		Controller.second->SetChannelStatus(vecChannel, CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE);
	}
}

void CSelfCheck::PrintWarning()
{
	if (0 == m_vecWarning.size())
	{
		return;
	}
	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strFileName.c_str(), "a+");
	fprintf_s(pFileLog, "\n");
	for (int nIndex = 0; nIndex < 30; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");

	fprintf_s(pFileLog, "Warning\n");
	for (auto& Warning : m_vecWarning)
	{
		fprintf_s(pFileLog, Warning.c_str());
		fprintf_s(pFileLog, "\n");
	}
	fclose(pFileLog);
	pFileLog = nullptr;
}
