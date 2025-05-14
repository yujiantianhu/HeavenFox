/*
 * ZYNQ7 Board GPIO Initial
 *
 * File Name:   zynq7_led.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.21
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include "zynq7_common.h"

/*!< The defines */


/*!< API function */
/*!
 * @brief   initial and start led
 * @param   none
 * @retval  none
 * @note    none
 */
void zynq7_led_init(void)
{
    XGpioPs sgtc_gpio;
    XGpioPs_Config *sptr_cfg;

    sptr_cfg = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
    if (!isValid(sptr_cfg))
        return;

    XGpioPs_CfgInitialize(&sgtc_gpio, sptr_cfg, sptr_cfg->BaseAddr);
    XGpioPs_SetDirectionPin(&sgtc_gpio, XGPIOPS_BANK_PIN(0, 0), XGPIOPS_PIN_DIR_OUTPUT);
    XGpioPs_SetOutputEnablePin(&sgtc_gpio, XGPIOPS_BANK_PIN(0, 0), true);
    XGpioPs_WritePin(&sgtc_gpio, XGPIOPS_BANK_PIN(0, 0), XGPIOPS_PIN_LEVEL_LOW);
}

/* end of file*/
