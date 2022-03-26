#pragma once
/**
 * @file RunAuthorization.h
 * @brief This head file is for the class CRunAuthorization which manage the run vector authozation
 * @author Guangyun Wang
 * @date 2021/06/28
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>
/**
 * @class CRunAuthorization
 * @brief The singleton class for vector running authorization management
*/
class CRunAuthorization
{
public:
	/**
	 * @brief Get the instance of current class
	 * @return The point of the run authorization
	*/
	static CRunAuthorization* Instance();
	/**
	 * @brief Apply running authorization
	*/
	void Apply();
	/**
	 * @brief Release running authorization
	*/
	void Release();
	/**
	 * @brief Destructor
	*/
	~CRunAuthorization();
private:
	/**
	 * @brief Constructor
	*/
	CRunAuthorization();
private:
	CRITICAL_SECTION m_criRun;///<The critical section for run vector
};

