#pragma once
/**
 * @file DiagnosisItem.h
 * @brief The pure virtual base class of diagnosis item
 * @author Guangyun Wang
 * @date 2020/09/13
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "IHDDoctorItem.h"
#include <string>
#include <vector>
#include <map>
#include <stack>
#include "..\HardwareFunction.h"
#include "IHDReportDevice.h"

extern bool operator==(const CHardwareFunction::DATA_RESULT& SourceResult, const CHardwareFunction::DATA_RESULT& TargetResult);

#define ERROR_PRINT 5
/**
 * @class CDiganosisItem
 * @brief The pure virtual base class of diagnosis item
*/
class CDiagnosisItem : public IHDDoctorItem
{
public:
	/**
	 * @enum USER_ROLE
	 * @brief The role type of the user
	*/
	enum USER_ROLE
	{
		PROCUCTION,///<The user is production
		DEVELOPER,///<The user is Research and Developer
		USER,///<The user is custom
	};
    /**
     * @brief Constructor
    */
    explicit CDiagnosisItem();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisItem();
    /**
     * @brief Get the indent count
     * @return The count of indent char
    */
    int Indent() const
	{
        return m_nIndent;
    }
    /**
     * @brief Get the indent format
     * @return The indent format
    */
    const char* IndentFormat() const
	{
        return m_strIndent.c_str();
    }
    /**
     * @brief Set the count of indent char
     * @param[in] nIndent The count of indent char
    */
    void SetIndent(int nIndent);
    /**
     * @brief The the indent char of next section of log
     * @return The indent char of next section
    */
    const std::string & IndentChar() const
	{
        return m_strIndentChar;
    }
	/**
	 * @brief Set the controller diagnosed
	 * @param[in] vecController The controller diagnosed, the value is controller ID
	*/
	void SetEnableController(const std::vector<UINT>& vecController);
	/**
	 * @brief Get the diagnosis controller
	 * @param[out] vecController The diagnosis controller, the value is controller ID
	 * @return Execute result
	 * - 0 Enable controller successfully
	*/
	int GetEnableController(std::vector<UINT>& vecController);
	/**
	 * @brief Bind controller
	 * @param[in] vecController The controller ID
	 * @param[in] uTargetController The target controller ID
	*/
	void Bind(const std::vector<UINT>& vecController, UINT uTargetController);
	/**
	 * @brief Clear bind
	*/
	void ClearBind();
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
    virtual int Doctor(IHDReportDevice * pReportDevice) = 0;
	/**
	 * @brief Whether the item is checkable
	 * @return Whether the item checkable
	 * - 1 Checkable
	 * - 0 Uncheckable
	*/
	virtual int IsCheckAble() const;
	/**
	 * @brief Set the user' role
	 * @param[in] UserRole The user's role
	*/
	virtual void SetUserRole(USER_ROLE UserRole);
	/**
	 * @brief Whether the user select to diagnose this item
	 * @return Whether the user select to diagnose this item
	 * - true User select to diagnose this item
	 * - false User not select to diagnose this item
	*/
	virtual bool IsUserCheck() const;
protected:
	/**
	 * @brief Start timer
	*/
	void StartTimer();
	/**
	 * @brief Stop timer
	 * @param[out] lpszTimeUnits The time units
	 * @param[in] nBuffSize The buff size
	 * @return The time consume
	*/
	double StopTimer(char *lpszTimeUnits = nullptr, int nBuffSize = 0);
	/**
	 * @brief Get the time units
	 * @param[in] dTime The time
	 * @param[out] lpszTimeUnits The units
	 * @param[in] nBuffSize The buff size
	 * @return The time consume in current units
	*/
	double GetTimeUnits(double dTime, char* lpszTimeUnits, int nBuffSize);
	/**
	 * @brief Get the period units
	 * @param[in] dPeriod The period in ns
	 * @param[out] lpszPeriodUnits The period units
	 * @param[in] nBuffSize The buff size
	 * @return The period in target units
	*/
	double GetPeriodUnits(double dPeriod, char* lpszPeriodUnits, int nBuffSize);
	/**
	 * @brief Wait us
	*/
	void Wait(UINT uUs);
	/**
	 * @brief Wait vector stop
	 * @return Execute result
	 * - 0 Stop normally
	 * - -1 Not ran vector before
	 * - -2 No stop before timeout
	*/
	int WaitStop();
	/**
	 * @brief Wait 
	 * @param[in] uControllerID 
	 * @return Execute result
	 * - 0 Stop normally
	 * - -1 Not ran vector before
	 * - -2 Timeout
	 */
	int WaitStop(UINT uControllerID);
	/**
	 * @brief Clear map data
	 * @tparam Key The key value type
	 * @tparam Value The value type
	 * @param[out] mapData The map needed clear
	*/
	template <typename Key, typename Value>
	inline void ClearMap(std::map<Key, Value*>& mapData);
	/**
	 * @brief Get the class point of CHardwareFunction
	 * @param[in] uControllerID The controller ID
	 * @return The point of CHardwareFunction
	*/
	CHardwareFunction* GetHardware(UINT uControllerID);
	/**
	 * @brief Connect the relay of the board which will be diagnosed
	*/
	void Connect();
	/**
	 * @brief Disconnect the relay of the board will be diagnosed
	*/
	void Disconnect();
	/**
	 * @brief Check the controller and filter the undiagnosable controller
	*/
	void CheckMutualDiagnosable();
protected:
	int m_nEnableStatus;///<The enable status of the item, - 0 not enable - 1 Not enable all - 2 Enable all item
	std::vector<UINT> m_vecEnableController;///<The diagnosis controller, the value is controller ID
	USER_ROLE	m_UserRole;///<The user's role
	std::map<UINT, CHardwareFunction*> m_mapHardware;///<The point of hardware class, the key is controller ID and the value is its' hardware
	IHDReportDevice* m_pReportDevice;///<The point of report device
	std::map<UINT, UINT> m_mapUndiagnosableController;///<The undiagnosable controller, key is the controller ID and value is the related controller ID
private:
	int m_nIndent;///<The indent char count
	std::string m_strIndentChar;///<The indent char of next section in log file
	std::string m_strIndent;///<The base indent
	std::stack<LARGE_INTEGER> m_stackStartTick;///<The start tick of each timer
};

