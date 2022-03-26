#pragma once
/**
 * @file I2CManage.h
 * @brief The file is the head file of singleton class CI2CManage
 * @author Guangyun Wang
 * @date 2021/08/11
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Co., Ltd.
*/
#include <windows.h>
#include <map>
#include "I2CRAM.h"
#include "I2CBoardManage.h"
class CI2C;
/**
 * @brief The singleton class for I2C board and RAM memory class management
*/
class CI2CManage
{
public:
	/**
	 * @brief Get the intance of the singleton class
	 * @return The point of the instance
	*/
	static CI2CManage* Instance();
	/**
	 * @brief Get the RAM management class
	 * @param[in] pI2C The point of class I2C
	 * @return The point of RAM
	*/
	CI2CRAM* GetRAMManage(const CI2C* const pI2C);
	/**
	 * @brief Get the board management class
	 * @param[in] pI2C The point of class I2C
	 * @return The point of board
	*/
	CI2CBoardManage* GetBoardManage(const CI2C* const pI2C);
	/**
	 * @brief Delete the intance
	 * @param[in] pI2C The point of class I2C
	*/
	void DeleteInstance(CI2C* pI2C);
	/**
	 * @brief Destructor
	*/
	~CI2CManage();
private:
	/**
	 * @brief Constructor
	*/
	CI2CManage();
	/**
	 * @brief Distribute class
	 * @param[in] dwAddress The address value of CI2C
	 * @param[in] bRAM Whether get the CI2CRAM
	 * @param[in] bDelete Whether delete
	 * @return The point of class
	*/
	void* DistributeClass(DWORD dwAddress, BOOL bRAM, BOOL bDelete);

private:
	CRITICAL_SECTION m_criAllocate;///<The critical section for class distribution
	std::map<DWORD, CI2CRAM*> m_mapRAM;///<The ram line management of I2C, key is class address of CI2C and value is point of ram class
	std::map<DWORD, CI2CBoardManage*> m_mapBoard;///<The board management of I2C, key is class address of CI2C and value is point of ram class
};

