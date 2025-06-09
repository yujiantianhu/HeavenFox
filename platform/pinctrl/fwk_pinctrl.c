/*
 * Hardware Abstraction Layer Pinctrl Interface
 *
 * File Name:   fwk_pinctrl.c
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2024.04.15
 *
 * Copyright (c) 2024   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <platform/base/fwk_pinctrl.h>

/*!< The defines */
typedef struct fwk_pinctrl_dev_info
{
    struct fwk_device *sptr_dev;
    struct fwk_pinctrl *sptr_pctl;

    struct fwk_pinctrl_state *sptr_default;
    struct fwk_pinctrl_state *sptr_idle;
    struct fwk_pinctrl_state *sptr_sleep;

} srt_fwk_pinctrl_dev_info_t;

typedef struct fwk_pinctrl_dt_map 
{
	struct list_head sgtc_link;
	struct fwk_pinctrl_dev *sptr_pctldev;
	struct fwk_pinctrl_maps *sptr_maps;
	kuint32_t num_maps;

} srt_fwk_pinctrl_dt_map_t;

/*!< The globals */
static DECLARE_LIST_HEAD(sgtc_fwk_pinctrl_list);
static DECLARE_LIST_HEAD(sgtc_fwk_pinctrl_dev_list);
static DECLARE_LIST_HEAD(sgtc_fwk_pinctrl_map_list);

#define foreach_fwk_pinctrl_maps(maps, map, list) \
    foreach_list_next_entry(maps, &sgtc_fwk_pinctrl_map_list, list) \
        if ((map = maps->sptr_maps))

/*!< API function */
/*!
 * @brief   find pinctrl by device node
 * @param   sptr_node: controller node
 * @retval  pinctrl_dev found
 * @note    none
 */
__unused
static struct fwk_pinctrl *fwk_get_pinctrl_from_node(struct fwk_device_node *sptr_node)
{
    struct fwk_pinctrl *sptr_pctl;

    foreach_list_next_entry(sptr_pctl, &sgtc_fwk_pinctrl_list, sgtc_link)
    {
        if (sptr_pctl->sptr_dev->sptr_node == sptr_node)
            return sptr_pctl;
    }

    return mr_nullptr;
}

/*!
 * @brief   find pinctrl_dev by device node
 * @param   sptr_node: controller node
 * @retval  pinctrl_dev found
 * @note    none
 */
static struct fwk_pinctrl_dev *fwk_get_pinctrl_dev_from_node(struct fwk_device_node *sptr_node)
{
    struct fwk_pinctrl_dev *sptr_pctldev;

    foreach_list_next_entry(sptr_pctldev, &sgtc_fwk_pinctrl_dev_list, sgtc_link)
    {
        if (sptr_pctldev->sptr_dev->sptr_node == sptr_node)
            return sptr_pctldev;
    }

    return mr_nullptr;
}

/*!
 * @brief   find pinctrl_dev by device name (from global list)
 * @param   name: sptr_dev->name
 * @retval  pinctrl_dev found
 * @note    none
 */
static struct fwk_pinctrl_dev *fwk_get_pinctrl_dev_from_name(const kchar_t *name)
{
    struct fwk_pinctrl_dev *sptr_pctldev;

    foreach_list_next_entry(sptr_pctldev, &sgtc_fwk_pinctrl_dev_list, sgtc_link)
    {
        if (!kstrcmp(mr_dev_get_name(sptr_pctldev->sptr_dev), name))
            return sptr_pctldev;
    }

    return mr_nullptr;
}

/*!
 * @brief   release pinctrl map
 * @param   sptr_pctldev: pinctrl dev
 * @param   sptr_map: map that will be released
 * @retval  none
 * @note    none
 */
static void fwk_pinctrl_map_free(struct fwk_pinctrl_dev *sptr_pctldev, struct fwk_pinctrl_map *sptr_map, kuint32_t num_maps)
{
    const struct fwk_pinctrl_ops *sptr_pctlops;

    if (!sptr_map || !num_maps)
        return;

    sptr_pctlops = sptr_pctldev ? sptr_pctldev->sptr_desc->sptr_pctlops : mr_nullptr;

    if (sptr_pctlops && sptr_pctlops->dt_free_map)
        sptr_pctlops->dt_free_map(sptr_pctldev, sptr_map, num_maps);
    else
        kfree(sptr_map);
}

/*!
 * @brief   release pinctrl maps
 * @param   sptr_pctldev: pinctrl dev
 * @param   sptr_maps: maps that will be released
 * @retval  none
 * @note    none
 */
static void fwk_pinctrl_maps_free(struct fwk_pinctrl_dev *sptr_pctldev, struct fwk_pinctrl_maps *sptr_maps)
{
    if (!sptr_maps)
        return;

    /*!< release sptr_map */
    fwk_pinctrl_map_free(sptr_pctldev, sptr_maps->sptr_maps, sptr_maps->num_maps);

    /*!< detach from global list */
    list_head_del(&sptr_maps->sgtc_link);
    kfree(sptr_maps);
}

/*!
 * @brief   release pinctrl
 * @param   sptr_pctl: current pinctrl
 * @retval  none
 * @note    none
 */
static void fwk_pinctrl_free(struct fwk_pinctrl *sptr_pctl)
{
    struct fwk_pinctrl_dev *sptr_pctldev;
    struct fwk_pinctrl_state *sptr_state, *sptr_statetemp;
    struct fwk_pinctrl_maps *sptr_maps;
    struct fwk_pinctrl_dt_map *sptr_dt_maps, *sptr_dt_maptemp;
    struct fwk_pinctrl_setting *sptr_setting, *sptr_settemp;

    if (!sptr_pctl)
        return;

    /*!< 1, release sptr_setting and sptr_state */
    if (!mr_list_empty(&sptr_pctl->sgtc_states))
    {
        foreach_list_next_entry_safe(sptr_state, sptr_statetemp, &sptr_pctl->sgtc_states, sgtc_link)
        {
            foreach_list_next_entry_safe(sptr_setting, sptr_settemp, &sptr_state->sgtc_settings, sgtc_link)
            {
                list_head_del(&sptr_setting->sgtc_link);
                kfree(sptr_setting);
            }

            list_head_del(&sptr_state->sgtc_link);
            kfree(sptr_state);
        }
    }

    if (mr_list_empty(&sptr_pctl->sgtc_dt_maps))
        return;

    /*!< 2, release sptr_map and sptr_maps */
    foreach_list_next_entry_safe(sptr_dt_maps, sptr_dt_maptemp, &sptr_pctl->sgtc_dt_maps, sgtc_link)
    {
        sptr_maps = sptr_dt_maps->sptr_maps;
        if (!sptr_maps)
            continue;
        
        sptr_pctldev = sptr_dt_maps->sptr_pctldev;
        fwk_pinctrl_maps_free(sptr_pctldev, sptr_maps);

        list_head_del(&sptr_dt_maps->sgtc_link);
        kfree(sptr_dt_maps);
    }
}

/*!
 * @brief   find pinctrl state
 * @param   sptr_pctl: current pinctrl
 * @param   name: "default", "idle", or "sleep"
 * @retval  state found
 * @note    none
 */
static struct fwk_pinctrl_state *fwk_pinctrl_find_state(struct fwk_pinctrl *sptr_pctl, const kchar_t *name)
{
    struct fwk_pinctrl_state *sptr_state;

    if (mr_list_empty(&sptr_pctl->sgtc_states))
        return mr_nullptr;

    foreach_list_next_entry(sptr_state, &sptr_pctl->sgtc_states, sgtc_link)
    {
        if (!kstrcmp(name, sptr_state->name))
            return sptr_state;
    }

    return mr_nullptr;
}

/*!
 * @brief   create pinctrl states
 * @param   sptr_pctl: current pinctrl
 * @param   pinctl_name: property name
 * @param   state_name: pinctrl-names = "default", "idle", or "sleep"
 * @retval  pinctrl_state
 * @note    none
 */
static struct fwk_pinctrl_state *fwk_pinctrl_create_state(struct fwk_pinctrl *sptr_pctl, 
                                                    const kchar_t *pinctl_name, const kchar_t *state_name)
{
    struct fwk_pinctrl_state *sptr_state;
    struct fwk_device_node *sptr_node, *sptr_cfg, *sptr_par;
    struct fwk_of_property *sptr_prop;
    struct fwk_pinctrl_dev *sptr_pctldev;
    struct fwk_pinctrl_map *sptr_map;
    struct fwk_pinctrl_maps *sptr_maps;
    const struct fwk_pinctrl_ops *sptr_pctlops;
    struct fwk_pinctrl_dt_map *sptr_dt_maps;

    kuint32_t idx, grps, lenth, phandle, num_maps;

#define FWK_PINCTRL_SET_MAP_NAME(len)          (((state_name) && (len)) ? (state_name) : FWK_PINCTRL_STATE_DUMMY)

    if (!sptr_pctl)
        return mr_nullptr;

    /*!< get current node */
    sptr_node = sptr_pctl->sptr_dev->sptr_node;

    /*!< property over */
    sptr_prop = fwk_of_find_property(sptr_node, pinctl_name, &lenth);
    if (!isValid(sptr_prop))
        return mr_nullptr;

    /*!< 
     * lenth >> 2 : how many phandles here
     * for example: pinctrl-0 = <&pinctrl_gpio1 &pinctrl_gpio2>
     * ===> the number of phandles is 2 (lenth = 8bytes), grp = [0, 1]
     */
    for (grps = 0; grps < (lenth >> 2); grps++)
    {
        /*!< fwk_of_property_read_u32_index(sptr_node, propname, grps, &phandle) */
        phandle = FDT_TO_ARCH_ENDIAN32(*((kuint32_t *)sptr_prop->value + grps));

        sptr_cfg = fwk_of_find_node_by_phandle(mr_nullptr, phandle);
        if (!sptr_cfg)
            goto fail;

        /*!< find pinctrl dev */
        foreach_fwk_of_parent(sptr_par, sptr_cfg)
        {
            /*!< judge if sptr_pctldev->sptr_dev->sptr_node is equaled to sptr_par */
            sptr_pctldev = fwk_get_pinctrl_dev_from_node(sptr_par);
            if (sptr_pctldev)
                break;

            /*!< not found */
            if (!sptr_pctldev && (sptr_par == fwk_of_node_root()))
                goto fail;
        }

        sptr_pctlops = sptr_pctldev->sptr_desc->sptr_pctlops;
        if (!sptr_pctlops || !sptr_pctlops->dt_node_to_map)
            goto fail;

        /*!< allocate sptr_map from memory and initialize it */
        if (sptr_pctlops->dt_node_to_map(sptr_pctldev, sptr_cfg, &sptr_map, &num_maps))
            goto fail;

        for (idx = 0; idx < num_maps; idx++)
        {
            /*!< sptr_map->name <===> sptr_state->name: "default", "sleep", ... */
            sptr_map[idx].dev_name = mr_dev_get_name(sptr_pctl->sptr_dev);
            sptr_map[idx].name = FWK_PINCTRL_SET_MAP_NAME(lenth);
            sptr_map[idx].ctrl_dev_name = mr_dev_get_name(sptr_pctldev->sptr_dev);
        }

        /*!< 
         * each group is represented by a sptr_maps; 
         * each pin is represented by a sptr_map;
         * a group consists of multiple pins (1 mux + n conf); 
         * num_maps is the number of sptr_map array, num_maps = the number of "mux" + the number of "conf"
         * sptr_maps->sptr_map = &sptr_map[0]
         */
        sptr_maps = (struct fwk_pinctrl_maps *)kzalloc(sizeof(*sptr_maps), GFP_KERNEL);
        if (!isValid(sptr_maps))
        {
            fwk_pinctrl_map_free(sptr_pctldev, sptr_map, num_maps);
            goto fail;
        }

        sptr_maps->sptr_maps = sptr_map;
        sptr_maps->num_maps  = num_maps;
        list_head_add_tail(&sgtc_fwk_pinctrl_map_list, &sptr_maps->sgtc_link);

        /*!< each sptr_dt_maps links to sptr_pctl */
        sptr_dt_maps = (struct fwk_pinctrl_dt_map *)kzalloc(sizeof(*sptr_dt_maps), GFP_KERNEL);
        if (!isValid(sptr_dt_maps))
        {
            fwk_pinctrl_maps_free(sptr_pctldev, sptr_maps);
            goto fail;
        }

        sptr_dt_maps->sptr_maps = sptr_maps;
        sptr_dt_maps->num_maps = 1;
        sptr_dt_maps->sptr_pctldev = sptr_pctldev;
        list_head_add_tail(&sptr_pctl->sgtc_dt_maps, &sptr_dt_maps->sgtc_link);
    }

    sptr_state = (struct fwk_pinctrl_state *)kzalloc(sizeof(*sptr_state), GFP_KERNEL);
    if (!isValid(sptr_state))
        goto fail;

    /*!< name: "default", "idle", or "sleep" */
    sptr_state->name = FWK_PINCTRL_SET_MAP_NAME(lenth);
    init_list_head(&sptr_state->sgtc_settings);

    return sptr_state;

fail:
    fwk_pinctrl_free(sptr_pctl);
    return mr_nullptr;

#undef FWK_PINCTRL_SET_MAP_NAME
}

/*!
 * @brief   create pinctrl states
 * @param   sptr_pctl: current pinctrl
 * @retval  error code
 * @note    none
 */
static kint32_t fwk_pinctrl_add_state(struct fwk_pinctrl *sptr_pctl)
{
    struct fwk_pinctrl_state *sptr_state;
    struct fwk_device_node *sptr_node;
    struct fwk_of_property *sptr_prop;
    kchar_t propname[16], *state_name;
    kuint32_t idx;

    if (!sptr_pctl)
        return -ER_NODEV;

    /*!< get current node */
    sptr_node = sptr_pctl->sptr_dev->sptr_node;

    /*!< pinctrl-0, pinctrl-1, ..., pinctrl-$(idx) */
    for (idx = 0; ; idx++)
    {
        memset(propname, 0, sizeof(propname));
        sprintk(propname, "pinctrl-%d", idx);

        /*!< sptr_prop == null ? property over */
        sptr_prop = fwk_of_find_property(sptr_node, propname, mr_nullptr);
        if (!isValid(sptr_prop))
        {
            if (!idx)
                goto fail;
            else
                break;
        }

        fwk_of_property_read_string_index(sptr_node, "pinctrl-names", idx, &state_name);
        sptr_state = fwk_pinctrl_find_state(sptr_pctl, state_name);
        if (sptr_state)
            continue;

        sptr_state = fwk_pinctrl_create_state(sptr_pctl, propname, state_name);
        if (!isValid(sptr_state))
            goto fail;

        list_head_add_tail(&sptr_pctl->sgtc_states, &sptr_state->sgtc_link);
    }

    return ER_NORMAL;

fail:
    fwk_pinctrl_free(sptr_pctl);
    return -ER_FAILD;
}

/*!
 * @brief   create pinctrl settings
 * @param   sptr_pctl: current pinctrl
 * @retval  error code
 * @note    none
 */
static kint32_t fwk_pinctrl_add_setting(struct fwk_pinctrl *sptr_pctl)
{
    struct fwk_pinctrl_setting *sptr_setting;
    struct fwk_pinctrl_state *sptr_state;
    struct fwk_pinctrl_map *sptr_map = mr_nullptr;
    struct fwk_pinctrl_maps *sptr_maps = mr_nullptr;
    kuint32_t i;
    kint32_t retval = 0;

    if (mr_list_empty(&sgtc_fwk_pinctrl_map_list))
        return -ER_EMPTY;

    /*!< get each sptr_maps */
    foreach_fwk_pinctrl_maps(sptr_maps, sptr_map, sgtc_link)
    {
        for (i = 0; i < sptr_maps->num_maps; i++, sptr_map++)
        {
            /*!< judge if it is belongs to current device */
            if (kstrcmp(sptr_map->dev_name, mr_dev_get_name(sptr_pctl->sptr_dev)))
                continue;

            /*!< found sptr_map; find state next*/
            sptr_state = fwk_pinctrl_find_state(sptr_pctl, sptr_map->name);
            if (!sptr_state)
                continue;

            sptr_setting = (struct fwk_pinctrl_setting *)kzalloc(sizeof(*sptr_setting), GFP_KERNEL);
            if (!isValid(sptr_setting))
                goto fail;

            /*!< type: mux or conf */
            sptr_setting->type = sptr_map->type;
            sptr_setting->dev_name = sptr_map->dev_name;

            sptr_setting->sptr_pctldev = fwk_get_pinctrl_dev_from_name(sptr_map->ctrl_dev_name);
            if (!sptr_setting->sptr_pctldev)
            {
                kfree(sptr_setting);
                goto fail;
            }

            switch (sptr_setting->type)
            {
                case NR_FWK_PINCTRL_PIN_MUX:
                    retval = fwk_pinmux_map_to_setting(sptr_map, sptr_setting);
                    break;

                case NR_FWK_PINCTRL_PIN_CONF:
                    retval = fwk_pinconf_map_to_setting(sptr_map, sptr_setting);
                    break;

                default: break;
            }

            if (retval)
            {
                kfree(sptr_setting);
                goto fail;
            }

            list_head_add_tail(&sptr_state->sgtc_settings, &sptr_setting->sgtc_link);
        }
    }
   
    return ER_NORMAL;

fail:
    fwk_pinctrl_free(sptr_pctl);
    return -ER_FAILD;
}

/*!
 * @brief   find pinctrl if it is exsited
 * @param   sptr_dev: current device (sptr_pctl->dev may equal to it)
 * @retval  pinctrl that was found
 * @note    all pinctrls are linked to global list, you should loop up from it
 */
static struct fwk_pinctrl *fwk_find_pinctrl(struct fwk_device *sptr_dev)
{
    struct fwk_pinctrl *sptr_pctl;

    if (!sptr_dev || mr_list_empty(&sgtc_fwk_pinctrl_list))
        return mr_nullptr;

    foreach_list_next_entry(sptr_pctl, &sgtc_fwk_pinctrl_list, sgtc_link)
    {
        /*!< compare sptr_dev */
        if (sptr_pctl->sptr_dev == sptr_dev)
            return sptr_pctl;
    }

    return mr_nullptr;
}

/*!
 * @brief   create a new pinctrl
 * @param   sptr_dev: current device
 * @retval  pinctrl that was allocated
 * @note    none
 */
static struct fwk_pinctrl *fwk_create_pinctrl(struct fwk_device *sptr_dev)
{
    struct fwk_pinctrl *sptr_pctl;

    if (!sptr_dev)
        return mr_nullptr;

    sptr_pctl = (struct fwk_pinctrl *)kzalloc(sizeof(*sptr_pctl), GFP_KERNEL);
    if (!isValid(sptr_pctl))
        return mr_nullptr;

    /*!< save current device, you can find pinctrl by comparing sptr_dev in the future */
    sptr_pctl->sptr_dev = sptr_dev;
    init_list_head(&sptr_pctl->sgtc_states);
    init_list_head(&sptr_pctl->sgtc_dt_maps);

    /*!< create state by "pinctrl-names"; all states are linked to sptr_pctl->sgtc_states */
    if (fwk_pinctrl_add_state(sptr_pctl))
        goto fail1;

    /*!< 
     * create setting for per state
     * for example, assuming that there are n pin-configurations for a group in device_node:
     *      setting for mux: at least one;
     *      setting for conf: at least n (the number of pin configurations)
     * therefore, the number of settings for a state is: n + 1
     */
    if (fwk_pinctrl_add_setting(sptr_pctl))
        goto fail2;

    /*!< link to global list */
    list_head_add_tail(&sgtc_fwk_pinctrl_list, &sptr_pctl->sgtc_link);

    return sptr_pctl;

fail2:
    fwk_pinctrl_free(sptr_pctl);
fail1:
    kfree(sptr_pctl);
    return mr_nullptr;
}

/*!
 * @brief   register pinctrl
 * @param   sptr_desc: device operations
 * @param   sptr_dev: current device
 * @param   driver_data: driver private data
 * @retval  pinctrl_dev
 * @note    none
 */
struct fwk_pinctrl_dev *fwk_pinctrl_register(struct fwk_pinctrl_desc *sptr_desc, struct fwk_device *sptr_dev, void *driver_data)
{
    struct fwk_pinctrl_dev *sptr_pctldev;
    struct fwk_pinctrl *sptr_pctl;
    struct fwk_pinctrl_state *sptr_state;

    if (!sptr_desc)
        return mr_nullptr;

    if ((!sptr_desc->sptr_pctlops) || (!sptr_desc->sptr_pmxops) || (!sptr_desc->sptr_confops))
        return mr_nullptr;

    sptr_pctldev = (struct fwk_pinctrl_dev *)kzalloc(sizeof(*sptr_pctldev), GFP_KERNEL);
    if (!isValid(sptr_pctldev))
        return mr_nullptr;

    sptr_pctldev->sptr_desc = sptr_desc;
    sptr_pctldev->sptr_dev = sptr_dev;
    sptr_pctldev->driver_data = driver_data;
    mutex_init(&sptr_pctldev->sgtc_mutex);
    init_list_head(&sptr_pctldev->sgtc_gpio_ranges);

    list_head_add_tail(&sgtc_fwk_pinctrl_dev_list, &sptr_pctldev->sgtc_link);

    /*!< init pinctrl state after adding to sgtc_fwk_pinctrl_dev_list */
    sptr_pctl = fwk_pinctrl_get(sptr_dev);
    if (!sptr_pctl)
        goto fail1;

    sptr_pctldev->sptr_pctl = sptr_pctl;
    sptr_pctldev->sptr_hog_sleep = fwk_pinctrl_lookup_state(sptr_pctl, FWK_PINCTRL_STATE_SLEEP);

    sptr_state = fwk_pinctrl_lookup_state(sptr_pctl, FWK_PINCTRL_STATE_DEFAULT);
    if (!sptr_state)
        goto END;

    if (fwk_pinctrl_select_state(sptr_pctl, sptr_state))
        goto fail2;

    sptr_pctldev->sptr_hog_default = sptr_state;
    
END:
    return sptr_pctldev;

fail2:
    fwk_pinctrl_put(sptr_pctl);
fail1:
    list_head_del(&sptr_pctldev->sgtc_link);
    kfree(sptr_pctldev);

    return mr_nullptr;
}

/*!
 * @brief   register pinctrl
 * @param   sptr_desc: device operations
 * @param   sptr_dev: current device
 * @param   driver_data: driver private data
 * @retval  pinctrl_dev
 * @note    none
 */
void fwk_pinctrl_unregister(struct fwk_pinctrl_dev *sptr_pctldev)
{
    if (!sptr_pctldev)
        return;

    if (sptr_pctldev->sptr_pctl)
        fwk_pinctrl_put(sptr_pctldev->sptr_pctl);

    list_head_del(&sptr_pctldev->sgtc_link);
    kfree(sptr_pctldev);
}

/*!
 * @brief   device binds pinctrl
 * @param   sptr_dev: current device 
 * @retval  errno
 * @note    pinctrl-device-info ===> sptr_dev->pinctrl_info = sptr_info
 */
kint32_t fwk_pinctrl_bind_pins(struct fwk_device *sptr_dev)
{
    struct fwk_pinctrl_dev_info *sptr_info;
    struct fwk_pinctrl *sptr_pctl;
    struct fwk_pinctrl_state *sptr_state;

    sptr_info = kzalloc(sizeof(*sptr_info), GFP_KERNEL);
    if (!sptr_info)
        return -ER_NOMEM;

    /*!< init pinctrl state after adding to sgtc_fwk_pinctrl_dev_list */
    sptr_pctl = fwk_pinctrl_get(sptr_dev);
    if (!sptr_pctl)
        goto fail1;

    sptr_dev->sptr_pctlinfo = sptr_info;
    sptr_info->sptr_pctl = sptr_pctl;
    sptr_info->sptr_idle = fwk_pinctrl_lookup_state(sptr_pctl, FWK_PINCTRL_STATE_IDLE);
    sptr_info->sptr_sleep = fwk_pinctrl_lookup_state(sptr_pctl, FWK_PINCTRL_STATE_SLEEP);

    sptr_state = fwk_pinctrl_lookup_state(sptr_pctl, FWK_PINCTRL_STATE_DEFAULT);
    if (!sptr_state)
        goto END;

    if (fwk_pinctrl_select_state(sptr_pctl, sptr_state))
        goto fail2;

    sptr_info->sptr_default = sptr_state;

END:
    return ER_NORMAL;

fail2:
    fwk_pinctrl_put(sptr_pctl);
fail1:
    kfree(sptr_info);
    return -ER_ERROR;
}

/*!
 * @brief   device unbinds pinctrl
 * @param   sptr_dev: current device 
 * @retval  errno
 * @note    release sptr_dev->pinctrl_info
 */
void fwk_pinctrl_unbind_pins(struct fwk_device *sptr_dev)
{
    struct fwk_pinctrl_dev_info *sptr_info;
    
    sptr_info = sptr_dev->sptr_pctlinfo;
    if (!sptr_info)
        return;

    if (sptr_info->sptr_sleep)
        fwk_pinctrl_select_state(sptr_info->sptr_pctl, sptr_info->sptr_sleep);
    else if (sptr_info->sptr_idle)
        fwk_pinctrl_select_state(sptr_info->sptr_pctl, sptr_info->sptr_idle);

    fwk_pinctrl_put(sptr_info->sptr_pctl);
    kfree(sptr_info);
    sptr_dev->sptr_pctlinfo = mr_nullptr;
}

/*!
 * @brief   release pinctrl
 * @param   sptr_pctl: pinctrl 
 * @retval  none
 * @note    none
 */
void fwk_pinctrl_put(struct fwk_pinctrl *sptr_pctl)
{
    fwk_pinctrl_free(sptr_pctl);
    list_head_del(&sptr_pctl->sgtc_link);
    kfree(sptr_pctl);
}

/*!
 * @brief   request pinctrl
 * @param   sptr_dev: current device 
 * @retval  sptr_pctl
 * @note    none
 */
struct fwk_pinctrl *fwk_pinctrl_get(struct fwk_device *sptr_dev)
{
    struct fwk_pinctrl *sptr_pctl;

    sptr_pctl = fwk_find_pinctrl(sptr_dev);
    if (sptr_pctl)
        return sptr_pctl;

    return fwk_create_pinctrl(sptr_dev);
}

/*!
 * @brief   look up pinctrl state
 * @param   sptr_pctl: pinctrl
 * @param   state_name: "default", "sleep" or "idle"
 * @retval  pinctrl state structure
 * @note    none
 */
struct fwk_pinctrl_state *fwk_pinctrl_lookup_state(struct fwk_pinctrl *sptr_pctl, const kchar_t *state_name)
{
    struct fwk_pinctrl_state *sptr_state;

    if (!sptr_pctl || mr_list_empty(&sptr_pctl->sgtc_states))
        return mr_nullptr;

    if (sptr_pctl->sptr_state && !kstrcmp(sptr_pctl->sptr_state->name, state_name))
        return sptr_pctl->sptr_state;

    foreach_list_next_entry(sptr_state, &sptr_pctl->sgtc_states, sgtc_link)
    {
        if (!kstrcmp(sptr_state->name, state_name))
            return sptr_state;
    }

    return mr_nullptr;
}

/*!
 * @brief   select pinctrl state
 * @param   sptr_pctl: pinctrl
 * @param   sptr_state: state that will be selected
 * @retval  error code
 * @note    none
 */
kint32_t fwk_pinctrl_select_state(struct fwk_pinctrl *sptr_pctl, struct fwk_pinctrl_state *sptr_state)
{
    struct fwk_pinctrl_setting *sptr_setting;
    struct fwk_pinctrl_state *sptr_cur;
    struct fwk_pinctrl_dev *sptr_pctldev;
	const struct fwk_pinmux_ops *sptr_pmxops;
	const struct fwk_pinconf_ops *sptr_confops;
    kint32_t retval;

    if (!sptr_pctl || !sptr_state)
        return -ER_NODEV;

    sptr_cur = sptr_pctl->sptr_state;

    if (sptr_cur == sptr_state)
        return -ER_EXISTED;

    /*!< disable current pins */
    if (sptr_cur)
    {
        foreach_list_next_entry(sptr_setting, &sptr_cur->sgtc_settings, sgtc_link)
        {
            /*!< do nothing */
        }
    }

    /*!< enable new pin configurations */
    foreach_list_next_entry(sptr_setting, &sptr_state->sgtc_settings, sgtc_link)
    {
        sptr_pctldev = sptr_setting->sptr_pctldev;
        sptr_pmxops = sptr_pctldev->sptr_desc->sptr_pmxops;
        sptr_confops = sptr_pctldev->sptr_desc->sptr_confops;

        if (!sptr_pmxops || !sptr_confops)
            return -ER_NSUPPORT;

        retval = false;

        switch (sptr_setting->type)
        {
            /*!< each setting may only have one mux */
            case NR_FWK_PINCTRL_PIN_MUX:
                if (sptr_pmxops->set_mux)
                    retval = sptr_pmxops->set_mux(sptr_pctldev, 
                                        sptr_setting->ugtr_data.sgtc_mux.func, 
                                        sptr_setting->ugtr_data.sgtc_mux.group);
                break;

            case NR_FWK_PINCTRL_PIN_CONF:
                if (sptr_confops->pin_config_set)
                    retval = sptr_confops->pin_config_set(sptr_pctldev, 
                                        sptr_setting->ugtr_data.sgtc_configs.group_or_pin, 
                                        sptr_setting->ugtr_data.sgtc_configs.configs,
                                        sptr_setting->ugtr_data.sgtc_configs.num_configs);
                break;

            default:
                return -ER_FAULT;
        }

        if (retval)
            return retval;
    }

    sptr_pctl->sptr_state = sptr_state;

    return ER_NORMAL;
}

/*!< end of file */
