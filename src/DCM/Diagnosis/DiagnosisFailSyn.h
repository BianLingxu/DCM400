#pragma once
/**
 * @file DiagnosisFailSyn.h
 * @brief The diagnosis item of fail synchronization
 * @author Guangyun Wang
 * @date 2021/09/17
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
/**
 * @class CDiagnosisFailSyn
 * @brief The diagnosis item of fail information sychronization
*/
class CDiagnosisFailSyn :
    public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisFailSyn();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisFailSyn();
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
	 * @brief Diagnosis conditional instruction of FJUMP
	 * @param lpszBaseIndent The base indent for current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisFJUMP(const char* lpszBaseIndent);
	/**
	 * @brief Load pattern for diagnosis
	 * @param[in] vecFailSynController The controller whose fail information will be synchronized
	 * @return >=0 The pattern count
	 * - >=0 The pattern count
	 * - -1 The fail synchronization type is not supported
	*/
	int LoadPattern(std::vector<BYTE>& vecFailSynController);
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
private:
	std::set<UINT> m_setFailController;///<The fail controller
	std::vector<CHardwareFunction::DATA_RESULT> m_avecBRAMFailExpected[DCM_MAX_CONTROLLERS_PRE_BOARD];///<The fail information of each board
};

