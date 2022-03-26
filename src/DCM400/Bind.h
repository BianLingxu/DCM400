#pragma once
/**
 * @file Bind.h
 * @brief The bind information of all board
 * @author 2020/07/20
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>
#include <set>
/**
 * @class CBindInfo
 * @brief Bind information of DCM, the class is singleton
*/
class CBindInfo
{
public:
	/**
	 * @brief Get the instance of bind information
	 * @return The point of bind information
	*/
	static CBindInfo* Instance();
	/**
	 * @brief Destructor
	*/
	~CBindInfo();
	/**
	* @brief Clear bind
	*/
	void ClearBind();
	/**
	 * @brief Whether bind board
	 * @return Whether bind board
	 * - TRUE Bind board
	 * - FALSE Not bind
	*/
	BOOL IsBind();
	/**
	 * @brief Get the bind information
	 * @param[out] setSlot The board be binded
	 * @param[out] byController The controller be binded
	 * @return The target slot
	*/
	BYTE GetBindInfo(std::set<BYTE>& setSlot, std::set<BYTE>& setController);
	/**
	 * @brief Bind board
	 * @param[in] setSlot The board need to bind
	 * @param[in] setController The controller need to be binded
	 * @param[in] byTargetSlot The target slot
	 * @return Execute result
	* - 0 Bind successfully
	* - -1 The follow slot is invalid
	* - -2 The bind controller is blank
	* - -3 The target slot is invalid
	* - -4 Binded before, must clear bind before bind
	*/
	int Bind(const std::set<BYTE>& setSlot, const std::set<BYTE>& setController, BYTE byTargetSlot);
	/**
	 * @brief Get the controller count in bind
	 * @return The controller count
	*/
	int GetBindControllerCount();
private:
	/**
	 * @brief Constructor
	*/
	CBindInfo();
	/**
	 * @brief Write bind information to board
	 * @param[in] byFollowSlot The follow board
	 * @param[in] byBindData The bind data
	 * @param[in] byTargetSlot The slot number of follow
	*/
	inline void Bind(BYTE byFollowSlot, BYTE byBindData, BYTE byTargetSlot);
private:
	BYTE m_byTargetSlot;///<The target slot
	std::set<BYTE> m_setFollowSlot;///<The slot be binded to target slot
	std::set<BYTE> m_setBindController;///<The binded controller
};


