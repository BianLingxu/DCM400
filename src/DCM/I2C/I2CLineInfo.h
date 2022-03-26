#pragma once
/**
 * @file I2CLineInfo
 * @brief The head file is saving the class for the line information of I2C operation
 * @author Guangyun Wang
 * @date 2021/05/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "I2CBase.h"
#include <string>
#include <map>
/**
 * @class CI2CLineInfo
 * @brief The singleton class using for I2C line information management
*/
class CI2CLineInfo
{
public:
	/**
	 * @brief Get the instance of the singleton class
	 * @return The point of the instance
	*/
	static CI2CLineInfo* Instance();
	/**
	 * @brief Reset line information
	*/
	void Reset();
	/**
	 * @brief Set the I2C base
	 * @param[in] pI2CRead The I2C read base
	 * @param[in] pI2CWrite The i2C write base
	*/
	void SetI2CBase(CI2CBase* pI2CRead, CI2CBase* pI2CWrite);
	/**
	 * @brief Read I2C line information
	 * @param[in] pairI2C The I2C line
	 * @param[in] bRead Whether the I2CLine is read operation
	*/
	void RecordLine(const std::pair<std::string, CI2CLine*>& pairI2C, BOOL bRead);
	/**
	 * @brief Free I2C line
	 * @param[in] nLineCount The line count needed
	 * @return Execute result
	 * - 0 Free line successfully
	 * - -1 The line count needed is less than 1
	 * - -2 Free line fail
	*/
	int FreeLine(int nLineCount);
private:
	/**
	 * @brief Constructor
	*/
	CI2CLineInfo();
	/**
	 * @brief Record all line of I2C
	*/
	void RecordAllLine();
private:
	struct LINE_INFO 
	{
		bool m_bRead;///<Whether the line is I2C read
		int m_nLineCount;///<The line count of the operation
		std::string m_strKey;///<The key of I2C operation
		LINE_INFO()
		{
			m_bRead = true;
			m_nLineCount = 0;
		}
	};
	CI2CBase* m_pI2CBase[2];///<The I2C base operation for I2C read and write
	BOOL m_bRecord;///<Whether record line information
	std::map<UINT, LINE_INFO> m_mapLineInfo;///<The line information of the I2C operation
};

