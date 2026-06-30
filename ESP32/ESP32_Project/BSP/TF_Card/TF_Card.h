#ifndef __TF_CARD_H__
#define __TF_CARD_H__

#include <stdio.h>
#include <string.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#define BSP_SD_CLK          (48)
#define BSP_SD_CMD          (47)
#define BSP_SD_D0           (41)

// // microSD card
// #define SD_SCK 48
// #define SD_MISO 41
// #define SD_MOSI 47
// #define SD_CS 42

#define MOUNT_POINT              "/sdcard"
#define EXAMPLE_MAX_CHAR_SIZE    1024

void TF_Card_Init(void);
void TF_Card_Read_audio(const char *path);

#endif
