/*
 * IMX6ULL Image Creator
 *
 * File Name:   image_tool.cpp
 * Author:      Yang Yujun
 * E-mail:      <yujiantianhu@163.com>
 * Created on:  2025.06.18
 *
 * Copyright (c) 2025   Yang Yujun <yujiantianhu@163.com>
 *
 */

/*!< The includes */
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "image_tool.h"

/*!< The defines */
#define BIN_OFFSET                                  (3072)

#ifndef IMAGE_NAME
#define IMAGE_NAME                                  "HeavenFox.img"
#endif

#ifndef BINARY_NAME
#define BINARY_NAME                                 "HeavenFox.bin"
#endif

#ifndef BINARY_PATH
#define BINARY_PATH                                 "../../../boot/image"
#endif

/*!< global class */
class image_engineer {
public:
    image_engineer(const unsigned int *dcd, const size_t dcd_len)
        : bin_buffer(nullptr)
        , bin_size(0)
        , total_size(0)
    {
        FILE *sptr_fp;
        size_t ret;

        memset(img_fullname, 0, sizeof(img_fullname));
        memset(bin_fullname, 0, sizeof(bin_fullname));

        sprintf(img_fullname, "%s/%s", BINARY_PATH, IMAGE_NAME);
        sprintf(bin_fullname, "%s/%s", BINARY_PATH, BINARY_NAME);

        /*!< Open HeaveneFox.bin */
        sptr_fp = fopen(bin_fullname, "rb");
        if (!sptr_fp) {
            std::cout << "Open " << BINARY_NAME << " failed!" << std::endl;
            return;
        }

        /*!< Get bin size */
        fseek(sptr_fp, 0, SEEK_END);
        bin_size = ftell(sptr_fp);
        fseek(sptr_fp, 0, SEEK_SET);

        std::cout << "Open " << BINARY_NAME << " success, size = " << bin_size << "(Bytes)" << std::endl;

        total_size = bin_size + BIN_OFFSET;
        bin_buffer = new char[total_size];
        if (bin_buffer == nullptr) {
            fclose(sptr_fp);
            std::cout << "Allocate buffer failed!" << std::endl; 
            return;
        }

        memset(bin_buffer, 0, total_size);
        memcpy(bin_buffer, dcd, dcd_len);

        ret = fread(bin_buffer + BIN_OFFSET, 1, bin_size, sptr_fp);
        fclose(sptr_fp);

        unlink(img_fullname);
        std::cout << "Delate the old " << IMAGE_NAME << std::endl;

        sptr_fp = fopen(img_fullname, "wb");
        if (!sptr_fp) {
            std::cout << "Create new " << IMAGE_NAME << " failed!" << std::endl;
            goto fail;
        }

        std::cout << "Create new " << IMAGE_NAME << " success!" << std::endl;

        if (total_size != fwrite(bin_buffer, 1, total_size, sptr_fp)) {
            fclose(sptr_fp);
            std::cout << "Write ivtdcd and " << BINARY_NAME << " to " << IMAGE_NAME << " failed!" << std::endl;
            goto fail;
        }

        fclose(sptr_fp);
        std::cout << "Write ivtdcd and " << BINARY_NAME << " to " << IMAGE_NAME << " success!" << std::endl;
        return;

    fail:
        delete[] bin_buffer;
    }

    ~image_engineer()
    {
        if (bin_buffer)
            delete[] bin_buffer;
    }

private:
    char *bin_buffer;
    size_t bin_size, total_size;

    char bin_fullname[128];
    char img_fullname[128];
};

/*!< API functions */
/*!
 * @brief   main
 * @param   argc: number of parameters
 * @param   argv: parameter lists
 * @retval  errno
 * @note    none
 */
int main(int argc, char **argv)
{
    image_engineer sgtc_img(imx6_ivtdcd_table, imx6_ivtdcd_table_size);
    return 0;
}
