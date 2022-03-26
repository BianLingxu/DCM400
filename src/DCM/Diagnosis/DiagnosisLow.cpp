#include "DiagnosisLow.h"
#include <Windows.h>
#include "IHDReportDevice.h"
#include "..\HDModule.h"
using namespace std;
CDiagnosisLow::CDiagnosisLow()
{
}

CDiagnosisLow::~CDiagnosisLow()
{
	m_setFailController.clear();
}

int CDiagnosisLow::QueryInstance(const char * name, void ** ptr)
{
    return -1;
}

void CDiagnosisLow::Release()
{}

const char * CDiagnosisLow::Name() const
{
    return "Low Speed Memory Diagnosis";
}

int CDiagnosisLow::GetChildren(STSVector<IHDDoctorItem *> & children) const
{
	return 0;
}

int CDiagnosisLow::Doctor(IHDReportDevice * pReportDevice)
{
	m_pReportDevice = pReportDevice;
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	const char* lpszBaseIndent = IndentFormat();
	std::string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<LowSpeedMemoryDiagnosis>\n", lpszBaseIndent);
	int nRet = -1;
    int nFailCount = 0;
	do
	{
		m_pReportDevice->PrintfToUi(" BRAM Diagnosis\n");
		if (0 != BRAMDiagnosis(lpszNextIndent))
		{
			++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAM'/>\n", lpszNextIndent);
			break;
		}

		m_pReportDevice->PrintfToUi(" DRAM Diagnosis\n");
        if (0 != DRAMDiagnosis(lpszNextIndent))
        {
			++nFailCount;
            if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
        
    } while (false);

	if (1 == m_pReportDevice->IsStop())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop/>\n", lpszNextIndent);
	}
    if (0 == nFailCount)
    {
        nRet = 0;
        m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
    }
    else
    {
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
    }
	
	dTimeConsume = StopTimer(lpszTimeUnits,sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume,lpszTimeUnits);

    m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</LowSpeedMemoryDiagnosis>\n", lpszBaseIndent);
    return nRet;
}

int CDiagnosisLow::BRAMDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	char lpszTimeUnits[4] = { 0 };
	double dTimeConsume = 0;

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<BRAMTest>\n", lpszBaseIndent);
	BOOL bAllPass = TRUE;
	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

	const BYTE byDataTypeCount = 18;
	const CHardwareFunction::RAM_TYPE RAMType[byDataTypeCount] = { CHardwareFunction::RAM_TYPE::IMM1, CHardwareFunction::RAM_TYPE::IMM2, CHardwareFunction::RAM_TYPE::FM,
		CHardwareFunction::RAM_TYPE::MM,CHardwareFunction::RAM_TYPE::IOM, CHardwareFunction::RAM_TYPE::MEM_PERIOD, CHardwareFunction::RAM_TYPE::MEM_RSU_SVM1,
		CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1, CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2, CHardwareFunction::RAM_TYPE::MEM_HIS_SVM, CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1,
		CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2, CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT,	CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R, CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F,
		CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR, CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF, CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR};
	int nRetVal = 0;
	for (int nDataTypeIndex = 0; nDataTypeIndex < byDataTypeCount; ++nDataTypeIndex)
	{
		nRetVal = BRAMDataDiagnosis(lpszNextIndent, RAMType[nDataTypeIndex]);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
		nRetVal = 0;
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}
	ShowUIResult();
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</BRAMTest>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisLow::BRAMDataDiagnosis(const char* lpszBaseIndent, CHardwareFunction::RAM_TYPE RAMType)
{
	if (0 == m_vecEnableController.size())
	{
		return 0;
	}
	UINT uValiDataBit = 0;
	UINT uDataCount = 0;
	string strDataType;
	switch (RAMType)
	{
	case CHardwareFunction::RAM_TYPE::IMM1:
		uDataCount = 0x1000;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "IMM1";
		break;
	case CHardwareFunction::RAM_TYPE::IMM2:
		uDataCount = 0x1000;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "IMM2";
		break;
	case CHardwareFunction::RAM_TYPE::FM:
		uDataCount = 0x1000;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "FM";
		break;
	case CHardwareFunction::RAM_TYPE::MM:
		uDataCount = 0x1000;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "MM";
		break;
	case CHardwareFunction::RAM_TYPE::IOM:
		uDataCount = 0x1000;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "IOM";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_PERIOD:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "PERIOD";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_RSU_SVM1:
		uDataCount = 0x400;
		uValiDataBit = 0x1FFFFFFF;
		strDataType = "RSU_SVM1";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM1:
		uDataCount = 0x0400;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "RSU_LVM1";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_RSU_LVM2:
		uDataCount = 0x0400;
		uValiDataBit = 0x0000FFFF;
		strDataType = "RSU_LVM2";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_HIS_SVM:
		uDataCount = 0x0400;
		uValiDataBit = 0x1FFFFFFF;
		strDataType = "HIS_SVM";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM1:
		uDataCount = 0x400;
		uValiDataBit = 0xFFFFFFFF;
		strDataType = "HIS_LVM1";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_HIS_LVM2:
		uDataCount = 0x0400;
		uValiDataBit = 0x0000FFFF;
		strDataType = "HIS_LVM2";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_FMT:
		uDataCount = 0x20;
		uValiDataBit = 0x0000003F;
		strDataType = "MEM_TIMING_FMT";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1R:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "T1R";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_T1F:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "T1F";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOR:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "IOR";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_IOF:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "IOF";
		break;
	case CHardwareFunction::RAM_TYPE::MEM_TIMING_STBR:
		uDataCount = 0x20;
		uValiDataBit = 0x003FFFFF;
		strDataType = "STBR";
		break;
	default:
		return -1;
		break;
	}
	StartTimer();

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszBaseIndent, strDataType.c_str());
	ULONG* pulData = nullptr;
	ULONG* pulReadData = nullptr;
	try
	{
		pulData = new ULONG[uDataCount];
		memset(pulData, 0, uDataCount * sizeof(ULONG));
		pulReadData = new ULONG[uDataCount];
		memset(pulReadData, 0, uDataCount * sizeof(ULONG));
	}
	catch (const std::exception&)
	{
		return -1;
	}

	for (UINT uDataIndex = 0; uDataIndex < uDataCount;++uDataIndex)
	{
		pulData[uDataIndex] = GetData(uDataIndex, uValiDataBit);
	}
	UINT uBindControllerID = m_vecEnableController[0];
	CHardwareFunction* pHardware = GetHardware(uBindControllerID);

	Bind(m_vecEnableController, uBindControllerID);
	auto iterHardware = m_mapHardware.find(uBindControllerID);
	//Start 0, write 1
	pHardware->WriteBRAMMemory(RAMType, 0, uDataCount, pulData);

	ClearBind();

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	BOOL bPass = TRUE;
	BYTE bySlotNo = 0;
	BYTE byControllerIndex = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		StartTimer();
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byControllerIndex, bySlotNo);
		BOOL bControllerPass = TRUE;

		pHardware = GetHardware(uControllerID);
		pHardware->ReadBRAMMemory(RAMType, 0, uDataCount, pulReadData);
		if (0 != memcmp(pulData, pulReadData, uDataCount * sizeof(ULONG)))
		{
			if (m_setFailController.end() == m_setFailController.find(uControllerID))
			{
				m_setFailController.insert(uControllerID);
			}
			int nErrorCount = 0;
			bPass = FALSE;
			bControllerPass = FALSE;
			for (UINT uDataIndex = 0; uDataIndex < uDataCount; ++uDataIndex)
			{
				if (pulReadData[uDataIndex] == pulData[uDataIndex])
				{
					continue;
				}
				nErrorCount++;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='addr=0x%X, wrdata=0x%X, rdata=0x%X'/>\n",
					lpszSecondIndent, uDataIndex, pulData[uDataIndex], pulReadData[uDataIndex]);
				if (nErrorCount > 10)
				{
					break;
				}
			}
		}
		if (bControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);

		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", lpszFirstIndent);

	}
	if (nullptr != pulReadData)
	{
		delete[] pulReadData;
		pulReadData = nullptr;
	}
	if (nullptr != pulData)
	{
		delete[] pulData;
		pulData = nullptr;
	}
	int nRetVal = 0;
	if (bPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);

	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszBaseIndent, strDataType.c_str());
	return nRetVal;
}

int CDiagnosisLow::DRAMDiagnosis(const char* lpszBase)
{
	StartTimer();
	char lpszTimeUnits[4] = { 0 };
	double dTimeConsume = 0;

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMTest>\n", lpszBase);
	BOOL bAllPass = TRUE;
	string strNextIndent = lpszBase + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	int nRetVal = 0;
	do 
	{
		nRetVal = DRAMStartAddressTest(lpszNextIndent);
		if (0 != nRetVal)
		{
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
			bAllPass = FALSE;
		}
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=SimplifiedDRAM'/>\n", lpszNextIndent);
			break;
		}

		nRetVal = DRAMSimplifiedTest(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE; 
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMStability'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMStabilityTest(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE; 
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
		}
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FullMemory'/>\n", lpszNextIndent);
			break;
		}
		nRetVal = DRAMFullMemTest(lpszNextIndent);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop/>\n", lpszNextIndent);
			break;
		}
	} while (false);
	
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = 1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}
	ShowUIResult();
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMTest>\n", lpszBase);
	return nRetVal;
}

int CDiagnosisLow::TestStartAddressItem(const char* lpszBaseIndent, UINT uStartAddr, UINT uLineCount, USHORT* pusWriteData, const USHORT* const pusExpectData)
{
	USHORT usTestControllerCount = m_vecEnableController.size();
	if (0 >= usTestControllerCount)
	{
		return -2;
	}
	if (nullptr == pusWriteData || nullptr == pusExpectData)
	{
		return -3;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };


	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DataCount value='%d'>\n", lpszBaseIndent, uLineCount);

	const BYTE byMemCount = 4;
	const char* lpszMemType[byMemCount] = { "FM","MM","IOM", "C"};
	const DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM,DATA_TYPE::IOM,DATA_TYPE::CMD };
	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	std::string strThirdIndent = strSecondIndent + IndentChar();

	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	BOOL bCurItemPass = TRUE;
	BOOL bCurControllerPass = TRUE;

	UINT uControllerID = m_vecEnableController[0];
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);

	CHardwareFunction* pHardare = GetHardware(uControllerID);
	
	Bind(m_vecEnableController, uControllerID);
	//Start 0, write 1

	for (int nWriteType = 0; nWriteType < byMemCount; ++nWriteType)
	{
		pHardare->WriteDataMemory(MEM_TYPE::DRAM, DataType[nWriteType], uStartAddr, uLineCount, pusWriteData);
	}
	ClearBind();

	auto iterController = m_setFailController.begin();

	BOOL bMemPass = TRUE;
	for (auto uCurControllerID : m_vecEnableController)
	{
		StartTimer();
		bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
		pHardare = GetHardware(uCurControllerID);
		
		bCurControllerPass = TRUE;
		for (int nMemType = 0; nMemType < byMemCount; ++nMemType)
		{
			bMemPass = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s' >\n", lpszSecondIndent, lpszMemType[nMemType]);

			StartTimer();
			pHardare->ReadDataMemory(MEM_TYPE::DRAM, DataType[nMemType], uStartAddr, uLineCount, pusWriteData);
			if (0 != memcmp(pusWriteData, pusExpectData, uLineCount * sizeof(USHORT)))
			{
				bCurItemPass = FALSE;
				bCurControllerPass = FALSE;
				bMemPass = FALSE;
				for (UINT uDataIndex = 0; uDataIndex < uLineCount; ++uDataIndex)
				{
					if (pusWriteData[uDataIndex] != pusExpectData[uDataIndex])
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='addr=0x%X, cmp_data=0x%X, rdata=0x%X'/>\n", lpszThirdIndent,
							uDataIndex, pusWriteData[uDataIndex], pusExpectData[uDataIndex]);
					}
				}
			}
			if (bMemPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
			}

			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszSecondIndent);
		}
		if (bCurControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			iterController = m_setFailController.find(uCurControllerID);
			if (m_setFailController.end() == iterController)
			{
				m_setFailController.insert(uCurControllerID);
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
	}
	int nRetVal = 0;
	if (bCurItemPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DataCount>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisLow::DRAMStartAddressTest(const char* lpszBaseIndent)
{
	USHORT usTestControllerCount = m_vecEnableController.size();
	if (0 >= usTestControllerCount)
	{
		return -2;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	   
	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	std::string strSecondIndent = strFirstIndent + IndentChar();
	std::string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAMStartAddressTests>\n", lpszBaseIndent);

	USHORT* pusDataBuff = nullptr;
	USHORT* pusExpectData = nullptr;
	try
	{
		pusDataBuff = new USHORT[0x2000];
		pusExpectData = new USHORT[0x2000];
		for (int nIndex = 0; nIndex < 0x2000; ++nIndex)
		{
			pusDataBuff[nIndex] = GetData(nIndex, USHORT(-1));
		}
		memcpy(pusExpectData, pusDataBuff, 0x2000 * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -2;
	}
	const BYTE byMemCount = 4;
	const char* lpszMemType[byMemCount] = { "FM","MM","IOM", "C" };
	const DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM,DATA_TYPE::IOM,DATA_TYPE::CMD };
	UINT uControllerID = 0;
	UINT uBindControllerID = m_vecEnableController[0];
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = HDModule::Instance()->ID2Board(uBindControllerID, byBoardControllerIndex);
	const BYTE byAddressCount = 4;
	USHORT usWriteAddr[byAddressCount] = { 0, 1000, 2044, 2046 };
	const BYTE byTestDataCount = 7;
	UINT uDataCount[byTestDataCount] = { 1,2,3,4,5,0x800,0x2000 };
	
	//=====================Write 1/2/3/4/5 Data=========================//
	auto iterHardware = m_mapHardware.begin();
	BOOL bAllPass = TRUE;
	BOOL bAddrPass = TRUE;
	ULONG ulStartAddr = 0;
	auto iterFailController = m_setFailController.begin();

	int nRetVal = 0;

	for (int nWriteAddrIndex = 0; nWriteAddrIndex < byAddressCount; ++nWriteAddrIndex)
	{
		StartTimer();
		ulStartAddr = usWriteAddr[nWriteAddrIndex];
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Address value='%d'>\n", lpszFirstIndent, ulStartAddr);
		bAddrPass = TRUE;
		for (int nTestDataIndex = 0; nTestDataIndex < byTestDataCount; ++nTestDataIndex)
		{
			UINT uCurDataCount = uDataCount[nTestDataIndex];
			memcpy_s(pusDataBuff, uCurDataCount * sizeof(USHORT), pusExpectData, uCurDataCount * sizeof(USHORT));
			nRetVal = TestStartAddressItem(lpszSecondIndent, ulStartAddr, uDataCount[nTestDataIndex], pusDataBuff, pusExpectData);
			if (0 != nRetVal)
			{
				bAddrPass = FALSE;
				bAllPass = FALSE;
			}
		}

		if (bAddrPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Address>\n", lpszFirstIndent);
	}


	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAMStartAddressTests>\n", lpszBaseIndent);

	if (nullptr != pusDataBuff)
	{
		delete[] pusDataBuff;
		pusDataBuff = nullptr;
	}
	if (nullptr != pusExpectData)
	{
		delete[] pusExpectData;
		pusExpectData = nullptr;
	}

	return nRetVal;
}

int CDiagnosisLow::DRAMSimplifiedTest(const char* lpszBaseIndent)
{
	if (USER != m_UserRole)
	{
		return 0;
	}
	if (0 >= m_vecEnableController.size())
	{
		return 0;
	}

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SimplifiedDRAMTest>\n", lpszBaseIndent);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	const BYTE byMemCount = 4;
	const char* lpszMemType[byMemCount] = { "FM","MM","IOM","C" };
	const DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM,DATA_TYPE::IOM,DATA_TYPE::CMD };
	UINT uControllerID = 0;
	UINT uBindControllerID = m_vecEnableController[0];
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = HDModule::Instance()->ID2Board(uBindControllerID, byBoardControllerIndex);

	UINT uSaveFailCount = 5;

	int nTestCount = HDModule::DRAMLineCount / 0x100000;


	USHORT* pusDataBuff = nullptr;
	USHORT* pusExpectData = nullptr;

	try
	{
		pusDataBuff = new USHORT[0x800];
		memset(pusDataBuff, 0, 0x800 * sizeof(USHORT));
		pusExpectData = new USHORT[0x800];
		for (int nIndex = 0; nIndex < 0x800; ++nIndex)
		{
			pusExpectData[nIndex] = GetData(nIndex, USHORT(-1));
		}
		memcpy(pusDataBuff, pusExpectData, 0x800 * sizeof(USHORT));
	}
	catch (const std::exception&)
	{
		return -2;
	}
	CHardwareFunction* pHardare = GetHardware(uBindControllerID);

	Bind(m_vecEnableController, uBindControllerID);
	for (int nTestIndex = 0; nTestIndex < nTestCount;++nTestIndex)
	{
		for (int nMemType = 0; nMemType < byMemCount; ++nMemType)
		{
			UINT ulStartAddr = nTestIndex * 0x100000 + nTestIndex * 0x800;
			pHardare->WriteDataMemory(MEM_TYPE::DRAM, DataType[nMemType], ulStartAddr, 0x800, pusExpectData);
		}
	}

	ClearBind();

	auto iterFailController = m_setFailController.begin();

	BOOL bAllPass = TRUE;
	BOOL bControllerPass = TRUE;
	BOOL bMemPass = TRUE;
	for (auto uCurControllerID : m_vecEnableController)
	{
		StartTimer();

		bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardControllerIndex);
		pHardare = GetHardware(uCurControllerID);
		
		bControllerPass = TRUE;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);

		for (int nMemType = 0; nMemType < byMemCount; ++nMemType)
		{
			StartTimer();

			bMemPass = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszSecondIndent, lpszMemType[nMemType]);
			for (int nTestIndex = 0; nTestIndex < nTestCount;++nTestIndex)
			{
				UINT uStartAddr = nTestIndex * 0x100000 + nTestIndex * 0x800;
				pHardare->ReadDataMemory(MEM_TYPE::DRAM, DataType[nMemType], uStartAddr, 0x800, pusDataBuff);
				if (0 != memcmp(pusDataBuff,pusExpectData, 0x400 * sizeof(USHORT)))
				{
					bAllPass = FALSE;
					bControllerPass = FALSE;
					bMemPass = FALSE;
					ULONG ulPageValue = nTestIndex * 0x100000;
					ULONG ulAddress = nTestIndex * 0x800;
					UINT uCurFailCount = 0;
					for (int nLineIndex = 0; nLineIndex < 0x800; ++nLineIndex)
					{
						if (pusDataBuff[nLineIndex] != pusExpectData[nLineIndex])
						{
							m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='addr=0x%X, cmp_data=0x%X, rdata=0x%X'/>\n", lpszThirdIndent, 
								nLineIndex, pusExpectData[nLineIndex], pusDataBuff[nLineIndex]);
							++uCurFailCount;
							if (uSaveFailCount <= uCurFailCount)
							{
								break;
							}
						}
					}
				}
			}
			if (bMemPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
			}

			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszSecondIndent);
		}
		if (bControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{

			iterFailController = m_setFailController.find(uCurControllerID);
			if (m_setFailController.end() == iterFailController)
			{
				m_setFailController.insert(uCurControllerID);
			}
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}

		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszThirdIndent);
	}

	int nRetVal = 0;
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SimplifiedDRAMTest>\n", lpszBaseIndent);
	if (nullptr != pusDataBuff)
	{
		delete[] pusDataBuff;
		pusDataBuff = nullptr;
	}
	if (nullptr != pusExpectData)
	{
		delete[] pusExpectData;
		pusExpectData = nullptr;
	}
	return nRetVal;
}

int CDiagnosisLow::DRAMStabilityTest(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	std::string strSecondIndent = strFirstIndent + IndentChar();
	std::string strThirdIndent = strSecondIndent + IndentChar();
	std::string strForthIndent = strThirdIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();
	const char* lpszForthIndent = strForthIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<StabilityTest>\n", lpszBaseIndent);


	int nSaveFailCount = 5;

	USHORT usControllerCount = m_vecEnableController.size();
	if (0 == usControllerCount)
	{
		return 0;
	}

	BOOL bAllPass = TRUE;
	int nRetVal = 0;
	USHORT* pusWriteData = nullptr;
	USHORT* pusReadData = nullptr;
	int i = 0;
	auto iterFailController = m_setFailController.begin();
	do
	{
		BYTE res = 0;

		const int nMemSize = 2048;
		const int nCheckTimes = 3;

		try
		{
			pusWriteData = new USHORT[nMemSize];
			for (i = 0; i < nMemSize; i++)
			{
				pusWriteData[i] = i | (i << 10);
			}
			pusReadData = new USHORT[nMemSize];
		}
		catch (const std::exception&)
		{
			nRetVal = -1;
			break;
		}

		const BYTE byMemCount = 4;
		const char* lpszMemType[byMemCount] = { "FM","MM","IOM","C" };
		const DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM,DATA_TYPE::IOM,DATA_TYPE::CMD };
		UINT uControllerID = 0;
		UINT uBindControllerID = m_vecEnableController[0];
		BYTE byBoardControllerIndex = 0;
		BYTE bySlotNo = HDModule::Instance()->ID2Board(uBindControllerID, byBoardControllerIndex);

		UINT uStartAddr = 0;
		UINT uTestLineCount = nMemSize;
		int nErrorCount = 0;
		
		//Bind all controllers ,then write data once
		CHardwareFunction* pHardare = GetHardware(uBindControllerID);
		Bind(m_vecEnableController, uBindControllerID);
		for (BYTE byMemType = 0; byMemType < byMemCount; ++byMemType)
		{
			pHardare->WriteDataMemory(MEM_TYPE::DRAM, DataType[byMemType], uStartAddr, uTestLineCount, pusWriteData);
		}
		ClearBind();

		BOOL bHasLoadVector = FALSE;
		BOOL bAllPass = TRUE;
		int nReadIndex = 0;
		BOOL bControllerPass = TRUE;
		BOOL bCurReadPass = TRUE;

		for (auto uCurControllerID : m_vecEnableController)
		{
			StartTimer();

			bControllerPass = TRUE;
			bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardControllerIndex);
			
			pHardare = GetHardware(uCurControllerID);
			
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);


			for (BYTE byMemType = 0; byMemType < byMemCount; ++byMemType)
			{
				nErrorCount = 0;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszSecondIndent, lpszMemType[byMemType]);

				StartTimer();
				for (nReadIndex = 0; nReadIndex < nCheckTimes; nReadIndex++)
				{
					bCurReadPass = TRUE;
					if (1 == m_pReportDevice->IsStop())
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextTimes=%d'/>\n", lpszFirstIndent, nReadIndex);
						break;
					}

					StartTimer();
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<ReadTimes value='%d'>\n", lpszThirdIndent, nReadIndex);

					pHardare->ReadDataMemory(MEM_TYPE::DRAM, DataType[byMemType], uStartAddr, uTestLineCount, pusReadData);

					if (0 != memcmp(pusReadData, pusWriteData, uTestLineCount * sizeof(USHORT)))
					{
						for (UINT uLineIndex = 0; uLineIndex < uTestLineCount; uLineIndex++)
						{
							if (pusReadData[uLineIndex] != pusWriteData[uLineIndex])
							{
								bAllPass = FALSE;
								nErrorCount++;
								m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='addr=0x%X, wrdata=0x%X, rdata=0x%X'/>\n", 
									lpszForthIndent, uLineIndex, pusWriteData[uLineIndex], pusReadData[uLineIndex]);
								if (nErrorCount > 10)
								{
									break;
								}
							}
						}
						bCurReadPass = FALSE;
						bControllerPass = FALSE;
						bAllPass = FALSE;
						
					}

					if (bCurReadPass)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszForthIndent);

					}
					else
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszForthIndent);
					}

					dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszForthIndent, dTimeConsume, lpszTimeUnits);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</ReadTimes>\n", lpszThirdIndent);


					if (nErrorCount > 10)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='More than 10 errors...'/>\n", lpszThirdIndent);
						res = 1;
						break;
					}

				}
				if (0 == nErrorCount)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);

				}
				else
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
				}

				dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszSecondIndent);
			}


			if (bControllerPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			else
			{
				iterFailController = m_setFailController.find(uCurControllerID);
				if (m_setFailController.end() == iterFailController)
				{
					m_setFailController.insert(uCurControllerID);
				}
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}

			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", strFirstIndent.c_str());
		}
	} while (false);

	if (nullptr != pusWriteData)
	{
		delete[] pusWriteData;
		pusWriteData = nullptr;
	}
	if (nullptr != pusReadData)
	{
		delete[] pusReadData;
		pusReadData = nullptr;
	}
	
	if (0 == nRetVal && bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", strFirstIndent.c_str());

	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", strFirstIndent.c_str());
	}


	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</StabilityTest>\n", lpszBaseIndent);
	return nRetVal;
}
int CDiagnosisLow::DRAMFullMemTest(const char* lpszBaseIndent)
{
	if (USER == m_UserRole)
	{
		//Not check in current user.
		return 0;
	}
	int nTestControllerCount = m_vecEnableController.size();
	if (0 == nTestControllerCount)
	{
		return 0;
	}
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	auto iterFailController = m_setFailController.begin();


	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	std::string strSecondIndent = strFirstIndent + IndentChar();
	std::string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<FullMemTest>\n", lpszBaseIndent);

	map<UINT, UINT> mapControllerFailCount;///<The print fail count of each controller, key is controller ID

	BYTE f_fail = 0;

	USHORT* pusWriteData = nullptr;
	USHORT* pusReadData = nullptr;

	USHORT nPageLineCount = 2048;
	UINT uStartAddr = 0;
	const int nPageCount = DCM_DRAM_PATTERN_LINE_COUNT / nPageLineCount; //32M*32bit深度

	const int MAXCRC = 6;
	auto iterHardware = m_mapHardware.begin();
	int i = 0;
	int errcnt = 0;
	BOOL bAllPass = TRUE;
	do
	{
		try
		{
			pusWriteData = new USHORT[nPageLineCount];
			pusReadData = new USHORT[nPageLineCount];
		}
		catch (const std::exception&)
		{
			break;
		}
		for (i = 0; i < nPageLineCount; i++)
		{
			pusWriteData[i] = i | (i << 10);
		}

		const BYTE byMemCount = 4;
		const char* lpszMemType[byMemCount] = { "FM","MM","IOM","C" };
		const DATA_TYPE DataType[byMemCount] = { DATA_TYPE::FM, DATA_TYPE::MM,DATA_TYPE::IOM,DATA_TYPE::CMD };
		UINT uControllerID = 0;
		UINT uBindControllerID = m_vecEnableController[0];
		BYTE byBoardControllerIndex = 0;
		BYTE bySlotNo = HDModule::Instance()->ID2Board(uBindControllerID, byBoardControllerIndex);

		StartTimer();

		for (BYTE byMemType = 0; byMemType < byMemCount; ++byMemType)
		{
			if (1 == m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextMem=%s'/>\n", lpszFirstIndent, lpszMemType[byMemType]);
				break;
			}
			mapControllerFailCount.clear();
			BOOL bMemPass = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MemType value='%s'>\n", lpszFirstIndent, lpszMemType[byMemType]);
			StartTimer();
			for (auto uCurControllerID : m_vecEnableController)
			{
				StartTimer();

				BOOL bControllerPass = TRUE;

				bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardControllerIndex);				

				CHardwareFunction* pHardware = GetHardware(uCurControllerID);
				
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszSecondIndent, byBoardControllerIndex, bySlotNo);

				for (int nPageIndex = 0; nPageIndex < nPageCount; nPageIndex++)//一次F_DRAM 32M*32的深度数据自检
				{
					if (1 == m_pReportDevice->IsStop())
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextCheckPage=0x%X'/>\n", lpszThirdIndent, nPageIndex);
						break;
					}
					uStartAddr = nPageIndex * nPageLineCount;
					int nFailcount = 0;
					ULONG cmp_data = 0;
					if (uBindControllerID == uCurControllerID)
					{
						//Bind all controllers ,then write data once
						Bind(m_vecEnableController, uBindControllerID);
						CHardwareFunction* pBindHardware = GetHardware(uBindControllerID);
						pBindHardware->WriteDataMemory(MEM_TYPE::DRAM, DataType[byMemType], uStartAddr, nPageLineCount, pusWriteData);

						ClearBind();
					}

					int ir = 0;
					for (ir = 0; ir < MAXCRC; ir++)//CRC 冗余次数最大为10次
					{
						nFailcount = 0;
						cmp_data = 0;

						f_fail = 0;
						pHardware->ReadDataMemory(MEM_TYPE::DRAM, DataType[byMemType], uStartAddr, nPageLineCount, pusReadData);

						if (0 != memcmp(pusReadData, pusWriteData, nPageLineCount * sizeof(USHORT)))
						{
							bAllPass = FALSE;
							bMemPass = FALSE;
							bControllerPass = FALSE;

							auto iterPrintCount = mapControllerFailCount.find(uCurControllerID);
							if (mapControllerFailCount.end() == iterPrintCount)
							{
								mapControllerFailCount.insert(make_pair(uCurControllerID, 0));
								iterPrintCount = mapControllerFailCount.find(uCurControllerID);
							}
							if (30 <= iterPrintCount->second)
							{
								break;
							}

							for (i = 0; i < nPageLineCount; i++)
							{
								cmp_data = pusWriteData[i];
								if (pusReadData[i] == cmp_data)
								{
									continue;
								}
								f_fail = 1;
								m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false', PAGE=0x%X, value='addr=0x%X, LOOP=%d, cmp_data=0x%X, rdata=0x%X'/>\n", 
									lpszThirdIndent, nPageIndex,  i, ir, cmp_data, pusReadData[i]);
								++nFailcount;

								
								++(iterPrintCount->second);

								if (ERROR_PRINT < nFailcount)
								{
									break;
								}
							}
						}
						else
						{
							break;
						}
					}
				}
				if (bControllerPass)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
				}
				else
				{
					if (m_setFailController.end() == m_setFailController.find(uCurControllerID))
					{
						m_setFailController.insert(uCurControllerID);
					}
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
				}


				dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszSecondIndent);

			}

			if (bMemPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}


			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MemType>\n", lpszFirstIndent);

		}
	} while (FALSE);

	if (nullptr != pusWriteData)
	{
		delete[] pusWriteData;
		pusWriteData = nullptr;
	}
	if (nullptr != pusReadData)
	{
		delete[] pusReadData;
		pusReadData = nullptr;
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}


	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</FullMemTest>\n", lpszBaseIndent);

	return nRetVal;
}

#define GET_DATA(Data) ((Data << 10) | Data)

inline ULONG CDiagnosisLow::GetData(USHORT usData, ULONG ulValidBit)
{
	ULONG ulData = GET_DATA(usData) | GET_DATA(usData) << 20;

	return ulData & ulValidBit;
}

void CDiagnosisLow::ShowUIResult()
{
	BYTE byBoardControllerIndex = 0;
	BYTE bySlotNo = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (m_setFailController.end() == m_setFailController.find(uControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
	}
	m_setFailController.clear();
}
