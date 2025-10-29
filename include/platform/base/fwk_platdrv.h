/*
 * Platform Bus Interface of Hardware Abstraction Layer
 *
 * File Name:   fwk_platdrv.h
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2023.05.23
 *
 * Copyright (c) 2023   Yang Yujun <yujiantianhu@163.com>
 *
 */

#ifndef __FWK_PLATDRV_H_
#define __FWK_PLATDRV_H_

#ifdef __cplusplus
    extern "C" {
#endif

/*!< The includes */
#include <platform/base/fwk_basic.h>
#include <platform/base/fwk_platform.h>
#include <platform/base/fwk_platdev.h>

/*!< The defines */
struct fwk_id_table
{
    kchar_t *name;
    kint32_t driver_data;
};

typedef struct fwk_platdrv
{
    kint32_t (*probe) 	(struct fwk_platdev *sptr_dev);
    kint32_t (*remove) (struct fwk_platdev *sptr_dev);

    struct fwk_id_table *sptr_idTable;
    kusize_t num_idTable;

    struct fwk_driver sgtc_driver;
    
} srt_fwk_platdrv_t;

/*!< The functions */
extern kint32_t fwk_driver_register(struct fwk_driver *sptr_driver);
extern kint32_t fwk_driver_unregister(struct fwk_driver *sptr_driver);
extern kint32_t fwk_register_platdriver(struct fwk_platdrv *sptr_platdrv);
extern kint32_t fwk_unregister_platdriver(struct fwk_platdrv *sptr_platdrv);

/*!< API function */
/*!
 * @brief   fwk_platform_set_drvdata
 * @param   sptr_pdev, ptrData
 * @retval  errno
 * @note    none
 */
static inline void fwk_platform_set_drvdata(struct fwk_platdev *sptr_pdev, void *ptrData)
{
    sptr_pdev->sgtc_dev.privData = ptrData;
}

/*!
 * @brief   fwk_platform_get_drvdata
 * @param   sptr_pdev
 * @retval  privData
 * @note    none
 */
static inline void *fwk_platform_get_drvdata(struct fwk_platdev *sptr_pdev)
{
    return sptr_pdev->sgtc_dev.privData;
}

#ifdef __cplusplus
    }
#endif

#endif /*!< __FWK_PLATDRV_H_ */
