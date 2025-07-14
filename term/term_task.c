/*
 * Terminal Task Interface
 *
 * File Name:   term_task.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.12.21
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_fcntl.h>
#include <platform/base/fwk_fs.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/instance.h>
#include <kernel/mailbox.h>
#include <term/term.h>

/*!< The defines */
struct term_kbd_priv;

#define TERM_TASK_THREAD_STACK_SIZE             THREAD_STACK_PAGE(1)    /*!< 1 page (4kbytes) */

struct term_kbd_handle
{
    kint32_t key;
    void (*do_cmd) (struct term_kbd_priv *sptr_priv, kuint32_t *inc);
};

struct term_kbd_priv
{  
    kubyte_t *msg;

    kint32_t key;
    kuint8_t len;
};

/*!< The globals */
static tid_t g_term_task_tid;
static struct thread_attr sgtc_term_task_attr;
static THREAD_STACK_DEFINE(g_term_task_stack, TERM_TASK_THREAD_STACK_SIZE);
static struct mailbox sgtc_term_task_mailbox;

static kubyte_t g_term_cmdline[TERM_MSG_RECV_LEN];
static kuint32_t g_term_inc;

static struct pq_queue *sptr_term_cmd_queue;
static kint32_t g_term_cmd_queue_cur = -1;

static kint32_t g_term_fd = -1;

BLOCKING_NOTIFIER_HEAD(sgtc_pause_notifier_chain);

/*!< API functions */
/*!
 * @brief   get cmdline address
 * @param   none
 * @retval  g_term_cmdline
 * @note    none
 */
kchar_t *term_cmdline_get(void)
{
    return (kchar_t *)&g_term_cmdline[0];
}

/*!
 * @brief   get cmdline queue
 * @param   none
 * @retval  sptr_term_cmd_queue
 * @note    none
 */
struct pq_queue *term_cmd_queue_get(void)
{
    return sptr_term_cmd_queue;
}

/*!
 * @brief   to next line
 * @param   none
 * @retval  none
 * @note    none
 */
void term_cmd_wrap_line(void)
{
    io_putstr_async((const kubyte_t *)"\r\n", 3);
}

/*!
 * @brief   queue release API
 * @param   sptr_pqd
 * @retval  none
 * @note    none
 */
static void term_cmd_queue_free(struct pq_data *sptr_pqd)
{
    struct term_cmd_his *sptr_his;

    sptr_his = mr_container_of(sptr_pqd, struct term_cmd_his, sgtc_pqd);
    kfree(sptr_his);
}

/*!
 * @brief   cursor move left
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_cursor_toleft(void)
{
//  io_putstr_async((const kubyte_t *)"\033[1C", 5);
    io_putc(CHAR_ASC_BS);
}

/*!
 * @brief   cursor move right
 * @param   none
 * @retval  none
 * @note    none
 */
static __unused
void term_cursor_toright(void)
{
    io_putstr_async((const kubyte_t *)"\033[1D", 5);
}

/*!
 * @brief   delete one line
 * @param   none
 * @retval  none
 * @note    none
 */
static void term_clear_line(void)
{
    io_putstr_async((const kubyte_t *)"\033[2K\r", 6);
}

/*!
 * @brief   delete all
 * @param   none
 * @retval  none
 * @note    none
 */
static __unused
void term_clear_screen(void)
{
    io_putstr_async((const kubyte_t *)"\033[2J\r", 6);
}

/*!
 * @brief   delete repeat char
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_repeat(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{   
    if ((sptr_priv->key >= CHAR_ASC_SPACE) || 
        (sptr_priv->key < CHAR_ASC_DEL))
    {
        kbyte_t key = (kbyte_t)sptr_priv->key;
        kuint32_t cur_idx;
        kubyte_t *msg;

        cur_idx = *offset;
        msg = sptr_priv->msg;
        if ((*offset) && (msg[cur_idx - 1] == key))
            return;

        msg[cur_idx] = key;
        (*offset)++;
    }
}

/*!
 * @brief   Ctrl + C
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_pause(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{
    kubyte_t *msg = sptr_priv->msg;

    /*!< echo is the first task */
    term_cmd_print_login();

    /*!< notice another command */
    fwk_blocking_notifier_call_chain(
        &sgtc_pause_notifier_chain, TERM_PAUSE_NOTIFIER_CANCEL, mr_nullptr);

    *offset = 0;
    *(msg + *offset) = '\0';
}

/*!
 * @brief   \n
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_enter(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{
    kuint32_t cur_idx = *offset;
    kubyte_t *msg = sptr_priv->msg;
    struct term_cmd_his *sptr_his;
    kuint32_t align_size;

    *(msg + cur_idx) = '\0';
    if (*offset)
    {
        if (*(msg + cur_idx - 1) == CHAR_ASC_SPACE)
            *(msg + cur_idx - 1) = '\0';

        /*!< call function */
        term_cmdline_distribute((const kchar_t *)msg);

        /*!< save current command to history queue */
        if (sptr_term_cmd_queue)
        {
            align_size = mr_align(sizeof(*sptr_his), ARCH_PER_SIZE);

            sptr_his = (struct term_cmd_his *)kmalloc(align_size + (*offset), GFP_KERNEL);
            if (isValid(sptr_his))
            {
                sptr_his->length = *offset;
                sptr_his->cmd = ((void *)sptr_his) + align_size;
                sptr_his->sgtc_pqd.release = term_cmd_queue_free;
                sptr_his->sgtc_pqd.dequeue_chk = mr_nullptr;

                memcpy(sptr_his->cmd, msg, sptr_his->length);
                pq_enqueue(sptr_term_cmd_queue, &sptr_his->sgtc_pqd);
            }
        }
    }

    term_cmd_print_login();
    *offset = 0;
}

/*!
 * @brief   Tab
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_tab(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{

}

/*!
 * @brief   Backspace
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_bs(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{
    kubyte_t *msg = sptr_priv->msg;

    if (*offset)
    {
        /*!< cursor move left */
        term_cursor_toleft();
        /*!< send space to hide (*cursor) */
        io_putc(' ');
        /*!< cursor move left */
        term_cursor_toleft();

        (*offset)--;
        *(msg + *offset) = '\0';
    }
}

/*!
 * @brief   direction-up key
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_dir_up(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{
    kubyte_t *msg = sptr_priv->msg;
    struct term_cmd_his *sptr_his;
    struct pq_data *sptr_pqd;

    if (!sptr_term_cmd_queue)
        return;

    sptr_pqd = pq_lookback(sptr_term_cmd_queue, &g_term_cmd_queue_cur);
    if (!sptr_pqd)
        return;

    sptr_his = mr_container_of(sptr_pqd, struct term_cmd_his, sgtc_pqd);
    memcpy(msg, sptr_his->cmd, sptr_his->length);
    *offset = sptr_his->length;
    *(msg + *offset) = '\0';

    term_clear_line();
//  term_cmd_wrap_line();

    /*!< echo is the first task */
    term_cmd_print_login();

    io_putstr_async(msg, sptr_his->length);
}

/*!
 * @brief   direction-down key
 * @param   sptr_priv, *offset (size of cmdline)
 * @retval  none
 * @note    none
 */
static void term_kbd_dir_down(struct term_kbd_priv *sptr_priv, kuint32_t *offset)
{
    kubyte_t *msg = sptr_priv->msg;
    struct term_cmd_his *sptr_his;
    struct pq_data *sptr_pqd;

    if (!sptr_term_cmd_queue)
        return;

    sptr_pqd = pq_lookfront(sptr_term_cmd_queue, &g_term_cmd_queue_cur);
    if (!sptr_pqd)
        return;

    sptr_his = mr_container_of(sptr_pqd, struct term_cmd_his, sgtc_pqd);
    memcpy(msg, sptr_his->cmd, sptr_his->length);
    *offset = sptr_his->length;
    *(msg + *offset) = '\0';

    term_clear_line();
//  term_cmd_wrap_line();

    /*!< echo is the first task */
    term_cmd_print_login();

    io_putstr_async(msg, sptr_his->length);
}

/*!< super key */
static struct term_kbd_handle sgtc_term_kbd_handles[] =
{
    { CHAR_ASC_SPACE, term_kbd_repeat },                /*!< space */

    { CHAR_ASC_CR, term_kbd_enter },                    /*!< enter */
    { CHAR_ASC_ETX, term_kbd_pause },                   /*!< ctrl + c */
    { CHAR_ASC_BS, term_kbd_bs },                       /*!< backspace (ctrl + b) */
    { CHAR_ASC_DEL, term_kbd_bs },                      /*!< delete */
    { CHAR_ASC_HT, term_kbd_tab },                      /*!< tab */

    { ANSI_ASC_UP, term_kbd_dir_up },
    { ANSI_ASC_DOWN, term_kbd_dir_down },
};

/*!
 * @brief   send char that received just now
 * @param   msg
 * @retval  none
 * @note    none
 */
static void term_echo(kint32_t msg)
{
    switch (msg)
    {
        case CHAR_ASC_SPACE ... CHAR_ASC_TILDE:
            io_putc((const kbyte_t)msg);
            break;

        case CHAR_ASC_CR:
        case CHAR_ASC_ETX:
            term_cmd_wrap_line();
            g_term_cmd_queue_cur = -1;
            break;

        case ANSI_ASC_UP:
            break;

        default: break;
    }
}

/*!
 * @brief   save input char to cmdline[]
 * @param   msg, len
 * @retval  none
 * @note    none
 */
static void term_cmdline(kint32_t msg, kuint8_t len)
{
    struct term_kbd_handle *sptr_kbdh;
    kuint32_t num, idx;

    /*!< exclude ' ' */
    if ((msg > CHAR_ASC_SPACE) && 
        (msg < CHAR_ASC_DEL))
    {
        g_term_cmdline[g_term_inc] = (kubyte_t)msg;
        g_term_inc++;

        return;
    }

    sptr_kbdh = &sgtc_term_kbd_handles[0];
    num = ARRAY_SIZE(sgtc_term_kbd_handles);

    for (idx = 0; idx < num; idx++)
    {
        if ((msg == sptr_kbdh[idx].key) &&
            (sptr_kbdh[idx].do_cmd))
        {
            struct term_kbd_priv sgtc_priv;

            sgtc_priv.msg = &g_term_cmdline[0];
            sgtc_priv.key = msg;
            sgtc_priv.len = len;

            sptr_kbdh[idx].do_cmd(&sgtc_priv, &g_term_inc);
            return;
        }
    }
}

/*!< Terminal command defines */
static const term_cmd_fn_t g_term_cmd_fn[] =
{
    term_cmd_add_help,

    term_cmd_add_info,
    term_cmd_add_ts,
    term_cmd_add_ttc,
    term_cmd_add_user,
    term_cmd_add_kill,
    term_cmd_add_history,
    term_cmd_add_mem,
    term_cmd_add_runtime,

    mr_nullptr,
};

/*!
 * @brief   call per cmd initialization
 * @param   term_cmd_fn[]
 * @retval  none
 * @note    none
 */
static void term_new_command(const term_cmd_fn_t term_cmd_fn[])
{
    const term_cmd_fn_t *fn;

    for (fn = term_cmd_fn; (*fn); fn++)
        (*fn)();
}

/*!< --------------------------------------------------------------------- */
/*!
 * @brief   putc
 * @param   none
 * @retval  none
 * @note    send character
 */
static void term_putc(const kubyte_t ch)
{
    virt_write(g_term_fd, &ch, 1);
}

/*!
 * @brief   putstr
 * @param   none
 * @retval  none
 * @note    printk typedef
 */
static void term_putstr(const kubyte_t *msgs, kusize_t size)
{
    virt_write(g_term_fd, msgs, size);
}

/*!
 * @brief   getc
 * @param   none
 * @retval  none
 * @note    get character
 */
static kubyte_t term_getc(kubyte_t *ch)
{
    kssize_t size;

    size = virt_read(g_term_fd, &ch, 1);
    return (size > 0) ? (kubyte_t)size : 0;
}

/*!
 * @brief   getstr
 * @param   string
 * @retval  none
 * @note    string read
 */
static kssize_t term_getstr(kubyte_t *msgs, kusize_t size)
{
    return virt_read(g_term_fd, msgs, size);
}

/*!< io stream reality */
static struct io_stream_dev sgtc_term_io_stream =
{
    .name = CONFIG_CONSOLE_DEVICE,

    ._putc = term_putc,
    ._putstr = term_putstr,
    ._getc = term_getc,
    ._getstr = term_getstr,
};

/*!
 * @brief   term task main
 * @param   args
 * @retval  args
 * @note    none
 */
static void *term_entry(void *args)
{
    struct mailbox *sptr_mb = &sgtc_term_task_mailbox;
    kint32_t msg;
    kssize_t length;

    term_cmd_login_init(CONFIG_DEFAULT_LOGIN, CONFIG_DEFAULT_HOST);

    printk("\r\n");
    printk("Press Enter and use HeavenFox now\r\n");
    printk("\r\n");
    printk("%s", g_term_cmdline);

    mailbox_init(sptr_mb, mr_current->tid, "term-task-mailbox");

    /*!< system command */
    term_new_command(g_term_cmd_fn);

    /*!< create ring buffer to save history commands */
    sptr_term_cmd_queue = pq_queue_create(NR_PQ_RING, TERM_MSG_RECV_LEN);

    for (;;)
    {
        msg = 0;

        /*!< 
         * Each character entered by the Terminal will be immediately sent to receiver,
         * even if you do not press Enter Key;
         * As a receiver, the character needs to be read immediately.
         */
        length = io_getstr((kubyte_t *)&msg, sizeof(msg));
        if (length <= 0)
        {
            schedule_thread();
            continue;
        }

        term_echo(msg);
        term_cmdline(msg, (kuint8_t)length);
    }

    return args;
}

/*!
 * @brief   term task create
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t term_init(void)
{
    struct thread_attr *sptr_attr = &sgtc_term_task_attr;

    /*!< Open console device */
    g_term_fd = virt_open(CONFIG_CONSOLE_DEVICE, O_RDWR);
    if (g_term_fd < 0)
    {
        g_term_fd = -1;
        print_err("Open console device \"%s\" failed!\r\n", CONFIG_CONSOLE_DEVICE);
    }
    else
    {
        struct io_stream_dev *sprt_dev;
        struct io_stream_dev *sptr_stream = &sgtc_term_io_stream;

        /*!< Register IO Stream */
        init_list_head(&sptr_stream->sgtc_link);
        sptr_stream->is_opened = true;

        /*!< Existed? Unregister first */
        sprt_dev = find_io_stream_dev((const kchar_t *)sptr_stream->name);
        if (sprt_dev)
            unregister_io_stream(sprt_dev);

        if (register_io_stream(sptr_stream))
            return -ER_ERROR;
    }

    g_io_stream_flags |= IO_STREAM_ASYNC;

    sptr_attr->detachstate = THREAD_CREATE_JOINABLE;
    sptr_attr->inheritsched	= THREAD_INHERIT_SCHED;
    sptr_attr->schedpolicy = THREAD_SCHED_FIFO;

    /*!< thread stack */
    thread_set_stack(sptr_attr, mr_nullptr, g_term_task_stack, sizeof(g_term_task_stack));
    /*!< lowest priority */
    thread_set_priority(sptr_attr, THREAD_PROTY_TERM);
    /*!< default time slice */
    thread_set_time_slice(sptr_attr, THREAD_TIME_DEFUALT);

    /*!< register thread */
    g_term_task_tid = kernel_thread_create(-1, sptr_attr, term_entry, mr_nullptr);
    if (g_term_task_tid >= 0)
    {
        thread_set_name(g_term_task_tid, "terminal");
        return ER_NORMAL;
    }

    return -ER_FAILD;
}
