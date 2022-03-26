#pragma once
#include <windows.h>
#include <stack>
#include <vector>
#include <string>
/**
 * @file Timer.h
 * @brief The class user for timer
 * @author Guangyun Wang
 * @date 2020/07/17
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
/**
 * @class CTimer
 * @brief The timer class
*/
class CTimer
{
public:
	/**
	 * @brief Get the instance of CTimer
	 * @return The point of timer
	*/
	static CTimer* Instance();
	/**
	 * @brief Reset the timer added before
	*/
	void Reset();
	/**
	 * @brief Start timer
	 * @param[in] lpszFormat The format of sign timer
	 * @param[in]  The message will be formated
	*/
	void Start(const char* lpszFormat,...);
	/**
	 * @brief Stop timer
	 * @return The time calculated
	*/
	double Stop();
	/**
	 * @brief Print time of each timer to file
	 * @param[in] lpszFilePath The file path
	 * @return Execute result
	 * - 0 Save file successfully
	 * - -1 The file path is nullptr
	 * - -2 Create file fail
	*/
	int Print(const char* lpszFilePath);
private:
	/**
	 * @brief Constructor
	*/
	CTimer();
private:
	LARGE_INTEGER m_TickFreq;///<The frequency of tick
	std::vector<std::string> m_vecSign;///<The sign of timer
	std::vector<double> m_vecTime;///<The time of each timer
	std::stack<std::string> m_stackSign;///<The sign of timer before stop
	std::stack<LARGE_INTEGER> m_stackStartTick;///<The start tick of the timer running
};

