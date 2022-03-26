#pragma once
/**
 * @file I2CBoardManage
 * @brief The head file is saving the class for board management
 * @author Guangyun Wang
 * @date 2021/05/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "I2CBoard.h"
/**
 * @class CI2CBoardManage
 * @brief The class for I2C board management
*/
class CI2CBoardManage
{
public:
	/**
	 * @brief Destructor
	*/
	~CI2CBoardManage();
	/**
	 * @brief Set the I2C site
	 * @param[in] Site The I2C site
	*/
	void SetSite(const CI2CSite& Site);
	/**
	 * @brief Get the I2C board
	 * @param[in] bySlotNo The slot number of the board
	 * @return The point of I2CBoard
	 * - != nullptr The point of the I2CBoard
	 * - nullptr The board is not existed
	*/
	CI2CBoard* GetBoard(BYTE bySlotNo, CDriverAlarm* pAlarm);
	/**
	 * @brief Run vector
	 * @param[in] uStartLine The start line of BRAM
	 * @param[in] uStopLine The stop line of BRAM
	 * @param[in] bWithDRAM Whether the line is with DRAM
	 * @param[in] uDRAMStartLine The DRAM start line if the vector with DRAM
	*/
	void Run(UINT uStartLine, UINT uStopLine, BOOL bWithDRAM = FALSE, UINT uDRAMStartLine = 0);
	/**
	 * @brief Reset the board information
	*/
	void Reset();
	/**
	 * @brief The valid board
	 * @param[in] vecBoard The valid board
	*/
	void SetValidBoard(const std::vector<BYTE>& vecBoard);
	/**
	 * @brief Enable compare shield
	 * @param[in] bEnable Whether enabel compare shield
	*/
	void EnableCopmareShield(BOOL bEnable);
private:
	/**
	 * @brief Constructor
	*/
	CI2CBoardManage();
	friend class CI2CManage;
private:
	const CI2CSite* m_pSite;///<The I2C site
	BOOL m_bEnableCompareShield;///<Whether enable compare shield
	std::map<BYTE, CI2CBoard*> m_mapBoard;///<The I2C board, key is slot number and value is the point of class CI2CBoard
	std::set<BYTE> m_setBoardExisted;///<The board existed
};
