/*
 * Terminal Core API: Command history
 *
 * File Name:   history.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.03.09
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_basic.h>
#include <term/term.h>

/*!< The defines */


/*!< The globals */


/*!< The functions */

/*!< API functions */
/*!
 * @brief   cmd 'history': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_history(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    struct term_cmd_his *sptr_his;
    struct pq_data *sptr_pqd;
    struct pq_queue *sptr_pq = term_cmd_queue_get();
    kint32_t queue_cur = -1;
    kuint8_t cur_index[16];
    kuint32_t count, size;

    switch (argc)
    {
        case 1:
            count = 0;

            while ((sptr_pqd = pq_lookback(sptr_pq, &queue_cur)))
            {
                sptr_his = mr_container_of(sptr_pqd, struct term_cmd_his, sgtc_pqd);

                size = uint_to_str(cur_index, ++count);
                cur_index[size] = '.';
                cur_index[size + 1] = ' ';
                cur_index[size + 2] = '\0';

                io_putstr_async(cur_index, size + 2);
                io_putstr_async(sptr_his->cmd, sptr_his->length);
                term_cmd_wrap_line();
            }

            break;

        case 2:
            if (!strcmp(argv[1], "--help"))
                sptr_cmd->help();
            else
                goto fail;

            break;

        default: 
            goto fail;
    }

    return ER_NORMAL;

fail:
    printk("argument error, try entering \'%s --help\' to get usage\r\n", argv[0]);
    return -ER_FAULT;
}

/*!
 * @brief   cmd 'history': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_history_help(void)
{
    printk("usage: history\r\n");
}

/*!
 * @brief   cmd 'history' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_history(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("history", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_show_history;
    sptr_cmd->help = term_cmd_history_help;

    term_cmd_add(sptr_cmd);
}
