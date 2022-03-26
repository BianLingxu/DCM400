#pragma once
#include <windows.h>

/**
 * @brief Get the slot number of the board
 * @param[in] byBoardNo The board number
 * @return The slot number of the board inserted
 * - >0 The slot number of the board inserted
 * - -1 The board number is not existed
**/
int APIENTRY DCM400_GetSlotNo(BYTE byBoardNo);