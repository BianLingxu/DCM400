#pragma once
/**
 * @file DiagnosisHighMemory.h
 * @brief The high speed diagnosis for memory of BRAM and DRAM
 * @author Guangyun Wang
 * @date 2020/09/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"

class IHDReportDevice;
/**
 * @class CDiagnosisHighMemory
 * @brief The high speed diagnosis of memory
*/
class CDiagnosisHighMemory : public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisHighMemory();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisHighMemory();
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
     * @brief Diagnosis BRAM memory
     * @param[in] lpszBaseIndent The base indent
     * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
    */
    int BRAMDiagnosis(const char* lpszBaseIndent);
    /**
     * @brief Diagnosis DRAM
     * @param[in] lpszBaseIndent The base indent
     * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
    */
    int DRAMDiagnosis(const char * lpszBaseIndent);
	/**
	 * @brief Run vector used in high speed check of BRAM
	 * @param[in] dPeriod The period
	 * @param[in] DataType The data type checked
	 * @param[in] nBRAMStartAddr The BRAM start address
	 * @param[in] nBRAMDepth The BRAM depth
	 * @param[in] pusBRAMData The data of BRAM
	 * @return Execute result
	 * - 0 Ran vector successfully
	 * - -1 The controller count is 0
	 * - -2 The vector line count is over range
	 * - -3 The point of BRAM data is nullptr
	 * - -4 Allocate memory fail
	*/
	int BRAMHighSpeedCheck(double dPeriod, DATA_TYPE DataType, int nBRAMStartAddr, int nBRAMDepth, const USHORT *const pusBRAMData);     
	/**
	 * @brief Check the result of BRAM high speed check
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] mapExpectFailLine The expect fail line of BRAM
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	*/
	int BRAMResultCheck(const char* lpszBaseIndent, const std::map<int, USHORT>& mapExpectFailLine);
	/**
	 * @brief Run the vector using in high speed check of DRAM
	 * @param[in] dPeriod The pattern run period
	 * @param[in] DataType The data type checked
	 * @param[in] nSwitchTimes The switch times from BRAM to DRAM
	 * @param[in] puBRAMSwitchLineOffset The line number of BRAM switch to DRAM
	 * @param[in] puDRAMSwitchLineOffset The line number of DRAM switch to BRAM
	 * @param[in] uBRAMDepth The BRAM depth
	 * @param[in] pusBRAMData The BRAM data
	 * @param[in] uDRAMStartAddr The start line address of DRAM
	 * @param[in] uDRAMDepth The DRAM line count
	 * @param[in] pusDRAMData The DRAM data
	 * @return Execute result
	 * - 0 Ran vector successfully
     * - -1 The controller count is 0
	 * - -2 The vector line in BRAM is too little
	 * - -3 The vector line in DRAM is too little
	 * - -4 The point switch line offset is nullptr
	 * - -5 The switch line is invalid
	 * - -6 The switch line is match
	 * - -7 Allocate memory fail
	*/
	int DRAMHighSpeedCheck(double dPeriod, DATA_TYPE DataType, int nSwitchTimes, const UINT* const puBRAMSwitchLineOffset, const UINT* const puDRAMSwitchLineOffset,
		UINT uBRAMDepth, const USHORT* const pusBRAMData, UINT uDRAMStartAddr, UINT uDRAMDepth, const USHORT* const pusDRAMData);		
	/**
	 * @brief Check the result of DRAM high speed check
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] TestDataType The data type
	 * @param[in] uDRAMStartAddress The start address of DRAM
	 * @param[in] mapBRAMExpectFailLine The expect fail line of BRAM
	 * @param[in] mapDRAMExpectFailLine The expect fail line of DRAM
	 * @param[in] bSaveLog Whether save log
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	*/
	int DRAMResultCheck(const char* lpszBaseIndent, DATA_TYPE TestDataType, UINT uDRAMStartAddress, const std::map<int, USHORT>& mapBRAMExpectFailLine,
		const std::map<int, USHORT>& mapDRAMExpectFailLine, BOOL& bSaveLog);
	/**
     * @brief The switch test of DRAM
     * @param[in] lpszBaseIndent The base indent
     * @param[in] dPeriod The period of vector ran 
     * @param[in] uDRAMStartAddr The DRAM start address
     * @param[in] nSwitchTimes The switch time from BRAM to DRAM
     * @param[in] uDRAMDataLength The data line count of DRAM
     * @param[in] bSaveLog Whether save the log of pass
	 * @param[in] bSaveAddrssLog Whether save the log of the start address
     * @return Test result
     * - 0 Test pass
     * - -1 Test fail
    */
    int DRAMSwitchTest(const char* lpszBaseIndent, double dPeriod, UINT uDRAMStartAddr, int nSwitchTimes, UINT uDRAMDataLength, BOOL bSaveLog, BOOL bSaveAddressLog = TRUE);
	/**
     * @brief The multi-switch diagnosis of DRAM
     * @param[in] lpszBaseIndent The base indent
     * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
    */
    int DRAMMultiSwitchDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief The stability test of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Test result
     * - 0 Test pass
     * - -1 Test fail
	*/
	int DRAMStabilityDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief The multi-rate diagnosis of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
	*/
	int DRAMMultiRateDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief The multi-length diagnosis of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return The test result
     * - 0 Test result
     * - -1 Test fail
	*/
	int DRAMMultiLengthDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief The diagnosis all page of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
	*/
	int DRAMAllPageDiagnosis(const char* lpszBaseIndent);
    /**
     * @brief Diagnosis all page one ran
     * @param[in] lpszBaseIndent The base indent
     * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
    */
    int DRAMAllPageRanDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief The fast diagnosis of DRAM
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
     * - 0 Diagnosis pass
     * - -1 Diagnosis fail
	*/
	int DRAMFastDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Save the controller whose diagnosis result is fail
	 * @param[in] uControllerID The controller ID 
	*/
	inline void SaveFailController(UINT uControllerID);
	/**
	 * @brief Show the diagnosis result to UI
	*/
	inline void ShowUIResult();
	/**
	 * @brief Save the DRAM address log
	 * @param[in] uDRAMAddr The start address of DRAM
	*/
	inline void SaveDRAMAddressLog(UINT uDRAMAddr);
	/**
	 * @brief Save the data type to log in DRAM diagnosis
	 * @param[in] DataType The data type
	*/
	inline void SaveDRAMDataTypeLog(DATA_TYPE DataType);
private:
	std::set<UINT> m_setFailController;///<The controller whose diagnosis result is fail
	std::string m_strDRAMAddrIndent;///<The indent of the start address
	std::string m_strDRAMDataTypeIndent;///<The indent of the data type in DRAM diagnosis
};