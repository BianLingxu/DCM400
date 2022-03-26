#pragma once
/**
 * @file DiagnosisShieldChannel.h
 * @brief The diagnosis of channel comparison shielded
 * @author Guangyun Wang
 * @date 2021/08/04
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
/**
 * @class CDiagnosisComparisonShield
 * @brief The diagnosis item of channel comparison shielded
*/
class CDiagnosisComparisonShield :
    public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisComparisonShield();
    /**
     * @brief Destructor
    */
    ~CDiagnosisComparisonShield();	
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
	 * @brief Diagnosis the comparison of fail
	 * @param[in] setChannel The channel whose comparison will be enabled
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisFailComparison(const std::set<USHORT>& setChannel, const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis the comparison of pass
	 * @param[in] setChannel The channel whose comparison will be enabled
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisPassComparison(const std::set<USHORT>& setChannel, const char* lpszBaseIndent);
	/**
	 * @brief Get the shield item
	 * @param[out] mapItem The item diagnosised
	*/
	void GetItem(std::map<std::string, std::set<USHORT>>& mapItem);
	/**
	 * @brief Diagnosis the item which channels' run result is pass
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisPass(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis the item which channels' run result is fail
	 * @param[in] lpszBaseIndent The base indent of current item
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisFail(const char* lpszBaseIndent);
	/**
	 * @brief Show the diagnosis result to UI
	*/
	inline void ShowUIResult();
private:
	std::set<UINT> m_setFailController;///<The fail controller

};

