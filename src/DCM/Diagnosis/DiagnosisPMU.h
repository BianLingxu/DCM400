#pragma once
/**
 * @file DiagnosisPMU.h
 * @brief The diagnosis item of PM<U
 * @author Guangyun Wang
 * @date 2020/08/27
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
/**
 * @class CDiagnosisPMU
 * @brief The diagnosis of PMU function
*/
class CDiagnosisPMU :
    public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisPMU();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisPMU();
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
	 * @brief Whether the user select to diagnose this item
	 * @return Whether the user select to diagnose this item
	 * - true User select to diagnose this item
	 * - false User not select to diagnose this item
	*/
	virtual bool IsUserCheck() const;
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
	 * @brief Check whether the board is diagnosable
	*/
	void CheckDiagnosable();
	/**
	 * @brief Connect the relay of the board which will be diagnosed
	*/
	void Connect();
	/**
	 * @brief Disconnect the relay of the board will be diagnosed
	*/
	void Disconnect();
	/**
	 * @brief Diagnosis the PMU function
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int Diagnosis(const char* lpszBaseIndent, BOOL bMV);
	/**
	 * @brief Diagnosis the channels' PMU
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] vecChannel The channel will be diagnosed
	 * @param[in] lpszItemType The channel item type
	 * @return Diagnosis result
	 * - 0 Diagnosis pass
	 * - -1 Diagnosis fail
	*/
	int DiagnosisChannel(const char* lpszBaseIndent, const std::vector<USHORT>& vecChannel, BOOL bMV, const char* lpszItemType);
	/**
	 * @brief Print result to UI
	*/
	void ShowUIResult();
	/**
	 * @brief Save the fail controller
	 * @param[in] uControllerID The controller ID
	*/
	inline void SaveFailController(UINT uControllerID);
	/**
	 * @brief Bind controller
	 * @param[in] bEvenController Whether bind the even controller, 0 is Odd, 1 is even, -1 is all
	 * @return The target controller ID
	*/
	UINT Bind(int nEvenController, std::vector<UINT>& vecBindController);
	/**
	 * @brief Clear bind
	*/
	void ClearBind();
private:
	std::map<UINT, UINT> m_mapUndiagnosableController;///<The undiagnosable controller, key is the controller ID and value is the related controller ID
	std::set<UINT> m_setFailController;///<The fail controller
};

