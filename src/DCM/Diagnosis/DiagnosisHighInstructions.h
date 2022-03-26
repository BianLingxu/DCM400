#pragma once
/**
 * @file DiagnosisHighInstructions.h
 * @brief The diagnosis item of instruction
 * @author Guangyun Wang
 * @date 2020/08/27
 * @copyright AccoTEST Business Unit of Beijing Huangfeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"

class IHDReportDevice;
/**
 * @class CDiagnosisHighInstructions
 * @brief The diagnosis of instruction
*/
class CDiagnosisHighInstructions : public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisHighInstructions();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisHighInstructions();
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
     * @brief Check the INC instruction
     * @param[in] lpszBaseIndent The base indent
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckINC(const char* lpszBaseIndent);
    /**
     * @brief Check instruction REPEAT
     * @param[in] lpszBaseIndent The base indent
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckRepeat(const char* lpszBaseIndent);
    /**
     * @brief Check the instruction of CALL-RETURN
     * @param[in] lpszBaseIndent The base indent
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckCallReturn(const char* lpszBaseIndent);
    /**
     * @brief Check the instruction of LOOP
     * @param[in] lpszBaseIndent The base indent
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckLoop(const char* lpszBaseIndent);
    /**
     * @brief Check the instruction with specific period
     * @param[in] lpszBaseIndent The base indent
     * @param[in] dPeriod The period
     * @param[in] vecExpectLineOrder The expect line order
     * @return Check result
     * - 0 Check pass
     * - 1 Check fail
    */
    int CheckPeriod(const char* lpszBaseIndent, double dPeriod, UINT uLineCount, const std::vector<UINT>& vecExpectLineOrder, BOOL bSynRun = TRUE);
	/**
	 * @brief Print result to UI
	*/
	void ShowUIResult();
private:
    std::set<UINT> m_setFailController;///<The controller whose test result is fail
};