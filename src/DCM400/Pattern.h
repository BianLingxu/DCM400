#pragma once
/**
 * @file Pattern.h
 * @brief The head file for pattern load and read
 * @author Guangyun Wang
 * @date 2022/02/26
 * @copyright AccoTEST Business Unit of Beijing Huafneg Test & Control Co., Ltd.
*/
#include "HardwareFunction.h"
#include <map>
#include <set>
#include "DriverAlarm.h"
#include "PatternCMD.h"
/**
 * @class CVectorData
 * @brief The vector data
*/
class CVectorData
{
public:
	/**
	 * @brief Constructor
	 * @param[in] pAlarm The driver alarm
	*/
	CVectorData(CDriverAlarm* pAlarm);
	/**
	 * @brief Set the channel data
	 * @param[in] usChannel The channel number
	 * @param[in] cPattern The pattern of the channel
	 * @return Execute result
	 * - 0 Set channel data successfully
	 * - -1 The channel is over range
	*/
	int SetChannelData(USHORT usChannel, char cPattern);
	/**
	 * @brief Set the command data
	 * @param[in] usCMDData The command data
	*/
	void SetCommandData(USHORT usCMDData);
	/**
	 * @brief Get vector data
	 * @return The vector data, the element count is 2
	*/
	const ULONG* GetVectorData();
private:
	CDriverAlarm* m_pAlarm;///<The point pointed to class CDriverAlarm
	ULONG m_aulData[2];///<The vector data
};
/**
 * @class CCMDData
 * @brief The command data
*/
class CCMDData
{
public:
	/**
	 * @brief Constructor
	*/
	CCMDData();
	/**
	 * @brief Set the command data
	 * @param pulData The commmand data, the element count must not less tan 8
	 * @return Execute result
	 * - 0 Set the data successfully
	 * - -1 The point pointed to command data is nullptr
	*/
	int SetData(const ULONG* pulData);
	/**
	 * @brief Get the command data
	 * @return The point pointed to command data, the element count is 8
	*/
	const ULONG* GetCMDData();
private:
	ULONG m_aulData[8];///<The command data
};

/**
 * @class CPattern
 * @brief The pattern read or write class
*/
class CPattern
{
public:
	/**
	 * @brief Constructor
	 * @param[in] HardwareFunction The hardware function class for current controller
	 * @param[in] pAlarm The driver alarm
	*/
	CPattern(CHardwareFunction& HardwareFunction, CDriverAlarm* pAlaram = nullptr);
	/**
	 * @brief Destructor
	*/
	~CPattern();
	/**
	 * @brief Add the pattern of all channel
	 * @param[in] uPatternLine The line index in the memory
	 * @param[in] lpszPattern The pattern sign
	 * @param[in] usTimeset The timeset of current pattern line
	 * @param[in] setCMD The command of pattern
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The pattern index is over range
	 * - -2 The pattern has added before
	 * - -3 The point of pattern is nullptr
	 * - -4 The timeset is over range
	 * - -5 The command is not supported
	 * - -6 The operand is over range
	 * - -7 The pattern sign is not supported
	*/
	int AddPattern(UINT uPatternLine, const char* lpszPattern, USHORT usTimeset, const std::set<CMD_INFO>& setCMD);
	/**
	 * @brief Set the channel pattern
	 * @param[in] usChannel The channel number
	 * @param[in] uPatternLine The pattern line
	 * @param[in] cPattern The pattern of the channel
	 * @param[in] PatternCMD The pattern command
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The channel is over range
	 * - -2 The pattern number is over range
	 * - -3 Allocate memory fail
	*/
	int AddChannelPattern(USHORT usChannel, UINT uPatternLine, char cPattern, const CPatternCMD& PatternCMD);
	/**
	 * @brief Load pattern
	 * @return Execute result
	 * - 0 Load pattern successfully
	 * - -1 Allocate memory fail
	*/
	int Load();
private:
	CHardwareFunction* m_pHardware;///<The point pointed to the hardare function class
	CDriverAlarm* m_pAlarm;///<The point pointed to class CDriverAlarm
	std::map<UINT, CVectorData> m_mapVectorData;///<The vector data, the key is vector line number and value is its data
	std::map<UINT, CCMDData> m_mapCMDData;///<The vector data, the key is vector line number and value is its data
};

