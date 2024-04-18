/*
 * Hardware Abstraction Layer Pinctrl Interface
 *
 * File Name:   fwk_pinctrl.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.15
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_PINCTRL_H_
#define __FWK_PINCTRL_H_

/*!< The includes */
#include <platform/fwk_basic.h>
#include <platform/fwk_platform.h>
#include <kernel/kernel.h>
#include <kernel/mutex.h>

/*!< The defines */
typedef struct fwk_pinctrl_pin_desc 
{
	kuint32_t number;
	const kstring_t *name;
	void *drv_data;

#define FWK_PINCTRL_PIN(a, b)						{ .number = a, .name = b }
	
} srt_fwk_pinctrl_pin_desc_t;

typedef struct fwk_pinctrl_desc 
{
	const kstring_t *name;

	/*!< npins: sizeof(sprt_pins[])*/
	struct fwk_pinctrl_pin_desc const *sprt_pins;
	kuint32_t npins;

	const struct fwk_pinmux_ops *sprt_pmxops;
	const struct fwk_pinconf_ops *sprt_confops;

} srt_fwk_pinctrl_desc_t;

typedef struct fwk_pinctrl_dev 
{
	struct list_head sgrt_node;
	struct fwk_pinctrl_desc *sprt_desc;
	struct list_head sgrt_gpio_ranges;
	struct fwk_device *sprt_dev;
	struct fwk_pinctrl_state *sprt_hog_default;
	struct fwk_pinctrl_state *sprt_hog_sleep;
	struct mutex_lock sgrt_mutex;

} srt_fwk_pinctrl_dev_t;

typedef struct fwk_pinctrl_gpio_range 
{
	struct list_head sgrt_link;
	const kstring_t *name;
	kuint32_t id;
	kuint32_t base;
	kuint32_t pin_base;
	kuint32_t const *pins;
	kuint32_t npins;

	struct gpio_chip *sprt_gc;

} srt_fwk_pinctrl_gpio_range_t;

typedef struct fwk_pinmux_ops 
{
	ksint32_t (*request) (srt_fwk_pinctrl_dev_t *sprt_pctldev, kuint32_t offset);
	ksint32_t (*free) (srt_fwk_pinctrl_dev_t *sprt_pctldev, kuint32_t offset);

	ksint32_t (*set_mux) (srt_fwk_pinctrl_dev_t *sprt_pctldev, kuint32_t func_selector, kuint32_t group_selector);
	
	ksint32_t (*gpio_request_enable) (srt_fwk_pinctrl_dev_t *sprt_pctldev, 
										srt_fwk_pinctrl_gpio_range_t *sprt_range, kuint32_t offset);

	void (*gpio_disable_free) (srt_fwk_pinctrl_dev_t *sprt_pctldev, 
										srt_fwk_pinctrl_gpio_range_t *sprt_range, kuint32_t offset);

	ksint32_t (*gpio_set_direction) (srt_fwk_pinctrl_dev_t *sprt_pctldev, 
										srt_fwk_pinctrl_gpio_range_t *sprt_range, kuint32_t offset, bool input);
	
} srt_fwk_pinmux_ops_t;

typedef struct fwk_pinconf_ops 
{
	ksint32_t (*pin_config_get) (srt_fwk_pinctrl_dev_t *sprt_pctldev, kuint32_t pin, kuaddr_t *config);
	ksint32_t (*pin_config_set) (srt_fwk_pinctrl_dev_t *sprt_pctldev, kuint32_t pin, kuaddr_t *configs, kuint32_t num_configs);

} srt_fwk_pinconf_ops_t;

typedef struct fwk_pinctrl_mux 
{
	kuint32_t group;
	kuint32_t func;

} srt_fwk_pinctrl_mux_t;

typedef struct fwk_pinctrl_configs 
{
	kuint32_t group_or_pin;
	kuaddr_t *configs;
	kuint32_t num_configs;

} srt_fwk_pinctrl_configs_t;

typedef struct fwk_pinctrl_setting 
{
	struct list_head sgrt_node;
	struct fwk_pinctrl_dev *sprt_pctldev;
	const kstring_t *dev_name;

	union 
	{
		struct fwk_pinctrl_mux sgrt_mux;
		struct fwk_pinctrl_configs sgrt_configs;
	} sgrt_data;
};

typedef struct fwk_pinctrl_state 
{
	struct list_head sgrt_link;                         /* pinctrl_state链表, 用于挂接在pinctrl::states */
	const kstring_t *name;                              /* pinctrl-names */
	struct list_head settings;                          /* 当前state的引脚配置集合, 详见struct pinctrl_setting */

} srt_fwk_pinctrl_state_t;

typedef struct fwk_pinctrl
{
	struct list_head sgrt_link;
	struct fwk_device *sprt_dev;
	struct list_head sgrt_states;                       /* 各个pinctrl_state::node的链表头 */
	struct fwk_pinctrl_state *sprt_state;               /* 当前使用的state, 比如"default" */

} srt_fwk_pinctrl_t;

/*!< The functions */


#endif /*!< __FWK_PINCTRL_H_ */
