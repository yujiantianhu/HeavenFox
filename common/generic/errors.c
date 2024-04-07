/*
 * Errors and Eceptions General Function
 *
 * File Name:   errors.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.10.15
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>
#include <common/api_string.h>
#include <common/io_stream.h>

/*!< API function */
/*!
 * @brief   deal_assert_fail
 * @param   __assertion, __file, __line, __function
 * @retval  none
 * @note    assertion solution
 */
void deal_assert_fail(const kstring_t *__assertion, const kstring_t *__file,
			   				kuint32_t __line, const kstring_t *__function)
{
	print_err("Program Aborted. Here is Error Information:\n");

	print_err("---> assertion: %s\n", __assertion);
	print_err("---> file     : %s\n", __file);
	print_err("---> line     : %d\n", __line);
	print_err("---> function : %s\n", __function);
	print_err("\n");
	print_err("Please check for errors in time !\n");

	/*!< quit program */
	while (true)
	{}
}


/* end of file */
