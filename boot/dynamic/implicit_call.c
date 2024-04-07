/*
 * Dynamic initial/exit defines
 *
 * File Name:   implicit_call.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.11.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <boot/boot_text.h>
#include <boot/implicit_call.h>

/*!< The defines */
#define DYNAMIC_INIT_EXIT_WAYS                             IT_TRUE		/*!< 1: use API to init/exit; 0: use Macro defines */

/*!< The globals */
const dync_init_t *dync_init_sections[] =
{
    [Ert_Dync_Sec_Static]   = &__dync_init_0_start,
    [Ert_Dync_Sec_Early]    = &__dync_init_1_start,
    [Ert_Dync_Sec_Late]     = &__dync_init_2_start,
    [Ert_Dync_Sec_Kernel]   = &__dync_init_3_start,
    [Ert_Dync_Sec_Rootfs]   = &__dync_init_4_start,
    [Ert_Dync_Sec_Platform] = &__dync_init_5_start,
    [Ert_Dync_Sec_Pattern]  = &__dync_init_6_start,
    [Ert_Dync_Sec_Device]   = &__dync_init_7_start,
    [Ert_Dync_Sec_Driver]   = &__dync_init_8_start,
    [Ert_Dync_Sec_Others]   = &__dync_init_9_start,

    [Ert_Dync_Sec_Start]    = &__dync_init_start,
    [Ert_Dync_Sec_End]      = &__dync_init_end,
};

const dync_exit_t *dync_exit_sections[] =
{
    [Ert_Dync_Sec_Static]   = &__dync_exit_0_start,
    [Ert_Dync_Sec_Early]    = &__dync_exit_1_start,
    [Ert_Dync_Sec_Late]     = &__dync_exit_2_start,
    [Ert_Dync_Sec_Kernel]   = &__dync_exit_3_start,
    [Ert_Dync_Sec_Rootfs]   = &__dync_exit_4_start,
    [Ert_Dync_Sec_Platform] = &__dync_exit_5_start,
    [Ert_Dync_Sec_Pattern]  = &__dync_exit_6_start,
    [Ert_Dync_Sec_Device]   = &__dync_exit_7_start,
    [Ert_Dync_Sec_Driver]   = &__dync_exit_8_start,
    [Ert_Dync_Sec_Others]   = &__dync_exit_9_start,

    [Ert_Dync_Sec_Start]    = &__dync_exit_start,
    [Ert_Dync_Sec_End]      = &__dync_exit_end,
};

/* API functions */
/*!
 * @brief   dync_initcall_run_list
 * @param   none
 * @retval  none
 * @note    call every function from .dync.init 
 */
ksint32_t dync_initcall_run_list(const kuint32_t section)
{
	const dync_init_t *pFunc_init;

    if (section >= Ert_Dync_Sec_End)
    {
        return mrt_retval(Ert_isMemErr);
    }

	for (pFunc_init = dync_init_sections[section]; (*pFunc_init) && (pFunc_init < dync_init_sections[section + 1]); pFunc_init++)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

	return mrt_retval(Ert_isWell);
}

/*!
 * @brief   dync_exitcall_run_list
 * @param   none
 * @retval  none
 * @note    call every function from .dync.exit 
 */
void dync_exitcall_run_list(const kuint32_t section)
{
	const dync_exit_t *pFunc_exit;

    if (section >= Ert_Dync_Sec_End)
    {
        return;
    }

	for (pFunc_exit = dync_exit_sections[section]; (*pFunc_exit) && (pFunc_exit < dync_exit_sections[section + 1]); pFunc_exit++)
	{
		(*pFunc_exit)();
	}
}

/*!
 * @brief   board_early_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t board_early_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Early);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   board_early_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void board_early_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Early);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   board_late_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t board_late_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Late);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   board_late_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void board_late_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Late);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   system_kernel_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t system_kernel_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Kernel);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   system_kernel_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void system_kernel_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Kernel);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}


/*!
 * @brief   root_filesystem_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t root_filesystem_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Rootfs);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   root_filesystem_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void root_filesystem_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Rootfs);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   platform_built_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t platform_built_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Platform);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   platform_built_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void platform_built_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Platform);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   platform_reality_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t platform_reality_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Pattern);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   platform_reality_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void platform_reality_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Pattern);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   hardware_device_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t hardware_device_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Device);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   hardware_device_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void hardware_device_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Device);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   abstract_driver_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t abstract_driver_initcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    return dync_initcall_run_list(Ert_Dync_Sec_Driver);

#else
	dync_init_t *pFunc_init;

	mrt_foreach_early_init(pFunc_init)
	{
		if (mrt_isErr((*pFunc_init)()))
		{
			return mrt_retval(Ert_isAnyErr);
		}
	}

    return mrt_retval(Ert_isWell);

#endif
}

/*!
 * @brief   abstract_driver_exitcall
 * @param   none
 * @retval  none
 * @note    none
 */
void abstract_driver_exitcall(void)
{
#if DYNAMIC_INIT_EXIT_WAYS
    dync_exitcall_run_list(Ert_Dync_Sec_Driver);

#else
	dync_exit_t *pFunc_exit;

	mrt_foreach_early_exit(pFunc_exit)
	{
        (*pFunc_exit)();
	}

#endif
}

/*!
 * @brief   run_machine_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t run_machine_initcall(void)
{
	ksint32_t retval;

    retval = board_early_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

    retval = board_late_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

	return 0;
}

/*!
 * @brief   run_platform_initcall
 * @param   none
 * @retval  none
 * @note    if (*pFunc_init)() return error, other initcalls will be stopped immediately
 */
ksint32_t run_platform_initcall(void)
{
	ksint32_t retval;

    /*!< initcall */
    retval = platform_built_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

    retval = platform_reality_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

    retval = hardware_device_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

    retval = abstract_driver_initcall();
    if (mrt_isErr(retval))
    {
        return retval;
    }

	return 0;
}

/* end of file */
