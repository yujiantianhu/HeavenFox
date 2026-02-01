/*
 * Platform Object Defines
 *
 * File Name:   fwk_kboj.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.18
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_KOBJ_H_
#define __FWK_KOBJ_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <kernel/mutex.h>
#include <kernel/spinlock.h>

/*!< The defines */
struct fwk_inode;

struct fwk_kobject
{
	kchar_t *name;
	struct atomic sgtc_ref;

	struct list_head sgtc_link;
	struct fwk_kobject *sptr_parent;
	struct fwk_kset *sptr_kset;
	struct fwk_inode *sptr_inode;

	kbool_t is_dir;
	kbool_t is_disk;
	
	struct spin_lock sgtc_lock;
};

struct fwk_kset
{
	struct fwk_kobject sgtc_kobj;
	struct list_head sgtc_list;
};

#define mr_fwk_kset_get(sptr_kobj)	\
	(((sptr_kobj) && (sptr_kobj)->is_dir) ? mr_container_of(sptr_kobj, struct fwk_kset, sgtc_kobj) : mr_nullptr)

struct fwk_probes
{
	kuint32_t devNum;
	kuint32_t range;

	void *data;
	struct fwk_probes *sptr_next;
};

struct fwk_kobj_map
{
	struct mutex_lock sgtc_mutex;
	struct fwk_probes *sptr_probes[DEVICE_MAX_NUM];
};

/*!< The globals */
extern struct fwk_kobj_map *sptr_fwk_chrdev_map;
extern struct fwk_kobj_map *sptr_fwk_blkdev_map;

/*!< The functions */
/*!< -------------------------------------------------------------- */
extern kint32_t fwk_kobjmap_init(void);
extern void fwk_kobjmap_del(void);

extern kint32_t fwk_kobj_map(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range, void *data);
extern kint32_t fwk_kobj_unmap(struct fwk_kobj_map *sptr_domain, kuint32_t devNum, kuint32_t range);
extern void *fwk_kobjmap_lookup(struct fwk_kobj_map *sptr_domain, kuint32_t devNum);

/*!< -------------------------------------------------------------- */
extern kint32_t fwk_kobject_root_init(void);

extern void fwk_kobject_init(struct fwk_kobject *sptr_kobj);
extern struct fwk_kobject *fwk_kobject_create(void);
extern void fwk_kobject_del(struct fwk_kobject *sptr_kobj);
extern void fwk_kobject_destroy(struct fwk_kobject *sptr_kobj);
extern kint32_t fwk_kobject_add(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, ...);
extern kint32_t fwk_kobject_add_vargs(struct fwk_kobject *sptr_kobj, struct fwk_kobject *sptr_parent, const kchar_t *fmt, va_list sptr_list);
extern struct fwk_kobject *fwk_kobject_populate(struct fwk_kobject *sptr_head, const kchar_t *name);
extern struct fwk_kobject *fwk_find_kobject_by_path(struct fwk_kobject *sptr_head, const kchar_t *name);
extern kint32_t fwk_kobject_set_name(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...);
extern kint32_t fwk_kobject_set_name_args(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, va_list sptr_list);
extern kint32_t fwk_kobject_rename(struct fwk_kobject *sptr_kobj, const kchar_t *fmt, ...);
extern void fwk_kobject_del_name(struct fwk_kobject *sptr_kobj);
extern kchar_t *fwk_kobject_get_name(struct fwk_kobject *sptr_kobj);
extern struct fwk_kobject *fwk_kobject_get(struct fwk_kobject *sptr_kobj);
extern void fwk_kobject_put(struct fwk_kobject *sptr_kobj);
extern kbool_t fwk_kobject_is_refered(struct fwk_kobject *sptr_kobj);

extern void fwk_kset_init(struct fwk_kset *sptr_kset);
extern struct fwk_kset *fwk_kset_create(const kchar_t *name, struct fwk_kobject *sptr_parent);
extern void fwk_kset_destroy(struct fwk_kset *sptr_kset);
extern kint32_t fwk_kset_register(struct fwk_kset *sptr_kset);
extern struct fwk_kset *fwk_kset_create_and_register(const kchar_t *name, struct fwk_kobject *sptr_parent);
extern void fwk_kset_unregister(struct fwk_kset *sptr_kset);
extern void fwk_kset_kobject_remove(struct fwk_kobject *sptr_kobj);
extern struct fwk_kset *fwk_kset_get_root(void);
extern struct fwk_kset *fwk_find_kset_by_path(struct fwk_kobject *sptr_head, const kchar_t *name);
extern struct fwk_kobject *fwk_kset_first_child(struct fwk_kset *sptr_kset);
extern struct fwk_kobject *fwk_kset_next_child(struct fwk_kset *sptr_kset, struct fwk_kobject *sptr_kobj);

/*!< The defines */
#define foreach_next_kobject(_sptr_kset, _sptr_kobj)	\
	for (_sptr_kobj = fwk_kset_first_child(_sptr_kset);	\
		 _sptr_kobj;	\
		 _sptr_kobj = fwk_kset_next_child(_sptr_kset, _sptr_kobj))

/*!< API functions */
/*!
 * @brief   initial kref
 * @param   sptr_kref
 * @retval  none
 * @note    none
 */
static inline void fwk_kref_init(struct atomic *sptr_kref)
{
	atomic_set_val(sptr_kref, 0);
}

/*!
 * @brief   increase kref
 * @param   sptr_kref
 * @retval  none
 * @note    none
 */
static inline void fwk_kref_get(struct atomic *sptr_kref)
{
	atomic_inc(sptr_kref);
}

/*!
 * @brief   decrease kref
 * @param   sptr_kref
 * @retval  none
 * @note    none
 */
static inline void fwk_kref_put(struct atomic *sptr_kref)
{
	atomic_get_val(sptr_kref) ? atomic_dec(sptr_kref) : (void)0;
}

/*!
 * @brief   check if kref is the smallest
 * @param   sptr_kref
 * @retval  none
 * @note    none
 */
static inline kbool_t fwk_kref_is_zero(struct atomic *sptr_kref)
{
	return !!atomic_get_val(sptr_kref);
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_KOBJ_H_ */
