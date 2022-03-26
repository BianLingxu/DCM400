#pragma once
/**
 * @file Pattern.h
 * @brief Include the function class of CPattern
 * @author Guangyun Wang
 * @date 2020/05/26
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "StdAfx.h"
#include "HardwareFunction.h"
/**
 * @class CPattern
 * @brief Convert pattern to code and load to board
*/
class CPattern
{
public:
	/**
	 * @enum PARALLEL_INST
	 * @brief The parallel instruction
	*/
	enum class PARALLEL_INST
	{
		FAIL_ON = 0,///<Open saving fail selected
		FAIL_OFF = 1,///<Close saving fail selected

	};
	/**
	 * @enum PATTERN_SIGN
	 * @brief Pattern sign
	*/
	enum class PATTERN_SIGN
	{
		PAT_0 = '0',///<Drive low
		PAT_1 = '1',///<Drive high
		PAT_H = 'H',///<Expect High
		PAT_L = 'L',///<Expect Low
		PAT_X = 'X',///<Not compare
		PAT_M = 'M',///<Expect lower than High or higher than Low
		PAT_V = 'V',///<Expect high than high or lower than low
		PAT_S = 'S',///<Stay the status of latest pattern
	};
	/**
	 * @enum CODE_TYPE
	 * @brief Code type
	*/
	enum class CODE_TYPE
	{
		FM = 0,///<FM
		MM,///<MM
		IOM,///<IOM
	};
	/**
	 * @brief Constructor
	 * @param[in] HardwareFunction The class of hardware function
	 * @param[in] pAlaram The class point of the alarm
	*/
	CPattern(CHardwareFunction& HardwareFunction, CDriverAlarm* pAlarm = nullptr);
	/**
	 * @brief Destructor
	*/
	~CPattern();
	/**
	 * @brief Set the default pattern
	 * @param[in] usCMD The default command
	 * @param[in] usOperand The default operand
	*/
	void SetDefaultControlData(USHORT usCMD, USHORT usOperand);
	/**
	 * @brief Set the default pattern sign
	 * @param[in] DefaultPattern The pattern sign
	 * @return Execute result
	 * - 0 Set the default pattern successfully
	 * - -1 The pattern is not supported
	*/
	int SetDefaultPattern(PATTERN_SIGN DefaultPattern);
	/**
	 * @brief Get the added pattern count;
	 * @return The pattern count
	*/
	UINT GetPatternCount();
	/**
	 * @brief Add channel pattern
	 * @param[in] usChannel The channel number of the controller
	 * @param[in] bBRAM The memory which to save the pattern
	 * @param[in] uPatternLineIndex The line index in the memory
	 * @param[in] cPattern The pattern sign
	 * @param[in] byTimeset The timeset of current pattern line
	 * @param[in] lpszCMD The command of pattern
	 * @param[in] lpszParallelCMD The parallel command
	 * @param[in] ulOperand The operand
	 * @param[in] bCapture Whether the line is capture
	 * @param[in] bSwitch Whether the pattern line is the last line in current memory,next line will switch to other memory
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The channel is over range
	 * - -2 The pattern index is over range
	 * - -3 The pattern sign is not supported
	 * - -4 The timeset is over range
	 * - -5 The point of command is nullptr
	 * - -6 The command is not supported
	 * - -7 The operand is over range
	*/
	int AddChannelPattern(USHORT usChannel, BOOL bBRAM, UINT uPatternLineIndex, char cPattern, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, ULONG ulOperand, BOOL bCapture, BOOL bSwitch);
	/**
	 * @brief Add the pattern of all channel
	 * @param[in] uPatternLineIndex The line index in the memory
	 * @param[in] bBRAM The memory which to save the pattern
	 * @param[in] lpszPattern The pattern sign
	 * @param[in] byTimeset The timeset of current pattern line
	 * @param[in] lpszCMD The command of pattern
	 * @param[in] lpszParallelCMD The parallel command
	 * @param[in] usOperand The operand
	 * @param[in] bCapture Whether the line is capture
	 * @param[in] bSwitch Whether the pattern line is the last line in current memory,next line will switch to other memory
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The pattern index is over range
	 * - -2 The pattern have added before
	 * - -3 The point of pattern is nullptr
	 * - -4 The timeset is over range
	 * - -5 The point of command is nullptr
	 * - -6 The command is not supported
	 * - -7 The operand is over range
	 * - -8 The pattern sign is not supported
	*/
	int AddPattern(UINT uPatternLineIndex,BOOL bBRAM, const char* lpszPattern, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD, USHORT usOperand, BOOL bCapture, BOOL bSwitch);
	/**
	 * @brief Add pattern of all channel
	 * @param[in] uPatternLineIndex The pattern line index saved in memory type
	 * @param[in] bBRAM The memory type
	 * @param[in] lpszPattern The pattern of all channel
	 * @param[in] usCMD The command of pattern
	 * @param[in] usOperand The operand
	 * @return Execute result
	 * - 0 Add pattern successfully
	 * - -1 The pattern line is over range
	 * - -2 The line has been added before
	 * - -3 The point of pattern is nullptr
	 * - -4 The pattern sign is not supported
	*/
	int AddPattern(UINT uPatternLineIndex, BOOL bBRAM, const char* lpszPattern, USHORT usCMD, USHORT usOperand);
	/**
	 * @brief Load pattern
	 * @return Execute result
	 * - 0 Load pattern successfully
	 * - -1 Allocate memory fail
	*/
	int Load();
	/**
	 * @brief Get the pattern in memory
	 * @param[in] bBRAM Whether the pattern saved in BRAM
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line count
	 * @param[out] lpszPattern The pattern read
	 * @return Execute result
	 * - 0 Read pattern successfully
	 * - -1 The start line is over range
	 * - -2 The line count is over range
	 * - -3 The point of pattern is nullptr
	 * - -4 Allocate memory fail
	*/
	int ReadPattern(BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17]);
	/**
	 * @brief Get the pattern code of pattern loaded
	 * @param[in] bBRAM Whether the pattern is in BRAM
	 * @param[in] CodeType The pattern code type
	 * @param[out] pusPattern The pattern code of each line
	 * @param[in] uElementCount The element count of array
	 * @return The line count
	 * - >=0 The pattern line count
	 * - -1 The code type is not supported
	 * - -2 The point of array is nullptr or the element count is 0
	*/
	int GetPatternCode(BOOL bBRAM, CODE_TYPE CodeType, USHORT* pusPattern, UINT uElementCount);
	/**
	 * @brief Clear the memory
	*/
	void Reset();
	/**
	* @brief Set the operand of the vector line
	* @param[in] uBRAMLineNo The line number in BRAM
	* @param[in] usOperand The operand of the value
	* @param[in] bCheckRange Whether check the range of the operand
	* @return Execute result
	* - 0 Set operand successfully
	* - -1 The line number is over range
	* - -2 The operand is over range
	*/
	int SetOperand(UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange);
	/**
	 * @brief Get the command
	 * @param[in] bBRAM Whether the command in BRAM
	 * @param[in] byTimeset The timeset of current pattern
	 * @param[in] bCapture Whether use capture
	 * @param[in] bSwitch Whether switch memory
	 * @return The command code
	*/
	USHORT GetCommand(BOOL bBRAM, BYTE byTimeset, BOOL bCapture, BOOL bSwitch);
	/**
	 * @brief The line number is over range
	 * @param[in] uBRAMLineNo The line number in BRAM
	 * @param[in] lpszInstruction The instruction
	 * @param[in] usOperand The operand
	 * @return Execute result
	 * - 0 Set instruction successfully
	 * - -1 The line number is over range
	 * - -2 The instruction is nullptr
	 * - -3 The instruction is not supported
	 * - -4 The operand is over range
	*/
	int SetInstruction(UINT uBRAMLineNo, const char* lpszInstruction, USHORT usOperand);
	/**
	 * @brief Get the instruction of the line
	 * @param[in] uBRAMLineNo The line number of BRAM
	 * @param[out] lpszInstruction The buff for save instruction
	 * @param[in] nBuffSize The buff size
	 * @return Execute result
	 * - 0 Get the instruction successfully
	 * - -1 The line number is over range
	 * - -2 The point of the buff is nullptr
	 * - -3 The buff is too small
	*/
	int GetInstruction(UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize);
	/**
	 * @brief Get the operand of the line
	 * @param[in] uBRAMLineNo The BRAM line number
	 * @return The operand 
	 * - >=0 The operand
	 * - -1 The line number is over range
	*/
	int GetOperand(UINT uBRAMLineNo);
	/**
	 * @brief Set the parallel instruction
	 * @param[in] uRAMLineNo The RAM line number of BRAM or DRAM
	 * @param[in] lpszInstruction The parallel instruction
	 * @param[in] bBRAM Whether the line number is belongs to BRAM
	 * @return Execute result
	 * - 0 Set parallel instruction successfully
	 * - -1 The line number is over range
	 * - -2 The instruction is nullptr
	 * - -3 The instruction is not support
	*/
	int SetParallelInstruction(UINT uRAMLineNo, const char* lpszInstruction, BOOL bBRAM);
	/**
	 * @brief Get the instruction type
	 * @param[in] lpszInstruction The instruction sign
	 * @return The instruction type
	 * - 0 The normal single instruction
	 * - 1 The conditional instruction
	 * - 2 The parallel instruction
	 * - -1 The instruction is nullptr
	 * - -2 The instruction is not supported
	*/
	int GetInstructionType(const char* lpszInstruction);
private:
	/**
	 * @struct CONTROL_DATA
	 * @brief The Controller data of pattern
	*/
	struct CONTROL_DATA
	{
		USHORT m_usCMD;///<The command data
		USHORT m_usOperand;///<The operand data
		CONTROL_DATA()
		{
			m_usCMD = 0;
			m_usOperand = 0;
		}
	};
	/**
	 * @brief Convert pattern sign to code
	 * @param[in] PatternSign The pattern sign
	 * @return Pattern code
	 * - >=0 Pattern code
	 * - -1 The pattern sign is not supported
	*/
	int PatternSign2Code(PATTERN_SIGN PatternSign);
	/**
	 * @brief Convert code to pattern sign
	 * @param[in] byCode The pattern code
	 * @param[out] pcSign The pattern sign
	 * @return Code sign
	 * - >=0 The pattern sign
	 * - -1 The pattern code is not supported
	*/
	int Code2Pattern(BYTE byCode);
	/**
	 * @brief Set the value bit
	 * @param[in] Data The data
	 * @param[in] nBitIndex The bit index
	 * @param[in] bOne The bit value
	*/
	template <typename T>
	inline void SetValue(T& Data, int nBitIndex, bool bOne);
	/**
	 * @brief Get the Controller data
	 * @param[in] bBRAM Whether the pattern is saved in BRAM
	 * @param[in] byTimeset The timeset of pattern
	 * @param[in] lpszInstruction The command of pattern
	 * @param[in] ulOperand The operand
	 * @param[in] bCapture Whether the line is capture
	 * @param[in] lpszParallelCMD The parallel command of the vector
	 * @param[in] bSwitch Whether the pattern is the last pattern in current memory
	 * @param[out] ControlData The Controller data
	 * @return Execute result
	 * - 0 Get Controller data successfully
	 * - -1 The command is not supported
	 * - -2 The operand is over range
	 * - -3 The parallel instruction is not supported
	*/
	inline int GetControlData(BOOL bBRAM, BYTE byTimeset, const char* lpszInstruction, const char* lpszParallelCMD, ULONG ulOperand, BOOL bCapture, BOOL bSwitch, CONTROL_DATA& ControlData);
	/**
	 * @brief Convert command sign to code
	 * @param[in] lpszCMDSign The command sign
	 * @param[out] usCode The command code
	 * @param[out] pnInstructionType The point of the instruction type
	 * - 0 Normal single controller instruction
	 * - 1 Conditional instruction
	 * - -1 The instruction is not supported
	 * @return The maximum operand value
	 * - >=0 The maximum operand value
	 * - -1 The point of command sign is nullptr
	 * - -2 The command is not supported
	*/
	int Instruction2Code(const char* lpszCMDSign, USHORT& usCode, int* pnInstructionType = nullptr);
	/**
	 * @brief Convert parallel command sign to code
	 * @param[in] lpszCMDSign The parallel command sign
	 * @param[out] usCode The command code
	 * @return Execute result
	 * - 0 Convert successfully
	 * - -1 The comamnd sign is nullptr
	 * - -2 The command is not supported
	*/
	int ParallelInstruction2Code(const char* lpszCMDSign, USHORT& usCode);
	/**
	 * @brief Convert instruction code to name
	 * @param[in] usCode The instruction code
	 * @param[out] strInstruction The instruction name
	 * @return The maximum operand
	*/
	int Code2Instruction(USHORT usCode, std::string& strInstruction);
	/**
	 * @brief Add pattern of channel
	 * @param[in] usChannel The channel number of the controller
	 * @param[in] uPatternLineIndex The pattern line index saved in memory type
	 * @param[in] cPattern The pattern of all channel
	 * @param[in] bBRAM The memory type
	 * @param[in] ulCMD The command of pattern
	 * @param[in] ulOperand The operand
	 * @return Execute result
	 * - 0 Add pattern successfully
	 * - -1 The channel number is over range
	 * - -2 The pattern line is over range
	 * - -3 The pattern sign is not supported
	*/
	int AddChannelPattern(USHORT usChannel, UINT uPatternLineIndex, char cPattern, BOOL bBRAM, ULONG pulCMD, ULONG pulOperand);
	/**
	 * @brief Parse the command
	 * @param[in] usCMD The command
	 * @param[in] bBRAM Whether the command is in BRAM
	 * @param[out] pbyTimeset The timeset in command
	 * @param[out] pbCapture Whether capture in command
	 * @param[out] pbSwitch Whether switch line
	 * @param[out] strInstr The instruction in command
	 * @param[out] strParallelInstr The parallel instanction in command
	 * @return The maximum operand
	 * - >=0 The maximum operand
	 * - -1 The point is nullptr
	*/
	int ParseCMD(USHORT usCMD, BOOL bBRAM, BYTE* pbyTimeset, BOOL* pbCapture, BOOL* pbSwitch, std::string& strInstr, std::string& strParallelInstr);
private:
	/**
	 * @struct PATTERN_CODE
	 * @brief The pattern code
	*/
	struct PATTERN_CODE 
	{
		USHORT m_usFM;///<The FM value
		USHORT m_usMM;///<The MM value
		USHORT m_usIOM;///<The IO value
		PATTERN_CODE()
		{
			m_usFM = 0;
			m_usMM = 0;
			m_usIOM = 0;
		}
	};
	CDriverAlarm* m_pAlarm;///<The point of alarm class
	PATTERN_SIGN m_DefaultPatternSign;///<The default pattern sign
	CONTROL_DATA m_DefaultControlData;///<The default Controller data
	CHardwareFunction* m_pHardwareFunction;///<The class of CHardwareFunction
	std::map<UINT, PATTERN_CODE> m_mapBRAMLine;///<The pattern line in BRAM
	std::map<UINT, PATTERN_CODE> m_mapDRAMLine;///<The pattern line in DRAM
	std::map<UINT, CONTROL_DATA> m_mapBRAMControl;///<The Controller data in BRAM
	std::map<UINT, CONTROL_DATA> m_mapDRAMControl;///<The Controller data in DRAM
};

