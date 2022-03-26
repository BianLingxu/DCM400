#pragma once
/**
 * @file SelfCheck.h
 * @brief The head file for the class using for self test
 * @author Guangyun Wang
 * @date 2022/04/05
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HardwareFunction.h"
/**
 * 
 * @brief The class for self testing
*/
class CSelfCheck
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CSelfCheck(BYTE bySlotNo);
	/**
	 * @brief Destructor
	*/
	~CSelfCheck();
	/**
	 * @brief Check board
	 * @param[in] lpszLogFileName The log file name
	 * @param[in] nChannelResult The result of each channel, each result each channel
	 * - 0 Check PASS
	 * - 1 Check FAIL
	 * @return The check result
	 * - 1 Check PASS
	 * - 0 Check FAIL
	*/
	int Check(const char* lpszLogFileName, int& nChannelResult);
private:
	/**
	 * @brief Save the board information to file
	*/
	void SaveBoardInfo();
private:
	BYTE m_bySlotNo;///<The slot number
	std::map<BYTE, CHardwareFunction*> m_mapController;///<The hardware function for the board
	std::string m_strLogFile;///<The file name for the check log;
};

