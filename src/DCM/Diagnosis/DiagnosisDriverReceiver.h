#pragma once
/**
 * @file DiagnosisDriverReceiver.h
 * @brief The diagnosis item for diagnosis the function of driver and receiver
 * @author Guangyun Wang
 * @date 2020/08/27
 * @copyright AccoTEST Business Unit of Beijing Huangfeng Test & Control Technology Co., Ltd.
*/
#include "DiagnosisItem.h"
#include <set>
class IHDReportDevice;
/**
 * @class CDiagnosisDriverReceiver
 * @brief The diagnosis item for diagnosis the function of driver and receiver
*/
class CDiagnosisDriverReceiver : public CDiagnosisItem
{
public:
    /**
     * @brief Constructor
    */
    CDiagnosisDriverReceiver();
    /**
     * @brief Destructor
    */
    virtual ~CDiagnosisDriverReceiver();
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
     * @brief Check the driver and receiver
     * @param[in] lpszBaseIndent The base indent 
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckDriverReceiver(const char* lpszBaseIndent);
    /**
     * @brief Check the status word
     * @param[in] lpszBaseIndent The base indent
     * @param[in] cStatusWard The status word
     * @param[in] nStartLine The start line number of vector running
     * @param[in] nStopLine The stop line number of vector running
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
	 * - -2 Not controller valid for diagnosed
    */
    int CheckStatusWord(const char* lpszBaseIndent, char cStatusWard, int nStartLine, int nStopLine);
    /**
	 * @brief Check the pin level
	 * @param[in] lpszBaseIndent The base indent
	 * @param[in] dVIH The input voltage of logic high
	 * @param[in] dVIL The input voltage of logic low
	 * @param[in] dVOH The output voltage of logic high
	 * @param[in] dVOL The output voltage of logic low
     * @param[in] nStartLine The start line of vector
     * @param[in] nStopLine The stop line eof vector
     * @param[in] ulExpectStatus The expect status of channel
     * @return Check result
	 * - 0 Check pass
	 * - -1 Check fail
    */
    int CheckPinLevel(const char* lpszBaseIndent, double dVIH, double dVIL, double dVOH, double dVOL, int nStartLine, int nStopLine, unsigned long ulExpectStatus);
	/**
	 * @brief Set the controller whose diagnosis result is fail
	 * @param[in] uControllerID The controller ID
	*/
	inline void SetFailController(UINT uControllerID);
private:
    std::set<UINT> m_setFailController;///<The controller whose diagnosis result is fail
};
