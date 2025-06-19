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
#ifndef BIN_OFFSET
#define BIN_OFFSET                                  (3072)
#endif

#ifndef IMAGE_NAME
#define IMAGE_NAME                                  "HeavenFox.img"
#endif

#ifndef BINARY_NAME
#define BINARY_NAME                                 "HeavenFox.bin"
#endif

#ifndef IMAGE_PATH
#define IMAGE_PATH                                  "../image"
#endif

#ifndef BINARY_PATH
#define BINARY_PATH                                 "../../../boot/image"
#endif

/*!< The globals */
static char *image = (char *)IMAGE_NAME;
static char *image_path = (char *)IMAGE_PATH;
static char *binar = (char *)BINARY_NAME;
static char *binar_path = (char *)BINARY_PATH;

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

        sprintf(img_fullname, "%s/%s", image_path, image);
        sprintf(bin_fullname, "%s/%s", binar_path, binar);

        /*!< Open HeaveneFox.bin */
        sptr_fp = fopen(bin_fullname, "rb");
        if (!sptr_fp) {
            std::cout << "Open " << binar << " failed!" << std::endl;
            return;
        }

        /*!< Get bin size */
        fseek(sptr_fp, 0, SEEK_END);
        bin_size = ftell(sptr_fp);
        fseek(sptr_fp, 0, SEEK_SET);

        std::cout << "Open " << binar << " success, size = " << bin_size << "(Bytes)" << std::endl;

        total_size = bin_size + BIN_OFFSET;
        bin_buffer = new unsigned char[total_size];
        if (bin_buffer == nullptr) {
            fclose(sptr_fp);
            std::cout << "Allocate buffer failed!" << std::endl; 
            return;
        }

        memset(bin_buffer, 0, total_size);
        memcpy(bin_buffer, dcd, dcd_len);

        /*!< Adjust the size of image */
        if (dcd[9] < total_size)
            ((unsigned int *)bin_buffer)[9] = (total_size + (4 - 1)) & ~((4 - 1)) + BIN_OFFSET;

        ret = fread(bin_buffer + BIN_OFFSET, 1, bin_size, sptr_fp);
        fclose(sptr_fp);

        unlink(img_fullname);
        std::cout << "Delete the old " << image << std::endl;

        sptr_fp = fopen(img_fullname, "wb");
        if (!sptr_fp) {
            std::cout << "Create new " << image << " failed!" << std::endl;
            goto fail;
        }

        std::cout << "Create new " << image << " success!" << std::endl;

        if (total_size != fwrite(bin_buffer, 1, total_size, sptr_fp)) {
            fclose(sptr_fp);
            std::cout << "Write ivtdcd and " << binar << " to " << image << " failed!" << std::endl;
            goto fail;
        }

        fclose(sptr_fp);
        std::cout << "Write ivtdcd and " << binar << " to " << image << " success!" << std::endl;
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
    unsigned char *bin_buffer;
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
    if (argc == 3) {
        image = argv[1];
        image_path = (char *)".";
        binar = argv[2];
        binar_path = (char *)".";
    }

    image_engineer sgtc_img(imx6_ivtdcd_table, imx6_ivtdcd_table_size);
    return 0;
}
