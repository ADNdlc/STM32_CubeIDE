 /**
 * @file tm_user.c
 * @brief 物模型用户接口定义文件
 * @details 该文件由物联网平台根据用户配置的物模型自动生成，包含设备服务、属性、命令和事件的接口定义。
 *          用户需实现文件中声明的回调函数，以处理物模型数据的读写、命令执行和事件上报等操作。
 * @author 中移物联网有限公司
 * @date 2025-06-13
 * @version V1.0
 * @copyright Copyright © 2024 中移物联网有限公司. All rights reserved.
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "tm_data.h"
#include "tm_user.h"

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
/*************************** Property Func List ******************************/
struct tm_prop_tbl_t tm_prop_list[] = {
    TM_PROPERTY_RW($OneNET_OneOS_VERSION),
    TM_PROPERTY_RW(test1)
};
uint16_t tm_prop_list_size = ARRAY_SIZE(tm_prop_list);
/****************************** Auto Generated *******************************/

/***************************** Service Func List *******************************/
struct tm_svc_tbl_t tm_svc_list[] = {0};
uint16_t tm_svc_list_size = 0;
/****************************** Auto Generated *******************************/

/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/
/**************************** Property Func Read *****************************/
int32_t tm_prop_$OneNET_OneOS_VERSION_rd_cb(void *data)
{
    void *structure = tm_data_struct_create();
    struct prop_$OneNET_OneOS_VERSION_t val = {0};
    /** 根据业务逻辑获取功能点值，设置到val */

    tm_data_struct_set_string(structure, "OneNET_OneOS_VERSION", val.OneNET_OneOS_VERSION);
    tm_data_struct_set_data(data, "$OneNET_OneOS_VERSION", structure);
   return 0;
}

int32_t tm_prop_test1_rd_cb(void *data)
{
	int32_t val = 0;

	/** 根据业务逻辑获取功能点值，设置到val */


	tm_data_struct_set_int32(data, "test1", val);

	return 0;
}


/****************************** Auto Generated *******************************/

/**************************** Property Func Write ****************************/
int32_t tm_prop_$OneNET_OneOS_VERSION_wr_cb(void *data)
{
    struct prop_$OneNET_OneOS_VERSION_t val;

    tm_data_struct_get_string(data, "OneNET_OneOS_VERSION", &(val.OneNET_OneOS_VERSION));
    /** 根据变量val的值，填入下发控制逻辑 */

    /***/
    return 0;
}

int32_t tm_prop_test1_wr_cb(void *data)
{
    int32_t val = 0;
    tm_data_get_int32(data, &val);
    /** 根据变量val的值，填入下发控制逻辑 */

    /***/
    return 0;
}


/****************************** Auto Generated *******************************/

/**************************** Property Func Notify ***************************/
int32_t tm_prop_$OneNET_OneOS_VERSION_notify(void *data, struct prop_$OneNET_OneOS_VERSION_t val, uint64_t timestamp, uint32_t timeout_ms)
{
    void *resource = NULL;
    void *structure = tm_data_struct_create();
	int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

	tm_data_struct_set_string(structure, "OneNET_OneOS_VERSION", val.OneNET_OneOS_VERSION);

    tm_data_set_struct(resource, "$OneNET_OneOS_VERSION", structure, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}

int32_t tm_prop_test1_notify(void *data, int32_t val, uint64_t timestamp, uint32_t timeout_ms)
{
	void *resource = NULL;
    int32_t ret = 0;

    if(NULL == data)
    {
        resource = tm_data_create();
    }
    else
    {
        resource = data;
    }

    tm_data_set_int32(resource, "test1", val, timestamp);

    if(NULL == data)
    {
        ret = tm_post_property(resource, timeout_ms);
    }

    return ret;
}


/****************************** Auto Generated *******************************/

/***************************** Event Func Notify *****************************/

/****************************** Auto Generated *******************************/

/**************************** Service Func Invoke ****************************/

/****************************** Auto Generated *******************************/
