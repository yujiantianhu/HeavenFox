/*
 * Platform Object Defines
 *
 * File Name:   fwk_kboject.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.06.10
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/fwk_kobj.h>
#include <platform/fwk_inode.h>
#include <kernel/spinlock.h>

/*!< The globals */
static struct fwk_kset sgtc_fwk_kset_root;
static kbool_t is_fwk_root_existed = false;

/*!< API function */
/*!
 * @brief   join the new kobject to kset
 * @param   sptr_kobj
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_kobject_join_to_kset(struct fwk_kobject *sptr_kobj)
{
    struct fwk_kobject *sptr_parent;
    struct fwk_kobject *sptr_each;
    kint32_t retval;

    if (!sptr_kobj->sptr_kset)
        return -ER_FAULT;

    spin_lock(&sptr_kobj->sgtc_lock);
    
    foreach_list_next_entry(sptr_each, &sptr_kobj->sptr_kset->sgtc_list, sgtc_link)
    {
        if (!sptr_each->name && !sptr_kobj->name)
        {
            retval = -ER_EMPTY;
            goto fail;
        }

        if (!kstrcmp(sptr_kobj->name, sptr_each->name))
        {
            retval = -ER_EXISTED;
            goto fail;
        }
    }
    
    sptr_parent = sptr_kobj->sptr_parent;
    if (!sptr_parent)
        sptr_parent = &sptr_kobj->sptr_kset->sgtc_kobj;
    
    list_head_add_tail(&sptr_kobj->sptr_kset->sgtc_list, &sptr_kobj->sgtc_link);
    sptr_kobj->sptr_parent = sptr_parent;
    spin_unlock(&sptr_kobj->sgtc_lock);

    return ER_NORMAL;

fail:
    spin_unlock(&sptr_kobj->sgtc_lock);
    return retval;
}

/*!
 * @brief   delete the kobject from kset
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
static void fwk_kobject_detach_from_kset(struct fwk_kobject *sptr_kobj)
{
    if (!sptr_kobj->sptr_kset)
        return;

    spin_lock(&sptr_kobj->sgtc_lock);
    list_head_del_safe(&sptr_kobj->sptr_kset->sgtc_list, &sptr_kobj->sgtc_link);
    spin_unlock(&sptr_kobj->sgtc_lock);
}

/*!
 * @brief   join the new kobject to kset, and create inode
 * @param   sptr_kobj
 * @retval  errno
 * @note    none
 */
static kint32_t fwk_kobject_build_inode(struct fwk_kobject *sptr_kobj)
{
    struct fwk_inode *sptr_inode;
    kint32_t retval;

    retval = fwk_kobject_join_to_kset(sptr_kobj);
    if (retval)
        return retval;

    sptr_inode = fwk_mk_inode(sptr_kobj, NR_TYPE_NONE, -1);
    if (!isValid(sptr_inode))
    {
        fwk_kobject_detach_from_kset(sptr_kobj);
        return PTR_ERR(sptr_inode);
    }

    sptr_kobj->sptr_inode = sptr_inode;

    return ER_NORMAL;
}

/*!
 * @brief   initialize kobject
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kobject_init(struct fwk_kobject *sptr_kobj)
{
    fwk_kref_init(&sptr_kobj->sgtc_ref);
    init_list_head(&sptr_kobj->sgtc_link);
    spin_lock_init(&sptr_kobj->sgtc_lock);
    sptr_kobj->is_dir = false;
    sptr_kobj->sptr_kset = mr_nullptr;
}

/*!
 * @brief   create a new kobject
 * @param   none
 * @retval  kobject
 * @note    none
 */
struct fwk_kobject *fwk_kobject_create(void)
{
    struct fwk_kobject *sptr_kobj;

    sptr_kobj = kzalloc(sizeof(*sptr_kobj), GFP_KERNEL);
    if (!isValid(sptr_kobj))
        return mr_nullptr;

    fwk_kobject_init(sptr_kobj);

    return sptr_kobj;
}

/*!
 * @brief   add a new kobject
 * @param   sptr_kobj, sptr_parent, fmt (name)
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kobject_add(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, ...)
{
    va_list sptr_list;
    kint32_t retval;

    if (!sptr_kobj || !is_fwk_root_existed)
        return -ER_NOMEM;

    sptr_kobj->sptr_parent = sptr_parent;

    va_start(sptr_list, fmt);
    fwk_kobject_set_name_args(sptr_kobj, fmt, sptr_list);
    va_end(sptr_list);

    if (!sptr_kobj->sptr_kset)
        sptr_kobj->sptr_kset = &sgtc_fwk_kset_root;

    retval = fwk_kobject_build_inode(sptr_kobj);
    if (retval)
        fwk_kobject_del_name(sptr_kobj);

    return retval;
}

/*!
 * @brief   add a new kobject
 * @param   sptr_kobj, sptr_parent, fmt (unformatted name)
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kobject_add_vargs(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, va_list sptr_list)
{
    kint32_t retval;

    if (!sptr_kobj || !is_fwk_root_existed)
        return -ER_NOMEM;

    sptr_kobj->sptr_parent = sptr_parent;
    fwk_kobject_set_name_args(sptr_kobj, fmt, sptr_list);

    if (!sptr_kobj->sptr_kset)
        sptr_kobj->sptr_kset = &sgtc_fwk_kset_root;

    retval = fwk_kobject_build_inode(sptr_kobj);
    if (retval)
        fwk_kobject_del_name(sptr_kobj);

    return retval;
}

/*!
 * @brief   delete kobject
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kobject_del(struct fwk_kobject *sptr_kobj)
{
    fwk_rm_inode(sptr_kobj->sptr_inode);
    fwk_kobject_del_name(sptr_kobj);
    fwk_kobject_detach_from_kset(sptr_kobj);

    sptr_kobj->sptr_inode = mr_nullptr;
}

/*!
 * @brief   release kobject
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kobject_destroy(struct fwk_kobject *sptr_kobj)
{
    fwk_kobject_del(sptr_kobj);
    kfree(sptr_kobj);
}

/*!
 * @brief   create directory or file
 * @param   sptr_parent, name
 * @retval  kobject created
 * @note    dir: the format of name is "xxx/xxx", such as "dev/"; file: '/' should be moved, such as "dev"
 */
static struct fwk_kobject *__fwk_kobject_populate_dir(struct fwk_kobject *sptr_parent, const kchar_t *name)
{
    struct fwk_kset *sptr_kset;
    struct fwk_kobject *sptr_kobj;
    kchar_t *new_name;
    kuint32_t lenth;
    
    if (!sptr_parent)
        sptr_parent = &sgtc_fwk_kset_root.sgtc_kobj;
    new_name = kstrchr(name, '/');
    
    if (new_name)
    {
        sptr_kset = fwk_kset_create(name, sptr_parent);
        if (!isValid(sptr_kset))
            goto fail;
        
        sptr_kobj = &sptr_kset->sgtc_kobj;

        lenth = (kuint32_t)(new_name - name);
        fwk_kobject_del_name(sptr_kobj);
        
        new_name = kmalloc(lenth + 1, GFP_KERNEL);
        if (!isValid(new_name))
            goto dir_fail;
        
        kstrlcpy(new_name, name, lenth + 1);
        sptr_kobj->name = new_name;

        sptr_kobj->sptr_kset = mr_fwk_kset_get(sptr_parent);
        if (!sptr_kobj->sptr_kset || fwk_kset_register(sptr_kset))
            goto dir_fail;
        
        goto pass;
        
dir_fail:
        fwk_kobject_del(sptr_kobj);
        kfree(sptr_kset);
        goto fail;
    }
    
    sptr_kobj = fwk_kobject_create();
    if (!isValid(sptr_kobj))
        goto fail;
    
    sptr_kobj->sptr_kset = mr_fwk_kset_get(sptr_parent);
    if (!sptr_kobj->sptr_kset || fwk_kobject_add(sptr_kobj, sptr_parent, name))
    {
        kfree(sptr_kobj);
        goto fail;
    }
    
pass:
    return sptr_kobj;
    
fail:
    return mr_nullptr;
}

/*!
 * @brief   populate directory or file
 * @param   sptr_head, name
 * @retval  kobject created
 * @note    dir: the format of name is "xxx/xxx", such as "dev/"; file: '/' should be moved, such as "dev"
 */
struct fwk_kobject *fwk_kobject_populate(struct fwk_kobject *sptr_head, const kchar_t *name)
{
    kchar_t *str_start, *str_end;
    kusize_t lenth;
    struct fwk_kobject *sptr_kobj;
    struct fwk_kset *sptr_kset;
    kbool_t found, is_root;

    if (!is_fwk_root_existed)
        return ERR_PTR(-ER_FORBID);

    sptr_kobj = sptr_head;
    sptr_kset = sptr_kobj ? mr_fwk_kset_get(sptr_kobj) : &sgtc_fwk_kset_root;
    if (!sptr_kset)
        return ERR_PTR(-ER_NOMEM);

    is_root = (sptr_kset == (&sgtc_fwk_kset_root));
    str_start = str_end = (kchar_t *)name;

    if (is_root)
    {
        if (*str_start != '/')
            return ERR_PTR(-ER_FAULT);

        if (*(str_start + 1) == '\0')
            return ERR_PTR(-ER_FAULT);
    }

    /*!< for example: 123/yyx.txt, or 123/, or /123/, ... */
    while (str_end && (*str_end != '\0'))
    {
        str_start = is_root ? (str_end + 1) : str_end;
        if (*str_start == '\0')
            return sptr_kobj;
        
        /*!< if *(str_end + 1) == '/', the format of name maybe "//" */
        if (*str_start == '/')
            return ERR_PTR(-ER_FAULT);

        /*!< if '/' can be found, str_start is a directory */
        str_end = kstrchr(str_start, '/');
        is_root = true;
        found = false;
        lenth = str_end ? (kusize_t)(str_end - str_start) : kstrlen(str_start);

        spin_lock(&sptr_kset->sgtc_kobj.sgtc_lock);
        foreach_list_next_entry(sptr_kobj, &sptr_kset->sgtc_list, sgtc_link)
        {
            if (!sptr_kobj->name)
                continue;

            if (!kstrncmp(str_start, sptr_kobj->name, lenth))
            {
                /*!< str_end ? directory : file; if file is existed, file is repeated */
                if (!str_end)
                {
                    if (!sptr_kobj->is_dir)
                    {
                        spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);
                        return ERR_PTR(-ER_EXISTED);
                    }
                    
                    continue;
                }
                
                /*!< otherwise, it is a directory */
                /*!< it is possible that file and directory have the same name */
                if (!sptr_kobj->is_dir)
                    continue;

                spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);
                
                found = true;
                sptr_kset = mr_fwk_kset_get(sptr_kobj);
                goto out;
            }
        }

        spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);

out:
        if (!found)
        {
            sptr_kobj = __fwk_kobject_populate_dir(&sptr_kset->sgtc_kobj, str_start);
            if (!isValid(sptr_kobj))
                goto fail;
            
            sptr_kset = mr_fwk_kset_get(sptr_kobj);
        }
    }
    
    return sptr_kobj;

fail:
    return ERR_PTR(-ER_ERROR);
}

/*!
 * @brief   find kobject by directory path (name)
 * @param   sptr_head, name
 * @retval  kobject found
 * @note    dir: the format of name is "xxx/xxx", such as "dev/"; file: '/' should be moved, such as "dev"
 */
struct fwk_kobject *fwk_find_kobject_by_path(struct fwk_kobject *sptr_head, const kchar_t *name)
{
    kchar_t *str_start, *str_end;
    kusize_t lenth;
    struct fwk_kobject *sptr_kobj;
    struct fwk_kset *sptr_kset;
    kbool_t found, is_root;

    sptr_kset = sptr_head ? mr_fwk_kset_get(sptr_head) : &sgtc_fwk_kset_root;
    if (!sptr_kset || !is_fwk_root_existed)
        return mr_nullptr;

    is_root = (sptr_kset == (&sgtc_fwk_kset_root));
    str_start = str_end = (kchar_t *)name;

    if (is_root)
    {
        if (*str_start != '/')
            return mr_nullptr;

        if (*(str_start + 1) == '\0')
            return &sptr_kset->sgtc_kobj;
    }

    /*!< for example: 123/yyx.txt, or 123/, or /123/, ... */
    while (str_end && (*str_end != '\0'))
    {
        /*!< (*str_end) must be '/', if *(str_end + 1) is also '/', error occurred */
        str_start = is_root ? (str_end + 1) : str_end;
        if (*str_start == '\0')
            return &sptr_kset->sgtc_kobj;
        
        /*!< if *(str_end + 1) == '/', the format of name maybe "//" */
        if (*str_start == '/')
            break;

        /*!< if '/' can be found, str_start is a directory */
        str_end = kstrchr(str_start, '/');
        is_root = true;
        found = false;
        lenth = str_end ? (kusize_t)(str_end - str_start) : kstrlen(str_start);

        spin_lock(&sptr_kset->sgtc_kobj.sgtc_lock);

        foreach_list_next_entry(sptr_kobj, &sptr_kset->sgtc_list, sgtc_link)
        {
            if (!sptr_kobj->name)
                continue;

            if (!kstrncmp(str_start, sptr_kobj->name, lenth))
            {               
                /*!< str_end ? directory : file; if file is existed, file is repeated */
                if (!str_end)
                {
                    if (!sptr_kobj->is_dir)
                    {
                        spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);
                        return sptr_kobj;
                    }
                    
                    continue;
                }
                
                /*!< otherwise, it is a directory */
                /*!< it is possible that file and directory have the same name */
                if (!sptr_kobj->is_dir)
                    continue;

                spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);
                
                found = true;
                sptr_kset = mr_fwk_kset_get(sptr_kobj);
                goto out;
            }
        }

        spin_unlock(&sptr_kset->sgtc_kobj.sgtc_lock);

out:
        if (!found || !sptr_kset)
            break;
    }

    return mr_nullptr;
}

/*!
 * @brief   set kobject's name (unformatted)
 * @param   sptr_kobj, fmt
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kobject_set_name_args(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, va_list sptr_list)
{
    kchar_t *ptr;
    
    /*!< name is already defined */
    if (sptr_kobj->name && !fmt)
        return ER_NORMAL;

    ptr = vasprintk_safe(fmt, mr_nullptr, sptr_list);
    if (!isValid(ptr))
        return -ER_NOMEM;

    if (sptr_kobj->name)
        kfree(sptr_kobj->name);

    sptr_kobj->name = ptr;
    return ER_NORMAL;
}

/*!
 * @brief   set kobject's name (unformatted)
 * @param   sptr_kobj, fmt
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kobject_set_name(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...)
{
    va_list sptr_list;
    kint32_t retval;

    va_start(sptr_list, fmt);
    retval = fwk_kobject_set_name_args(sptr_kobj, fmt, sptr_list);
    va_end(sptr_list);

    return retval;
}

/*!
 * @brief   re-set kobject's name (unformatted)
 * @param   sptr_kobj, fmt
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kobject_rename(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...)
{
    va_list sptr_list;
    kint32_t retval;

    va_start(sptr_list, fmt);
    retval = fwk_kobject_set_name_args(sptr_kobj, fmt, sptr_list);
    va_end(sptr_list);

    return retval;
}

/*!
 * @brief   delete kobject's name
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kobject_del_name(struct fwk_kobject *sptr_kobj)
{
    if (sptr_kobj->name)
        kfree(sptr_kobj->name);

    sptr_kobj->name = mr_nullptr;
}

/*!
 * @brief   get kobject's name
 * @param   sptr_kobj
 * @retval  name
 * @note    none
 */
kchar_t *fwk_kobject_get_name(struct fwk_kobject *sptr_kobj)
{
    return sptr_kobj->name;
}

/*!
 * @brief   kobject get
 * @param   sptr_kobj
 * @retval  kobject
 * @note    none
 */
struct fwk_kobject *fwk_kobject_get(struct fwk_kobject *sptr_kobj)
{
    fwk_kref_get(&sptr_kobj->sgtc_ref);
    return sptr_kobj;
}

/*!
 * @brief   kobject put
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kobject_put(struct fwk_kobject *sptr_kobj)
{
    fwk_kref_put(&sptr_kobj->sgtc_ref);
}

/*!
 * @brief   check if kobject is using now
 * @param   sptr_kobj
 * @retval  non-zero: using; 0: closed
 * @note    none
 */
kbool_t fwk_kobject_is_referrd(struct fwk_kobject *sptr_kobj)
{
    return !fwk_kref_is_zero(&sptr_kobj->sgtc_ref);
}

/*!
 * @brief   initialize kset
 * @param   sptr_kset
 * @retval  none
 * @note    none
 */
void fwk_kset_init(struct fwk_kset *sptr_kset)
{
    init_list_head(&sptr_kset->sgtc_list);
    fwk_kobject_init(&sptr_kset->sgtc_kobj);
}

/*!
 * @brief   create kset
 * @param   name, sptr_parent
 * @retval  kset created
 * @note    none
 */
struct fwk_kset *fwk_kset_create(const kchar_t *name, struct fwk_kobject *sptr_parent)
{
    struct fwk_kset *sptr_kset;

    sptr_kset = kzalloc(sizeof(*sptr_kset), GFP_KERNEL);
    if (!isValid(sptr_kset))
        return mr_nullptr;

    if (fwk_kobject_set_name(&sptr_kset->sgtc_kobj, "%s", name))
    {
        kfree(sptr_kset);
        return mr_nullptr;
    }

    sptr_kset->sgtc_kobj.sptr_parent = sptr_parent;
    sptr_kset->sgtc_kobj.sptr_kset = mr_nullptr;

    return sptr_kset;
}

/*!
 * @brief   register kset
 * @param   sptr_kset
 * @retval  errno
 * @note    none
 */
kint32_t fwk_kset_register(struct fwk_kset *sptr_kset)
{
    struct fwk_kobject *sptr_kobj;
    struct fwk_kset *sptr_temp;

    if (!is_fwk_root_existed)
        return -ER_FORBID;

    sptr_kobj = &sptr_kset->sgtc_kobj;
    sptr_temp = sptr_kobj->sptr_kset;

    fwk_kset_init(sptr_kset);
    sptr_kobj->is_dir = true;
    sptr_kobj->sptr_kset = sptr_temp;

    if (!sptr_temp)
        sptr_kobj->sptr_kset = &sgtc_fwk_kset_root;
    
    return fwk_kobject_build_inode(&sptr_kset->sgtc_kobj);
}

/*!
 * @brief   create and register kset
 * @param   name, sptr_parent
 * @retval  kset created
 * @note    none
 */
struct fwk_kset *fwk_kset_create_and_register(const kchar_t *name, struct fwk_kobject *sptr_parent)
{
    struct fwk_kset *sptr_kset;

    sptr_kset = fwk_kset_create(name, sptr_parent);
    if (!isValid(sptr_kset))
        return mr_nullptr;

    if (fwk_kset_register(sptr_kset))
    {
        fwk_kobject_del_name(&sptr_kset->sgtc_kobj);
        kfree(sptr_kset);

        return mr_nullptr;
    }

    return sptr_kset;
}

/*!
 * @brief   unregister kset
 * @param   sptr_kset
 * @retval  none
 * @note    none
 */
void fwk_kset_unregister(struct fwk_kset *sptr_kset)
{
    if (!mr_list_head_empty(&sptr_kset->sgtc_list))
        return;

    fwk_kobject_del(&sptr_kset->sgtc_kobj);
    fwk_kset_init(sptr_kset);
}

/*!
 * @brief   unregister kset and kobject
 * @param   sptr_kobj
 * @retval  none
 * @note    none
 */
void fwk_kset_kobject_remove(struct fwk_kobject *sptr_kobj)
{
    if (!sptr_kobj->is_dir)
        fwk_kobject_destroy(sptr_kobj);
    else
    {
        struct fwk_kset *sptr_kset = mr_fwk_kset_get(sptr_kobj);
        fwk_kset_unregister(sptr_kset);
        kfree(sptr_kset);
    }
}

/*!
 * @brief   get root address
 * @param   none
 * @retval  fwk_kset
 * @note    none
 */
struct fwk_kset *fwk_kset_get_root(void)
{
    return &sgtc_fwk_kset_root;
}

/*!< ------------------------------------------------------- */
/*!
 * @brief   root directory init
 * @param   none
 * @retval  errno
 * @note    none
 */
kint32_t __plat_init fwk_kobject_root_init(void)
{
    struct fwk_kset *sptr_kset = &sgtc_fwk_kset_root;
    
    fwk_kset_init(sptr_kset);
    fwk_kobject_set_name(&sptr_kset->sgtc_kobj, "/");
    sptr_kset->sgtc_kobj.sptr_parent = mr_nullptr;
    sptr_kset->sgtc_kobj.sptr_kset = mr_nullptr;
    sptr_kset->sgtc_kobj.is_dir = true;

    is_fwk_root_existed = true;

    return ER_NORMAL;
}

/*!< end of file */
