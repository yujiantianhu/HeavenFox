/*
 * General Function
 *
 * File Name:   generic.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.18
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <common/generic.h>

/*!< API function */
kutype_t udiv_integer(kutype_t divied, kutype_t div)
{
	kutype_t count = 0;

	while (divied >= div)
	{
		divied -= div;
		count++;
	}

	return count;
}

kstype_t sdiv_integer(kstype_t divied, kstype_t div)
{
	kstype_t count = 0;
	kstype_t number1 = mrt_abs(divied);
	kstype_t number2 = mrt_abs(div);

	while (number1 >= number2)
	{
		number1 -= number2;
		count++;
	}

	return ((divied ^ div) < 0) ? -count : count;
}

kutype_t udiv_remainder(kutype_t divied, kutype_t div)
{
	while (divied >= div)
		divied -= div;

	return divied;
}

/* end of file */
