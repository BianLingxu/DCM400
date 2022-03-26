#pragma once
/**
 * @file DiagnosisLow.h
 * @brief The diagnosis item of low speed memory
 * @author Guangyun Wang
 * @date 2020/09/06
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"

class IHDReportDevice;
/**
 * @class CDiagnosisLow
 * @brief The diagnosis item of low speed memory
*/
class CDiagnosisLow : public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisLow();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisLow();
	/**
	 * @brief Get the instance
	 * @param[in] lpszName The name
	 * @param[in] ppPoint The point
	 * @return Execute result
	 * - -1 Not supported
	*/
	virtual int QueryInstance(const char* lpszName, void** ppPoint);
	/**
	 * @brief Release
	*/
	virtual void Release();
	/**
	 * @brief The diagnosis item name
	 * @return The name of current diagnosis item
	*/
	virtual const char* Name() const;
	/**
	 * @brief Get the child diagnosis item
	 * @param[in] vecChildren child The diagnosis item
	 * @return The count of children diagnosis item
	*/
	virtual int GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const;
	/**
	 * @brief Start the diagnosis item
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int Doctor(IHDReportDevice* pReportDevice);
private:
    /**
     * @brief Diagnose the memory of BRAM
     * @param[in] lpszBaseIndent The base indent of the item
     * @return 
    */
    int BRAMDiagnosis(const char* lpszBaseIndent);
    /**
	 * @brief Diagnosis BRAM
	 * @param[in] lpszBaseIndent The base indent of item
     * @param[in] RAMType The RAM type
     * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
    */
    int BRAMDataDiagnosis(const char* lpszBaseIndent, CHardwareFunction::RAM_TYPE RAMType);
    /**
     * @brief Diagnosis DRAM
     * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
    */
    int DRAMDiagnosis(const char * lpszBaseIndent);
	/**
	 * @brief test the start address
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] uStartAddr The start address
	 * @param[in] uLineCount The line count
	 * @param[in] pusWriteData The data will be written
	 * @param[in] pusExpectData The expected data
	 * @return Test result
	 * - 0 Test pass
	 * - -1 Test fail
	 * - -2 No controller tested
	 * - -3 The point of data is nullptr
	*/
	int TestStartAddressItem(const char* lpszBaseIndent, UINT uStartAddr, UINT uLineCount, USHORT *pusWriteData, const USHORT* const pusExpectData);
	/**
	 * @brief The test of start address of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Test result
	 * - 0 Test pass
	 * - -1 Test fail
	 * - -2 No controller tested
	*/
	int DRAMStartAddressTest(const char* lpszBaseIndent);
	/**
	 * @brief The simplified test of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Test result
	 * - 0 Test pass
	 * - -1 Test fail
	*/
	int DRAMSimplifiedTest(const char *lpszBaseIndent);
	/**
	 * @brief The stability test of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Test result
	 * - 0 Test pass
	 * - -1 Test fail
	*/
	int DRAMStabilityTest(const char* lpszBaseIndent);
	/**
	 * @brief The full memory test of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Test result
	 * - 0 Test pass
	 * - -1 Test fail
	*/
	int DRAMFullMemTest(const char* lpszBaseIndent);
    /**
     * @brief Get the test data
     * @param[in] usData The data in USHORT
     * @param[in] ulValidBit The valid bit of the test data gotten
     * @return The test data
    */
    inline ULONG GetData(USHORT usData, ULONG ulValidBit);
	/**
	 * @brief Show the diagnosis result to UI
	*/
    void ShowUIResult();
private:
	std::set<UINT> m_setFailController;///<The fail controller in DRAM diagnosis 
};
