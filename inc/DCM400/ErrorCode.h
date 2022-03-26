#pragma once
/**
 * @file ErrorCode.h
 * @brief The error code for user API
 * @author Guangyun Wang
 * @date 2022/02/19
 * @copyright Beijing Huafeng Test & Control Technology Co. Ltd.
*/
#define MEASURE_ERROR				1e15-1///<Measurement error for PMU and TMU
#define SUCCESS						 0///<The API execution successfully
#define NOT_LOAD_VECTOR				-1///<The vector not existed
#define START_LABEL_NOT_EXISTED		-2///<The label is not existed
#define STOP_LABEL_NOT_EXISTED		-3///<The stop label is not existed
#define MODULE_NAME_CONFILCT		-4///<The module name conflict
#define MODULE_NAME_ERROR			-5///<The module is not existed
#define RUN_ORDER_NAME_CONFILCT		-6///<The run order name is conflict
#define RUN_ORDER_NAME_ERROR		-7///<The run order name is not existed
#define MODULE_COUNT_NOT_EQUAL		-8///<The module count is not equal
#define OPERAND_ERROR				-9///<The operand is over range
#define PIN_NOT_BELONGS				-10///<The pin is not belongs to current instance
#define PIN_GROUP_CONFICT			-11///<The pin group name is conflict
#define PIN_GROUP_ERROR				-12///<The pin group is not defined or nullptr
#define PIN_NAME_ERROR				-13///<The pin name is not existed or nullptr
#define PIN_LEVEL_ERROR				-14///<The pin level is ove range
#define TIMESET_ERROR				-15///<The timeset is not existed or nullptr
#define PERIOD_ERROR				-16///<The period is over range
#define EDGE_ERROR					-17///<The edge is over range
#define OFFSET_ERROR				-18///<The offset is over range
#define DATA_LENGTH_ERROR			-19///<The vector line count is over range
#define SITE_ERROR					-20///<The site is over range
#define SITE_INVALID				-21///<The site is invalid
#define PARAM_NULLPTR				-22///<The parameter is nullptr
#define CHANNLE_ERROR				-23///<The channel is not existed or over range
#define ALLOCATE_MEM_ERROR			-24///<Allocate memory fail
#define FUNC_ERROR					-25///<The function order used is not right
#define CURRENT_ERROR				-26///<The current is over range
#define STOP_LABEL_FRONT			-27///<The stop label is not after of start label
#define NO_VALID_BOARD				-28///<No valid board existed
#define VECTOR_NOT_RAN				-29///<The vector not ran before
#define VECTOR_RUNNING				-30///<The vector is running now
#define FAIL_NOT_SAVE				-31///<The fail information are not all saved
#define CAPTURE_NOT_SAVE			-32///<The capture information are not all saved
#define TMU_UNIT_NOT_CONNECT		-34///<Not the TMU unit connect information

///<For I2C operation
#define I2C_SITE_OVER_RANGE				-100///<The site count is over range
#define I2C_CHANNEL_INFO_NULLPTR		-101///<The point pointed to channel information is nullptr
#define I2C_CHANNEL_FORMAT_ERROR		-102///<The format of I2C channel is error
#define I2C_CHANNEL_INFO_BLANK			-103///<The channel information is blank
#define I2C_CHANNEL_OVER_RANGE			-104///<The channel of SCL or SDA is not existed
#define I2C_CHANNEL_NOT_EXISTED			-105///<The channel is not existed
#define I2C_SITE_CHANNEL_NOT_MATCH		-106///<The site and channel count is not match
#define I2C_CHANNEL_CONFICT				-107///<The channel is conflict
#define I2C_NOT_SET_CHANNEL				-108///<Not set the channel information of I2C
#define I2C_NO_I2C_OPERATION			-109///<No I2C operation ever
#define I2C_VT_ERROR					-110///<The VT value is error
#define I2C_CURRENT_ERROR				-111///<The current for dynamic error
#define I2C_EDGE_ERROR					-112///<The edge setting is error
#define I2C_PIN_LEVEL_ERROR				-113///<The pin level setting is over range
#define I2C_DATA_SIZE_ERROR				-114///<The data size is over range
#define I2C_DATA_POINT_NULLPTR			-115///<The point pointed to data written is nullptr