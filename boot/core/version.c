/*
 * General Function
 *
 * File Name:   version.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.07.21
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <boot/core.h>

/*!< The defines */

/*!< The globals */
static const kchar_t *g_program_version = CONFIG_VERSION;
static const kchar_t *g_arch_name = CONFIG_ARCH;
static const kchar_t *g_arch_type = CONFIG_TYPE;
static const kchar_t *g_arch_class = CONFIG_CLASS;
static const kchar_t *g_cpu_verdor = CONFIG_VERDOR;
static const kchar_t *g_cpu_name = CONFIG_CPU;
static const kchar_t *g_board_manufacturer = CONFIG_MANUFACTURER;
static const kchar_t *g_board_name = CONFIG_BOARD;
static const kchar_t *g_core_mode[2] = { "AMP", "SMP" };

/*!< API function */
/*!
 * @brief   get program version
 * @param   none
 * @retval  g_program_version
 * @note    none
 */
kchar_t *get_version(void)
{
    return (kchar_t *)g_program_version;
}

/*!
 * @brief   get arch name
 * @param   none
 * @retval  g_arch_name
 * @note    none
 */
kchar_t *get_arch(void)
{
    return (kchar_t *)g_arch_name;
}

/*!
 * @brief   get arch type
 * @param   none
 * @retval  g_arch_type
 * @note    none
 */
kchar_t *get_arch_type(void)
{
    return (kchar_t *)g_arch_type;
}

/*!
 * @brief   get arch class
 * @param   none
 * @retval  g_arch_class
 * @note    none
 */
kchar_t *get_arch_class(void)
{
    return (kchar_t *)g_arch_class;
}

/*!
 * @brief   get cpu verdor
 * @param   none
 * @retval  g_cpu_verdor
 * @note    none
 */
kchar_t *get_cpu_verdor(void)
{
    return (kchar_t *)g_cpu_verdor;
}

/*!
 * @brief   get cpu name
 * @param   none
 * @retval  g_cpu_name
 * @note    none
 */
kchar_t *get_cpu_name(void)
{
    return (kchar_t *)g_cpu_name;
}

/*!
 * @brief   get board manufacturer
 * @param   none
 * @retval  g_board_manufacturer
 * @note    none
 */
kchar_t *get_board_manufacturer(void)
{
    return (kchar_t *)g_board_manufacturer;
}

/*!
 * @brief   get board name
 * @param   none
 * @retval  g_board_name
 * @note    none
 */
kchar_t *get_board_name(void)
{
    return (kchar_t *)g_board_name;
}

/*!
 * @brief   get cpu mode (AMP/SMP)
 * @param   none
 * @retval  g_core_mode[CONFIG_CORE]
 * @note    none
 */
kchar_t *get_cpu_mode(void)
{
    return (kchar_t *)g_core_mode[CONFIG_CORE];
}

/* end of file */
