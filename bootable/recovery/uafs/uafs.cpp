/**
* Entry point of Android FileSystem Update.
* 2012-12-26, hufeng.mao@generalmobi.com
*/
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/reboot.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include "uafs.h"
#include "../common.h"
#include "../device.h"
#include "../ui.h" //ADD BY EVA

char* locale = NULL;//ADD BY EVA
extern int FSMain(void);

static void
check_and_fclose(FILE *fp, const char *name) {
    fflush(fp);
    if (ferror(fp)) LOGE("Error in %s\n(%s)\n", name, strerror(errno));
    fclose(fp);
}

// open a given path, mounting partitions as necessary
FILE*
fopen_path(const char *path, const char *mode) {

    FILE *fp = fopen(path, mode);
    return fp;
}

static long tmplog_offset = 0;
static void
copy_log_file(const char* source, const char* destination, int append) {
    FILE *log = fopen_path(destination, append ? "a" : "w");
    if (log == NULL) {
        LOGE("Can't open %s\n", destination);
    } else {
        FILE *tmplog = fopen(source, "r");
        if (tmplog != NULL) {
            if (append) {
                fseek(tmplog, tmplog_offset, SEEK_SET);  // Since last write
            }
            char buf[4096];
            while (fgets(buf, sizeof(buf), tmplog)) fputs(buf, log);
            if (append) {
                tmplog_offset = ftell(tmplog);
            }
            check_and_fclose(tmplog, source);
        }
        check_and_fclose(log, destination);
    }
}


#ifdef BOOT_IMAGE_UPDATE
/* for boot.img update */
static int check_boot_img_file() {
	if (access(BOOT_IMG_FILE, F_OK) != 0) {
        	return -1;
	}
	return 0;
}

static int run_boot_img_update(const char* boot_dev) {
	pid_t pid;
	int status;
	if((pid = fork()) < 0) {
		LOGE("fork error:%s", strerror(errno));
		return -1;
	}
	else if(pid == 0) {		// child
		printf("exec %s,\t%s,\t%s", FLASH_IMAGE_TOOL, boot_dev, BOOT_IMG_FILE);
		execlp(FLASH_IMAGE_TOOL, FLASH_IMAGE_TOOL, boot_dev, BOOT_IMG_FILE, NULL);
		printf("couldn't execute: %s, %s", FLASH_IMAGE_TOOL, strerror(errno));
		return -1;
	}
	
	if((pid = waitpid(pid, &status, 0)) < 0) {
		LOGE("waitpid error:%s", strerror(errno));
	}
	LOGI("chilid return code:%d", WEXITSTATUS(status));
	
	return WEXITSTATUS(status);
}

#endif


static int save_result(int ret) {
	FILE* fp;

	printf("%s: ret: %d", __func__, ret);

	fp = fopen(RESULT_FILE, "w+");
	if (!fp)
	{
		printf("%s: open result file %s failed.\n", __func__, RESULT_FILE);
		return -1;
	}
	fprintf(fp, "%d", ret);
	fclose(fp);
	chmod(RESULT_FILE, 0666);
	return 0;
} 

RecoveryUI* ui = NULL;
void ui_init() {
    Device* device = make_device();
    ui = device->GetUI();
    ui->Init();
    ui->SetBackground(RecoveryUI::INSTALLING_UPDATE); //MODIFY BY EVA
}
void ui_show_progress(float portion, float seconds) {
    ui->ShowProgress(portion, seconds);
}
void ui_set_progress(float portion) {
    ui->SetProgress(portion);
}

#define ui_print(...) ui->Print(__VA_ARGS__)


int main(int argc, char* argv[]) {
    time_t start = time(NULL);
    int result;

    freopen(TEMPORARY_LOG_FILE, "a", stdout); setbuf(stdout, NULL);
    freopen(TEMPORARY_LOG_FILE, "a", stderr); setbuf(stderr, NULL);

    ui_init();
    //ui_set_background(BACKGROUND_ICON_INSTALLING);

    ui_print("Starting %s on %s\n", argv[0], ctime(&start));
    
    // backup uafs if yun uafs in normal mode 
    if(strstr(argv[0], "uafs.backup") == NULL) {
		FILE *uafs = fopen(argv[0], "r");
		FILE *uafs_backup = fopen(GMT_DEFAULT_UA_LOCATION_B, "w");
        if (uafs != NULL) {
			char buf[4096];
			int readn;
		
            while ((readn = fread(buf, 1, sizeof(buf), uafs))>0) {
				fwrite(buf, 1, readn, uafs_backup);
			}
            check_and_fclose(uafs, argv[0]);
        }
		else {
			ui_print("Open %s error.\n", argv[0]);
		}
		check_and_fclose(uafs_backup, GMT_DEFAULT_UA_LOCATION_B);
			
		ui_print("Backup %s to %s success.\n", argv[0], GMT_DEFAULT_UA_LOCATION_B);
		struct stat statbuf;
		stat(argv[0], &statbuf);	
		chmod(GMT_DEFAULT_UA_LOCATION_B, statbuf.st_mode);
		ui_print("Chmod %s to %04o success.\n", GMT_DEFAULT_UA_LOCATION_B, statbuf.st_mode);
		sync();
    }
    
    ui_show_progress(1, 60);
    result = FSMain();
    ui_print("Uafs result...%d\n\n", result);

    if(result == 0) {
// update Boot Image only on FS update successfully.
#ifdef BOOT_IMAGE_UPDATE
    if(argc >= 2 && !check_boot_img_file()) {
    	char* boot_device = argv[1];
	int image_update_result;
	// example: for S120-KTOUCH,is /dev/block/mmcblk0p8
	ui_print("need update boot.img\n");
	
	// MAX 2 seconds.
	image_update_result = run_boot_img_update(boot_device);
	ui_print("boot image update result...%d\n", image_update_result);
	
	//sleep(2);
    }
#endif
        unlink(DELTA_FILE);
    }
    save_result(result);
    unlink(GMT_DEFAULT_UA_LOCATION_B);

    copy_log_file(TEMPORARY_LOG_FILE, LOG_FILE, 1);
    copy_log_file(TEMPORARY_LOG_FILE, LAST_LOG_FILE, 0);
    chmod(LOG_FILE, 0600);
    chown(LOG_FILE, 1000, 1000);   // system user
    chmod(LAST_LOG_FILE, 0640);
    
    return result;
}
