#pragma once
/**
 * @file DiagnosisConnectivity.h
 * @brief The diagnosis item of connectivity
 * @author Guangyun Wang
 * @date 2020/09/06
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
class IHDReportDevice;
/**
 * @class CDiagnosisConnectivity
 * @brief The diagnosis item of connectivity
*/
class CDiagnosisConnectivity : public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisConnectivity();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisConnectivity();
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
     * @brief Check the controller and filter the undiagnosable controller
    */
    void CheckDiagnosable();
	/**
	 * @brief Check the connect type
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] bConnect Whether the relay is connect
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	*/
	int CheckConnect(const char* lpszBaseIndent, BOOL bConnect);
	/**
	 * @brief Check the channel status
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] bConnect Whether the relay is connect
	 * @param[in] ChannelStatus The channel status
	 * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	*/
	int CheckChannelStatus(const char* lpszBaseIndent, BOOL bConnect, CHANNEL_OUTPUT_STATUS ChannelStatus);
	/**
	 * @brief Print result to UI
	*/
	void ShowUIResult();
	/**
	 * @brief Save the fail controller
	 * @param[in] uControllerID The controller ID
	*/
	inline void SaveFailController(UINT uControllerID);
private:
	std::map<UINT, UINT> m_mapUndiagnosableController;///<The undiagnosable controller, key is the controller ID and value is the related controller ID
	std::set<UINT> m_setFailController;///<The fail controller
};
