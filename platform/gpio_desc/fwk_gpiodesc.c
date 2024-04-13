/*
 * Hardware Abstraction Layer Gpio Interface
 *
 * File Name:   fwk_gpiodesc.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.06.03
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_gpiodesc.h>

/*!< API function */
/*!
 * @brief   fwk_gpio_set_value
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_value(struct fwk_gpio_desc *sprt_desc, kuint32_t value)
{

}

/*!
 * @brief   fwk_gpio_get_value
 * @param   none
 * @retval  none
 * @note    none
 */
kuint32_t fwk_gpio_get_value(struct fwk_gpio_desc *sprt_desc)
{
    return true;
}

/*!
 * @brief   fwk_gpio_set_direction
 * @param   none
 * @retval  none
 * @note    none
 */
void fwk_gpio_set_direction(struct fwk_gpio_desc *sprt_desc, kuint32_t value)
{

}
