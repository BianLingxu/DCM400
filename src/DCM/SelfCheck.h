#pragma once
/**
 * @file SelfCheck.h
 * @brief Include the class of the CSelfCheck
 * @detail The class using for board's self-check
 * @author Guangyun Wang
 * @date 2020/06/15
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HardwareFunction.h"
#include "DCM.h"
#include <string>
#include "CheckLog.h"
/**
 * @class CSelfCheck
 * @brief The class using for board's self-check
*/
class CSelfCheck
{
public:
	/**
	 * @brief Constructor
	 * @param[in] DCM The DCM class
	*/
	CSelfCheck(CDCM& DCM);
	/**
	 * Destructor
	 */
	~CSelfCheck();
	/**
	 * @brief Check the board
	 * @param[in] lpszLogFileName The log file name 
	 * @param[in] bySlotNo The slot number of the board checked
	 * @param[in] pnCheckResult The check result of each channel
	 * @return The check result
	 * - 1 The board check pass
	 * - 0 The board check fail
	*/
	int Check(const char* lpszLogFileName, BYTE bySlotNo, int* pnCheckResult);
private:
	/**
	 * @brief Save the board information to file
	*/
	void SaveBoardInfo();
	/**
	 * @brief Save calibration information to file
	*/
	void SaveCalibrationInfo();
	/**
	 * @brief Get the calibration meter type
	 * @param[out] mapCalMeter The calibration meter type
	*/
	void GetCalibrationMeter(std::map<int, std::string>& mapCalMeter);
	/**
	 * @brief Check the BRAM of all controller
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int BRAMCheck();
	/**
	 * @brief Check the DRAM of all controller
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int DRAMCheck();
	/**
	 * @brief Check the instruction
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int InstructionCheck();
	/**
	 * @brief High speed memory check
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int HighSpeedMemoryCheck();
	/**
	 * @brief PMU check
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int PMUCheck(int* pnChannelResult);
	/**
	 * @brief TMU check
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int TMUCheck();
	/**
	 * @brief Check the serial to parallel function
	 * @return The check result, one bit one controller, 1 is pass and 0 is fail
	*/
	int SerialParallelTransferCheck();
	/**
	 * @brief Bind all controller
	*/
	void Bind();
	/**
	 * @brief Clear bind
	*/
	void ClearBind();
	/**
	 * @brief Save file log
	 * @param[in] pFileLog The point of file
	 * @param[in] lpszFormat The format of log
	 * @param[in]  The input data
	*/
	inline void SaveLog(FILE* const pFileLog, const char* lpszFormat, ...);
	/**
	 * @brief Format log
	 * @param[in] nMaxByteCount The log byte count
	 * @param[in] bAlignType Alignment type
	 - 0 is left alignment
	 - 1 is center alignment 
	 - 2 is right alignment
	 * @param[in] cAddCharacter The add character if the byte input is enough
	 * @param[in] lpszFormat The format of the log
	 * @param[in] The input data
	 * @return The formated log
	*/
	inline std::string& FormatLog(int nMaxByteCount, BYTE bAlignType, char cAddCharacter, const char* lpszFormat, ...);
	/**
	 * @brief Save the test item
	 * @param[in] pFileLog The point of file log
	 * @param[in] lpszItemName The item name
	*/
	inline void SaveTestItem(FILE*& pFileLog, const char* lpszItemName);
	/**
	 * @brief 
	 * @param[in] pFileLog  The point of file log
	 * @param[in] lpszItemName The item name
	*/
	inline void SaveSubTestItem(FILE* const pFileLog, const char* lpszItemName, ...);
	/**
	 * @brief No board existed
	 * @param[in] pnCheckResult The check result of each channel
	 * @param[in] lpszFormat The format of the information
	 * @param[in]  The information be format
	*/
	void NoBoardExist(int* pnCheckResult, const char* lpszFormat, ...);
	/**
	 * @brief Check whether the controller is match to the controller count in flash
	 * @param[out] bCheckPass The check result
	 * @return Check result of controller, one bit one controller, 1 is pass and 0 is fail
	*/
	int CheckAuthorization(BOOL& bCheckPass);
	/**
	 * @brief Save the channel check result
	 * @param[int] byControlCheckResult The check result of each controller
	 * @param[in] pnCheckResult The check result of each channel
	 * @return Check result
	 * - 1 All pass
	 * - 0 Have fail
	*/
	int SetChannelCheckResult(BYTE byControlCheckResult, int* pnCheckResult);
	/**
	 * @brief Get the check result
	 * @param[out] setFailController The check failed controller
	 * @return The check result
	*/
	inline int GetCheckResut(std::set<BYTE>& setFailController);
	/**
	 * @brief Initialize channel status to high impedance
	*/
	void InitializeChannelStatus();
	/**
	 * @brief Print waring information
	*/
	void PrintWarning();
private:
	CDCM* m_pDCM;///<The point of DCM
	BYTE m_bySlotNo;///<The slot number of the board checked
	std::map<BYTE, CHardwareFunction*> m_mapHardware;///<The hardware function of each controller
	std::string m_strFileName;///<The file log name
	std::string m_strLogMsg;///<The log message
	BYTE m_byRecordControllerCount;///<The controller count in current board
	BYTE m_byValidControllerCount;///<The valid controller count in current board
	CCheckLog m_CheckLog;///<The log of the self check
	std::vector<std::string> m_vecWarning;///<The warning information during self check
};

