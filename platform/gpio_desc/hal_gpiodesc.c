/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   hal_gpiodesc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/hal_gpiodesc.h>

/*!< API function */
/*!
 * @brief   hal_gpio_set_value
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_gpio_set_value(struct hal_gpio_desc *sprt_desc, kuint32_t value)
{

}

/*!
 * @brief   hal_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t hal_gpio_get_value(struct hal_gpio_desc *sprt_desc)
{
    return Ert_true;
}

/*!
 * @brief   hal_gpio_set_direction
 * @param   none
 * @retval  none
 * @note    none
 */
void hal_gpio_set_direction(struct hal_gpio_desc *sprt_desc, kuint32_t value)
{

}
