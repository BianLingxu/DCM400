#pragma once
/**
 * @file CMDCode.h
 * @brief The head file for command management
*/
#include <string>
#include <map>
#include <set>
/**
 * @brief The class for command code management
*/
class CCMDCode
{
public:
	/**
	 * @brief Constructor
	*/
	CCMDCode();
	/**
	 * @brief Get the command code
	 * @param lpszCMD The command name
	 * @return The command code
	 * - >=0 The command code
	 * - -1 The command point is nullptr
	 * - -2 The command is not supported
	*/
	int GetCMDCode(const char* lpszCMD);
	/**
	 * @brief Get the command name
	 * @param[in] nCode The command code
	 * @return The command name
	 * - != nullptr The command name
	 * - nullptr The command code is not existed
	*/
	const char* GetCMDName(int nCode);
	/**
	 * @brief Get the maximum operand for the command
	 * @param nCode The command code
	 * @return The maximum operand
	 * - >=0 The maximum operand
	 * - -1 TThe command code is not existed
	 * - -2 The command is without operand
	*/
	int GetCMDOperandRange(int nCode, int& nMinOperand);
	/**
	 * @brief Get the code of conditional command
	 * @param[in] setCode The code of conditional command
	*/	
	void GetConditionalCode(std::set<int>& setCode) const;
	/**
	 * @brief Get the general command code
	 * @param[in] setCode The general command code
	*/
	void GetGeneralCMDCode(std::set<int>& setCode) const;
	/**
	 * @brief Get the command code of specified command
	 * @param[out] setSpecified The specified command
	*/
	void GetSpecifiedCMDCode(std::set<int>& setSpecified) const;
	/**
	 * @brief Get the jump command code
	 * @param[out] setJumpCMD The jump command code
	*/
	void GetJumpCMDCode(std::set<int>& setJumpCMD) const;
private:
	struct DETAIL
	{
		std::string m_strName;///<The command name
		/**
		 * @brief The maximum operand
		 * - >=0 The maximum operand
		 * - -1 The command without operand
		*/
		int m_nMaxOperand;///<The maximum operand
		int m_nMinOperand;///<The minmum operand
		DETAIL()
		{
			m_nMaxOperand = -1;
		}
	};
	std::map<int, DETAIL> m_mapCMD;///<The command support, the key is code and value is detail information
	std::map<std::string, int> m_mapCode;///<The command code, key is name and value is its code
	std::set<int> m_setConditionalCode;///<The code of conditional command
	std::set<int> m_setJUMPCode;///<The command code for jumpping command
	std::set<int> m_setSpecified;///<The command code for specified command
};

