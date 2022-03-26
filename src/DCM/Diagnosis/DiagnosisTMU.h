#pragma once
/**
 * @file DiagnosisTMU.h
 * @brief The diagnosis item of TMU
 * @author Guangyun Wang
 * @date 2020/08/27
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
class IHDReportDevice;
/**
 * @class CDiagnosisTMU
 * @brief The class of TMU diagnosis item
*/
class CDiagnosisTMU :
    public CDiagnosisItem
{
public:
	/**
	 * @brief Constructor
	*/
	CDiagnosisTMU();
	/**
	 * @brief Destructor
	*/
	~CDiagnosisTMU();
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
	 * @brief Diagnosis the period mode measurement
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int PeriodModeDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis the delay mode measurement
	 * @param[in] lpszBaseIndent The base indent
	 * @return Diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int DelayModeDiagnosis(const char* lpszBaseIndent);
	/**
	 * @brief Diagnosis the period and duty
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] dPeriod The period
	 * @return Diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int PeriodDiagnosis(const char* lpszBaseIndent, double dPeriod);
	/**
	 * @brief The period diagnosis of each channel
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] dPeriod The period
	 * @param[in] dDuty The duty
	 * @param[in] bDutyCheck Whether check the duty measured
	 * @return Diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int PeriodChannelDiagnosis(const char* lpszBaseIndent, double dPeriod, double dDuty, BOOL bDutyCheck);
	/**
	 * @brief Load the pattern of period mode
	 * @param[in] usEvenChannel The channel number of the even controller
	 * @param[in] usOddChannel The channel number of the odd controller
	 * @param[in] dPeriod The period
	 * @param[in] dDuty The duty
	 * @param[in] uPeriodCount The period count of the pattern
	 * @return The pattern line count
	 * - >0 The pattern count
	 * - -1 The channel number of even controller is over range
	 * - -2 The channel number of odd controller is over range
	*/
	int LoadPeriodPattern(USHORT usEvenChannel, USHORT usOddChannel, double dPeriod, double dDuty, UINT uPeriodCount);
	/**
	 * @brief Diagnosis the delay
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] dDelay The delay value
	 * @return Diagnosis result
	 * - 0 Diagnosis result is PASS
	 * - -1 Diagnosis result is FAIL
	*/
	int DelayDiagnosis(const char* lpszBaseIndent, double dDelay);
	/**
	 * @brief Print result to UI
	*/
	void ShowUIResult();
	/**
	 * @brief Bind controller
	 * @param[in] nEvenController Whether bind the even controller, 0 is Odd, 1 is even, -1 is all
	 * @return The target controller ID
	*/
	UINT Bind(int nEvenController);
	/**
	 * @brief Clear bind
	*/
	void ClearBind();
	/**
	 * @brief Save the fail controller
	 * @param[in] uControllerID The controller ID
	*/
	inline void SaveFailController(UINT uControllerID);
	/**
	 * @brief Load the delay pattern
	 * @param[in] dDelay The delay value
	 * @param[in] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @return The pattern line count
	*/
	int LoadDelayPattern(double dDelay, BOOL bRaiseTriggerEdge);
	/**
	 * @brief Set the TMU parameter
	 * @param[in] MeasMode The measurement mode
	 * @param[in] bRaiseTriggerEdge Whether the trigger edge is raise
	 * @param[in] uHoldOffTime The hold off time
	 * @param[in] uHoldOffNum The hold off number
	 * @param[in] uSampleNum The sample number
	 * @param[in] dTimeout The timeout
	*/
	void SetTMUParam(TMU_MEAS_MODE MeasMode, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, UINT uSampleNum, double dTimeout);
	/**
	 * @brief Set the TMU parameter
	 * @param[in] MeasMode The measurement mode
	 * @param[in] bRaiseEdge The trigger edge
	 * @param[in] uHoldOffTime The hold off time
	 * @param[in] uHoldOffNum The hold off number
	 * @param[in] uSampleNum The sample number
	 * @param[in] dTimeout The timeout
	 * @param[in] usEvenControllerChannel The channel be measurement by the TMU unit of even controller
	 * @param[in] usOddControllerChannel The channel be measurement by the TMU unit of odd controller
	*/
	void SetTMUParam(TMU_MEAS_MODE MeasMode, BOOL bRaiseEdge, UINT uHoldOffTime, UINT uHoldOffNum, UINT uSampleNum, double dTimeout, USHORT usEvenControllerChannel, USHORT usOddControllerChannel);
private:
	std::map<UINT, UINT> m_mapUndiagnosableController;///<The undiagnosable controller, key is the controller ID and value is the related controller ID
	std::set<UINT> m_setFailController;///<The fail controller
};

