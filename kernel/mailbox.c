/*
 * Kernel MailBox Interface
 *
 * File Name:   mailbox.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.05.25
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The globals */
#include <common/basic_types.h>
#include <common/error_types.h>
#include <common/generic.h>
#include <common/io_stream.h>
#include <kernel/kernel.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/sleep.h>
#include <kernel/mutex.h>
#include <kernel/mailbox.h>

/*!< The defines */

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_kernel_mailboxs);

/*!< API functions */
/*!
 * @brief   find mailbox which named "name"
 * @param   name
 * @retval  sptr_mb
 * @note    none
 */
struct mailbox *mailbox_find(const kchar_t *name)
{
    struct mailbox *sptr_mb;

    if (!name || !(*name))
        return mr_nullptr;

    foreach_list_next_entry(sptr_mb, &sgtc_kernel_mailboxs, sgtc_link)
    {
        if (!kstrcmp(sptr_mb->name, name))
            return sptr_mb;
    }

    return mr_nullptr;
}

/*!
 * @brief   add new mailbox to global list
 * @param   sptr_mb
 * @retval  none
 * @note    none
 */
void mailbox_insert(struct mailbox *sptr_mb)
{
    struct mailbox *sptr_box;
    struct list_head *sptr_last = mr_nullptr;

    if (mr_list_head_empty(&sgtc_kernel_mailboxs))
        goto END;

    foreach_list_next_entry(sptr_box, &sgtc_kernel_mailboxs, sgtc_link)
    {
        if (sptr_box->tid > sptr_mb->tid)
            break;

        sptr_last = &sptr_box->sgtc_link;
    }

    if (!sptr_last)
        goto END;
    else
        list_head_add_tail(sptr_last, &sptr_mb->sgtc_link);

    return;

END:
    list_head_add_tail(&sgtc_kernel_mailboxs, &sptr_mb->sgtc_link);
}

/*!
 * @brief   initial mailbox
 * @param   sptr_mb, tid, name
 * @retval  none
 * @note    none
 */
kint32_t mailbox_init(struct mailbox *sptr_mb, tid_t tid, const kchar_t *name)
{
    struct thread *sptr_thread;
    struct mailbox *sptr_box;
    kchar_t label[MAILBOX_NAME_LEN] = {};

    sptr_thread = get_thread_handle(tid);
    if (sptr_thread->sptr_mb)
        return -ER_FORBID;

    if (name && (*name != '\0'))
        kstrlcpy(label, name, MAILBOX_NAME_LEN);
    else
        sprintk(label, "mailbox-tid-%d", tid);

    /*!< if name is registered */
    sptr_box = mailbox_find(label);
    if (sptr_box)
        return -ER_EXISTED;

    sptr_mb->num_mails = 0;
    sptr_mb->tid = tid;
    kstrlcpy(sptr_mb->name, label, MAILBOX_NAME_LEN);
    init_list_head(&sptr_mb->sgtc_mail);
    mutex_init(&sptr_mb->sgtc_lock);
    
    mailbox_insert(sptr_mb);
    sptr_thread->sptr_mb = sptr_mb;

    return ER_NORMAL;
}

/*!
 * @brief   delete mailbox
 * @param   sptr_mb
 * @retval  none
 * @note    none
 */
void mailbox_deinit(struct mailbox *sptr_mb)
{
    struct thread *sptr_thread;

    sptr_thread = get_thread_handle(sptr_mb->tid);
    sptr_thread->sptr_mb = mr_nullptr;

    list_head_del(&sptr_mb->sgtc_link);
    mutex_init(&sptr_mb->sgtc_lock);
}

/*!
 * @brief   create mailbox
 * @param   tid, name
 * @retval  mailbox created
 * @note    none
 */
struct mailbox *mailbox_create(tid_t tid, const kchar_t *name)
{
    struct mailbox *sptr_mb;

    sptr_mb = kmalloc(sizeof(*sptr_mb), GFP_KERNEL);
    if (!isValid(sptr_mb))
        return sptr_mb;

    if (mailbox_init(sptr_mb, tid, name))
    {
        kfree(sptr_mb);
        return ERR_PTR(-ER_FAILD);
    }

    return sptr_mb;
}

/*!
 * @brief   destroy mailbox
 * @param   sptr_mb
 * @retval  none
 * @note    none
 */
void mailbox_destroy(struct mailbox *sptr_mb)
{
    if (mr_unlikely(!sptr_mb))
        return;

    mailbox_deinit(sptr_mb);
    kfree(sptr_mb);
}

/*!< ------------------------------------------------------------- */
/*!
 * @brief   initialize mail
 * @param   sptr_mb, sptr_mail
 * @retval  none
 * @note    none
 */
void mail_init(struct mailbox *sptr_mb, struct mail *sptr_mail)
{
    if (mr_unlikely(!sptr_mb) || 
        mr_unlikely(!sptr_mail))
        return;

    memset(sptr_mail, 0, sizeof(*sptr_mail));
    sptr_mail->src_name = sptr_mb->name;
    mutex_init(&sptr_mail->sgtc_lock);
    init_list_head(&sptr_mail->sgtc_link);
}

/*!
 * @brief   create mail
 * @param   sptr_mb
 * @retval  mail created
 * @note    none
 */
struct mail *mail_create(struct mailbox *sptr_mb)
{
    struct mail *sptr_mail;

    sptr_mail = kmalloc(sizeof(*sptr_mail), GFP_KERNEL);
    if (!isValid(sptr_mail))
        return sptr_mail;

    mail_init(sptr_mb, sptr_mail);
    return sptr_mail;
}

/*!
 * @brief   destroy mail
 * @param   sptr_mb, sptr_mail
 * @retval  none
 * @note    none
 */
void mail_destroy(struct mailbox *sptr_mb, struct mail *sptr_mail)
{
    if (mr_unlikely(!sptr_mail))
        return;

    if (sptr_mb && !strcmp(sptr_mb->name, sptr_mail->src_name))
        kfree(sptr_mail);
}

/*!
 * @brief   send mail
 * @param   mb_name, sptr_mail
 * @retval  errno
 * @note    none
 */
kint32_t mail_send(const kchar_t *mb_name, struct mail *sptr_mail)
{
    struct mailbox *sptr_mb;
    struct mail *sptr_to;
    struct mail_msg *sptr_msg;
    kuint8_t *buffer;
    kuint32_t msg_idx;
    kusize_t msg_size, size;

    if (mr_unlikely(!sptr_mail) || mr_unlikely(!sptr_mail->sptr_msg))
        return -ER_NOMEM;

    sptr_mb = mailbox_find(mb_name);
    if (!isValid(sptr_mb))
        return PTR_ERR(sptr_mb);

    size = sizeof(*sptr_mail);
    size = mr_align(size, 8);
    msg_size = sptr_mail->num_msgs * sizeof(*sptr_mail->sptr_msg);

    sptr_to = kzalloc(size + msg_size, GFP_KERNEL);
    if (!isValid(sptr_to))
        return PTR_ERR(sptr_to);

    /*!< copy to new */
    sptr_msg = (struct mail_msg *)((kuint8_t *)sptr_to + size);
    for (msg_idx = 0; msg_idx < sptr_mail->num_msgs; msg_idx++)
    {
        buffer = kcalloc(sizeof(*buffer), sptr_mail->sptr_msg[msg_idx].size, GFP_KERNEL);
        if (!isValid(buffer))
            goto fail;
        
        memcpy(&sptr_msg[msg_idx], &sptr_mail->sptr_msg[msg_idx], sizeof(*sptr_mail->sptr_msg));
        memcpy(buffer, sptr_msg[msg_idx].buffer, sizeof(*buffer) * sptr_msg[msg_idx].size);
        sptr_msg[msg_idx].buffer = buffer;           
    }

    sptr_to->num_msgs = sptr_mail->num_msgs;
    sptr_to->sptr_msg = sptr_msg;
    sptr_to->src_name = sptr_mail->src_name;
    sptr_to->status = NR_MAIL_NONE;

    mutex_init(&sptr_to->sgtc_lock);
    init_list_head(&sptr_to->sgtc_link);

    mutex_lock(&sptr_mb->sgtc_lock);
    list_head_add_tail(&sptr_mb->sgtc_mail, &sptr_to->sgtc_link);
    sptr_mb->num_mails++;
    mutex_unlock(&sptr_mb->sgtc_lock);

    return ER_NORMAL;

fail:
    while (msg_idx)
        kfree(sptr_msg[--msg_idx].buffer);

    kfree(sptr_to);
    return ER_FAILD;
}

/*!
 * @brief   recieve mail
 * @param   sptr_mb, sptr_mail, timeout
 * @retval  errno
 * @note    none
 */
struct mail *mail_recv(struct mailbox *sptr_mb, kutime_t timeout)
{
    struct mail *sptr_recv = mr_nullptr;

    if (mr_unlikely(!sptr_mb))
        return ERR_PTR(-ER_NOMEM);

    while (mr_list_head_empty(&sptr_mb->sgtc_mail))
    {
        if (!timeout)
            return ERR_PTR(-ER_EMPTY);

        msleep(timeout);
    }

    /*!< get each mail */
    mutex_lock(&sptr_mb->sgtc_lock);
    sptr_recv = mr_list_first_entry(&sptr_mb->sgtc_mail, typeof(*sptr_recv), sgtc_link);
    if (sptr_recv->num_msgs > 1)
        mr_nop();
    
    /*!< make sure that sptr_recv is still valid */
    if (mr_list_head_empty(&sptr_recv->sgtc_link))
    {
        mutex_unlock(&sptr_mb->sgtc_lock);
        return ERR_PTR(-ER_EMPTY);
    }

    sptr_mb->num_mails--;
    list_head_del(&sptr_recv->sgtc_link);
    mutex_unlock(&sptr_mb->sgtc_lock);

    return sptr_recv;
}

/*!
 * @brief   deal with the aftermath
 * @param   sptr_mail
 * @retval  none
 * @note    none
 */
void mail_recv_finish(struct mail *sptr_mail)
{
    kuint32_t idx;

    if (sptr_mail->sptr_msg)
    {
        for (idx = 0; idx < sptr_mail->num_msgs; idx++)
        {
            if (sptr_mail->sptr_msg[idx].buffer)
                kfree(sptr_mail->sptr_msg[idx].buffer);
        }
    }

    kfree(sptr_mail);
}

/*!< end of file */
