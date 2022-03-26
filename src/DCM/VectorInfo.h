#pragma once
#include "Pin.h"
#include "Timeset.h"
#include "map"
#include "IACVInstance.h"
#include "IACVProject.h"
#include "IACVUnknown.h"
#include "IACVFailTag.h"
using GETPROJECT = IACVInstance * (_stdcall*)();

struct LINE_DATA;
//The vector information of one pin.
typedef struct _Pin_Pattern_Info
{
	_Pin_Pattern_Info()
	{
		ucSerial = 1;
		memset(lpszPattern, 16, sizeof(lpszPattern));
	};
	unsigned char ucSerial;
	char          lpszPattern[16];//!< ÎÞ½áÎ²·ûºÅ
}PinPatternInfo, * pPinPatternInfo;

class CVectorLine
{
public:
	CVectorLine(USHORT uPinCount);
	~CVectorLine();
	int SetVectorLine(pPinPatternInfo pPinPattern, BYTE byTimeSet, USHORT uCMD, USHORT usParallelCMD, char* cOperand, char* linelabel, BOOL bCapture = TRUE);
	void GetVectLine(pPinPatternInfo& pPinPattern, BYTE& byTimeSet, USHORT& ucCMD, USHORT& usParallelCMD, const char*& cOperand, const char*& linelabel, BOOL& bCapture);
private:
	USHORT m_uPinCount;
	USHORT m_usCMDMSG;
	USHORT m_usParallelCMD;///<The parallel command code
	std::string m_strOperand;///<The operand
	std::string m_strLineLabel;///<The line label
	BOOL m_bCapture;
	BYTE m_byTimeSet;
	pPinPatternInfo m_pPinPattern;
};
/**
 * @class CVectorInfo
 * @brief The class of vector inforamtion
*/
class CVectorInfo
{
public:
	struct LINE_BLOCK
	{
		BOOL m_bBRAM;///<Whether the vector in BRAM
		int m_nLineCount;///<The line count of current block
		LINE_BLOCK()
		{
			m_bBRAM = TRUE;
			m_nLineCount = 0;
		}
	};
	/**
	 * @struct OFFSET_LINE_INFO
	 * @brief The information of line number offset to the BRAM start line
	*/
	struct OFFSET_LINE_INFO
	{
		BOOL m_bBRAM;///<Whether the line is in BRAM
		int m_nOffsetLineNo;///<The offset line number, if the line number is in BRAM, the line number is real line number, otherwise the line number is offset to first DRAM line after BRAM start line
		OFFSET_LINE_INFO()
		{
			m_bBRAM = TRUE;
			m_nOffsetLineNo = -1;
		}
	};
	struct LINE_INFO
	{
		int m_nTotalLineNo;///<The line number
		BOOL m_bBRAM;///<Whether the line is in BRAM
		int m_nLineNo;///<The line number or offset line number
	};
public:
	/**
	 * @brief Constructor
	*/
	CVectorInfo();
	/**
	 * @brief Destructor
	*/
	~CVectorInfo();
	/**
	 * @brief Open vector file
	 * @param[in] lpszFileName The full path of the vector file
	 * @param[out] mapPin The pin information in vector file, the key is pin name and value is pin information
	 * @param[out] mapTimeset The timeset information in vector file, the key is timeset name and value is timeset information
	 * @return Execute result
	 * - 0 Open file successfully
	 * - -1 The point of file name is nullptr
	 * - -2 The vector file is not existed
	 * - -3 Load vector editor module fail
	 * - -4 The vector format is wrong
	 * - -5 Allocate memory fail
	*/
	int OpenFile(const char* lpszFileName, std::map<std::string, CPin*>& mapPin, std::map<BYTE, CTimeset*>& mapTimeset);
	/**
	* @brief Read the vector line from vector file.
	* @param[in] nStartLine The line number be read start from.
	* @param[I/O] nReadLineCount The number to be read and the count read successfully.
	* @param[out] bSaveDRAM Whether vector line read is saved in BRAM.
	* @param[out] bLastVectorCurMemory Whether the last line current read is the last line of current type of memory.
	* @return Execute result 
	* - 0 Read the vector line successfully
	* - -1 The vector is not the right vector file
	* - -2 Must open file before
	*/
	int ReadLine(int nStartLine, int& nReadLineCount, BOOL& ucSaveBRAM, BOOL* bLastVectorCurMemory);
	/**
	* @brief Get the information of the vector line.
	* @param[out] pPinPattern The all pin's pattern of the vector line.
	* @param[out] uCMDMSG The command code of the pattern line.
	* @param[out] cOperand The operand of the pattern line.
	* @param[out] cLineLabel The label of the pattern line.
	* @param[out] byTimeSet The time set of the pattern line.
	* @param[out] parallelIns The information of the parallel instruction.
	* @param[out] nParallenInsCount The count of the parallel instruction.
	* @return Execute result
	* - 0 Get the information successfully
	* - -1 The vector line is not read.
	*/
	int GetReadLine(int nLineIndex, pPinPatternInfo& pPinPattern, BYTE& byTimeSet, const char*& lpszCMD, const char*& lpszParallelCMD, const char*& cOperand, const char*& linelabel, BOOL& bCapture);
	/**
	 * @brief Close vector file
	*/
	void CloseFile();
	/**
	* @brief Get the count of the vector line in BRAM.
	* @return No return.
	*/
	int GetBRAMLineCount();
	/**
	* @brief Get the count of the vector line in DRAM.
	* @return No return.
	*/
	int GetDRAMLineCount();
	/**
	 * @brief Get whether the vector editor is in debug mode
	 * @return
	 * - TRUE Debug mode
	 * - FALSE No debug mode
	*/
	BOOL IsDebugMode();
	/**
	* @brief Get the count of DRAM block.
	* @param[in] nBRAMStartLine The start line of BRAM line
	* @param[in] nBRAMStopLine The stop line of BRAM line
	* @return The DRAM block count between BRAM start line and stop line.
	*/
	int GetDRAMBlockCount(UINT uBRAMStartLine = 0, UINT nBRAMStopLine = -1);
	/**
	* @brief Get the block message of the vector in BRAM.
	* @param[in] nBlockIndex The index of the block.
	* @param[out] uStartLineNo The start line number of current block.
	* @param[out] uGlobalLineNo The line number of the start line in global.
	* @param[out] uBlockLineCount The count of the line in current block.
	* @return 
	* - TRUE Get the block information successfully
	* - FALSE No this block.
	*/
	BOOL GetBRAMBlock(int nBlockIndex, UINT& uStartLineNo, UINT& uGlobalLineNo, UINT& uBlockLineCount);
	/**
	* @brief Get the count of the label.
	* @return The count of the label.
	*/
	int GetLabelCount();
	/**
	* @brief Get the block message of the vector in DRAM.
	* @param[in] nBlockIndex The index of the block.
	* @param[out] uStartLineNo The start line number of current block.
	* @param[out] uGlobalLineNo The line number of the start line in global.
	* @param[out] uBlockLineCount The count of the line in current block.
	* @return 
	* - TRUE Get the block information successfully
	* - FALSE No this block.
	*/
	BOOL GetDRAMBlock(int nBlockIndex, UINT& uStartLineNo, UINT& uGlobalLineNo, UINT& uBlockLineCount);
	/**
	* @brief Add the label in the vector.
	* @param[in] nLabelIndex The index of the label.
	* @param[out] lpszLabelName The name of the label.
	* @param[in] nArrayLength The length of the array cLabelName.
	* @return 0:Get the label successfully* - -1:The index is bigger than sum of the label.
	*/
	int GetLabelNameWithLabelIndex(UINT uLabelIndex, std::string& strLabelName);
	/**
	* @brief Get the line number of the label.
	* @param[in] lpszLabel The label name.
	* @param[in] bGlobalLine Whether get the global line number.
	* @return The label line number
	* - >=0 The label line number
	* - -1 The label is nullptr
	* - -2 The label is not defined in vector.
	*/
	int GetLabelLineNo(const char* lpszLabel, BOOL bGlobalLine = FALSE);
	/**
	* @brief Get the line number in global.
	* @param[in] uRAMLineNo The line number of RAM
	* @param[in] bBRAMLine Whether the line is in BRAM
	* @return The number of the line in global
	* - >=0 The number of the line in global
	* - -1 Can't find the line number
	*/
	int GetGlobalLineNo(UINT uRAMLineNo, BOOL bBRAMLine = TRUE);
	/**
	 * @brief Get the global line number through the DRAM line number offset to the first DRAM line of latest ran
	 * @param[in] nStartBRAMLine The BRAM line number of the start line
	 * @param[in] nStopBRAMLine The BRAM line number of the start line
	 * @param[in] nDRAMOffsetLine the DRAM line number offset to the first DRAM line of latest ran
	 * @return The global line number of the DRAM offset line
	 * >=0 The global lin enumber of the DRAM offset line
	 * - -1 The vector is not loaded
	 * - -2 The line number is over range
	*/
	int GetDRAMOffsetGlobalLineNo(int nStartBRAMLine, int nStopBRAMLine, int nDRAMOffsetLine);
	/**
	 * @brief Get the line number
	 * @param[in] uGlobalLineNo The global line number
	 * @param[out] uLineNo The line number in its' type of memory
	 * @return The memory type
	 * - 1 The line in BRAM
	 * - 0 The line in DRAM
	 * - -1 The global line number is over range
	*/
	int GetLineNo(UINT uGlobalLineNo, UINT& uLineNo);
	/**
	 * @brief Get the label name of the line
	 * @param[in] nLabelLineNo The label line number
	 * @param[out] strLabelName The label name
	 * @return Execute result
	 * - 0 Find label name
	 * - -1 The line number is not label line
	*/
	int GetLabelName(int nLabelLineNo, std::string& strLabelName);
	/**
	* @brief Get the run start line of DRAM if current section vector have some vector saved in DRAM.
	* @param[in] ulStartLine The start line of running vector
	* @param[in] ulEndLine The end line of running vector
	* @param[out] ulDRAMStartLine The start line of DRAM in this running vector
	* @param[out] nDRAMBlockIndex The index of the DRAM block
	* @return Whether have DRAM line between start line and stop line
	* - TRUE Some of the vector line of this running vector had been saved in DRAM
	* - FALSE No DRAM vector is saved in this running vector
	*/
	BOOL GetDRAMRunStartLine(UINT uStartLine, UINT uStopLine, UINT& uDRAMStartLine, int& nDRAMBlockIndex);
	/**
	* @brief get the information of vector save.
	* @param[in] nStartGlobalLine The global line of start line
	* @param[in] nStopGlobalLine The global line of stop line
	* @param[out] pnStartLine The start line of BRAM and DRAM, [0] is BRAM,[1] is DRAM
	* @param[out] pnLineCount The line count of BRAM and DRAM, [0] is BRAM,[1] is DRAM
	* @param[out] mapLineInfo The line information
	* @return  Execute result
	* - >=0 The global start line number
	* - -1 Vector not loaded
	* - -2 Line number over range
	* - -3 puStartLine or puLineCount is nullptr
	*/
	int GetLineInfo(int nStartLine, int nStopLine, int* pnStartLine, int* pnLineCount, std::map<int, LINE_BLOCK>& mapLineInfo, BOOL bBRAMLine = TRUE);
	/**
	 * @brief Combine line of BRAM and DRAM
	 * @param[in] nBRAMStartLine The BRAM line number of start line
	 * @param[in] nBRAMStopLine The BRAM line number of stop line
	 * @param[in] vecBRAMLine The number of BRAM
	 * @param[in] vecDRAMOffsetLine The DRAM line number offset to the first DRAM line between start and stop
	 * @param[out] vecOffsetLine The line number offset to the start line
	 * @return Execute result
	 * - 0 Combine line successfully
	* - -1 Vector not loaded
	* - -2 Line number over range
	*/
	int CombineLine(int nBRAMStartLine, int nBRAMStopLine, const std::vector<int>& vecBRAMLine, const std::vector<int>& vecDRAMOffsetLine, std::vector<int>& vecOffsetLine);
	/**
	 * @brief Combine line in BRAM and DRAM
	 * @param[in] nBRAMStartLine  The BRAM line number of start line
	 * @param[in] nBRAMStopLine The BRAM line number of stop line
	 * @param[in] vecBRAMLine The line number of BRAM
	 * @param[in] vecDRAMOffsetLine The DRAM line number offset to the first DRAM line between start and stop
	 * @param[out] mapOffsetLine The offset line number to start line
	 * @return Execute result
	 * - 0 Combine line successfully
	* - -1 Vector not loaded
	* - -2 Line number over range
	*/
	int CombineLine(int nBRAMStartLine, int nBRAMStopLine, const std::vector<int>& vecBRAMLine, const std::vector<int>& vecDRAMOffsetLine, std::vector<LINE_INFO>& vecLineNo);
	/**
	 * @brief Combine the line in BRAM and DRAM in order
	 * @param[in] nBRAMStartLine The BRAM line of start line
	 * @param[in] nBRAMStopLine The BRAM line of stop line
	 * @param[out] mapBRAMLine The BRAM line information and output the combined line
	 * @param[in] mapDRAMLine The DRAM line data
	 * @return Execute result
	* - 0 Combine line information successfully
	* - -1 Vector not loaded
	* - -2 Line number over range
	*/
	int CombineLine(int nBRAMStartLine, int nBRAMStopLine, std::vector<LINE_DATA>& vecBRAMLine, const std::vector<LINE_DATA>& vecDRAMLine);
	/**
	 * @brief Split the order data to BRAM and DRAM data
	 * @param[in] mapLineInfo The line information
	 * @param[in] pbyData The data in order
	 * @param[in] nLineCount The data line count of the data
	 * @param[out] pbyBRAMData The data of BRAM
	 * @param[in] nBRAMDataSize The size of BRAM data buff
	 * @param[out] pbyDRAMData The data of DRAM
	 * @param[in] nDRAMDataSize The size of DRAM data buff
	 * @return Execute result
	 * - 0 Split data successfully
	 * - -1 The line information is blank
	 * - -2 The point order data is nullptr
	 * - -3 The point of BRAM data is nullptr
	 * - -4 The point of DRAM data is nullptr
	 * - -5 The data size of BRAM is not enough
	 * - -6 The data buff size of DRAM is not enough
	*/
	int SplitData(const std::map<int, LINE_BLOCK>& mapLineInfo, const BYTE* pbyData, int nLineCount, BYTE* pbyBRAMData, int nBRAMDataSize, BYTE* pbyDRAMData, int nDRAMDataSize);
	/**
	* @brief Add the label in the vector.
	* @param[in] strLabelName The name of the label.
	* @param[in] uLabelLineNo The line number of the label.
	*/
	void AddLabel(const std::string& strLabelName, UINT uLabelLineNo);
	/**
	* @brief Add the block message of the vector in BRAM.
	* @param[in] uStartLineNo The start line number of current block.
	* @param[in] uGlobalLineNo The line number of the start line in global.
	* @param[in] uBlockLineCount The count of the line in current block.
	* @return No return.
	*/
	void AddBRAMBlock(UINT uStartLineNo, UINT uGlobalLineNo, UINT uBlockLineCount);
	/**
	* @brief Add the block message of the vector in DRAM.
	* @param[in] uStartLineNo The start line number of current block.
	* @param[in] uGlobalLineNo The line number of the start line in global.
	* @param[in] uBlockLineCount The count of the line in current block.
	* @return No return.
	*/
	void AddDRAMBlock(UINT uStartLineNo, UINT uGlobalLineNo, UINT uBlockLineCount);	
	/**
	*Set the line count of vector file.
	* @param[in] uBRAMLineCount The line count in BRAM.
	* @param[in] uDRAMLineCount The line count in DRAM.
	* @return 0:Successfully; -1:Modifcation is not allowed in new version vector file.
	*/
	int SetLineCount(UINT uBRAMLineCount, UINT uDRAMLineCount);	
	/**
	* @brief Get the count of the instruction with label in vector.
	* @return The count of the instruction with label.
	*/
	int GetInsWithLabelCount();
	/**
	* @brief Get the line number and operand of the specific instruction with label used in vector.
	* @param[in] nInsIndex The number index of the instruction with label.
	* @param[out] nInsLabelNo The line number of the parallel instruction's label.
	* @param[out] byTimeSet The time set of the parallel instruction label.
	* @return The line number of the instruction
	* - >0 the line number of the instruction used in
	* - -1 The index over the count of the instruction
	* - -2 The label of the instruction is not defined
	*/
	int GetInsWithLabelLineNo(int nInsIndex, int& nInsLabelNo);
	/**
	 * @brief Get the vector file name
	 * @return The vector file name
	*/
	const char* GetVectorFileName();
	/**
	 * @brief Get the vector file ID
	 * @param[out] lpszID The ID buff
	 * @param[in] nIDLengh The buff size
	*/
	void GetID(char* lpszID, int nIDLengh);
	/**
	 * @brief Get the vector save mark
	 * @param[out] lpszSaveMark The save mark
	 * @param[in] nMarkLength The mark length
	*/
	void GetSaveMark(char* lpszSaveMark, int nMarkLength);
	/**
	 * @brief Reset vector information
	*/
	void Reset();
private:
	/**
	 * @brief Check whether the vector file is right
	 * @param[in] lpszFileName The vector file name
	 * @return Whether the file is right
	 * - 0 The file is right
	 * - -1 The point of the file is nullptr
	 * - -2 The file is not existed
	 * - -3 Can't load VectorEditor module
	 * - -4 The format of file is wrong
	 */
	int CheckFile(const char* lpszFileName);
	/**
	 * @brief Get the command string from code
	 * @param[in] usCMDCode The command code
	 * @return The command name
	 * - !=0 The command name
	 * - nullptr The command code is not supported
	*/
	const char* GetCommand(USHORT usCMDCode);
	/**
	 * @brief Get the parallel string from code
	 * @param[in] usParallelCMDCode The parallel command code
	 * @return The command name
	 * - !=0 The parallal command code
	 * - nullptr The command code is not supported
	*/
	const char* GetParallelCommand(USHORT usParallelCMDCode);
	/**
	 * @brief Copy the bit to destination data
	 * @param[out] pbyDest The destination data
	 * @param[in] nDestStartBit The start bit of the destination
	 * @param[in] pbySource The source data
	 * @param[in] nSourceBitCount The bit count of the source data
	 * @param[in] bCopyStartBit The start bit of the source data which will copy from
	 * @param[in] nCopyBitCount The count of the copy bit
	 * @return Execute result
	 * - 0 Copy data successfully
	 * - -1 The point of destination data or source data is nullptr
	 * - -2 The bit count is over range
	 */
	inline int CopyDataBit(BYTE* pbyDest, int nDestStartBit, const BYTE* pbySource, int nSourceBitCount, int nCopyStartBit, int nCopyBitCount);
	/**
	 * @brief Get the byte and bit index
	 * @param[in] nCurBit The bit index
	 * @param[out] nByteBit The bit index in byte
	 * @param[in] Whether the data is full byte
	 * @param[in] nBitCount The bit count of the data
	 * @return The byte index to which the bit belongs
	 * - 
	 */
	inline int GetByteBitIndex(int nCurBit, int& nByteBit, BOOL bFullByte = TRUE, int nBitCount = -1);
	/**
	 * @brief Shift data is the data is not full byte
	 * @param[out] pbyData The data shifted
	 * @param[in] uBitCount The bit count of the data 
	 * @return Execute result
	 * - 0 Shift data successfully
	 * - -1 The point of data is nullptr
	 */
	inline int ShiftLastByteData(BYTE* pbyData, UINT uBitCount);
private:
	typedef struct BLOCK_MSG
	{
		BLOCK_MSG()
		{
			m_nStartLineNo = 0;
			m_nGlobalLineNo = 0;
			m_nLineCount = 0;
		}
		int m_nStartLineNo;//<The start line of current block in current type memory.
		int m_nGlobalLineNo;//<The line number in the whole vector file.
		int m_nLineCount;//<The line number of current block.
	}BLOCK_MSG;

	typedef struct INS_LABEL
	{
		INS_LABEL()
		{
			m_nLineNo = 0;
			m_nInsLineNo = -1;
		}
		int m_nLineNo;//<The line number of the instruction with label.
		int m_nInsLineNo;//<The line number of the label used by instruction.
		std::string m_strInsLabel;//<The name of the label used by instruction.
	}INS_LABEL;

	HMODULE m_hDll;///<The module of vector editor
	std::string m_strFileName;///<The file name of vector file
	int m_nFileVersion;///<The file version of vector
	IACVInstance* m_pIns;///<The point of vector editor instance
	IACVProject* m_pVector;///<The point of project of vector editor
	IACVFailTag* m_pFailTag;///<The point of fail tag
	int m_nBRAMLineCount;///<The sum line count in BRAM of vector file
	int m_nDRAMLineCount;///<The sum line count in DRAM of vector file
	std::map<std::string, int> m_mapLabel;//<The information of the label in vector, key is label name,value is the line number.
	std::map<int, BLOCK_MSG> m_mapBRAMBlock;//<The information of the BRAM block, key is the index of the block, value is detail message of the block.
	std::map<int, BLOCK_MSG> m_mapDRAMBlock;//<The information of the DRAM block, key is the index of the block, value is detail message of the block.
	std::map<USHORT, CVectorLine*> m_mapVectorLine;//<The detail information of the vector line, key is the index of the read line, value is the information of vector line.
	std::vector<INS_LABEL> m_vecInsInVector;//<The information of the instruction with label.
	BLOCK_MSG m_struBlockMsg;//<The block message of last read.
	int m_nDRAMBlockCount;//<The count of the DRAM block.
	USHORT m_usPinCount;///<The pin count
	BOOL m_bFileReadError;///<Whether read file error
	int m_nCurBRAMIndex; //<The current line number of current vector line
	char m_lpszID[256];///<The vector file ID
	char m_lpszSaveMark[256];///<The vector save mark
};

