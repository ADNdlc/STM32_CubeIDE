/*
 * TOUCH_GT9147.c
 *
 *  Created on: Jul 13, 2025
 *      Author: 12114
 */

#include "touch_gt9xxx.h"
#include "string.h"

/* 注意: 除了GT9271支持10点触摸之外, 其他触摸芯片只支持 5点触摸 */
//这里使用GT1158
uint8_t g_gt_tnum = 5;      /* 支持的触摸屏点数:5点触摸 */
GT9147_TypeDef touch_GT1158;

/**
 * @brief       向gt9xxx写入一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 写数据长度
 * @retval      0, 成功; 1, 失败;
 */
uint8_t gt9xxx_wr_reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    uint8_t ret = 0;
    ct_iic_start();
    ct_iic_send_byte(GT9XXX_CMD_WR);    /* 发送写命令 */
    ct_iic_wait_ack();
    ct_iic_send_byte(reg >> 8);         /* 发送高8位地址 */
    ct_iic_wait_ack();
    ct_iic_send_byte(reg & 0XFF);       /* 发送低8位地址 */
    ct_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        ct_iic_send_byte(buf[i]);       /* 发数据 */
        ret = ct_iic_wait_ack();

        if (ret)break;
    }

    ct_iic_stop();                      /* 产生一个停止条件 */
    return ret;
}

/**
 * @brief       从gt9xxx读出一次数据
 * @param       reg : 起始寄存器地址
 * @param       buf : 数据缓缓存区
 * @param       len : 读数据长度
 * @retval      无
 */
void gt9xxx_rd_reg(uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    ct_iic_start();
    ct_iic_send_byte(GT9XXX_CMD_WR);                        /* 发送写命令 */
    ct_iic_wait_ack();
    ct_iic_send_byte(reg >> 8);                             /* 发送高8位地址 */
    ct_iic_wait_ack();
    ct_iic_send_byte(reg & 0XFF);                           /* 发送低8位地址 */
    ct_iic_wait_ack();
    ct_iic_start();
    ct_iic_send_byte(GT9XXX_CMD_RD);                        /* 发送读命令 */
    ct_iic_wait_ack();

    for (i = 0; i < len; i++)
    {
        buf[i] = ct_iic_read_byte(i == (len - 1) ? 0 : 1);  /* 读取数据 */
    }

    ct_iic_stop();                                          /* 产生一个停止条件 */
}


/**
 * @brief       初始化gt9xxx触摸屏
 * @param       无
 * @retval      0, 初始化成功; 1, 初始化失败;
 */
uint8_t gt9xxx_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;
    uint8_t temp[5];//存放命令参数和读取内容

    GT9XXX_RST_GPIO_CLK_ENABLE();                           /* RST引脚时钟使能 */
    GT9XXX_INT_GPIO_CLK_ENABLE();                           /* INT引脚时钟使能 */

    gpio_init_struct.Pin = GT9XXX_RST_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;     /* 高速 */
    HAL_GPIO_Init(GT9XXX_RST_GPIO_PORT, &gpio_init_struct); /* 初始化RST引脚 */

    gpio_init_struct.Pin = GT9XXX_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;     /* 高速 */
    HAL_GPIO_Init(GT9XXX_INT_GPIO_PORT, &gpio_init_struct); /* 初始化INT引脚 */

    ct_iic_init();                                          /* 初始化电容屏的I2C总线 */

    //设定地址为0x28/0x29的时序: 释放RST时,INT为高
#ifdef use_CMD_2829
    HAL_GPIO_WritePin(GT9XXX_INT_GPIO_PORT,GT9XXX_INT_GPIO_PIN,GPIO_PIN_SET);//将INT置高(0x28/0x29)
#endif
    //设定地址为0xBA/OxBB的时序: 释放RST时,INT为低
    GT9XXX_RST(0);                                          /* 复位 */
    HAL_Delay(10);
    GT9XXX_RST(1);                                          /* 释放复位 */
    HAL_Delay(10);
    /* 转换INT引脚模式设置, 输入模式, 浮空输入 */
    gpio_init_struct.Pin = GT9XXX_INT_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_INPUT;                /* 输入 */
    gpio_init_struct.Pull = GPIO_NOPULL;                    /* 不带上下拉，浮空模式 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;     /* 高速 */
    HAL_GPIO_Init(GT9XXX_INT_GPIO_PORT, &gpio_init_struct); /* 初始化INT引脚 */

    HAL_Delay(100);
    gt9xxx_rd_reg(GT9XXX_PID_REG, temp, 4);                 /* 读取产品ID */
    temp[4] = 0;
    /* 判断一下是否是特定的触摸屏 */
    if (strcmp((char *)temp, "911") && strcmp((char *)temp, "9147") && strcmp((char *)temp, "1158") && strcmp((char *)temp, "9271"))
    {
    	printf("gt9xxx_init:fail ");
    	printf("CTP ID:%s\r\n", temp);
        return 1;   /* 若不是触摸屏用到的GT911/9147/1158/9271，则初始化失败，需硬件查看触摸IC型号以及查看时序函数是否正确 */
    }
    printf("CTP ID:%s\r\n", temp);                          /* 打印ID */

    if (strcmp((char *)temp, "9271") == 0)                  /* ID==9271, 支持10点触摸 */
    {
         g_gt_tnum = 10;                                    /* 支持10点触摸屏 */
    }

    temp[0] = 0X02;
    gt9xxx_wr_reg(GT9XXX_CTRL_REG, temp, 1);                /* 软复位GT9XXX */

    //delay_ms(10);
    HAL_Delay(10);

    temp[0] = 0X00;
    gt9xxx_wr_reg(GT9XXX_CTRL_REG, temp, 1);                /* 结束复位, 进入读坐标状态 */

    return 0;
}

/* GT9XXX 10个触摸点(最多) 对应的寄存器表 */
const uint16_t GT9XXX_TPX_TBL[10] ={
    GT9XXX_TP1_REG, GT9XXX_TP2_REG, GT9XXX_TP3_REG, GT9XXX_TP4_REG, GT9XXX_TP5_REG,
    GT9XXX_TP6_REG, GT9XXX_TP7_REG, GT9XXX_TP8_REG, GT9XXX_TP9_REG, GT9XXX_TP10_REG,
};

/**
 * @brief       扫描触摸屏(采用查询方式)
 * @param       mode : 电容屏未用到此参数, 为了兼容电阻屏
 * @retval      当前触屏状态
 *   @arg       0, 触屏无触摸;
 *   @arg       1, 触屏有触摸;
 *   @arg       2, 错误;
 */
uint8_t gt9xxx_scan(void){
    uint8_t temp[4];
    uint8_t i = 0, state = 0, cmd = 0;

    gt9xxx_rd_reg(GT9XXX_GSTID_REG, &state, 1);      //读取触摸点的状态
	if(state & 0X80){ //有坐标可读取
		cmd = 0;
		gt9xxx_wr_reg(GT9XXX_GSTID_REG, &cmd, 1);//清标志(寄存器写0)
	}else{
		return 0;
	}

	if( (state & 0xF) && ((state & 0xF) < 6)){ 	//判断触摸点个数
		for(i = 0; i < (state & 0xF); i++){		//读取触摸个数的点
			gt9xxx_rd_reg(GT9XXX_TPX_TBL[i], temp, 4);//读取XY坐标值,并转换后存入
			touch_GT1158.x[i] = (((uint16_t)temp[1] << 8) + temp[0]);
			touch_GT1158.y[i] = (((uint16_t)temp[3] << 8) + temp[2]);
		}
		return 1;
	}
	return 2;
}


