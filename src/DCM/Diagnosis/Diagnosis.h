#pragma once
/**
 * @file Diagnosis.h
 * @brief The top diagnosis item of DCM's diagnosis function
 * @author Guangyun Wang
 * @date 2020/09/13
 * @copyright AccoTEST Business Unit of Beijing Huangfeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
/**
 * @class CDiagnosis
 * @brief The top diagnosis item of DCM's diagnosis function
*/
class CDiagnosis :
	public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosis();
	/**
	 * @brief Destructor
	*/
	~CDiagnosis();
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
	 * @brief Enable the diagnosis item
	 * @param[in] nEnable Whether enable the diagnosis item
	 * @return Execute result
	 * - 0 Enable item successfully
	*/
	virtual int SetEnabled(int nEnable);
	/**
	 * @brief Check whether the item is enabled
	 * @return Whether the item is enabled
	*/
	virtual int IsEnabled() const;
	/**
	 * @brief Start the diagnosis item
	 * @param[in] pReportDevice The point of the report device
	 * @return The diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int Doctor(IHDReportDevice* pReportDevice);
	/**
	 * @brief Set the controller diagnosed
	 * @param[in] vecController The controller diagnosed, the value is controller ID
	*/
	virtual void SetEnableController(const std::vector<UINT>& vecController);
	/**
	 * @brief Set the user's role
	 * @param[in] UserRole The user's role
	*/
	virtual void SetUserRole(CDiagnosisItem::USER_ROLE UserRole);
private:
	/**
	 * @brief Show and save the information of board diagnosed
	 * @param[in] lpszBaseIndent The base indent of current item
	*/
	void ShowBoardInfo(const char* lpszBaseIndent);
private:
	std::vector<CDiagnosisItem*> m_vecDoctorItem;///<The sub diagnosis item in current item
};
