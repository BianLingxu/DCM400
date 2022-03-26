#pragma once
/**
 * @file DiagnosisFailSelected.h
 * @brief The diagnosis of fail selected
 * @author Guangyun Wang
 * @date 2021/08/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
/**
 * @class CDiagnosisFailSelected
 * @brief The diagnosis item of saving selected line's fail information
*/
class CDiagnosisHardwareCapture :
	public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisHardwareCapture();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisHardwareCapture();
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
	virtual int Doctor(IHDReportDevice* pReportDevice);
	/**
	 * @brief Whether the user select to diagnose this item
	 * @return Whether the user select to diagnose this item
	 * - true User select to diagnose this item
	 * - false User not select to diagnose this item
	*/
	virtual bool IsUserCheck() const;
private:
	/**
	 * @brief Diagnosis in different rate
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int BRAMRateDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis BRAM
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int BRAMDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis section in BRAM
	 * @para[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int BRAMSectionDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis capture with save fail selected in BRAM
	 * @para[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int BRAMSaveSelectedFailDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis DRAM
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DRAMDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis section in DRAM
	 * @para[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DRAMSectionDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis capture with save fail selected in BRAM
	 * @para[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DRAMSaveSelectedFailDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis saving select fail
	 * @param[in] setBRAMOut The line number switch from BRAM to DRAM
	 * @param[in] setDRAMIn The line number switch from DRAM to BRAM
	 * @param[in] dPeriod The test period
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	 * - -2 The switch line number is not in couple
	*/
	int SaveSelectedFailDiagnosis(const std::set<UINT>& setBRAMOut, const std::set<UINT>& setDRAMIn, double dPeriod, const char* lpszBaseIndent);
	/**
	 * @brief Analyze the data result
	 * @param[in] vecBRAMExpected The expected result in BRAM
	 * @param[in] vecDRAMExpected The expected result in DRAM
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int ResultAnalyze(const char* lpszBaseIndent);
	/**
	 * @brief Show the diagnosis result to UI
	*/
	inline void ShowUIResult();
	/**
	 * @brief Load pattern for diagnosis
	 * @param[in] setStartLineNo The start line number of capture
	 * @param[in] setStopLineNo The stop line number of capture
	 * @param[in] setBRAMOut The line number for BRAM switch to DRAM
	 * @param[in] setDRAMIn The line number for DRAM switch to BRAM
	 * @param[in] bSaveSelectFail Whether save select fail
	 * @return The stop line number of BRAM
	*/
	int LoadPattern(const std::set<UINT>& setStartLineNo, const std::set<UINT>& setStopLineNo, const std::set<UINT>& setBRAMOut, const std::set<UINT>& setDRAMIn, BOOL bSaveSelectFail = FALSE);
private:
	std::set<UINT> m_setFailController;///<The fail controller
	std::vector<CHardwareFunction::DATA_RESULT> m_avecBRAMExpected[2];///<The BRAM expected capture data
	std::vector<CHardwareFunction::DATA_RESULT> m_avecDRAMExpected[2];///<The DRAM expected capture data
	std::vector<CHardwareFunction::DATA_RESULT> m_avecBRAMFailExpected[2];///<The BRAM expected fail data
	std::vector<CHardwareFunction::DATA_RESULT> m_avecDRAMFailExpected[2];///<The DRAM expected fail data
};