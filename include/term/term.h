/*
 * String Function Declare
 *
 * File Name:   term.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.09.26
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __TERM_H
#define __TERM_H

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/ascii.h>
#include <platform/input/fwk_kbd.h>
#include <platform/notifier/fwk_notifier.h>

/*!< The defines */
#define TERM_MSG_RECV_LEN                               1024

typedef void (*term_cmd_fn_t)(void);

/*!< Command structure */
struct term_cmd
{
    kchar_t name[32];

    kint32_t (*do_excute)(struct term_cmd *, kint32_t argc, kchar_t **argv);
    void (*help)(void);

    struct list_head sgtc_link;
};

/*!< History command */
struct term_cmd_his
{
    kuint32_t length;
    struct pq_data sgtc_pqd;

    void *cmd;
};

/*!< Global variable r/w by terminal */
struct term_variable
{
    kchar_t name[32];

    kint32_t *var;
    kusize_t num;
    struct list_head sgtc_link;
};

/*!< Pause key event (Ctrl + C) */
#define TERM_PAUSE_NOTIFIER_CANCEL                      (0x00000001)

/*!< The globals */
BLOCKING_NOTIFIER_DECLARE(sgtc_pause_notifier_chain);

/*!< The functions */
extern struct term_cmd *term_cmd_allocate(const kchar_t *name, nrt_gfp_t gfp_mask);
extern void term_cmd_free(struct term_cmd *sptr_cmd);

extern struct term_cmd *term_cmd_find_by_name(kchar_t *name);
extern kint32_t term_cmd_add(struct term_cmd *sptr_cmd);
extern void term_cmd_del(struct term_cmd *sptr_cmd);

extern struct term_variable *term_variable_find_by_name(kchar_t *name);
extern struct term_variable *term_variable_next(struct term_variable *sptr_var);
extern struct term_variable *term_variable_allocate(const kchar_t *name, kint32_t *var, kusize_t num);
extern void term_variable_destory(struct term_variable *sptr_var);
extern kint32_t term_variable_add(struct term_variable *sptr_var);
extern void term_variable_del(struct term_variable *sptr_var);
extern kint32_t term_variable_add_more(struct term_variable *sptr_var, kusize_t num);
extern void term_variable_del_more(struct term_variable *sptr_var, kusize_t num);
extern kint32_t term_variable_read(kchar_t *name);
extern kint32_t term_variable_write(kchar_t *name, kint32_t value);

extern kchar_t *term_cmdline_get(void);
extern struct pq_queue *term_cmd_queue_get(void);

extern void term_cmd_wrap_line(void);

extern void term_cmdline_excute(kint32_t argc, kchar_t **argv);
extern void term_cmdline_distribute(const kchar_t *cmdline);

extern void term_cmd_print_login(void);
extern void term_cmd_login_init(const kchar_t *login, const kchar_t *host);
extern void term_cmd_set_login(const kchar_t *login);
extern void term_cmd_set_host(const kchar_t *host);

/*!< commands */
extern void term_cmd_add_help(void);
extern void term_cmd_add_info(void);
extern void term_cmd_add_ts(void);
extern void term_cmd_add_ttc(void);
extern void term_cmd_add_user(void);
extern void term_cmd_add_kill(void);
extern void term_cmd_add_history(void);
extern void term_cmd_add_mem(void);
extern void term_cmd_add_runtime(void);
extern void term_cmd_add_value(void);

#ifdef __cplusplus
    }
#endif

#endif /* __TERM_H */
