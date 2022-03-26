#pragma once
/**
 * @file ClassDistribution.h
 * @brief This head file is for the class which distribute the class
 * @author Guangyun Wang
 * @date 2021/07/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Co., Ltd.
*/
#include <windows.h>
#include <vector>
#include <map>
/**
 * @class CClassDistribution
 * @brief The singleton class for class point distribution
*/

template<typename T> class CClassDistribution
{
public:
	/**
	 * @brief Get the instance of the point of class
	 * @return The point of the class
	*/
	static CClassDistribution* Instance();
	/**
	 * @brief Destructor
	*/
	~CClassDistribution();
	/**
	 * @brief Set the current instance ID
	 * @param[in] nInstanceID The instance ID
	*/
	void SetInstanceID(int nInstanceID);
	/**
	 * @brief Get class for the instance
	 * @param[out] pnInstanceID The instance ID of current instance
	 * @param[in] nInstanceID The instance ID
	 * @return The point of the class for the instance
	*/
	T* GetClass(int* pnInstanceID = nullptr, int nInstanceID = -1);
	/**
	 * @brief Initialize instance
	 * @param[in] nInstanceID The instance ID
	 * @param[in] bDelte Whether delete DCM instance
	 * @return The point of the DCM class
	*/
	T* Initialize(int nInstanceID, BOOL bDelete = FALSE);
	/**
	 * @brief Get the instance ID valid
	 * @param[out] vecInstancID The instance ID valid
	*/
	void GetInstanceValid(std::vector<int>& vecInstancID);
private:
	/**
	 * @brief Constructor
	*/
	CClassDistribution();
	/**
	 * @brief Distribute class
	 * @param[in] nInstanceID The instance ID
	 * @param[out] pnInstanceID The instance ID of current instance
	 * @param[in] bDelete Whether delete
	 * @return The point of class
	*/
	T* DistributeClass(int nInstanceID, int* pnInstanceID, BOOL bDelete);
private:
	struct INSTANCE_INFO
	{
		int m_nInstanceID;///<The instance ID
		T* m_pClass;///<The point of the class
		INSTANCE_INFO()
		{
			m_nInstanceID = 0;
			m_pClass = nullptr;
		}
	};
	CRITICAL_SECTION m_criAuthority;///<The critical section for distribution thread safety
	std::map<int, T*> m_mapClass;///<The point of class for each instance, the key is instance ID and value is point of class
	std::map<DWORD, INSTANCE_INFO> m_mapClassAuthorized;///<The class has been authorized for instance, just record the point of DCM, key is thread ID and value is point of the class distributed
	int m_nAuthorizedInstanceID;///<The ID of current authorized instance
};