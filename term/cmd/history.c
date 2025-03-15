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
 * @param   sprt_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_show_history(struct term_cmd *sprt_cmd, kint32_t argc, kchar_t **argv)
{
    struct term_cmd_his *sprt_his;
    struct pq_data *sprt_pqd;
    struct pq_queue *sprt_pq = term_cmd_queue_get();
    kint32_t queue_cur = -1;
    kuint8_t cur_index[16];
    kuint32_t count, size;

    switch (argc)
    {
        case 1:
            count = 0;

            while ((sprt_pqd = pq_lookback(sprt_pq, &queue_cur)))
            {
                sprt_his = mrt_container_of(sprt_pqd, struct term_cmd_his, sgrt_pqd);

                size = convert_number_to_string(cur_index, ++count);
                cur_index[size] = '.';
                cur_index[size + 1] = ' ';
                cur_index[size + 2] = '\0';

                io_putstr(cur_index, size + 2);
                io_putstr(sprt_his->cmd, sprt_his->length);
                term_cmd_wrap_line();
            }

            break;

        case 2:
            if (!strcmp(argv[1], "--help"))
                sprt_cmd->help();
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
    struct term_cmd *sprt_cmd;

    sprt_cmd = term_cmd_allocate("history", GFP_KERNEL);
    if (!isValid(sprt_cmd))
        return;

    sprt_cmd->do_excute = term_cmd_show_history;
    sprt_cmd->help = term_cmd_history_help;

    term_cmd_add(sprt_cmd);
}
