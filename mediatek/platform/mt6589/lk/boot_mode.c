/*
 * Copyright (c) 2008-2009 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/*This file implements MTK boot mode.*/

#include <sys/types.h>
#include <debug.h>
#include <err.h>
#include <reg.h>

#include <platform/mt_typedefs.h>
#include <platform/boot_mode.h>
#include <platform/mt_reg_base.h>

//Ivan added
#define MENU_BOOT_ENABLE 1

// global variable for specifying boot mode (default = NORMAL)
BOOTMODE g_boot_mode = NORMAL_BOOT;
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
extern BOOL kernel_power_off_charging_detection(void);
#endif
BOOL meta_mode_check(void)
{
	if(g_boot_mode == META_BOOT || g_boot_mode == ADVMETA_BOOT || g_boot_mode ==ATE_FACTORY_BOOT || g_boot_mode == FACTORY_BOOT)
	{
	  return TRUE;
	}
	else
	{	return FALSE;
	}
}


// check the boot mode : (1) meta mode or (2) recovery mode ...
void boot_mode_select(void)
{
//Ivan    
    ulong begin;
    
    if (meta_detection())
    {
      return;
    }

#if defined (HAVE_LK_TEXT_MENU)
    if(Check_RTC_PDN1_bit13())
    {
      printf("[FASTBOOT] reboot to boot loader\n");
      g_boot_mode = FASTBOOT;
      Set_Clr_RTC_PDN1_bit13(false);
   	  return;
    }
//Ivan
#ifdef MENU_BOOT_ENABLE
    boot_mode_dkey_check();
    if (g_boot_mode == MENU_BOOT)
    {
	    /* clean console screen */
	video_clean_screen();
//	mt65xx_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);

	video_set_cursor(2,0);
	video_printf("Recovery Mode: Volume Up\r\n");
	video_set_cursor(4,0);
	video_printf("Factory Mode: Volume Down\r\n");
//	mt65xx_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
	mt65xx_backlight_on();
	video_set_cursor(49,0);

	while (mt65XX_get_key() != 0xFFFF);
	printf(" > Key release!!!\n");
        mdelay(500);
	
//Ivan added 
	mtk_wdt_restart();
	begin = get_timer(0);
	
	while (g_boot_mode == MENU_BOOT)
	{
	    if (factory_detection())
	    {
		video_clean_screen();
		return;
	    }
	    if(boot_menu_detection())//recovery, fastboot, normal boot.
	    {
		video_clean_screen();
		return;
	    }
//Ivan add 20s time limit
	    if (get_timer(begin) < 20000)
		mtk_wdt_restart();
	}

//	video_clean_screen();
//	mt65xx_disp_fill_rect(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT, 0x0);
//	mt65xx_disp_update(0, 0, CFG_DISPLAY_WIDTH, CFG_DISPLAY_HEIGHT);
//	video_set_cursor(49,0);
//	mt65xx_disp_wait_idle();
    }

    if(Check_RTC_Recovery_Mode())
    {	
	    g_boot_mode = RECOVERY_BOOT;
    }

    recovery_check_command_trigger();
    
#else  //MENU_BOOT_ENABLE
    
    if (factory_detection())
    {
      return;
    }
    if(boot_menu_detection())//recovery, fastboot, normal boot.
    {
        return;
    }
    recovery_detection();
#endif //MENU_BOOT_ENABLE
#else	//HAVE_LK_TEXT_MENU
#ifdef MTK_FASTBOOT_SUPPORT
    if(fastboot_trigger())
    {
      return;
    }
#endif	//MTK_FASTBOOT_SUPPORT
    if (factory_detection())
    {
      return;
    }
    if(recovery_detection())
    {
      //**************************************
  		//* CHECK IMAGE
  		//**************************************
  		if(DRV_Reg32(0x40002300)==0xE92D4800)
  		{
  		  printf(" > do recovery_check\n");
  			//jump(0x40002300);
  		}
  		else
  		{
  		  printf(" > bypass recovery_check\n");
  		}
    	return;
    }
#endif
#ifdef MTK_KERNEL_POWER_OFF_CHARGING
	if(kernel_power_off_charging_detection())
	{
		printf(" < Kernel Power Off Charging Detection Ok> \n");
		return;
	}
	else
	{
		printf("< Kernel Enter Normal Boot > \n");
	}
#endif
}

CHIP_VER get_chip_eco_ver(void) /* TO BE REMOVED*/
{   
    return DRV_Reg32(APHW_VER);
}

unsigned int get_chip_sw_ver_code(void)
{  
    return DRV_Reg32(APSW_VER);
}

CHIP_SW_VER mt_get_chip_sw_ver(void)
{
    return (CHIP_SW_VER)get_chip_sw_ver_code();
}
