#ifndef _UAFS_H_
#define _UAFS_H_

/* common const string and defines */

#define BOOT_IMAGE_UPDATE 1

#define GMT_DEFAULT_UA_LOCATION 	"/system/bin/uafs"
#define GMT_DEFAULT_UA_LOCATION_B 	"/system/bin/uafs.backup"

#define TEMPORARY_LOG_FILE 	"/data/uafs.log"
#define LOG_FILE 		"/data/data/com.redbend.dmClient/files/uafs.log"
#define LAST_LOG_FILE		"/data/data/com.redbend.dmClient/files/uafs.last_log"

#ifdef BOOT_IMAGE_UPDATE
/* for boot.img update */
#define BOOT_IMG_FILE		"/system/boot.img"
#define FLASH_IMAGE_TOOL	"/system/bin/flash_image_gmobi"
#endif

#define DELTA_FILE 		"/data/data/com.redbend.dmClient/files/vdm_update"
#define RESULT_FILE 		"/data/data/com.redbend.dmClient/files/vrm_result"

// for android 4.1.x
#include "../ui.h"
extern RecoveryUI* ui;
void ui_init();
void ui_show_progress(float portion, float seconds);
void ui_set_progress(float portion);
#define ui_print(...) ui->Print(__VA_ARGS__)


#endif
