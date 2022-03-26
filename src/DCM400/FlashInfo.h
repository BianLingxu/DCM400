#pragma once
/**
 * @file FlashInfo.h
 * @brief The information of the flash in DCM board
 * @author Guangyun Wang
 * @date 2020/06/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controler Co., Ltd.
 */

 //The byte size of each page
#define FLASH_PAGE_SIZE			256			//Bytes
///<The page count in each sector
#define SECTOR_PAGE_COUNT       256
//The size of flash's per sector.
#define FLASH_SECTOR_SIZE		SECTOR_PAGE_COUNT * FLASH_PAGE_SIZE		//Bytes
///<The sector count in board
#define SECTOR_COUNT			64
///<The flash ID of DCM
#define FLASH_ID 0x00202016

 ///<The sector which save the initialization data
#define DELAY_DATA_SECTOR	0
///<The backup sector in which the delay date be saved
#define BACKUP_DELAY_DATA_SECTOR 12
///<The pages occupation of per SE8212's delay data
#define DELAY_DATA_PAGE_PER_CONTROL 5


//The sector which save the hard information.
#define HARD_INFO_SECTOR		1
//The start page no of the hard information.
#define HARD_INFO_PAGE_START	0

///<The start sector NO which save the calibration data
#define CAL_DATA_SECTOR_START	2
///<The page number of the calibration data occupied per channel
#define CAL_DATA_PAGE_PER_CH	16

//The sector in which the calibration information saved
#define CAL_INFO_SECTOR_START	6
//The page number of the calibration information per channel
#define CAL_INFO_PAGE_PER_CH	16

///<The sector which save the controller count
#define CONTROL_COUNT_SAVE_SECTOR 10
///<The backup sector which save the controller count
#define BACKUP_CONTROL_COUNT_SECTOR 11
///<The page index in which the controller count be saved
#define	CONTROL_COUNT_PAGE 0