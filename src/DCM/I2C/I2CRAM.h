#pragma once
/**
 * @file I2CRAM.h
 * @brief The class be used to allocate vector line of I2C
 * @author Guangyun Wang
 * @date 2020/05/12
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "..\StdAfx.h"
#include <map>
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
	 * @param[in] uBRAMBaseLine The start of vector line number in RAM
	 * @param[in] uBRAMBaseLine The start of vector line number in DRAM
	 * @return Execute result
	 * - 0 Set base line successfully
	 * - -1 Not allowed
	*/
	int SetBaseRAM(UINT uBRAMBaseLine, UINT uDRAMBaseLine);
	/**
	 * @brief Allocate vector line
	 * @param[in] uLineCount The line count need to allocate
	 * @param[out] puBRAMStartLine The allocated start line in BRAM
	 * @param[out] puBRAMLineCount The allocated line count in BRAM
	 * @param[out] puDRAMStartLine The allocated start line in DRAM
	 * @param[out] puDRAMLineCount The allocated line count in DRAM
	 * @return Execute result
	 * - >0 The vector line count has allocated
	 * - -1 The point parameter is nullptr
	 * - -2 No enough vector line, must reset vector line pool before
	*/
	int AllocateLine(UINT uLineCount, UINT* puBRAMStartLine, UINT* puBRAMLineCount, UINT* puDRAMStartLine, UINT* puDRAMLineCount);
	/**
	 * @brief Free the vector line allocated before
	 * @param[in] uBRAMStartLine The start line in BRAM
	 * @param[in] uBRAMLineCount The vector line count in BRAM need to be free
	 * @param[in] uDRAMStartLine The start line in DRAM
	 * @param[in] uDRAMLineCount The vector line count in DRAM need to be free
	 * @return Execute result
	 * - >=0 The line count be freed
	 * - -1 Free vector line in BRAM fail
	 * - -2 Free vector line in DRAM fail
	*/
	int FreeLine(UINT uBRAMStartLine, UINT uBRAMLineCount, UINT uDRAMStartLine, UINT uDRAMLineCount);
	/**
	 * @brief Get the command line in BRAM
	 * @param[out] puOddStartLine The start line number shared by whose total line is odd
	 * @param[out] puOddStopLine The odd stop line number shared by whose total line is odd
	 * @param[out] puEvenStartLine The even start line number shared by whose total line is even
	 * @param[out] puEvenStopLine The even stop line number shared by whose total line is even
	*/
	int GetCommonLine(UINT* puOddStartLine, UINT* puOddStopLine, UINT* puEvenStartLine, UINT* puEvenStopLine);
	/**
	 * @brief Get the common line count
	 * @return The line count of common line
	*/
	int GetCommonLineCount();
	/**
	 * @brief Get the information of common line
	 * @param[in] bLineCountOdd Whether the line count is odd
	 * @param[out] pbyHeadLineCount The head line count
	 * @param[out] pbyEndLineCount The end line count
	 * @return Execute result
	 * - 0 Get line information successfully
	 * - -1 The point parameter is nullptr
	*/
	int GetCommonLineInfo(BOOL bLineCountOdd, BYTE* pbyHeadLineCount, BYTE* pbyEndLineCount);
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
	 * @brief Allocate vector line
	 * @param[in] bBRAM The memory type will be allocated
	 * @param[in] uLineCount The vector line count
	 * @param[out] puStartLine The start line of memory
	 * @return Execute result
	 * - 0 Allocate vector line successfully
	 * - -1 No enough vector line
	*/
	int AllocateLine(BOOL bBRAM, UINT uLineCount, UINT* puStartLine);
	/**
	 * @brief Free vector line allocated before
	 * @param[in] bBRAM The memory type, whether the type is BRAM
	 * @param[in] nStartLine The start line will be freed
	 * @param[in] nLineCount The line count will be frea
	 * @return Execute result
	 * - 0 Free line successfully
	 * - -1 The start line want to be free is illegal
	 * - -2 The line count is illegal
	*/
	int FreeLine(BOOL bBRAM, int nStartLine, int nLineCount);
	friend class CI2CManage;
private:
	int m_nBRAMBaseLine;///<The base line of BRAM
	int m_nBRAMBaseValidLine;///<The base valid line of BRAM
	int m_nDRAMBaseValidLine;///<The base valid line of DRAM
	int m_nCommonLineCount;///<The common line count
	int m_nTotalBRAMValidLineCount;///<The total BRAM valid line count
	int m_nTotalDRAMValidLineCount;///<The Total DRAM valid line count
	std::map<int, UINT> m_mapBRAMLine;///<Key is start line number, value is the line count
	std::map<int, UINT> m_mapDRAMLine;///<Key is start line number, value is the line count
};

