#pragma once
/**
 * @file ChannelData.h
 * @brief Include the class using for prread vector function
 * @detail The preread function is using reduce the time for modifying the vector of some channels
 * @author Guangyun Wang
 * @date 2020/05/28
 * @copyright Copyright (c) AccoTEST business unit of Beijing Huafeng Test & Control Technology Co,. Ltd.
*/
#include "stdafx.h"
#include "HardwareInfo.h"
#include <set>
class CHardwareFunction;
/**
 * @class CPrereadVector
 * @brief The preread function is using reduce the time for modifying the vector of some channels
*/
class CChannelData
{
public:
    /**
     * @brief Constructor
     * @param[in] HardwareFunction The hardware function class
    */
    CChannelData(CHardwareFunction &HardwareFunction);
    /**
     * @brief Destructor
    */
    ~CChannelData();
    /**
     * @brief Set the preread vector information
     * @param[in] uStartLineIndex The start line number
	 * @param[in] uLineCount The line count
	 * @param[in] MemType The memory type
     * @return Execute result
     * - 0 Set the vector information successfully
     * - -1 Had loaded line before or the line loaded before is not contain the line information 
     * - -2 The start line is over range
     * - -3 The line count is over range
     * - -4 Allocate memory fail
    */
    int SetVectorInfo(UINT uStartLineIndex, UINT uLineCount, MEM_TYPE MemType);
    /**
     * @brief Set the line information whose wave data will be written in future
     * @param[in] uStartLine The start line 
	 * @param[in] uLineCount The line number count
	 * @param[in] MemType The memory type
     * @return Execute result
	 * - 0 Set the channel line successfully
	 * - -1 The preread vector is not valid for this sector
     * - -2 The start line is over range
	 * - -3 The line count is over range
     * - -4 Allocate memory fail
    */
    int SetLineInfo(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType);
    /**
     * @brief Set the channel data
     * @param[in] usChannel The channel number
     * @param[in] uStartLineIndex The start line number
     * @param[in] uLineCount The line count
     * @param[in] pbyData The data of the channel
     * @param[in] MemType The memory type
     * @return Execute result
     * - 0 The channel data is set successfully
     * - -1 The channel is over range
     * - -2 The point of data is nullptr
    */
    int SetChannelData(USHORT usChannel, MEM_TYPE MemType, const BYTE *pbyData);
    /**
     * @brief Write the vector data to memory
    */
    void Write();
    /**
     * @brief Get the preread type
     * @return The type of preread type
     * - 0 BRAM type
     * - 1 DRAM type
     * - -1 Not preread type
    */
    int GetPrereadType();
private:
    /**
     * @brief Set the data bit
     * @param[out] pusData The data whose bit will be modified
     * @param[in] nBitIndex The bit index
     * @param[in] bSet Whether set the bit data to 1
    */
   inline void SetBit(USHORT *pusData, int nBitIndex, BOOL bSet);
    /**
     * @brief Allocate memory
     * @return Execute result
     * - 0 Allocate memory successfully
	 * - -1 The line count in BRAM is 0, but the point is not nullptr
	 * - -2 The line count in DRAM is 0, but the point is not nullptr
     * - -3 Allocate memory fail
    */
    int AllocateMemory();
    /**
     * @brief Read data from memory and combine with current channel data
     * @return Execute result
     * - 0 Read and combine data successfully
     * - -1 Allocate memory fail
    */
    int ReadCombineData();
    /**
     * @brief Write channel's data
     * @return 
    */
    int WriteChannelData();
public:
    CHardwareFunction* m_pHardwareFunction;///<The class point of hardware function
    UINT m_uBRAMStartLine;///<The start line index of vector data in BRAM
    UINT m_uDRAMStartLine;///<The start line index of vector data in DRAM
    UINT m_uBRAMLineCount;///<The line count of vector data in BRAM
    UINT m_uDRAMLineCount;///<The line count of vector data in DRAM
    USHORT *m_pusBRAMFM;///<The vector data of FM in BRAM
    USHORT *m_pusBRAMMM;///<The vector data of MM in BRAM
	USHORT* m_pusDRAMFM;///<The vector data of FM in DRAM
	USHORT* m_pusDRAMMM;///<The vector data of MM in DRAM
    UINT m_uCurBRAMStartLine;///<The start line of current write in BRAM
    UINT m_uCurDRAMStartLine;///<The start line of current write in DRAM
    UINT m_uCurBRAMLineCount;///<The line count of current write in BRAM
    UINT m_uCurDRAMLineCount;///<The line count of current write in DRAM
    std::set<USHORT> m_setSetChannel;///<The channel whose pattern data will be written
    BOOL m_bPreread;///<Whether preread vector
    BOOL m_bBRAMSame[2];///<Whether the vector data of BRAM is same with the data in board
    BOOL m_bDRAMSame[2];///<Whether the vector data of DRAM is same with the data in board
};

