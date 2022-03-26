#pragma once
/**
 * @file I2CRAM.h
 * @brief The class be used to allocate vector line of I2C
 * @author Guangyun Wang
 * @date 2022/02/16
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <map>
#include <windows.h>
class CI2CManage;
/**
 * @class CI2CRAM
 * @brief The class of vector line allocated
*/
class CI2CRAM
{
public:
	/**
	 * @brief Set the base start vector line
	 * @param[in] uBaseLine The start of vector line number
	 * @return Execute result
	 * - 0 Set base line successfully
	 * - -1 Not allowed
	*/
	int SetBaseLine(UINT uBaseLine);
	/**
	 * @brief Allocate vector line
	 * @param[in] uLineCount The line count need to allocate
	 * @param[out] puStartLine The allocated start line
	 * @return Execute result
	 * - >0 The vector line count has allocated
	 * - -1 The point parameter is nullptr
	 * - -2 No enough vector line, must reset vector line pool before
	*/
	int AllocateLine(UINT uLineCount, UINT* puStartLine);
	/**
	 * @brief Free the vector line allocated before
	 * @param[in] uStartLine The start line in BRAM
	 * @param[in] uLineCount The vector line count in BRAM need to be free
	 * @return Execute result
	 * - >=0 The line count be freed
	 * - -1 Free vector line fail
	*/
	int FreeLine(UINT uStartLine, UINT uLineCount);
	/**
	 * @brief Reset vector line pool
	*/
	void Reset();
	/**
	 * @brief Free the memory allocated
	*/
	void Clear();
private:
	/**
	 * @brief Constructor
	*/
	CI2CRAM();
	/**
	 * @brief Free vector line allocated before
	 * @param[in] nStartLine The start line will be freed
	 * @param[in] nLineCount The line count will be frea
	 * @return Execute result
	 * - 0 Free line successfully
	 * - -1 The start line want to be free is illegal
	 * - -2 The line count is illegal
	*/
	int FreeLine(int nStartLine, int nLineCount);
	friend class CI2CManage;
private:
	int m_nBaseLine;///<The base line
	int m_nTotalValidLineCount;///<The total BRAM valid line count
	std::map<int, UINT> m_mapPatternLine;///<Key is start line number, value is the line count
};

