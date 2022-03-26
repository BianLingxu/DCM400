#pragma once
/**
 * @file AlarmID.h
 * @brief The alarm ID of each type alarm
 * @author Guangyun Wang
 * @date 2020/09/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
/**
 * @enum ALARM_ID
 * @brief The alarm ID
*/
enum class ALARM_ID
{
	ALARM_NOALARM = 0,///<No alarm, the default value
	ALARM_VECTOR_NOT_LOAD = 821301,///<Not load vector
	ALARM_PIN_GROUP_ERROR = 821302,///<The pin group is nullptr or not defined
	ALARM_TIMESET_ERROR = 821303,///<The timeset name is not existed
	ALARM_PERIOD_ERROR = 821304,///<The period is over range
	ALARM_PIN_LEVEL_ERROR = 821305,///<The pin level is over range
	ALARM_PMU_SET_VALUE_ERROR = 821306,///<The set value of PMU is over range
	ALARM_CMD_NOT_SUPPORTED = 821307,///<The instruction in the vector line is not supported
	ALARM_SITE_INVALID = 821308,///<The site is invalid
	ALARM_START_LABEL_ERROR = 821309,///<The start label is not existed
	ALARM_STOP_LABEL_ERROR = 821310,///<The stop label is not existed
	ALARM_START_AFTER_STOP_LABEL = 821311,///<The stop label is before start label
	ALARM_SITE_OVER_RANGE = 821312,///<The site number is over range
	ALARM_OFFSET_OVER_RANGE = 821313,///<The line offset value is over range
	ALARM_LINE_COUNT_OVER_RANGE = 821314,///<The line count is over range
	ALARM_PARAM_NULLPTR = 821315,///<The point of the parameter is nullptr
	ALARM_PIN_NAME_ERROR = 821316,///<The pin name is not existed or its point is nullptr
	ALARM_CURRENT_ERROR = 821317,///<The current is error
	ALARM_FAIL_LINE_NOT_SAVE = 821318,///<The fail line number not saved for other channel's fail line occupy the fail memory
	ALARM_I2C_CHANNEL_CONFICT = 821319,///<The channel of I2C is conflict
	ALARM_BOARD_NOT_EXISTED = 821320,///<The board is not existed
	ALARM_SITE_COUNT_OVER_RANGE = 821321,///<The site count is over range
	ALARM_I2C_DATA_LENGTH_ERROR = 821322,///<The data length of I2C is error
	ALARM_PARAM_BLANK = 821323,///<The paramter is blank
	ALARM_I2C_NOT_SET_SITE = 821324,///<Not set the I2C channel before I2C operation
	ALARM_EDGE_ERROR = 821325,///<The edge value is over range
	ALARM_VT_VALUE_ERROR = 821326,///<The VT value is over range
	ALARM_MODE_ERROR = 821327,///<The mode is error or not supported
	ALARM_NO_CAPTURE = 821328,///<No hardware capture in the vector of latest ran
	ALARM_CLAMP_VALUE_OVER_RANGE = 821329,///<The clamp value is over range
	ALARM_PIN_NAME_STRING_FORMT_WRONG = 82130,///<The format of pin name string is wrong
	ALARM_PIN_GRUUOP_CONFLICT = 821331,///<The pin group set is conflict
	ALARM_GET_STOP_LINE_LABEL_ERROR = 821332,///<Get the stop line label error
	ALARM_PMU_SAMPLE_TIMES_ERROR = 821333,///<The sample times of PMU is over range
	ALARM_PMU_SAMPLE_PERIOD_ERROR = 821334,///<The sample interval of PMU is over range
	ALARM_I2C_CHANNEL_STRING_FORMAT_WRONG = 821335,///<The format of I2C channel string is wrong
	ALARM_I2C_CHANNEL_NOT_EQUAL_SITE_COUNT = 821336,///<The channel count of the channel string is not equal to the site count
	ALARM_CHANNEL_OVER_RANGE = 821337,///<The channel number is over range
	ALARM_I2C_NOT_READ_ERROR = 821338,///<Not read through I2C before
	ALARM_I2C_GET_BIT_START_ERROR = 821339,///<The start bit of I2C operation is error
	ALARM_ALLOCTE_MEMORY_ERROR = 821340,///<Allocate memory fail
	ALARM_FUNCTION_USE_ERROR = 821341,///<The function used is not as the sequence requested
	ALARM_I2C_OPERATION_ERROR = 821342,///<The I2C operation is error
	ALARM_CHANNEL_NOT_EXISTED = 821343,///<The channel is not existed
	ALARM_UNKNOWN_ERROR = 821344,///<Unknown error
	ALARM_NOT_RAN_VECTOR = 821345,///<Not ran vector before
	ALARM_VECTOR_RUNNING = 821346,///<The vector is running now
	ALARM_PMU_CLAMP = 821347,///<PMU has clamp
	ALARM_CHANNEL_NOT_CONNECT_TMU = 821348,///<The channel is not connect the TMU unit
	ALARM_TMU_MEAS_MODE_ERROR = 821349,///<The measurement mode of TMU is error
	ALARM_TMU_UNIT_CONNECT_CHANNEL_OVER_RANGE = 821350,///<The count of the channel connected to TMU unit is over range
	ALARM_TMU_UNIT_INDEX_OVER_RANGE = 821351,///<The TMU unit index is over range
	ALARM_TMU_HOLDOFF_TMIE_OVER_RANGE = 821352,///<The hold off time is over range
	ALARM_TMU_HOLDOFF_NUM_OVER_RANGE = 821353,///<The hold off number is over range
	ALARM_TMU_TIMEOUT_OVER_RANGE = 821354,///<The timeout of TMU is over range
	ALARM_TMU_SAMPLE_NUM_OVER_RANGE = 821355,///<The sample number is over range
	ALARM_TMU_MEASURE_RESULT_ERROR = 821356,///<The TMU measurement result is error
	ALARM_TMU_NOT_MEASURE_ERROR = 821357,///<The measurement type of TMU is error
	ALARM_TMU_NOT_STOP = 821358,///<The TMU unit is measuring now
	ALARM_TMU_TIMEOUT = 821359,///<The TMU unit is timeout
	ALARM_OPERAND_ERROR = 821360,///<The operand is over range
	ALARM_LINE_NO_OPERAND = 821361,///<The line number has no operand
	ALARM_PMU_MEAS_ERROR = 821362,///<PMU measurement error
	ALARM_CAPTURE_NOT_SAVING = 821363,///<PMU measurement error
	ALARM_I2C_DATA_SIZE_OVER_RANGE = 821364,///<I2C read or write data is over range
	ALARM_INSTANCE_OVERLAP = 821365,///<The controller are used in more than one instance
	ALARM_PIN_UNOWNED = 821366,///<The pin name is not belongs to current instance
	ALARM_CALIBRATION_ERRPR = 821367,///<Get the calibraiton information fail
	ALARM_CONDITIONAL_INSTR_ERROR = 821368,///<The site channels distrubition not meet the requirement of conditional instruction
	ALARM_CPAUTE_NOT_SUPPORT = 821369,///<The capture data not supported
};