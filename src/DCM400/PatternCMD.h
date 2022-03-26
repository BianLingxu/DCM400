#pragma once
/**
 * @file PatternCMD.h
 * @brief The head file for command information 
 * @author Guangyun Wang
 * @date 2022/03/01
 * @copyright AccoTEST Business Unit of beijing Huafeng Test & Control Co., Ltd
*/
#include "DriverAlarm.h"
#include "DCM400HardwareInfo.h"
#include <set>
#include "CMDCode.h"

#define CMD_INFO_COUNT 8


bool operator <(const CMD_INFO& CMD1, const CMD_INFO& CMD2);
/**
 * @brief The line command
*/
class CLineCMD
{
public:
	/**
	 * @brief Constructor
	*/
	CLineCMD();
	/**
	 * @brief Set the command of line
	 * @param[in] usTimeSet The Time Set
	 * @param[in] vecCMD The command of current line
	*/
	void SetCMD(USHORT usTimeSet, const std::vector<CMD_INFO>& vecCMD);
	/**
	 * @brief Get the comamnd of the pattern
	 * @param[in] usTimeset The time set
	 * @param[out] vecCMD The command
	*/
	void GetCMD(USHORT& usTimeset, std::vector<CMD_INFO>& pvecCMD) const;
private:
	USHORT m_usTimeSet;///<The time set of current line
	std::vector<CMD_INFO> m_vecCMD;///<The command of current line
};

/**
 * @class CPatternCMD
 * @brief The pattern command
*/
class CPatternCMD
{
public:
	/**
	 * @brief Constructor
	 * @param[in] pAlarm The point pointed to class CDriverAlarm
	*/
	CPatternCMD(CDriverAlarm* pAlarm = nullptr);
	/**
	 * @brief Set the command information
	 * @param[in] uLineNo The line number
	 * @param[in] usTimeSet The time set of current command
	 * @param[in] setCurLineCMD 
	 * @param[in] mapOtherLinesCMD 
	 * @param[in] setLineNeeded 
	 * @return Execute result
	 * - 0 Set command information successfully
	 * - -1 The line number is over range
	 * - -2 The time set is over range
	 * - -3 The operand is over range
	 * - -4 Other lines' command needed
	 * - -5 The time set is over range
	 * - -6 The operand is over range
	*/
	int SetCommandInfo(UINT uLineNo, USHORT usTimeSet, const std::vector<CMD_INFO>& setCurLineCMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded);
	/**
	 * @brief Get the general command data
	 * @return The command data, the element count is 8
	*/
	const ULONG* GetGeneralCMDData() const;
	/**
	 * @brief Get the command data which saved with pattern
	 * @return The specified command data
	*/
	const USHORT GetSpecifiedCMDData() const;
private:
	/**
	 * @brief Parse the specified command
	 * @param[in] vecCMD The command
	 * @return Execute result
	 * - 0 The specified command successfully
	 * - -1 The command is not supported
	*/
	int ParseSpecifiedCMD(std::vector<CMD_INFO>& vecCMD);
	/**
	 * @brief Save the first two command memorys' data
	 * @param[in] CMD The command
	 * @param[in] mapOtherLinesCMD The command of others line 
	 * @param[out] setLineNeeded The line need
	 * @return Execute result
	 * - 0 Save the command successfully
	 * - -1 The command is not supported
	 * - -2 The others line command neeeded is not provided
	 * - -3 The time set is over range
	 * - -4 The operand is over range
	*/
	int SaveFirstTwoCMD(const CMD_INFO& CMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded);
	/**
	 * @brief Save the last two command  memorys' data
	 * @param[in] CMD The command
	 * @param[in] mapOtherLinesCMD The command of others line
	 * @param[out] setLineNeeded The line need
	 * @return Execute result
	 * - 0 Save the command successfully
	 * - -1 The command is not supported
	 * - -2 The others line command neeeded is not provided
	 * - -3 The time set is over range
	 * - -4 The operand is over range
	*/
	int SaveLastTwoCMD(const CMD_INFO& CMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded);
	/**
	 * @brief Sav the command data to memory
	 * @param[in] nCMDIndex The command index
	 * @param[in] usTimeSet The time set
	 * @param[in] nCode The command code
	 * @param[in] ulOperand The operand
	 * @param[in] uLineNo The line number of current command
	 * @return Execute result
	 * - 0 Save command successfully
	 * - -1 The command index is over range
	 * - -2 The time set is over range
	 * - -3 The command is not support
	 * - -4 The operand is over range
	*/
	int SaveCMD(int nCMDIndex, USHORT usTimeSet, int nCode, ULONG ulOperand, UINT uLineNo);
	/**
	 * @brief Get the command from line commnad
	 * @param[in] LineCMD The line command
	 * @param[out] usTimeSet The timeset of the line
	 * @param[out] CMDInfo The command information
	*/
	void GetGeneralCMD(const CLineCMD& LineCMD, USHORT usTimeSet, CMD_INFO& CMDInfo);
private:
	CDriverAlarm* m_pAlarm;
	int m_nLineNo;///<Current line number
	BOOL m_bScan;///<Whether in scan mode
	ULONG m_aulGeneral[CMD_INFO_COUNT];///<The general command data which saved 
	USHORT m_usSpecified;///<The specified command data
};

