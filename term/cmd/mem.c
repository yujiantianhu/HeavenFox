/*
 * Terminal Core API: Command mem
 *
 * File Name:   mem.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.07
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
 * @brief   cmd 'mem': excute function
 * @param   sptr_cmd, argc, argv
 * @retval  errno
 * @note    none
 */
static kint32_t term_cmd_mem(struct term_cmd *sptr_cmd, kint32_t argc, kchar_t **argv)
{
    switch (argc)
    {
        case 1:
            break;

        case 2:
            if (!kstrcmp(argv[1], "--help"))
                sptr_cmd->help();
            else if (!kstrcmp(argv[1], "info"))
            {
                struct m_area *sptr_record, sgtc_record;
                struct m_area sgtc_area;
                kchar_t *name;

                printk("Memory Pool:\r\n");

                for (kint32_t area_index = 0; area_index < NR_FWK_MEMPOOL_TYPE_MAX; area_index++)
                {
                    name = (kchar_t *)kmget_area_label(area_index);
                    sgtc_area.base = (void *)kmget_area_base_address(area_index);
                    sgtc_area.size = kmget_area_total_size(area_index);
                    sptr_record = kmget_area_record(area_index);

                    memcpy(&sgtc_record, sptr_record, sizeof(sgtc_record));
                    printk("    \"%-16s\": base = 0x%08p, size = %8u(KB); max allocted = 0x%08p, size = %8u(B)\r\n",
                            name, sgtc_area.base, __BYTES_TO_KB(sgtc_area.size), sgtc_record.base, sgtc_record.size);
                }
            }
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
 * @brief   cmd 'mem': help function
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cmd_mem_help(void)
{
    printk("usage: mem [info]\r\n");
}

/*!
 * @brief   cmd 'mem' init and add
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_add_mem(void)
{
    struct term_cmd *sptr_cmd;

    sptr_cmd = term_cmd_allocate("mem", GFP_KERNEL);
    if (!isValid(sptr_cmd))
        return;

    sptr_cmd->do_excute = term_cmd_mem;
    sptr_cmd->help = term_cmd_mem_help;

    term_cmd_add(sptr_cmd);
}
