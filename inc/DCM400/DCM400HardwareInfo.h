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
#include <string>
#define ALL_SITE 0xFFFF
#define DCM400_MAX_BOARD_NUM (4) //The maximum number of DCM board inserted in test board.
#define DCM400_MAX_CONTROLLERS_PRE_BOARD (4) ///<The maximum controller count in board
#define DCM400_MAX_PATTERN_COUNT 0x4000000///<The pattern line count for each controller


#define DCM400_CHANNELS_PER_CONTROL 16///<The channel count of controller
#define DCM400_MAX_CHANNELS_PER_BOARD (DCM400_MAX_CONTROLLERS_PRE_BOARD * DCM400_CHANNELS_PER_CONTROL)///<The maximum channel count in board
#define EQUAL_ERROR 1e-15///<The equal error of double
#define EDGE_COUNT 6///<The edge count

#define MIN_PERIOD ((double)5)
#define MAX_PERIOD ((double)4E6)
#define TIME_SERIES_MAX_COUNT (32)
#define TIME_SET_MAX_COUNT (256)

#define DCM400_MAX_SITE_COUNT (DCM400_MAX_CHANNELS_PER_BOARD * DCM400_MAX_BOARD_NUM)

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
	RZ = 1,///<Return Zero
	RO = 2,///<Return One
	SBC = 3,///<Surround By Compliment
	SBH = 4,///<Surround By High
	SBL = 5,///<Surround By Low
	STAY = 8,///<The wave stay latest driver
	FH = 9,///<Force high
	FL = 10,///<Force low
};
/**
 * @enum COMPARE_MODE
 * @brief Compare mode
*/
enum class COMPARE_MODE
{
	EDGE = 0,///<Edge compare
	WINDOW = 1,///<Window compare
	OFF = 2,
};
/**
 * @enum IO_FORMAT
 * @brief IO format
*/
enum class IO_FORMAT
{
	NRZ = 0,///<Not Return Zero
	RO = 1,///<Return One
	OFF = 2,
};
/**
 * @struct CHANNEL_INFO
 * @brief The channel information
*/
struct CHANNEL_INFO
{
	BYTE m_bySlotNo;///<The slot number the channel belong to
	USHORT m_usChannel;///<The channel number
	USHORT m_usChannelID;///<The channel ID which is global unique in all channel
	CHANNEL_INFO(BYTE bySlotNo = 0, USHORT usChannel = -1, USHORT usChannelID = -1)
	{
		m_bySlotNo = bySlotNo;
		m_usChannel = usChannel;
		m_usChannelID = usChannelID;
	}
};
/**
 * @enum RELAY_TYPE
 * @brief The relay type
*/
enum class RELAY_TYPE
{
	FUNC_RELAY = 0,///<The functional relay
	HIGH_VOLTAGE_RELAY,///<The high voltage relay
};
#define  DCM400_PMU_IRANGE_COUNT 5///<The range count of PMU current
/**
 * @enum DCM400_CAL_DATA
 * @brief The calibration data
*/
 struct DCM400_CAL_DATA
{
	// 各档位(校准项目)的增益和零位(各功能校准数据顺序：小量程档→大量程档)
	float m_fDVHGain[1];///<The gain of driver high
	float m_fDVHOffset[1];///<The offset of driver high
	float m_fDVLGain[1];///<The gain of driver low
	float m_fDVLOffset[1];///<The offset of driver low
	float m_fVOHGain[1];///<The gain of compare high voltage
	float m_fVOHOffset[1];///<The offset of compare low voltage
	float m_fVOLGain[1];///<The gain of compare low voltage
	float m_fVOLOffset[1];///<The offset of compare low voltage
	float m_fVTGain[1];///<The gain of VT or VCOM voltage
	float m_fVTOffset[1];///<The offset of VT or VCOM voltage
	float m_fVHHGain[1];///<The gain of high voltage
	float m_fVHHOffset[1];///<The offset of high voltage
	float m_fIOHGain[1];///<The gain of high current output for dynamic load
	float m_fIOHOffset[1];///<The offset of high current output for dynamic load
	float m_fIOLGain[1];///<The gain of low current output for dynamic load
	float m_fIOLOffset[1];///<The offset of low current output for dynamic load
	float m_fFVGain[1];///<The gain of force voltage
	float m_fFVOffset[1];///<The offset of force voltage
	float m_fFIGain[DCM400_PMU_IRANGE_COUNT];///<The gain of force current
	float m_fFIOffset[DCM400_PMU_IRANGE_COUNT];///<The offset of force current
	float m_fMVGain[1];///<The gain of voltage measurement
	float m_fMVOffset[1];///<The offset of voltage measurement
	float m_fMIGain[DCM400_PMU_IRANGE_COUNT];///<The gain of current measurement
	float m_fMIOffset[DCM400_PMU_IRANGE_COUNT];///<The offset of current measurement
	///<Can't initialize struct in its constructor, otherwise memory can't shared
};
 
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
 * @brief The channel mode
*/
enum class CHANNLE_MODE
{
	VECTOR_MODE,///<The vector submode of MCU
	FORCE_MODE,///<The force submode of MCU
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

/**
 * @enum EDGE_TYPE
 * @brief The edge type
*/
enum class EDGE_TYPE
{
	T1R = 0,///<The raise edge of drive
	T1F,///<The fall edge of drive
	IOR,///<The raise edge of IO
	IOF,///<The fall edge of IO
	STBR,///<The raise edge of compare
	STBF,///<The fall edge of compare
};

/**
 * @struct CMD_INFO
 * @brief The command information
*/
struct CMD_INFO
{
	int m_nCode;///<The name
	int m_nOperand;///<The operand
	CMD_INFO()
	{
		m_nCode = 0;
		m_nOperand = 0;
	}
};
