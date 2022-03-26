#pragma once
/**
 * @file HardwareInfo.h
 * @brief The parameter of hardware
 * @detail This file provide the const parameter of hardware
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>
#define DCM_MAX_BOARD_NUM 4//The maximum number of DCM board inserted in test board.
#define DCM_MAX_CONTROLLERS_PRE_BOARD 4///<The maximum controller count in board
#define DCM_CHANNELS_PER_CONTROL 16///<The channel count of controller
#define DCM_MAX_CHANNELS_PER_BOARD (DCM_MAX_CONTROLLERS_PRE_BOARD * DCM_CHANNELS_PER_CONTROL)///<The maximum channel count in board
#define DCM_BRAM_PATTERN_LINE_COUNT 0x2000 ///<The pattern line count in BRAM
#define DCM_DRAM_PATTERN_LINE_COUNT 0x4000000 ///<The pattern line count in DRAM
#define DCM_DRAM_LINE_COUNT 0x2000000  ///<The line count in DRAM, each line has two pattern line
#define DCM_TOTAL_LINE_COUNT (DCM_BRAM_PATTERN_LINE_COUNT + DCM_DRAM_PATTERN_LINE_COUNT)///<The total pattern line count in memory
#define MIN_RAM_LINE_COUNT 2///<The minimum line count in BRAM
#define MIN_DRAM_LINE_COUNT 2///<The minimum line count in DRAM
#define EQUAL_ERROR 1e-15///<The equal error of double
#define EDGE_COUNT 6///<The edge count
#define MIN_PERIOD (double)12///<The minimum period
#define MAX_PERIOD (double)4e6///<The maximum period
#define TIMESET_COUNT 32///<The timeset count

#define BRAM_MAX_SAVE_FAIL_LINE_COUNT 0x3FF///<The maximum fail line number saved in BRAM
#define DRAM_MAX_SAVE_FAIL_LINE_COUNT 0x3FF///<The minimum fail line number saved in DRAM

#define MAX_FAIL_LINE_COUNT_OPEN min(BRAM_MAX_SAVE_FAIL_LINE_COUNT, DRAM_MAX_SAVE_FAIL_LINE_COUNT) ///<The maximum fail line count supported

#define BRAM_MAX_SAVE_CAPTURE_COUNT 0x3FF///<The maximum capture saved in BRAM
#define DRAM_MAX_SAVE_CAPTURE_COUNT 0x3FF///<The minimum capture saved in DRAM
#define MAX_CAPTURE_COUNT_OPEN min(BRAM_MAX_SAVE_CAPTURE_COUNT, DRAM_MAX_SAVE_CAPTURE_COUNT) ///<The maximum capture count supported

#define DCM_MAX_SITE_COUNT (DCM_MAX_CHANNELS_PER_BOARD * DCM_MAX_BOARD_NUM)

#define PIN_LEVEL_MIN (-1.5 - EQUAL_ERROR)///<The minimum voltage with equal error of double
#define PIN_LEVEL_MAX (6.0 + EQUAL_ERROR)///<The maximum voltage with equal error of double
#define CLx_LEVEL_MIN (-2.5 - EQUAL_ERROR)///<The minimum clamp voltage with equal error of double
#define CLx_LEVEL_MAX (7.5 + EQUAL_ERROR)///<The maximum clamp voltage with equal error of double
#define IOx_LEVEL_MIN (-0.006 - EQUAL_ERROR)///<The minimum current with equal error of double
#define IOx_LEVEL_MAX (0.018 + EQUAL_ERROR)///<The maximum current with equal error of double

#define PMU_HIGH_MODE_LEVEL_MIN (- EQUAL_ERROR)///<The minimum voltage of PMU high mode with equal error
#define PMU_HIGH_MODE_LEVEL_MAX (13.5 + EQUAL_ERROR)///<The maximum voltage of PMU high mode with equal error

#define DRIVER_CHIP_COUNT 8

#define PMU_SAMPLE_DEPTH (512)///<The sample depth of PMU

#define MAX_MEASURE_VALUE (1e15 - 1)///<The maximum measure value if any error occurs in measurement

#define MAX_PREREAD_SECTOR_COUNT 256///<The pattern sector count in preread
#define MAX_PREREAD_LINE_COUT 256///<The line count of each preread vector


#define  AVERAGE_RESULT -1///<The value signs to get average result in PMU

#define TMU_UNIT_COUNT_PER_CONTROLLER 2///<The TMU unit count in controller
#define TMU_MAX_TIMEOUT 4 * 0x0FFFFFFF///<The maximum timeout, the unit is ns
#define TMU_ERROR  (1e15 - 1) ///<The TMU error

#define I2C_WRITE_MAX_BYTE_COUNT 1000///<The maximum byte count of I2C write
#define I2C_READ_MAX_BYTE_COUNT 100///<The maximum byte count of I2C read
/**
 * @enum WAVE_FORMAT
 * @brief The wave format
*/
enum class WAVE_FORMAT
{
	NRZ = 0,///<Not Return Zero
	RO = 2,///<Return One
	RZ = 4,///<Return Zero
	SBL = 5,///<Surround By Low
	SBH = 3,///<Surround By High
	SBC = 6,///<Surround By Compliment
};
/**
 * @enum COMPARE_MODE
 * @brief Compare mode
*/
enum class COMPARE_MODE
{
	EDGE = 0,///<Edge compare
	WINDOW,///<Window compare
};
/**
 * @enum IO_FORMAT
 * @brief IO format
*/
enum class IO_FORMAT
{
	NRZ = 0,///<Not Return Zero
	RO = 1,///<Return One
};

/**
* @enum MEM_TYPE
* @brief The memory type
*/
enum class MEM_TYPE
{
	BRAM = 0,///<The BRAM
	DRAM,///<The DRAM
};

/**
 * @enum DATA_TYPE
 * @brief The data type of vector
*/
enum class DATA_TYPE
{
	FM = 0,///<The vector code of FM
	MM,///<The vector code of MM
	IOM,///<The vector code of IOM
	CMD,///<The command type
	OPERAND,///<The operand
};
/**
 * @struct CHANNEL_INFORMATION
 * @brief The channel information
*/
typedef struct CHANNEL_INFOMATION
{
	BYTE m_bySlotNo;///<The slot number the channel belong to
	USHORT m_usChannel;///<The channel number
	USHORT m_usChannelID;///<The channel ID which is global unique in all channel
	CHANNEL_INFOMATION(BYTE bySlotNo = 0, USHORT usChannel = -1, USHORT usChannelID = -1)
	{
		m_bySlotNo = bySlotNo;
		m_usChannel = usChannel;
		m_usChannelID = usChannelID;
	}
}CHANNEL_INFO;
/**
 * @enum RELAY_TYPE
 * @brief The relay type
*/
enum class RELAY_TYPE
{
	FUNC_RELAY = 0,///<The functional relay
	HIGH_VOLTAGE_RELAY,///<The high voltage relay
	DC_RELAY,///<The DC relay
};
#define  PMU_IRANGE_COUNT 5///<The range count of PMU current
/**
 * @enum CALIBRATION_DATA
 * @brief The calibration data
*/
typedef struct CALIBRATION_DATA
{
	// 各档位(校准项目)的增益和零位(各功能校准数据顺序：小量程档→大量程档)
	float m_fDVHGain[1];///<The gain of driver high
	float m_fDVHOffset[1];///<The offset of driver high
	float m_fDVLGain[1];///<The gain of driver low
	float m_fDVLOffset[1];///<The offset of driver low
	float m_fFVGain[1];///<The gain of force voltage
	float m_fFVOffset[1];///<The offset of force voltage
	float m_fFIGain[PMU_IRANGE_COUNT];///<The gain of force current
	float m_fFIOffset[PMU_IRANGE_COUNT];///<The offset of force current
	float m_fMVGain[1];///<The gain of voltage measurement
	float m_fMVOffset[1];///<The offset of voltage measurement
	float m_fMIGain[PMU_IRANGE_COUNT];///<The gain of current measurement
	float m_fMIOffset[PMU_IRANGE_COUNT];///<The offset of current measurement
	///<Can't initialize struct in its constructor, otherwise can't shared
}CAL_DATA;

/**
 * @enum LEVEL_TYPE
 * @brief The pin level type
*/
enum class LEVEL_TYPE
{
	VIH = 0,///<The input voltage of logic high
	VIL,///<The input voltage of logic low
	VOH,///<The output voltage of logic high
	VOL,///<The output voltage of logic low
	VT,///<The voltage of VT
	IOH,//The dynamic high current
	IOL,///<The dynamic low current
	CLAMP_HIGH,///<The clamp high voltage
	CLAMP_LOW///<The clamp low voltage
};
/**
 * @enum CURRENT_TYPE
 * @brief The current type of dynamic load
*/
enum class CURRENT_TYPE
{
	IOH,///<The dynamic high current
	IOL,///<The dynamic low current
};

/**
 * @enum VT_MODE
 * @brief The operation mode of VT
*/
enum class VT_MODE
{
	FORCE = 0,///<Force VT
	REALTIME,///<real time VT
	TRISTATE,///<Tristate VT
	CLOSE,///<Close VT mode
};
/**
 * @enum PMU_MODE
 * @brief PMU mode
*/
enum class PMU_MODE
{
	FVMI = 0,///<Fix voltage and measure current
	FIMV,///<Fix current and measure voltage
	FIMI,///<Fix current and measure current
	FVMV///<Fix voltage and measure voltage
};
/**
 * @enum PMU_RANGE
 * @brief PMU range
*/
enum class PMU_IRANGE
{
	IRANGE_2UA = 0,///<2uA
	IRANGE_20UA,///<20uA
	IRANGE_200UA,///<200uA
	IRANGE_2MA,///<2mA
	IRANGE_32MA,///<32mA
};

/**
 *@enum TMU_MEAS_MODE
 *@brief TMU mode
*/
enum class TMU_MEAS_MODE
{
	DUTY_PERIOD = 1,///<Period
	EDGE_TIME,///<Edge time
	SIGNAL_DELAY,///<Signal delay
};
/**
 * @enum TMU_MEAS_TYPE
 * @brief The measure result of TMU
*/
enum class TMU_MEAS_TYPE
{
	FREQ = 0,///<The frequency
	HIGH_DUTY,///<High level time
	LOW_DUTY,///<Low level time
	EDGE,///<The edge value
	DELAY,///<The delay value
};
/**
 * @enum CHANNEL_OUTPUT_STATUS
 * @brief The channel status
*/
enum class CHANNEL_OUTPUT_STATUS
{
	LOW = 0,///<The channel output low
	HIGH,///<The channel output high
	HIGH_IMPEDANCE,///<The channel is in high impedance
};