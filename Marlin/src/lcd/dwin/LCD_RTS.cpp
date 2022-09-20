#include "LCD_RTS.h"
#include "lcdAutoUI.h"
#include <arduino.h>
// #include <wstring.h>
#include <stdio.h>
// #include <libmaple/usart.h>
#include "../../inc/MarlinConfig.h"
#include "../../../Version.h"
#include "../../MarlinCore.h"
#include "../../sd/cardreader.h"
#include "../../module/temperature.h"
#include "../../module/planner.h"
#include "../../module/stepper.h"
#include "../../module/settings.h"
#include "../../module/printcounter.h"
#include "../../module/probe.h"
#include "../../feature/babystep.h"
#include "../../gcode/gcode.h"
#include "../../feature/bedlevel/mbl/mesh_bed_leveling.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "../../libs/duration_t.h"
#include "../../../Configuration.h"
#include "../../libs/BL24CXX.h"
#include "../../feature/powerloss.h"
#include "../../debug/debug.h"

#if ENABLED(BLTOUCH)
  #include "endstops.h"
#endif

// #define DEFAULT_LANGUAGE      1     /* default language that display; 1: CN, 2: EN */
#define LANG_NUM_SUPPORT      2     /* number of language type that support        */

char errorway = 0;
char errornum = 0;
char error_sd_num = 0;

float zprobe_zoffset = 0.00;
float last_zoffset = 0.0;

int power_off_type_yes = 0;

const float manual_feedrate_mm_m[] = { 50*60, 50*60, 4*60, 60 };

HMI_ValueTypeDef HMI_ValueStruct;

int startprogress = 0;
CRec CardRecbuf;
int temphot = 0;
int tempbed = 0;
int PLAhead = 0;
int PLAbed = 0;
int PLAfan = 0;
int ABShead = 0;
int ABSbed = 0;
int ABSfan = 0;
xyze_pos_t pause_position;
// float pause_x = 0, pause_y = 0, pause_z = 0, pause_e = 0;
bool filament_pause_check = true;
bool door_pause_check = true;
bool pause_action_flag = false;

millis_t next_rts_update_ms = 0;
millis_t start_print_time = 0;

int last_target_temperature[4] = {0};
int last_target_temperature_bed;
// 0: no waiting  6: home in level page  7: waiting to stop print  8: home in manual level
char waitway = 0;
int change_page_font = 0;
int recnum = 0;
unsigned char Percentrecord = 0;
float FilamentLOAD = 0;

// 1 represent 10mm, 2 represent 1mm, 3 represent 0.1mm
unsigned char AxisUnitMode;
float axis_unit = 10.0;

// represents to update file list
bool CardUpdate = false;
extern CardReader card;
bool first_load_language = 0;
unsigned int language_change_font = DEFAULT_LANGUAGE;
// represents SD-card status, true means SD is available, false means opposite.
bool lcd_sd_status = 0;

char Checkfilenum = 0;
char checkpause = 0;
char FilenamesCount = 0;

bool LEDStatus = true;
bool WIFILEDStatus = true;
RTSSHOW rtscheck;
unsigned long VolumeSet = 0x80;

bool print_finish = false;
bool finish_home = false;

// flag
bool gStopPrintFlag = false;   /* true: device need to stop print; false:keep current state */
bool heat_flag = false;
bool PowerOn_flag = true;
bool CheckDoor_flag = true;
bool home_flag = false;
bool G29_flag = false;
bool AutohomeZflag = false;
bool start_manual_flag = false;
bool TakeTurn_heat_flag = false;
char cmdbuf[20] = {0};

unsigned short int checktime = 0;

/**
 * [ZaxisMoveToZMax :make Z axis get to position of Z-Max-Value]
 * @Author Creality
 * @Time   2021-04-27
 */
void ZaxisMoveToZMax(void)
{
    char command[50];
    
    memset(command, 0, sizeof(command));
    sprintf(command, "%s%s%s%d", "G1 ", " F600", " Z", Z_MAX_POS);
    queue.enqueue_now_P(command);

    current_position[Z_AXIS] = Z_MAX_POS;
    rtscheck.RTS_SndData(10*current_position[Z_AXIS], AXIS_Z_COORD_VP);
}

void RTS_line_to_current(AxisEnum axis) 
{
  if (!planner.is_full())
  {
    planner.buffer_line(current_position, MMM_TO_MMS(manual_feedrate_mm_m[(int8_t)axis]), active_extruder);
  }
}

void Take_Turn_Preheat(void)
{
  if((thermalManager.temp_bed.celsius >= (thermalManager.temp_bed.target - 2)) && TakeTurn_heat_flag)
  {
    if(thermalManager.temp_bed.target == HMI_ValueStruct.preheat_bed_temp[0])
    {
        thermalManager.setTargetHotend(HMI_ValueStruct.preheat_hotend_temp[0], 0);
        rtscheck.RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[0], HEAD_SET_TEMP_VP);
    }
    else if(thermalManager.temp_bed.target == HMI_ValueStruct.preheat_bed_temp[1])
    {
        thermalManager.setTargetHotend(HMI_ValueStruct.preheat_hotend_temp[1], 0);
        rtscheck.RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[1], HEAD_SET_TEMP_VP);
    }
    TakeTurn_heat_flag = false;
  }
}

float offset_negative_001(float cmd)
{
  if(cmd < 0) return cmd - (float)(0.001);
  else return cmd + (float)(0.001);
}

RTSSHOW::RTSSHOW()
{
  recdat.head[0] = snddat.head[0] = FHONE;
  recdat.head[1] = snddat.head[1] = FHTWO;
  memset(databuf, 0, sizeof(databuf));
}

#if 0
void RTSSHOW::RTS_SDCardInit(void)
{
  if(!IS_SD_INSERTED())
  {
    card.mount(); 
  }
  delay(2);
  if(IS_SD_INSERTED())
  {
    uint16_t fileCnt = card.get_num_Files();
    card.getWorkDirName();
    if (card.filename[0] == '/') 
    {
      card.mount();
    }
    else 
    {
      card.cdup();
    }

    int addrnum = 0;
    int num = 0;
    for(uint16_t i = 0;(i < fileCnt) && (i < MaxFileNumber + addrnum);i ++) 
    {
      card.selectFileByIndex(fileCnt - 1 - i);
      char *pointFilename = card.longFilename;
      int filenamelen = strlen(card.longFilename);
      int j = 1;
      while((strncmp(&pointFilename[j],".gcode",6) && strncmp(&pointFilename[j],".GCODE",6)) && ((j++) < filenamelen));
      if(j >= filenamelen)
      {
        addrnum++;
        continue;
      }

      if(j >= TEXTBYTELEN)
      {
        strncpy(&card.longFilename[TEXTBYTELEN -3], "~~", 2);
        card.longFilename[TEXTBYTELEN-1] = '\0';
        j = TEXTBYTELEN-1;
      }

      delay(3);
      strncpy(CardRecbuf.Cardshowfilename[num], card.longFilename, j);

      strcpy(CardRecbuf.Cardfilename[num], card.filename);
      CardRecbuf.addr[num] = FILE1_TEXT_VP + num * 10;
      RTS_SndData(CardRecbuf.Cardshowfilename[num],CardRecbuf.addr[num]);
      CardRecbuf.Filesum = (++num);
    }
    lcd_sd_status = IS_SD_INSERTED();
    SERIAL_ECHOLN("***Card is inserted***");
  }
  else
  {
    SERIAL_ECHOLN("***Card is removed***");
  }
}
#endif

void RTSSHOW::RTS_SDCardInit(void)
{
    // if(RTS_SD_Detected()) 
    {
      // card.mount();

      if(CardReader::flag.mounted) {
        //SD卡挂载成功
        SERIAL_ECHOLN("***sd card mount success***");

        uint16_t fileCnt = card.get_num_Files();
        card.getWorkDirName();
        if (card.filename[0] != '/') {
          card.cdup();
        }

        int addrnum =0;
        int num = 0;
        for (uint16_t i = 0; i < fileCnt && i < MaxFileNumber + addrnum; i++) 
        {
          card.selectFileByIndex(fileCnt-1-i);
          char *pointFilename = card.longFilename;//longFilename这个数组存放的是真实的文件名
          int filenamelen = strlen(card.longFilename);
          int j = 1;
          while((strncmp(&pointFilename[j],".gcode",6) && strncmp(&pointFilename[j],".GCODE",6)) && (j++) < filenamelen);
          if(j >= filenamelen)
          {
            addrnum++;
            continue;
          }
          if(j >= TEXTBYTELEN)  
          {
            strncpy(&card.longFilename[TEXTBYTELEN -3],"..",2);
            card.longFilename[TEXTBYTELEN-1] = '\0';
            j = TEXTBYTELEN-1;
          }
          //delay(3);
          strncpy(CardRecbuf.Cardshowfilename[num], card.longFilename,j);
          strcpy(CardRecbuf.Cardfilename[num], card.filename);
          // CardRecbuf.addr[num] = SDFILE_ADDR +num*10;
          CardRecbuf.Filesum = (++num);
        }
        lcd_sd_status = true;
        return;
      }
    }
    //未检测到SD卡，或者挂载失败
    // SERIAL_ECHOLN("***sd not detected, or init failed***");
}

bool RTSSHOW::RTS_SD_Detected(void)
{
  static bool last, state, flag_stable;
  static uint32_t stable_point_time;

  bool tmp = IS_SD_INSERTED();

  if(tmp != last)
    flag_stable = false;
  else {
    //两次检测到引脚状态相同,标记引脚稳定
    if(!flag_stable) {
      flag_stable = true;
      stable_point_time = millis();
    }
  }

  if(flag_stable) {
    if(millis() - stable_point_time > 30) state = tmp;
  }

  last = tmp;

  return state;
}

void RTSSHOW::RTS_SDCardUpate(void)
{
  const bool sd_status = IS_SD_INSERTED();
  if (sd_status != lcd_sd_status)
  {
    if(sd_status)
    {
      card.mount();
      RTS_SDCardInit();
      if(recovery.info.recovery_flag) power_off_type_yes = 0;
    }
    else
    {
      card.release();
      for(int i = 0;i < CardRecbuf.Filesum;i ++)
      {
        for(int j = 0;j < 10;j++)
        RTS_SndData(0, CardRecbuf.addr[i] + j);
        // white
        RTS_SndData((unsigned long)0xFFFF, FilenameNature + (i + 1) * 16);
      }

      for(int j = 0;j < 10;j ++)
      {
        // clean screen.
        RTS_SndData(0, CONTINUE_PRINT_FILE_TEXT_VP + j);
      }
      // clean filename Icon
      for(int j = 1;j <= 20;j ++)
      {
        RTS_SndData(10, FILE1_SELECT_ICON_VP - 1 + j);
      }

      if(printingIsActive() || printingIsPaused())
      {
        RTS_SDcard_Stop();
      }

      memset(&CardRecbuf, 0, sizeof(CardRecbuf));
    }
    lcd_sd_status = sd_status;
  }

  // represents to update file list
  if(CardUpdate && lcd_sd_status && IS_SD_INSERTED())
  {
    for(uint16_t i = 0;i < CardRecbuf.Filesum;i ++) 
    {
      delay(3);
      RTS_SndData(CardRecbuf.Cardshowfilename[i], CardRecbuf.addr[i]);
      RTS_SndData((unsigned long)0xFFFF,FilenameNature + (i+1)*16);
      RTS_SndData(10, FILE1_SELECT_ICON_VP + i);
    }
    CardUpdate = false;
  }
}

void lcd_select_language(void)
{
  #if 0
  BL24CXX_Read(FONT_EEPROM+2, (uint8_t*)&first_load_language, sizeof(first_load_language));
  delay(10);
  if(first_load_language == 0)
  {
    BL24CXX_Read(FONT_EEPROM, (uint8_t*)&language_change_font, 1);
    if(language_change_font < 1 || language_change_font > 9) language_change_font = DEFAULT_LANGUAGE;
  }
  else
  {
    first_load_language = 0;
    BL24CXX_Write(FONT_EEPROM+2, (uint8_t*)&first_load_language, sizeof(first_load_language));
    if(DEFAULT_LANGUAGE == 1) language_change_font = 1;
    else if(DEFAULT_LANGUAGE == 2) language_change_font = 2;
    else if(DEFAULT_LANGUAGE == 3) language_change_font = 3;
    else if(DEFAULT_LANGUAGE == 4) language_change_font = 4;
    else if(DEFAULT_LANGUAGE == 5) language_change_font = 5;
    else if(DEFAULT_LANGUAGE == 6) language_change_font = 6;
    else if(DEFAULT_LANGUAGE == 7) language_change_font = 7;
    else if(DEFAULT_LANGUAGE == 8) language_change_font = 8;
    else if(DEFAULT_LANGUAGE == 9) language_change_font = 9;
    BL24CXX_Write(FONT_EEPROM, (uint8_t*)&language_change_font, 1);
  }
  #endif
    /* modify by clh 20210505 */
    BL24CXX::read(FONT_EEPROM, (uint8_t*)&language_change_font, 1);
    if(language_change_font < 1 || language_change_font > LANG_NUM_SUPPORT)
    {
        /* 'language_change_font' is out of range or loading language firstly, so display deault language */
        language_change_font = DEFAULT_LANGUAGE;
        /* record language type that display now in EEPROM at address of 'FONT_EEPROM' */
        BL24CXX::write(FONT_EEPROM, (uint8_t*)&language_change_font, 1);
    }
    else
    {
        /* display language by value of 'language_change_font' that load from 'EEPROM', execute by function 'lcd_eight_language()' */
    }
}

void lcd_eight_language(void)
{
  rtscheck.RTS_SndData(language_change_font, Home_VP);
  rtscheck.RTS_SndData(language_change_font, Home_print_VP);
  rtscheck.RTS_SndData(language_change_font, Home_prepare_VP);
  rtscheck.RTS_SndData(language_change_font, Home_control_VP);
  rtscheck.RTS_SndData(language_change_font, Home_level_VP);
  rtscheck.RTS_SndData(language_change_font, File_select_VP);
  // rtscheck.RTS_SndData(language_change_font, Back_VP);
  rtscheck.RTS_SndData(language_change_font, Printing_VP);
  rtscheck.RTS_SndData(language_change_font, Print_time_VP);
  rtscheck.RTS_SndData(language_change_font, Print_finish_VP);
  rtscheck.RTS_SndData(language_change_font, Print_setup_VP);
  rtscheck.RTS_SndData(language_change_font, Print_pause_VP);
  rtscheck.RTS_SndData(language_change_font, Print_stop_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_speed_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_hotend_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_bed_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_Zoffset_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_fan_VP);
  rtscheck.RTS_SndData(language_change_font, Setup_light_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_move_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_E_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_motor_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_PLA_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_ABS_VP);
  rtscheck.RTS_SndData(language_change_font, Perpare_cool_VP);
  rtscheck.RTS_SndData(language_change_font, Move_VP);
  rtscheck.RTS_SndData(language_change_font, Move_unit_VP);
  rtscheck.RTS_SndData(language_change_font, Move_X_VP);
  rtscheck.RTS_SndData(language_change_font, Move_Y_VP);
  rtscheck.RTS_SndData(language_change_font, Move_Z_VP);
  rtscheck.RTS_SndData(language_change_font, Feed_Retrun_VP);
  rtscheck.RTS_SndData(language_change_font, Feed_VP);
  rtscheck.RTS_SndData(language_change_font, Return_VP);
  rtscheck.RTS_SndData(language_change_font, Control_VP);
  rtscheck.RTS_SndData(language_change_font, Control_temp_VP);
  rtscheck.RTS_SndData(language_change_font, Control_light_VP);
  rtscheck.RTS_SndData(language_change_font, Control_language_VP);
  rtscheck.RTS_SndData(language_change_font, Control_recover_VP);
  rtscheck.RTS_SndData(language_change_font, Control_info_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_hotend_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_bed_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_fan_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_PLA_VP);
  rtscheck.RTS_SndData(language_change_font, Temp_ABS_VP);
  rtscheck.RTS_SndData(language_change_font, PLA_VP);
  rtscheck.RTS_SndData(language_change_font, PLA_hotend_VP);
  rtscheck.RTS_SndData(language_change_font, PLA_bed_VP);
  rtscheck.RTS_SndData(language_change_font, PLA_save_VP);
  rtscheck.RTS_SndData(language_change_font, ABS_VP);
  rtscheck.RTS_SndData(language_change_font, ABS_hotend_VP);
  rtscheck.RTS_SndData(language_change_font, ABS_bed_VP);
  rtscheck.RTS_SndData(language_change_font, ABS_save_VP);
  rtscheck.RTS_SndData(language_change_font, Info_VP);
  rtscheck.RTS_SndData(language_change_font, Info_size_VP);
  rtscheck.RTS_SndData(language_change_font, Info_version_VP);
  rtscheck.RTS_SndData(language_change_font, Info_call_VP);
  rtscheck.RTS_SndData(language_change_font, Level_VP);
  rtscheck.RTS_SndData(language_change_font, Level_Zoffset_VP);
  rtscheck.RTS_SndData(language_change_font, Level_mode_VP);
  rtscheck.RTS_SndData(language_change_font, Level_auto_VP);
  rtscheck.RTS_SndData(language_change_font, Window_Filament_UseUp_VP);
  rtscheck.RTS_SndData(language_change_font, Window_HeatUp_VP);
  rtscheck.RTS_SndData(language_change_font, Window_StopPrint_VP);
  rtscheck.RTS_SndData(language_change_font, Window_Filament_load_VP);
  rtscheck.RTS_SndData(language_change_font, Window_Print_VP);
  rtscheck.RTS_SndData(language_change_font, Window_pause_VP);
  rtscheck.RTS_SndData(language_change_font, Window_confirm_VP);
  rtscheck.RTS_SndData(language_change_font, Window_cancel_VP);
  rtscheck.RTS_SndData(language_change_font, Window_stop_VP);
  rtscheck.RTS_SndData(language_change_font, Window_recover_VP);
  rtscheck.RTS_SndData(language_change_font, Window_home_VP);
  rtscheck.RTS_SndData(language_change_font, Eight_language_VP);
  // rtscheck.RTS_SndData(language_change_font, Leve_heat_VP);
  // rtscheck.RTS_SndData(language_change_font, PoweOn_VP);
  rtscheck.RTS_SndData(language_change_font, CheckDoor_VP);
  rtscheck.RTS_SndData(language_change_font, Window_done_VP);
  rtscheck.RTS_SndData(language_change_font, Box_fan_VP);
  rtscheck.RTS_SndData(language_change_font, WIFI_LED_VP);
}

void RTSSHOW::RTS_Init()
{
  lcd_select_language();
  /* wait for DWIN LCD ready to communication */
  delay(500);
  lcd_eight_language();
  BL24CXX::read(FONT_EEPROM+4, (uint8_t*)&CheckDoor_flag, 1);
  AxisUnitMode = 1;
  #if HAS_BED_PROBE
    last_zoffset = probe_offset.z;
    RTS_SndData(probe_offset.z * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
  #elif ENABLED(MESH_BED_LEVELING)
    last_zoffset = mbl.z_offset;
    RTS_SndData(offset_negative_001(mbl.z_offset) * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
    RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
  #else
    last_zoffset = zprobe_zoffset;
    RTS_SndData(zprobe_zoffset * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
  #endif
  last_target_temperature[0] = thermalManager.temp_hotend[0].target;
  last_target_temperature_bed = thermalManager.temp_bed.target;
  feedrate_percentage = 100;
  RTS_SndData(feedrate_percentage, PRINT_SPEED_RATE_VP);

  /***************turn off motor*****************/
  RTS_SndData(2, MOTOR_FREE_ICON_VP); 

  /***************transmit temperature to screen*****************/
  RTS_SndData(0, HEAD_SET_TEMP_VP);
  RTS_SndData(0, BED_SET_TEMP_VP);
  RTS_SndData(thermalManager.temp_hotend[0].celsius, HEAD_CURRENT_TEMP_VP);
  RTS_SndData(thermalManager.temp_bed.celsius, BED_CURRENT_TEMP_VP);
  // perheat temperture to screen
  RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[0], PLA_HEAD_SET_DATA_VP);
  RTS_SndData(HMI_ValueStruct.preheat_bed_temp[0], PLA_BED_SET_DATA_VP);
  RTS_SndData(HMI_ValueStruct.preheat_fan_speed[0], PLA_FAN_SET_DATA_VP);
  RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[1], ABS_HEAD_SET_DATA_VP);
  RTS_SndData(HMI_ValueStruct.preheat_bed_temp[1], ABS_BED_SET_DATA_VP);
  RTS_SndData(HMI_ValueStruct.preheat_fan_speed[1], ABS_FAN_SET_DATA_VP);

  /***************transmit Fan speed to screen*****************/
  #if FAN_COUNT > 0
    // turn off fans
    for (uint8_t i = 0; i < FAN_COUNT; i++) thermalManager.fan_speed[i] = 255;
  #endif
  RTS_SndData(1, POWERON_ICON_VP);
  if(CheckDoor_flag) RTS_SndData(1, CHECKDOOR_ICON_VP);
  else RTS_SndData(2, CHECKDOOR_ICON_VP);
  RTS_SndData(1, PRINTER_FANOPEN_TITLE_VP);
  RTS_SndData(1, BOXFAN_ICON_VP); 
  RTS_SndData(2, WIFILED_ICON_VP);
  RTS_SndData(2, PRINTER_LEDOPEN_TITLE_VP);
  LEDStatus = true;
  WIFILEDStatus = true;

  /*********transmit SD card filename to screen***************/
  RTS_SDCardInit();
  /***************transmit Printer information to screen*****************/
  // clean filename
  for(int j = 0;j < 20;j ++)
  {
    RTS_SndData(0, PRINTER_MACHINE_TEXT_VP + j);
  }
  RTS_SndData(SOFTVERSION, PRINTER_VERSION_TEXT_VP);
  RTS_SndData(MACHINE_SIZE, PRINTER_PRINTSIZE_TEXT_VP);
  if(language_change_font == 1)
    RTS_SndData(CORP_WEBSITE_C, PRINTER_WEBSITE_TEXT_VP);
  else
    RTS_SndData(CORP_WEBSITE_E, PRINTER_WEBSITE_TEXT_VP);

  /**************************some info init*******************************/
  RTS_SndData(0, PRINT_PROCESS_TITLE_VP);

  /************************clean screen*******************************/
  for(int i = 0;i < MaxFileNumber;i ++)
  {
    for(int j = 0;j < 10;j ++)
    {
      RTS_SndData(0, FILE1_TEXT_VP + i * 10 + j);
    }
  }

  for(int j = 0;j < 10;j ++)
  {
    // clean screen.
    RTS_SndData(0, CONTINUE_PRINT_FILE_TEXT_VP + j);
  }
  for(int j = 1;j <= MaxFileNumber;j ++)
  {
    RTS_SndData(10, FILE1_SELECT_ICON_VP - 1 + j);
  }

  rtscheck.RTS_SndData(ExchangePageBase, ExchangepageAddr);
  change_page_font = 0;
  for(startprogress = 0; startprogress <= 100; startprogress++)
  {
    rtscheck.RTS_SndData(startprogress, START_PROCESS_ICON_VP);
    delay(30);
  }

  rtscheck.RTS_SndData(StartSoundSet, SoundAddr);

  if(language_change_font != 0)
  {
    rtscheck.RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
    change_page_font = 1;
  }
  RTS_Backlight();
  SERIAL_ECHOLN("===Initing RTS has finished===");
}

int RTSSHOW::RTS_RecData()
{
  while(MYSERIAL1.available() > 0 && (recnum < SizeofDatabuf))
  {
    delay(1);
    databuf[recnum] = MYSERIAL1.read();

    if(databuf[0] == FHONE)
    {
      recnum++;
    }
    else if(databuf[0] == FHTWO)
    {
      databuf[0] = FHONE;
      databuf[1] = FHTWO;
      recnum += 2;
    }
    else if(databuf[0] == FHLENG)
    {
      databuf[0] = FHONE;
      databuf[1] = FHTWO;
      databuf[2] = FHLENG;
      recnum += 3;
    }
    else if(databuf[0] == VarAddr_R)
    {
      databuf[0] = FHONE;
      databuf[1] = FHTWO;
      databuf[2] = FHLENG;
      databuf[3] = VarAddr_R;
      recnum += 4;
    }
    else
    {
      recnum = 0;
    }
  }

  // receive nothing    
  if(recnum < 1)
  {
    return -1;
  }
  else  if((recdat.head[0] == databuf[0]) && (recdat.head[1] == databuf[1]) && recnum > 2)
  {
    // gDebug.PrintBytesHex(databuf, recnum);

    recdat.len = databuf[2];
    recdat.command = databuf[3];
    if(recdat.len == 0x03 && (recdat.command == 0x82 || recdat.command == 0x80) && (databuf[4] == 0x4F) && (databuf[5] == 0x4B))  //response for writing byte
    {   
      memset(databuf,0, sizeof(databuf));
      recnum = 0;
      return -1;
    }
    else if(recdat.command == 0x83)
    {
      // response for reading the data from the variate
      recdat.addr = databuf[4];
      recdat.addr = (recdat.addr << 8 ) | databuf[5];
      recdat.bytelen = databuf[6];
      for(unsigned int i = 0;i < recdat.bytelen;i+=2)
      {
        recdat.data[i/2]= databuf[7+i];
        recdat.data[i/2]= (recdat.data[i/2] << 8 )| databuf[8+i];
      }
    }
    else if(recdat.command == 0x81)
    {
      // response for reading the page from the register
      recdat.addr = databuf[4];
      recdat.bytelen = databuf[5];
      for(unsigned int i = 0;i < recdat.bytelen;i++)
      {
        recdat.data[i]= databuf[6+i];
        // recdat.data[i]= (recdat.data[i] << 8 )| databuf[7+i];
      }
    }
  }
  else
  {
    memset(databuf,0, sizeof(databuf));
    recnum = 0;
    // receive the wrong data
    return -1;
  }
  memset(databuf,0, sizeof(databuf));
  recnum = 0;
  return 2;
}
// 
// int RTSSHOW::RTS_RecData()
// {
//   static bool recvflag = false;
//   static int recvnum = 0;
//   static unsigned char databuf[16];

//   // 解析数据帧
//   if (MYSERIAL1.available() > 0 && recvnum < (signed)sizeof(databuf)) {
//     databuf[recvnum++] = MYSERIAL1.read();

//     #define RECV_DEBUG
//     #if defined(RECV_DEBUG)
//       char buf[16];
//       sprintf_P(buf, PSTR("%02x "), databuf[recvnum - 1]);
//       serialprintPGM(buf);
//     #endif

//     if(recvnum == 1 && databuf[0] != 0x5A) {
//       recvnum = 0;
//     }
//     else if(recvnum == 2 && databuf[1] != 0xA5) {
//       recvnum = 0;
//     }
//     else if(recvnum >= 3 && databuf[2] == (recvnum - 3)) {
//       recvflag = true;
//       recvnum = 0;

//       #if defined(RECV_DEBUG)
//         serialprintPGM("\n");
//         SERIAL_ECHO_MSG("dwin rx ok");
//       #endif
//     }
//   }

//   // 响应
//   if(recvflag == true)
//   {
//     recdat.len = databuf[2];
//     recdat.command = databuf[3];
//     if(recdat.len == 0x03 && (recdat.command == 0x82 || recdat.command == 0x80) && (databuf[4] == 0x4F) && (databuf[5] == 0x4B))  //response for writing byte
//     {   
//       memset(databuf,0, sizeof(databuf));
//       recnum = 0;
//       return -1;
//     }
//     else if(recdat.command == 0x83)
//     {
//       // response for reading the data from the variate
//       recdat.addr = databuf[4];
//       recdat.addr = (recdat.addr << 8 ) | databuf[5];
//       recdat.bytelen = databuf[6];
//       for(unsigned int i = 0;i < recdat.bytelen;i+=2)
//       {
//         recdat.data[i/2]= databuf[7+i];
//         recdat.data[i/2]= (recdat.data[i/2] << 8 )| databuf[8+i];
//       }
//     }
//     else if(recdat.command == 0x81)
//     {
//       // response for reading the page from the register
//       recdat.addr = databuf[4];
//       recdat.bytelen = databuf[5];
//       for(unsigned int i = 0;i < recdat.bytelen;i++)
//       {
//         recdat.data[i]= databuf[6+i];
//         // recdat.data[i]= (recdat.data[i] << 8 )| databuf[7+i];
//       }
//     }

    
//     recvflag = false;

//     return 1;
//   }

//   return -1;
// }

void RTSSHOW::RTS_SndData(void)
{
  if((snddat.head[0] == FHONE) && (snddat.head[1] == FHTWO) && (snddat.len >= 3))
  {
    databuf[0] = snddat.head[0];
    databuf[1] = snddat.head[1];
    databuf[2] = snddat.len;
    databuf[3] = snddat.command;
    // to write data to the register
    if(snddat.command == 0x80)
    {
      databuf[4] = snddat.addr;
      for(int i =0;i <(snddat.len - 2);i++)
      {
        databuf[5 + i] = snddat.data[i];
      }
    }
    else if((snddat.len == 3) && (snddat.command == 0x81))
    {
      // to read data from the register
      databuf[4] = snddat.addr;
      databuf[5] = snddat.bytelen;
    }
    else if(snddat.command == 0x82)
    {
      // to write data to the variate
      databuf[4] = snddat.addr >> 8;
      databuf[5] = snddat.addr & 0xFF;
      for(int i =0;i <(snddat.len - 3);i += 2)
      {
        databuf[6 + i] = snddat.data[i/2] >> 8;
        databuf[7 + i] = snddat.data[i/2] & 0xFF;
      }
    }
    else if((snddat.len == 4) && (snddat.command == 0x83))
    {
      // to read data from the variate
      databuf[4] = snddat.addr >> 8;
      databuf[5] = snddat.addr & 0xFF;
      databuf[6] = snddat.bytelen;
    }
    for(int i = 0;i < (snddat.len + 3);i ++)
    {
      MYSERIAL1.write(databuf[i]);
      delayMicroseconds(1);
    }
    // usart_tx(MYSERIAL1.c_dev(), databuf, snddat.len + 3);
    // MYSERIAL1.flush();

    memset(&snddat, 0, sizeof(snddat));
    memset(databuf, 0, sizeof(databuf));
    snddat.head[0] = FHONE;
    snddat.head[1] = FHTWO;
  }
}

void RTSSHOW::RTS_SndData(const String &s, unsigned long addr, unsigned char cmd /*= VarAddr_W*/)
{
  if(s.length() < 1)
  {
    return;
  }
  RTS_SndData(s.c_str(), addr, cmd);
}

void RTSSHOW::RTS_SndData(const char *str, unsigned long addr, unsigned char cmd/*= VarAddr_W*/)
{
  int len = strlen(str);
  if(len > 0)
  {
    databuf[0] = FHONE;
    databuf[1] = FHTWO;
    databuf[2] = 3+len;
    databuf[3] = cmd;
    databuf[4] = addr >> 8;
    databuf[5] = addr & 0x00FF;
    for(int i = 0;i < len;i ++)
    {
      databuf[6 + i] = str[i];
    }

    for(int i = 0;i < (len + 6);i ++)
    {
      MYSERIAL1.write(databuf[i]);
      delayMicroseconds(1);
    }
    memset(databuf, 0, sizeof(databuf));
  }
}

void RTSSHOW::RTS_SndData(char c, unsigned long addr, unsigned char cmd/*= VarAddr_W*/)
{
  snddat.command = cmd;
  snddat.addr = addr;
  snddat.data[0] = (unsigned long)c;
  snddat.data[0] = snddat.data[0] << 8;
  snddat.len = 5;
  RTS_SndData();
}

void RTSSHOW::RTS_SndData(unsigned char* str, unsigned long addr, unsigned char cmd){RTS_SndData((char *)str, addr, cmd);}

void RTSSHOW::RTS_SndData(int n, unsigned long addr, unsigned char cmd/*= VarAddr_W*/)
{
  if(cmd == VarAddr_W )
  {
    if(n > 0xFFFF)
    {
      snddat.data[0] = n >> 16;
      snddat.data[1] = n & 0xFFFF;
      snddat.len = 7;
    }
    else
    {
      snddat.data[0] = n;
      snddat.len = 5;
    }
  }
  else if(cmd == RegAddr_W)
  {
    snddat.data[0] = n;
    snddat.len = 3;
  }
  else if(cmd == VarAddr_R)
  {
    snddat.bytelen = n;
    snddat.len = 4;
  }
  snddat.command = cmd;
  snddat.addr = addr;
  RTS_SndData();
}

// void RTSSHOW::RTS_SndData(unsigned int n, unsigned long addr, unsigned char cmd){ RTS_SndData((int)n, addr, cmd); }
void RTSSHOW::RTS_SndData(unsigned int n, unsigned long addr, unsigned char cmd)
{ 
  if(cmd == VarAddr_W )
  {
      snddat.data[0] = n >> 16;
      snddat.data[1] = n & 0xFFFF;
      snddat.len = 7;
  }
  else if(cmd == RegAddr_W)
  {
    snddat.data[0] = n;
    snddat.len = 3;
  }
  else if(cmd == VarAddr_R)
  {
    snddat.bytelen = n;
    snddat.len = 4;
  }
  snddat.command = cmd;
  snddat.addr = addr;
  RTS_SndData();
}

void RTSSHOW::RTS_SndData(float n, unsigned long addr, unsigned char cmd){ RTS_SndData((int)n, addr, cmd); }

void RTSSHOW::RTS_SndData(long n, unsigned long addr, unsigned char cmd){ RTS_SndData((unsigned long)n, addr, cmd); }

void RTSSHOW::RTS_SndData(unsigned long n, unsigned long addr, unsigned char cmd/*= VarAddr_W*/)
{
  if(cmd == VarAddr_W )
  {
    if(n > 0xFFFF)
    {
      snddat.data[0] = n >> 16;
      snddat.data[1] = n & 0xFFFF;
      snddat.len = 7;
    }
    else
    {
      snddat.data[0] = n;
      snddat.len = 5;
    }
  }
  else if(cmd == VarAddr_R)
  {
    snddat.bytelen = n;
    snddat.len = 4;
  }
  snddat.command = cmd;
  snddat.addr = addr;
  RTS_SndData();
}

void RTSSHOW::RTS_Backlight(unsigned short light/* = Var_light*/, unsigned short time/* = Var_time*/)
{
  unsigned char Backlight_buf[10] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x82, 0x64, 0x20, 0xEA, 0x60}; // 10 minutes, reduce the backlight to 0x20 ( light )
  
  if(light != Var_light) Backlight_buf[7] = light;

  if(time != Var_time)
  {
    Backlight_buf[8] = time >> 8;
    Backlight_buf[9] = time & 0xFF;
  }

  // usart_tx(MYSERIAL1.c_dev(), Backlight_buf, 10);
  // MYSERIAL1.flush();
  for(int i = 0; i < 10; i ++)
  {
    MYSERIAL1.write(Backlight_buf[i]);
    delayMicroseconds(1);
  }
}

void RTSSHOW::RTS_SDcard_Stop()
{
  waitway = 7;   // used in home
  if(home_flag) planner.synchronize();
  if(heat_flag)
  {
    card.stopSDPrint();
    queue.clear();
    quickstop_stepper();
    print_job_timer.stop();
    thermalManager.disable_all_heaters();
    thermalManager.zero_fan_speeds();
    wait_for_heatup = false;
    #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
      card.removeJobRecoveryFile();
    #endif
    #ifdef EVENT_GCODE_SD_STOP
      queue.inject_P(PSTR(EVENT_GCODE_SD_STOP));
    #endif
  }
  else
  {
    card.flag.abort_sd_printing = true;
  }
  wait_for_heatup = false;
  filament_pause_check = door_pause_check = true;
}

void RTSSHOW::RTS_HandleData()
{
  int Checkkey = -1;
  // for waiting
  if(waitway > 0)
  {
    memset(&recdat, 0, sizeof(recdat));
    recdat.head[0] = FHONE;
    recdat.head[1] = FHTWO;
    return;
  }
  for(int i = 0;Addrbuf[i] != 0;i ++)
  {
    if(recdat.addr == Addrbuf[i])
    {
      Checkkey = i;
    }
  }

  if(Checkkey < 0)
  {
    memset(&recdat, 0, sizeof(recdat));
    recdat.head[0] = FHONE;
    recdat.head[1] = FHTWO;
    return;
  }

  switch(Checkkey)
  {
    case MainEnterKey:
      if(recdat.data[0] == 1)
      {
        CardUpdate = true;
        RTS_SDCardUpate();
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 2, ExchangepageAddr);
          change_page_font = 2;
          }
      }
      else if(recdat.data[0] == 2)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 15, ExchangepageAddr);
          change_page_font = 15;
        }
      }
      else if(recdat.data[0] == 3)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
          change_page_font = 20;
        }
      }
      else if(recdat.data[0] == 4)
      {
        rtscheck.RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 25, ExchangepageAddr);
          change_page_font = 25;
        }
      }
      else if(recdat.data[0] == 5)
      {
        RTS_SndData(1, MOTOR_FREE_ICON_VP); 
        RTS_SndData(0, PRINT_PROCESS_TITLE_VP);
        RTS_SndData(0, PRINT_PROCESS_VP);
        delay(2);
        RTS_SndData(0, PRINT_TIME_HOUR_VP);
        RTS_SndData(0, PRINT_TIME_MIN_VP);
        print_job_timer.reset();
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
          change_page_font = 1;
        }
      }
      else if(recdat.data[0] == 6)
      {
        waitway = 8;
        start_manual_flag = 1;
        RTS_SndData(1, AUTO_BED_LEVEL_TITLE_VP);
        // if(language_change_font != 0)
        // {
        //   RTS_SndData(ExchangePageBase + 26, ExchangepageAddr);
        //   change_page_font = 26;
        // }
        // RTS_SndData(AUTO_BED_LEVEL_PREHEAT, AUTO_BED_PREHEAT_HEAD_DATA_VP);
        // thermalManager.setTargetHotend(AUTO_BED_LEVEL_PREHEAT, 0);
        // RTS_SndData(AUTO_BED_LEVEL_PREHEAT, HEAD_SET_TEMP_VP);
        // if(thermalManager.temp_hotend[0].celsius < (AUTO_BED_LEVEL_PREHEAT - 5))
        // {
        //   queue.enqueue_now_P(PSTR("G4 S40"));
        // }
        queue.inject_P(PSTR("G29 S1"));
        RTS_SndData(1, MOTOR_FREE_ICON_VP);

        /* turn on the led */
        RTS_SndData(1, PRINTER_LEDOPEN_TITLE_VP); 
        digitalWrite(LED_CONTROL_PIN, HIGH);
        LEDStatus = false;
      }
      break;
    case AdjustEnterKey:
      if(recdat.data[0] == 1)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 14, ExchangepageAddr);
          change_page_font = 14;
        }
      }
      else if(recdat.data[0] == 2)
      {
        if(card.isPrinting())
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
            change_page_font = 10;
          }
        }
        else
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 12, ExchangepageAddr);
            change_page_font = 12;
          }
        }
      }
      else if(recdat.data[0] == 3)
      {
        // turn on the fan
        if(thermalManager.fan_speed[0] == 0)
        {
          RTS_SndData(1, PRINTER_FANOPEN_TITLE_VP); 
          thermalManager.fan_speed[0] = 255;
        }
        else
        {
          // turn off the fan
          RTS_SndData(2, PRINTER_FANOPEN_TITLE_VP); 
          thermalManager.fan_speed[0] = 0;
        }
      }
      else if(recdat.data[0] == 4)
      {
        // turn on the LED
        if(LEDStatus)
        {
          RTS_SndData(1, PRINTER_LEDOPEN_TITLE_VP); 
          digitalWrite(LED_CONTROL_PIN, HIGH);
          LEDStatus = false;
        }
        else
        {
          // turn off the LED
          RTS_SndData(2, PRINTER_LEDOPEN_TITLE_VP); 
          digitalWrite(LED_CONTROL_PIN, LOW);
          LEDStatus = true;
        }
      }
      else if(recdat.data[0] == 5)
      {
        if(PowerOn_flag)
        {
          PowerOn_flag = false;
          RTS_SndData(2, POWERON_ICON_VP);
        }
        else
        {
          PowerOn_flag = true;
          RTS_SndData(1, POWERON_ICON_VP);
        }
      }
      else if(recdat.data[0] == 6)
      {
        if(CheckDoor_flag)
        {
          CheckDoor_flag = false;
          RTS_SndData(2, CHECKDOOR_ICON_VP);
        }
        else
        {
          CheckDoor_flag = true;
          RTS_SndData(1, CHECKDOOR_ICON_VP);
        }
        BL24CXX::write(FONT_EEPROM+4, (uint8_t*)&CheckDoor_flag, 1);
      }
      else if(recdat.data[0] == 7)
      {
        // turn on the box fan
        if(READ(BOX_FAN_PIN) == 0)
        {
          RTS_SndData(1, BOXFAN_ICON_VP); 
          WRITE(BOX_FAN_PIN, 1);
        }
        else
        {
          // turn off the box fan
          RTS_SndData(2, BOXFAN_ICON_VP); 
          WRITE(BOX_FAN_PIN, 0);
        }
      }
      else if(recdat.data[0] == 8) // wifi LED
      {
        if(WIFILEDStatus)
        {
          // turn off the WIFI LED
          RTS_SndData(1, WIFILED_ICON_VP);
          WIFILEDStatus = false;
        }
        else
        {
          // turn off the WIFI LED
          RTS_SndData(2, WIFILED_ICON_VP);
          WIFILEDStatus = true;
        }
      }
      break;
    case PrintSpeedEnterKey:
      feedrate_percentage = recdat.data[0];
      break;
    case StopPrintKey:
      if(recdat.data[0] == 1)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 13, ExchangepageAddr);
          change_page_font = 13;
          }
      }
      else if(recdat.data[0] == 2)
      {
        RTS_SndData(0, PRINT_TIME_HOUR_VP);
        RTS_SndData(0, PRINT_TIME_MIN_VP);
        tempbed = 0;
        temphot = 0;
        // RTS_SDcard_Stop();
        gStopPrintFlag = true;
      }
      else if(recdat.data[0] == 3)
      {
        if(card.isPrinting())
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
            change_page_font = 10;
          }
        }
        else
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 12, ExchangepageAddr);
            change_page_font = 12;
          }
        }
      }
      else if(recdat.data[0] == 4)
      {
        settings.reset();
        settings.save();
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
          change_page_font = 20;
        }
      }
      else if(recdat.data[0] == 5)
      {
        #if ENABLED(PSU_CONTROL)
          PSU_OFF();
        #endif
      }
      break;
    case PausePrintKey:
      if(recdat.data[0] == 1)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 11, ExchangepageAddr);
          change_page_font = 11;
        }
      }
      else if(recdat.data[0] == 2)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 12, ExchangepageAddr);
          change_page_font = 12;
        }

        if(!temphot)
          temphot = thermalManager.degTargetHotend(0);
        if(!tempbed)
          tempbed = thermalManager.degTargetBed();

        queue.inject_P(PSTR("M25"));

        pause_action_flag = true;
      }
      else if(recdat.data[0] == 3)
      {
        if(card.isPrinting())
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
            change_page_font = 10;
          }
        }
        else
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 12, ExchangepageAddr);
            change_page_font = 12;
          }
        }
      }
      break;
    case ResumePrintKey:
      if(recdat.data[0] == 1)
      {
        /* if pause-actions havn't finished, can't return to print-status */
        if(pause_action_flag)
          break;

        #if DOOR_CHECK_SWITCH
          if(CheckDoor_flag && READ(CHECK_DOOR_PIN) == 1)
          {
            if(language_change_font != 0)
            {
              rtscheck.RTS_SndData(ExchangePageBase + 29, ExchangepageAddr);
              change_page_font = 29;
            }
            break;
          }
        #endif

        #if 0
        char commandbuf[20];
        char pause_str_E[16];
        char pause_str_X[16], pause_str_Y[16], pause_str_Z[16];
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
          change_page_font = 10;
        }
        #if HAS_RESUME_CONTINUE
          if(wait_for_user)
          {
            wait_for_user = false;
          }
          else
        #endif
          {
            memset(pause_str_X, 0, sizeof(pause_str_X));
            dtostrf(pause_position.x, 3, 2, pause_str_X);
            memset(pause_str_Y, 0, sizeof(pause_str_Y));
            dtostrf(pause_position.y, 3, 2, pause_str_Y);
            memset(pause_str_Z, 0, sizeof(pause_str_Z));
            dtostrf(pause_position.z, 3, 2, pause_str_Z);
            memset(pause_str_E, 0, sizeof(pause_str_E));
            dtostrf(pause_position.e, 3, 2, pause_str_E);

            memset(commandbuf, 0, sizeof(commandbuf));
            sprintf_P(commandbuf, PSTR("G1 X%s Y%s F1200"), pause_str_X, pause_str_Y);
            gcode.process_subcommands_now_P(commandbuf);
            memset(commandbuf, 0, sizeof(commandbuf));
            sprintf_P(commandbuf, PSTR("G0 Z%s"), pause_str_Z);
            gcode.process_subcommands_now_P(commandbuf);
            sprintf_P(commandbuf, PSTR("G92 E%s"), pause_str_E);
            gcode.process_subcommands_now_P(commandbuf);

            gcode.process_subcommands_now_P(PSTR("M24"));
            if(!filament_pause_check) filament_pause_check = true;
            if(!door_pause_check) door_pause_check = true;
          }
        #endif
      }
      else if(recdat.data[0] == 2)
      {
        thermalManager.setTargetHotend(temphot, 0);
      }
      break;
    case ZoffsetEnterKey:
      #if HAS_BED_PROBE
        last_zoffset = probe_offset.z;
        if(recdat.data[0] >= 32768)
        {
          probe_offset.z = ((float)recdat.data[0] - 65536)/100;
        }
        else
        {
          probe_offset.z = ((float)recdat.data[0])/100;
        }
        if(WITHIN((probe_offset.z), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
        {
          babystep.add_mm(Z_AXIS, probe_offset.z - last_zoffset);
          settings.save();
        }
        RTS_SndData((Z_MAX_POS - probe_offset.z) * 100, PRINT_HEITH_VP);
      #elif ENABLED(MESH_BED_LEVELING)
        last_zoffset = mbl.z_offset;
        if(recdat.data[0] >= 32768)
        {
          mbl.z_offset = ((float)recdat.data[0] - 65536)/100;
        }
        else
        {
          mbl.z_offset = ((float)recdat.data[0])/100;
        }
        if(WITHIN((mbl.z_offset), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
        {
          babystep.add_mm(Z_AXIS, mbl.z_offset - last_zoffset);
          settings.save();
        }      
        RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
      #else
        last_zoffset = zprobe_zoffset;
        if(recdat.data[0] >= 32768)
        {
          zprobe_zoffset = ((float)recdat.data[0] - 65536)/100;
        }
        else
        {
          zprobe_zoffset = ((float)recdat.data[0])/100;
        }
        if(WITHIN((zprobe_zoffset), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
        {
          babystep.add_mm(Z_AXIS, zprobe_zoffset - last_zoffset);
          settings.save();
        }
        RTS_SndData((Z_MAX_POS - zprobe_zoffset) * 100, PRINT_HEITH_VP);
     #endif
      break;
    case TempControlKey:
      if(recdat.data[0] == 2)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 21, ExchangepageAddr);
          change_page_font = 21;
        }
      }
      else if(recdat.data[0] == 3)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 22, ExchangepageAddr);
          change_page_font = 22;
        }
      }
      else if(recdat.data[0] == 4)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 23, ExchangepageAddr);
          change_page_font = 23;
        }
      }
      else if(recdat.data[0] == 5)
      {
        // thermalManager.setTargetHotend(HMI_ValueStruct.preheat_hotend_temp[0], 0);
        // RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[0], HEAD_SET_TEMP_VP);
        thermalManager.setTargetBed(HMI_ValueStruct.preheat_bed_temp[0]);
        RTS_SndData(HMI_ValueStruct.preheat_bed_temp[0], BED_SET_TEMP_VP);
        TakeTurn_heat_flag = true;
      }
      else if(recdat.data[0] == 6)
      {
        // thermalManager.setTargetHotend(HMI_ValueStruct.preheat_hotend_temp[1], 0);
        // RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[1], HEAD_SET_TEMP_VP);
        thermalManager.setTargetBed(HMI_ValueStruct.preheat_bed_temp[1]);
        RTS_SndData(HMI_ValueStruct.preheat_bed_temp[1], BED_SET_TEMP_VP);
        TakeTurn_heat_flag = true;
      }
      else if(recdat.data[0] == 7)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
          change_page_font = 20;
        }
      }
      break;
    case CoolDownKey:
      if(recdat.data[0] == 1)
      {
        thermalManager.setTargetHotend(0, 0);
        thermalManager.setTargetBed(0);
        TakeTurn_heat_flag = false;

        RTS_SndData(0, HEAD_SET_TEMP_VP);
        RTS_SndData(0, BED_SET_TEMP_VP);
      }
      else if(recdat.data[0] == 2)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 21, ExchangepageAddr);
          change_page_font = 21;
        }
      }
      break;
    case HeaterTempEnterKey:
      thermalManager.temp_hotend[0].target = recdat.data[0];
      thermalManager.setTargetHotend(thermalManager.temp_hotend[0].target, 0);
      RTS_SndData(thermalManager.temp_hotend[0].target, HEAD_SET_TEMP_VP);
      break;
    case HotBedTempEnterKey:
      thermalManager.temp_bed.target = recdat.data[0];
      thermalManager.setTargetBed(thermalManager.temp_bed.target);
      RTS_SndData(thermalManager.temp_bed.target, BED_SET_TEMP_VP);
      break;
    case PrepareEnterKey:
      if(recdat.data[0] == 2)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 31, ExchangepageAddr);
          change_page_font = 31;
        }
      }
      else if(recdat.data[0] == 3)
      {
        rtscheck.RTS_SndData(10*current_position[X_AXIS], AXIS_X_COORD_VP);
        rtscheck.RTS_SndData(10*current_position[Y_AXIS], AXIS_Y_COORD_VP);
        rtscheck.RTS_SndData(10*current_position[Z_AXIS], AXIS_Z_COORD_VP);
        delay(2);
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 16, ExchangepageAddr);
          change_page_font = 16;
        }
      }
      else if(recdat.data[0] == 4)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 30, ExchangepageAddr);
          change_page_font = 30;
        }
      }
      else if(recdat.data[0] == 5)
      {
        if(language_change_font != 0)
        {
          if(language_change_font == 1)
            RTS_SndData(CORP_WEBSITE_C, PRINTER_WEBSITE_TEXT_VP);
          else
            RTS_SndData(CORP_WEBSITE_E, PRINTER_WEBSITE_TEXT_VP);
          RTS_SndData(ExchangePageBase + 24, ExchangepageAddr);
          change_page_font = 24;
        }
      }
      else if(recdat.data[0] == 6)
      {
        queue.enqueue_now_P(PSTR("M84"));
        RTS_SndData(2, MOTOR_FREE_ICON_VP);
      }
      else if(recdat.data[0] == 7)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
          change_page_font = 20;
        }
      }
      else if(recdat.data[0] == 8)
      {
        settings.save();
      }
      else if(recdat.data[0] == 9)
      {
        RTS_SndData(offset_negative_001(mbl.z_offset) * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
          change_page_font = 1;
        }
      }
      break;
    case BedLevelKey:
      if(recdat.data[0] == 1)
      {
        waitway = 6;
        if (!TEST(axis_known_position, X_AXIS) || !TEST(axis_known_position, Y_AXIS))
        {
          queue.enqueue_now_P(PSTR("G28"));
        }
        else
        {
          queue.enqueue_now_P(PSTR("G28 Z"));
        }
        RTS_SndData(1, MOTOR_FREE_ICON_VP);
      }
      else if(recdat.data[0] == 3)
      {
        #if HAS_BED_PROBE
          if (WITHIN((probe_offset.z + 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            babystep.add_mm(Z_AXIS, 0.05);
            probe_offset.z = (probe_offset.z + 0.05);
            RTS_SndData(probe_offset.z * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - probe_offset.z) * 100, PRINT_HEITH_VP);
        #elif ENABLED(MESH_BED_LEVELING)
          if (WITHIN((mbl.z_offset + 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            mbl.z_offset = (mbl.z_offset + 0.05);
            line_to_current_position();
            RTS_SndData(offset_negative_001(mbl.z_offset) * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
        #else
          if (WITHIN((zprobe_zoffset + 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            babystep.add_mm(Z_AXIS, 0.05);
            zprobe_zoffset = (zprobe_zoffset + 0.05);
            RTS_SndData(zprobe_zoffset * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - zprobe_zoffset) * 100, PRINT_HEITH_VP);
        #endif
      }
      else if(recdat.data[0] == 2)
      {
        #if HAS_BED_PROBE
          if (WITHIN((probe_offset.z - 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            babystep.add_mm(Z_AXIS, -0.05);
            probe_offset.z = (probe_offset.z - 0.05);
            RTS_SndData(probe_offset.z * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - probe_offset.z) * 100, PRINT_HEITH_VP);
        #elif ENABLED(MESH_BED_LEVELING)
          if (WITHIN((mbl.z_offset - 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            mbl.z_offset = (mbl.z_offset - 0.05);
            line_to_current_position();
            RTS_SndData(offset_negative_001(mbl.z_offset) * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
        #else
          if (WITHIN((zprobe_zoffset - 0.05), Z_PROBE_OFFSET_RANGE_MIN, Z_PROBE_OFFSET_RANGE_MAX))
          {
            babystep.add_mm(Z_AXIS, -0.05);
            zprobe_zoffset = (zprobe_zoffset - 0.05);
            RTS_SndData(zprobe_zoffset * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
          }
          RTS_SndData((Z_MAX_POS - zprobe_zoffset) * 100, PRINT_HEITH_VP);
        #endif
      }
      else if(recdat.data[0] == 4)
      {
        rtscheck.RTS_SndData((Z_MAX_POS - mbl.z_offset) * 100, PRINT_HEITH_VP);
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 25, ExchangepageAddr);
          change_page_font = 25;
        }
      }
      break;
    case ManualLevelKey:
      if(recdat.data[0] == 1)
      {
        settings.save();
        queue.inject_P(PSTR("G29 S2"));
        if(start_manual_flag) 
        {
          mbl.z_offset = current_position.z;
          start_manual_flag = false;
        }
      }
      else if(recdat.data[0] == 2)
      {
        current_position.z += 0.05;
        RTS_line_to_current(Z_AXIS);
        RTS_SndData(offset_negative_001(current_position.z) * 100, ZOFFSET_DATA_VP);
      }
      else if(recdat.data[0] == 3)
      {
        current_position.z -= 0.05;
        RTS_line_to_current(Z_AXIS);
        RTS_SndData(offset_negative_001(current_position.z) * 100, ZOFFSET_DATA_VP);
      }
      else if(recdat.data[0] == 4)
      {
        // leveling to complete ?

      }
      else if(recdat.data[0] == 5)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
          change_page_font = 1;
        }
        // save when from the leveling page
        settings.save();
      }      
      break;
    case AutoHomeKey:
      if(recdat.data[0] == 1)
      {
        AxisUnitMode = 1;
        axis_unit = 10.0;
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 17, ExchangepageAddr);
          change_page_font = 17;
        }
      }
      else if(recdat.data[0] == 2)
      {
        AxisUnitMode = 2;
        axis_unit = 1.0;
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 16, ExchangepageAddr);
          change_page_font = 16;
        }
      }
      else if(recdat.data[0] == 3)
      {
        AxisUnitMode = 3;
        axis_unit = 0.1;
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 18, ExchangepageAddr);
          change_page_font = 18;
        }
      }
      else if(recdat.data[0] == 4)
      {
        waitway = 4;
        queue.enqueue_now_P(PSTR("G28"));
        RTS_SndData(1, MOTOR_FREE_ICON_VP);
      }
      break;
    case XaxismoveKey:
      float x_min, x_max;
      waitway = 4;
      x_min = 0;
      x_max = X_MAX_POS;
      current_position[X_AXIS] = ((float)recdat.data[0])/10;
      if(current_position[X_AXIS] < x_min)
      {
        current_position[X_AXIS] = x_min;
      }
      else if(current_position[X_AXIS] > x_max)
      {
        current_position[X_AXIS] = x_max;
      }
      RTS_line_to_current(X_AXIS);
      RTS_SndData(10 * current_position[X_AXIS], AXIS_X_COORD_VP);
      delay(1);
      RTS_SndData(1, MOTOR_FREE_ICON_VP);
      waitway = 0;
      break;
    case YaxismoveKey:
      float y_min, y_max;
      waitway = 4;
      y_min = 0;
      y_max = Y_MAX_POS;
      current_position[Y_AXIS] = ((float)recdat.data[0])/10;
      if (current_position[Y_AXIS] < y_min)
      {
        current_position[Y_AXIS] = y_min;
      }
      else if (current_position[Y_AXIS] > y_max)
      {
        current_position[Y_AXIS] = y_max;
      }
      RTS_line_to_current(Y_AXIS);
      RTS_SndData(10 * current_position[Y_AXIS], AXIS_Y_COORD_VP);
      delay(1);
      RTS_SndData(1, MOTOR_FREE_ICON_VP);
      waitway = 0;
      break;
    case ZaxismoveKey:
      float z_min, z_max;
      waitway = 4;
      z_min = Z_MIN_POS;
      z_max = Z_MAX_POS;
      current_position[Z_AXIS] = ((float)recdat.data[0])/10;
      if (current_position[Z_AXIS] < z_min)
      {
        current_position[Z_AXIS] = z_min;
      }
      else if (current_position[Z_AXIS] > z_max)
      {
        current_position[Z_AXIS] = z_max;
      }
      RTS_line_to_current(Z_AXIS);
      RTS_SndData(10 * current_position[Z_AXIS], AXIS_Z_COORD_VP);
      delay(1);
      RTS_SndData(1, MOTOR_FREE_ICON_VP);
      waitway = 0;
      break;
    case HeaterLoadEnterKey:
      FilamentLOAD = ((float)recdat.data[0])/10;
      break;
    case HeaterLoadStartKey:
      if(recdat.data[0] == 1)
      {
        // current_position[E_AXIS] += FilamentLOAD;

        if(thermalManager.temp_hotend[0].celsius < 250)                    //PREHEAT_1_TEMP_HOTEND
        {
          thermalManager.temp_hotend[0].target = 250;                      //PREHEAT_1_TEMP_HOTEND
          thermalManager.setTargetHotend(thermalManager.temp_hotend[0].target, 0);
          RTS_SndData(thermalManager.temp_hotend[0].target, HEAD_SET_TEMP_VP);

          feedFlag = true;
        }
        else
        {
            current_position[E_AXIS] += 70;
            line_to_current_position(feedRate_t(5));
            // RTS_line_to_current(E_AXIS);
            RTS_SndData(10 * FilamentLOAD, HEAD_FILAMENT_LOAD_DATA_VP);
        }
      }
      else if(recdat.data[0] == 2)
      {
        // current_position[E_AXIS] -= FilamentLOAD;

        if(thermalManager.temp_hotend[0].celsius < 250)                    //PREHEAT_1_TEMP_HOTEND
        {
          thermalManager.temp_hotend[0].target = 250;                      // PREHEAT_1_TEMP_HOTEND
          thermalManager.setTargetHotend(thermalManager.temp_hotend[0].target, 0);
          RTS_SndData(thermalManager.temp_hotend[0].target, HEAD_SET_TEMP_VP);
        }
        else
        {
            current_position[E_AXIS] += 10;
            line_to_current_position(feedRate_t(15));
            current_position[E_AXIS] -= 70;
            line_to_current_position(feedRate_t(30));
        }
      }
      else if(recdat.data[0] == 3)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 15, ExchangepageAddr);
          change_page_font = 15;
        }
      }
      else if(recdat.data[0] == 4)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 19, ExchangepageAddr);
          change_page_font = 19;
        }
      }
      break;
    case SelectLanguageKey:
      if(recdat.data[0] == 1)
      {
        RTS_SndData(ExchangePageBase + 28, ExchangepageAddr);
        change_page_font = 28;
      }
      else if(recdat.data[0] == 2)
      {
        RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
        change_page_font = 20;
      }
      break;
    case EightLanguageKey:
      if(recdat.data[0] == 1) language_change_font = 1;
      else if(recdat.data[0] == 2) language_change_font = 2;
      else if(recdat.data[0] == 3) language_change_font = 3;
      else if(recdat.data[0] == 4) language_change_font = 4;
      else if(recdat.data[0] == 5) language_change_font = 5;
      else if(recdat.data[0] == 6) language_change_font = 6;
      else if(recdat.data[0] == 7) language_change_font = 7;
      else if(recdat.data[0] == 8) language_change_font = 8;
      else if(recdat.data[0] == 9) language_change_font = 9;
      BL24CXX::write(FONT_EEPROM, (uint8_t*)&language_change_font, 1);
      SERIAL_ECHOLNPAIR("\r\nlanguage_change_font: ", language_change_font);
      lcd_eight_language();
      RTS_SndData(ExchangePageBase + 20, ExchangepageAddr);
      change_page_font = 20;
      break;
    case PowerContinuePrintKey:
      if(recdat.data[0] == 1)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
          change_page_font = 10;
        }
        if(recovery.info.recovery_flag) 
        {
          power_off_type_yes = 1;

          RTS_SndData(1, PRINTER_FANOPEN_TITLE_VP);

          heat_flag = 1;

          recovery.resume();
        }
      }
      else if(recdat.data[0] == 2)
      {
        waitway = 3;
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
          change_page_font = 1;
        }
        card.stopSDPrint();
        queue.clear();
        quickstop_stepper();
        print_job_timer.stop();
        thermalManager.disable_all_heaters();
        print_job_timer.reset();

        #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
          card.removeJobRecoveryFile();
        #endif
        wait_for_heatup = false;
        filament_pause_check = door_pause_check = true;
        RTS_SndData(1, MOTOR_FREE_ICON_VP);
        delay(500);
        waitway = 0;
      }
      break;
    case FanSpeedEnterKey:
      #if FAN_COUNT > 0
        for (uint8_t i = 0; i < FAN_COUNT; i++)
        {
          thermalManager.fan_speed[i] = recdat.data[0];
          RTS_SndData(thermalManager.fan_speed[i], FAN_SPEED_CONTROL_DATA_VP);
        }
      #endif
      break;
    case PLAHeadSetEnterKey:
      HMI_ValueStruct.preheat_hotend_temp[0] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[0], PLA_HEAD_SET_DATA_VP);
      break;
    case PLABedSetEnterKey:
      HMI_ValueStruct.preheat_bed_temp[0] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_bed_temp[0], PLA_BED_SET_DATA_VP);
      break;
    case PLAFanSetEnterKey:
      HMI_ValueStruct.preheat_fan_speed[0] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_fan_speed[0], PLA_FAN_SET_DATA_VP);
      break;
    case ABSHeadSetEnterKey:
      HMI_ValueStruct.preheat_hotend_temp[1] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_hotend_temp[1], ABS_HEAD_SET_DATA_VP);
      break;
    case ABSBedSetEnterKey:
      HMI_ValueStruct.preheat_bed_temp[1] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_bed_temp[1], ABS_BED_SET_DATA_VP);
      break;
    case ABSFanSetEnterKey:
      HMI_ValueStruct.preheat_fan_speed[1] = recdat.data[0];
      RTS_SndData(HMI_ValueStruct.preheat_fan_speed[1], ABS_FAN_SET_DATA_VP);
      break;
    case SelectFileKey:
      if(card.flag.mounted)
      {
        if(recdat.data[0] > CardRecbuf.Filesum) break;

        CardRecbuf.recordcount = recdat.data[0] - 1;
        delay(2);
        for(int j = 1;j <= CardRecbuf.Filesum;j ++)
        {
          RTS_SndData((unsigned long)0xFFFF, FilenameNature + j * 16);
          RTS_SndData(10, FILE1_SELECT_ICON_VP - 1 + j);
        }
        RTS_SndData((unsigned long)0x87F0, FilenameNature + recdat.data[0] * 16);
        RTS_SndData(6, FILE1_SELECT_ICON_VP - 1 + recdat.data[0]);
      }
      break;
    case StartFileKey:
      if((recdat.data[0] == 1) && card.flag.mounted)
      {
        if(CardRecbuf.recordcount < 0)
        {
          break;
        }

        char cmd[30];
        char* c;
        sprintf_P(cmd, PSTR("M23 %s"), CardRecbuf.Cardfilename[CardRecbuf.recordcount]);
        for (c = &cmd[4]; *c; c++) *c = tolower(*c);

        FilenamesCount = CardRecbuf.recordcount;
        memset(cmdbuf, 0, sizeof(cmdbuf));
        strcpy(cmdbuf, cmd);

        queue.enqueue_one_now(cmd);
        queue.enqueue_now_P(PSTR("M24"));

        card.removeJobRecoveryFile();

        heat_flag = 1;

        // clean screen.
        for(int j = 0;j < 10;j ++)
        {
          RTS_SndData(0, CONTINUE_PRINT_FILE_TEXT_VP + j);
        }

        int filelen = strlen(CardRecbuf.Cardshowfilename[CardRecbuf.recordcount]);
        filelen = (TEXTBYTELEN - filelen)/2;
        if(filelen > 0)
        {
          char buf[20];
          memset(buf, 0, sizeof(buf));
          strncpy(buf,"         ",filelen);
          strcpy(&buf[filelen], CardRecbuf.Cardshowfilename[CardRecbuf.recordcount]);
          RTS_SndData(buf, CONTINUE_PRINT_FILE_TEXT_VP);
        }
        else
        {
          RTS_SndData(CardRecbuf.Cardshowfilename[CardRecbuf.recordcount], CONTINUE_PRINT_FILE_TEXT_VP);
        }
        delay(2);
        #if FAN_COUNT > 0
          for (uint8_t i = 0; i < FAN_COUNT; i++) thermalManager.fan_speed[i] = 255;
        #endif
        RTS_SndData(1, PRINTER_FANOPEN_TITLE_VP);

        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
          change_page_font = 10;
        }
      }
      else if(recdat.data[0] == 4)
      {
        if(language_change_font != 0)
        {
          RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
          change_page_font = 1;
        }
      }
      break;
    case ChangePageKey:
    {
      lcd_eight_language();
      // clean screen.
      for(int j = 0;j < 10;j ++)
      {
        RTS_SndData(0, CONTINUE_PRINT_FILE_TEXT_VP + j);
      }

      int filelen = strlen(CardRecbuf.Cardshowfilename[CardRecbuf.recordcount]);
      filelen = (TEXTBYTELEN - filelen)/2;
      if(filelen > 0)
      {
        char buf[20];
        memset(buf, 0, sizeof(buf));
        strncpy(buf, "         ", filelen);
        strcpy(&buf[filelen], CardRecbuf.Cardshowfilename[CardRecbuf.recordcount]);
        RTS_SndData(buf, CONTINUE_PRINT_FILE_TEXT_VP);
      }
      else
      {
        RTS_SndData(CardRecbuf.Cardshowfilename[CardRecbuf.recordcount], CONTINUE_PRINT_FILE_TEXT_VP);
      }

      // represents to update file list
      if(IS_SD_INSERTED())
      {
        for(uint16_t i = 0;i < CardRecbuf.Filesum;i ++) 
        {
          delay(3);
          RTS_SndData(CardRecbuf.Cardshowfilename[i], CardRecbuf.addr[i]);
          RTS_SndData((unsigned long)0xFFFF,FilenameNature + (i+1)*16);
          RTS_SndData(10, FILE1_SELECT_ICON_VP + i);
        }
      }

      RTS_SndData(SOFTVERSION, PRINTER_VERSION_TEXT_VP);
      RTS_SndData(MACHINE_SIZE, PRINTER_PRINTSIZE_TEXT_VP);
      if(language_change_font == 1)
        RTS_SndData(CORP_WEBSITE_C, PRINTER_WEBSITE_TEXT_VP);
      else
        RTS_SndData(CORP_WEBSITE_E, PRINTER_WEBSITE_TEXT_VP);

      if(thermalManager.fan_speed[0])
        RTS_SndData(1, PRINTER_FANOPEN_TITLE_VP); 
      else
        RTS_SndData(2, PRINTER_FANOPEN_TITLE_VP); 

      if(READ(BOX_FAN_PIN) == 0)
        RTS_SndData(1, BOXFAN_ICON_VP); 
      else
        RTS_SndData(2, BOXFAN_ICON_VP); 

      if(LEDStatus)
        RTS_SndData(1, PRINTER_LEDOPEN_TITLE_VP); 
      else
        RTS_SndData(2, PRINTER_LEDOPEN_TITLE_VP); 

      Percentrecord = card.percentDone() + 1;
      if(Percentrecord <= 100)
      {
        rtscheck.RTS_SndData((unsigned int)Percentrecord, PRINT_PROCESS_TITLE_VP);
      }

      #if HAS_BED_PROBE
        RTS_SndData(probe_offset.z * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
      #elif ENABLED(MESH_BED_LEVELING)
        RTS_SndData(offset_negative_001(mbl.z_offset) * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
      #else
        RTS_SndData(zprobe_zoffset * 100, AUTO_BED_LEVEL_ZOFFSET_VP);
      #endif

      RTS_SndData(feedrate_percentage, PRINT_SPEED_RATE_VP);
      RTS_SndData(thermalManager.temp_hotend[0].target, HEAD_SET_TEMP_VP);
      RTS_SndData(thermalManager.temp_bed.target, BED_SET_TEMP_VP);

      RTS_SndData(ExchangePageBase + change_page_font, ExchangepageAddr);
      break;
    }
    case ErrorKey:
    {
      if(recdat.data[0] == 1)
      {
        if(printingIsActive()) // printing
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 10, ExchangepageAddr);
            change_page_font = 10;
          }
        }
        else if(printingIsPaused()) // pause
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 12, ExchangepageAddr);
            change_page_font = 12;
          }
        }
        else  // other
        {
          if(language_change_font != 0)
          {
            RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
            change_page_font = 1;
          }
        }

        if(errorway == 4)
        {
          NVIC_SystemReset();   // reboot
        }
      }
      break;
    }
    default:
      break;
  }
  memset(&recdat, 0, sizeof(recdat));
  recdat.head[0] = FHONE;
  recdat.head[1] = FHTWO;
}

void EachMomentUpdate()
{
  millis_t ms = millis();
  if(ms > next_rts_update_ms)
  {
    // print the file before the power is off.
    if((power_off_type_yes == 0) && lcd_sd_status && recovery.info.recovery_flag)
    {
      power_off_type_yes = 1;
      for(uint16_t i = 0;i < CardRecbuf.Filesum;i ++) 
      {
        if(!strcmp(CardRecbuf.Cardfilename[i], &recovery.info.sd_filename[1]))
        {
          int filelen = strlen(CardRecbuf.Cardshowfilename[i]);
          filelen = (TEXTBYTELEN - filelen)/2;
          if(filelen > 0)
          {
            char buf[20];
            memset(buf, 0, sizeof(buf));
            strncpy(buf, "         ", filelen);
            strcpy(&buf[filelen],CardRecbuf.Cardshowfilename[i]);
            rtscheck.RTS_SndData(buf, CONTINUE_PRINT_FILE_TEXT_VP);
          }
          else
          {
            rtscheck.RTS_SndData(CardRecbuf.Cardshowfilename[i], CONTINUE_PRINT_FILE_TEXT_VP);
          }
          if(language_change_font != 0)
          {
            rtscheck.RTS_SndData(ExchangePageBase + 27, ExchangepageAddr);
            change_page_font = 27;
          }
          break;
        }
      }
      return;
    }
    else if((power_off_type_yes == 0) && !recovery.info.recovery_flag)
    {
      power_off_type_yes = 1;
      if(language_change_font != 0)
      {
        rtscheck.RTS_SndData(ExchangePageBase + 1, ExchangepageAddr);
        change_page_font = 1;
      }
      return;
    }
    else
    {
      // need to optimize
      if(gcode.previous_move_ms != 0)
      {
        duration_t elapsed = print_job_timer.duration();
        static unsigned int last_cardpercentValue = 101;
        rtscheck.RTS_SndData(elapsed.value/3600, PRINT_TIME_HOUR_VP);
        rtscheck.RTS_SndData((elapsed.value%3600)/60, PRINT_TIME_MIN_VP);

        if(card.isPrinting() && last_cardpercentValue != card.percentDone())
        {
          if((unsigned int) card.percentDone() > 0)
          {
            Percentrecord = card.percentDone() + 1;
            if(Percentrecord <= 100)
            {
              rtscheck.RTS_SndData((unsigned int)Percentrecord, PRINT_PROCESS_TITLE_VP);
            }
          }
          else
          {
            rtscheck.RTS_SndData(0, PRINT_PROCESS_TITLE_VP);
          }
          rtscheck.RTS_SndData((unsigned int) card.percentDone(), PRINT_PROCESS_VP);
          last_cardpercentValue = card.percentDone();
        }
      }

      if(print_finish && !planner.has_blocks_queued())
      {
        print_finish = false;
        #if ENABLED(PSU_CONTROL)
          if(PowerOn_flag)
          {
            gcode.process_subcommands_now_P(PSTR("G28 XY"));
            PSU_OFF();
          }
          else 
        #endif
            finish_home = true;

          /* make Z axis get to position of Z-Max-Value when printing has finished */
          ZaxisMoveToZMax();
      }

      // float temp_buf = thermalManager.temp_hotend[0].celsius;
      rtscheck.RTS_SndData(thermalManager.temp_hotend[0].celsius, HEAD_CURRENT_TEMP_VP);
      rtscheck.RTS_SndData(thermalManager.temp_bed.celsius, BED_CURRENT_TEMP_VP);
      if((false == card.isPrinting()) && (false == filament_pause_check))
      {
        if(0 == READ(CHECK_MATWEIAL))
        {
          if(language_change_font != 0)
          {
            rtscheck.RTS_SndData(ExchangePageBase + 8, ExchangepageAddr);
            change_page_font = 8;
          }
          filament_pause_check = true;
        }
      }

      // Take turn to preheat
      Take_Turn_Preheat();

      #if DOOR_CHECK_SWITCH
        // if((true == card.isPrinting()) && CheckDoor_flag && (heat_flag == 0) && (false == home_flag) && (true == door_pause_check))
        if((true == card.isPrinting()) && CheckDoor_flag && (true == door_pause_check) && (true == home_flag))
        {
          // if((READ(CHECK_DOOR_PIN) == 1) && ((ms - start_print_time) > 70000))
          if((READ(CHECK_DOOR_PIN) == 1))
          {
            if(!temphot)
              temphot = thermalManager.degTargetHotend(0);
            if(!tempbed)
              tempbed = thermalManager.degTargetBed();

            queue.inject_P(PSTR("M25"));

            pause_action_flag = true;
            if(language_change_font != 0)
            {
              rtscheck.RTS_SndData(ExchangePageBase + 29, ExchangepageAddr);
              change_page_font = 29;
            }
            door_pause_check = false;
          }
        }
      #endif

      if(1 == READ(CHECK_MATWEIAL))
      {
        if(language_change_font != 0)
        {
          rtscheck.RTS_SndData(language_change_font, FILAMENT_LOAD_ICON_VP);
        }
      }
      else if(0 == READ(CHECK_MATWEIAL))
      {
        if(language_change_font != 0)
        {
          rtscheck.RTS_SndData(10, FILAMENT_LOAD_ICON_VP);
        }
      }

      if(pause_action_flag && printingIsPaused() && !planner.has_blocks_queued()) 
      {
        pause_action_flag = false;
        pause_position = current_position;
        pause_position.e = current_position[E_AXIS] - 3;
        do_blocking_move_to_z(_MIN(current_position.z + Z_HOMING_HEIGHT, Z_MAX_POS), feedRate_t(NOZZLE_PARK_Z_FEEDRATE));
        // queue.enqueue_now_P(PSTR("G1 X0 Y165 F1200"));
        do_blocking_move_to_xy(0.0f, 165.0f, feedRate_t(20)); // Avoiding edge collision

        #if 0  /* return of material when 3D-print is pause */
        current_position[E_AXIS] += 10;
        line_to_current_position(feedRate_t(25));
        current_position[E_AXIS] -= 70;
        line_to_current_position(feedRate_t(25));
        #endif
      }

      if(last_target_temperature_bed != thermalManager.temp_bed.target || (last_target_temperature[0] != thermalManager.temp_hotend[0].target))
      {
        thermalManager.setTargetHotend(thermalManager.temp_hotend[0].target, 0);
        thermalManager.setTargetBed(thermalManager.temp_bed.target);
        rtscheck.RTS_SndData(thermalManager.temp_hotend[0].target, HEAD_SET_TEMP_VP);
        rtscheck.RTS_SndData(thermalManager.temp_bed.target, BED_SET_TEMP_VP);

        last_target_temperature_bed = thermalManager.temp_bed.target;
        last_target_temperature[0] = thermalManager.temp_hotend[0].target;
      }
    }
    ErrorHanding();

    /* stop sd card printing */
    if(gStopPrintFlag)
    {
      gStopPrintFlag = false;
      rtscheck.RTS_SDcard_Stop();
    }

    next_rts_update_ms = ms + RTS_UPDATE_INTERVAL;
  }
}

// looping at the loop function
void RTSUpdate()
{
  /* Check the status of card */
  rtscheck.RTS_SDCardUpate();

  #if CHECKFILEMENT
    if((true == card.isPrinting()) && (true == filament_pause_check) && (true == home_flag))
    {
      if(1 == READ(CHECK_MATWEIAL))
      {
        checktime++;
        delay(10);
      }
      else if(0 == READ(CHECK_MATWEIAL))
      {
        checktime = 0;
      }

      if(checktime > 50)
      {
        checktime = 0;
        // #if ENABLED(POWER_LOSS_RECOVERY)
        //   if (recovery.enabled) recovery.save(true, false);
        // #endif
        if(!temphot)
          temphot = thermalManager.degTargetHotend(0);
        if(!tempbed)
          tempbed = thermalManager.degTargetBed();

        queue.inject_P(PSTR("M25"));

        if(language_change_font != 0)
        {
          rtscheck.RTS_SndData(ExchangePageBase + 7, ExchangepageAddr);
          change_page_font = 7;
        }
        filament_pause_check = false;
        pause_action_flag = true;

        /* return of material when it was checked Lack of material */
        current_position[E_AXIS] += 10;
        line_to_current_position(feedRate_t(15));
        current_position[E_AXIS] -= 70;
        line_to_current_position(feedRate_t(30));

        /* stop heating(Hotend and Bed) */
        thermalManager.setTargetHotend(0, 0);
        thermalManager.setTargetBed(0);
        TakeTurn_heat_flag = false;
        rtscheck.RTS_SndData(0, HEAD_SET_TEMP_VP);
        rtscheck.RTS_SndData(0, BED_SET_TEMP_VP);
      }
    }
  #endif

  EachMomentUpdate();

  // wait to receive massage and response
  if(rtscheck.RTS_RecData() > 0)
  {
    rtscheck.RTS_HandleData();
  }
}

void ErrorHanding()
{
  if(errorway)
  {
    if(errornum < Retry_num) // try again 3 times
    {
      errornum++;
      if(errorway == 1)
      {
        errorway = errornum = 0;  // don't try again

      }
      else if(errorway == 2)
      {
        // No processing is done in printing
        if(!printingIsActive() && !printingIsPaused())
        {
          errorway = 0;
          waitway = 4;
          queue.enqueue_now_P(PSTR("G28"));
          rtscheck.RTS_SndData(1, MOTOR_FREE_ICON_VP);
        }
      }
      else if(errorway == 3)
      {
        #if HAS_LEVELING
          reset_bed_level();
        #endif
        errorway = errornum = 0;  // don't try again
      }
    }
    else 
    {
      errorway = errornum = 0;
    }
  }
}