/*
* @Author: 103066
* @Date:   2021-10-10 16:49:15
* @Last Modified by:   103066
* @Last Modified time: 2022-02-25 14:45:29
*/
#include "stdio.h"
#include "string.h"
#include "../gcode.h"
#include "../parser.h"
#include "../../core/serial.h"
#include "../../libs/BL24CXX.h"
#include "../../feature/powerloss.h"
#include "../../inc/MarlinConfig.h"
#include "../../lcd/dwin/lcdAutoUI.h"
#include "../../module/printcounter.h"
#include "../../../Configuration.h"

#if WIFI_BOARD_SUPPORT

#define COMMAND_STR_WIFI_MAC             "M79 A"
#define COMMAND_STR_WIFI_RESET           "M79 B"
#define COMMAND_STR_APPPRT_TIME          "M79 C"
#define COMMAND_STR_APPPRT_LEFT_TIME     "M79 D"
#define COMMAND_STR_PRINT_STA            "M79 S"
#define COMMAND_STR_PRINT_SCHE           "M79 T"

/***********************************************************************************************************************************
                                                          callback functions by wifi board
***********************************************************************************************************************************/
void AppPrintMultiStart(void)
{
    /* Reserved */
}

void AppPrintStart(void)
{
    /* refuse app-print when device is not idle */
    if(gLcdAutoUI.AutoUIGetStatus() != DEVSTA_IDLE)
    {
        /* todo: notify wifi-board device is not available */
        return;
    } 

    /* clear current printing time */
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_TIME_H);
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_TIME_M);
    /* clear current printing progress */
    gLcdAutoUI.SwitchIcon(0, ICONVAR_ADDR_PRINT_PROGRESS);
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_PROGRESS);
    /* change device status to "DEVSTA_APP_PRTHEAT" */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTHEAT);
}

void AppPrintPause(void)
{
    /* current status: app printing */
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING)
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
        /* change device status to "DEVSTA_APP_PRTPAUSE" */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTPAUSE);
    }
    /* current status: local printing */
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING)
    {
        gLcdAutoUI.AutoUIVirKeyEvent(TOUVAR_PIC014_PRINT_PAUSE, 0);
    }
}

void AppPrintRecovery(void)
{
    /* current status: app printing */
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE)
    {
        gLcdAutoUI.AutoUIAppManPauRecovey();
        /* resume print-job-timer */
        // print_job_timer.start();
    }
    /* current status: local printing */
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINT_PAUSE)
    {
        gLcdAutoUI.AutoUIVirKeyEvent(TOUVAR_PIC018_CONTINUE, 0);
    }
    /* power off recovery */
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IDLE)
    {
        /* recovery local-print */
        if(recovery.info.recovery_flag)
            gLcdAutoUI.AutoUIVirKeyEvent(TOUVAR_PIC036_CONTINUE, 0);
        /* recovery app-print */
        else
            gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTHEAT);
    }
}

void AppPrintStop(void)
{
    /* current status: app printing */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) || (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE) ||\
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT))
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);

        /* refresh print-total-time */
        gLcdAutoUI.RefreshPrintTimeTotal();
        /* change device status to "DEVSTA_IDLE" */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
    }
    /* current status: local printing */
    else if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING) || (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINT_PAUSE) ||\
            (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING))
    {
        gLcdAutoUI.AutoUIVirKeyEvent(TOUVAR_PIC017_CONFIRM, 0);
    }
    /* power off recovery */
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IDLE)
    {
        /* stop local-print */
        if(recovery.info.recovery_flag)
            gLcdAutoUI.AutoUIVirKeyEvent(TOUVAR_PIC036_CANCEL, 0);
        /* stop app-print */
        else
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
    }
}

void AppPrintFinish(void)
{
    gLcdAutoUI.SwitchIcon(100, ICONVAR_ADDR_PRINT_PROGRESS);
    gLcdAutoUI.DisplayData(100, DATAVAR_ADDR_PRINT_PROGRESS);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTFINISHED);

    /* refresh print-total-time */
    gLcdAutoUI.RefreshPrintTimeTotal();
    /* change device status to "DEVSTA_IDLE" */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
    /* move x/y/z axis to the special position for taking out of modle */
    LcdAutoUIMoveZCmd(Z_MAX_POS);
    LcdAutoUIMoveXYCmd(0, Y_BED_SIZE);
}

void AppPrintPowerOffRecovery(void)
{
    /* popup window, device was power off when printing */
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INTERCONTPW);

    /* clear printing-filename and display */
    for(uint32_t j=0; j<GCODE_FILE_BYTELEN; j++)
        gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_FILENAME_INTERRUPT + j);
    gLcdAutoUI.DisplayText((char*)&"CLOUDPRINT.gcode", TEXTVAR_ADDR_FILENAME_INTERRUPT);
}

/**
 * [PrintStaCmdHandler :APP print status commands handler]
 * @Author Creality
 * @Time   2021-11-12
 * @param  _cmdVal    [command value]
 */
void PrintStaCmdHandler(uint32_t _cmdVal)
{
    switch(_cmdVal)
    {
        case 0:
            /* APP-Print-Multiple-devices-Start */
            AppPrintMultiStart();
            break;

        case 1:
            /* APP-Print-Start */
            AppPrintStart();
            break;

        case 2:
            /* APP-Print-Pause */
            AppPrintPause();
            break;

        case 3:
            /* APP-Print-Recovery */
            AppPrintRecovery();
            break;

        case 4:
            /* APP-Print-Stop */
            AppPrintStop();
            break;

        case 5:
            /* APP-Print-Finish */
            AppPrintFinish();
            break;

        case 6:
            /* APP-Print-Power-off-Recovery */
            AppPrintPowerOffRecovery();
            break;

        default:
            break;
    }
}

/***********************************************************************************************************************************
                                                          G-code Response
***********************************************************************************************************************************/
void GcodeSuite::M79()
{
    uint32_t lCmdVal = false;

    /* get WIFI board MAC from WIFI board */
    if(parser.str_contain((char*)COMMAND_STR_WIFI_MAC))
    {
        if(parser.getStrAfterTargetStr((char*)COMMAND_STR_WIFI_MAC, gLcdAutoUI.wifiMacAddr))
        {
            /* Reacquire WIFI board MAC from command */
            memset(gLcdAutoUI.wifiMacAddr, 0, sizeof(gLcdAutoUI.wifiMacAddr));
            parser.getStrAfterTargetStr((char*)COMMAND_STR_WIFI_MAC, gLcdAutoUI.wifiMacAddr);
            /* set flag to indicate that getting WIFI board MAC successfully */
            gLcdAutoUI.getWifiMacAddrFlag = true;

            /* just for debug, print WIFI board MAC that get from WIFI board */
            SERIAL_ECHOLN(gLcdAutoUI.wifiMacAddr);
        }
        else
        {
            SERIAL_ECHOLN("get wifi board Mac address failed");
        }
    }
    /* WIFI board reset OK or not */
    else if(parser.str_contain((char*)COMMAND_STR_WIFI_RESET))
    {
        if(parser.getDecValAfterTargetStr((char*)COMMAND_STR_WIFI_RESET, &lCmdVal))
        {
            if(lCmdVal)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_WIFI_RESETPW);
            else
               SERIAL_ECHOLN("WIFI board reset error"); 
        }
    }
    /* time that have printed */
    else if(parser.str_contain((char*)COMMAND_STR_APPPRT_TIME))
    {
        if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT) || (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING))
        {
            if(parser.getDecValAfterTargetStr((char*)COMMAND_STR_APPPRT_TIME, &lCmdVal))
            {
                /* time limit 99H:59M:59S */
                lCmdVal = lCmdVal <= 359999 ? lCmdVal : 359999;

                gLcdAutoUI.DisplayData(lCmdVal/3600, DATAVAR_ADDR_PRINT_TIME_H);
                gLcdAutoUI.DisplayData((lCmdVal%3600)/60, DATAVAR_ADDR_PRINT_TIME_M);

                /* record the app printing time once print */
                gLcdAutoUI.appPrint.apPrintTime = lCmdVal/3600.0;
            } 
        }
    }
    /* left time */
    else if(parser.str_contain((char*)COMMAND_STR_APPPRT_LEFT_TIME))
    {
        /* how long time the device finish job at once print */
    }
    /* App print */
    else if(parser.str_contain((char*)COMMAND_STR_PRINT_STA))
    {
        if(parser.getDecValAfterTargetStr((char*)COMMAND_STR_PRINT_STA, &lCmdVal))
        {
            PrintStaCmdHandler(lCmdVal);
        }
    }
    /* App print progress */
    else if(parser.str_contain((char*)COMMAND_STR_PRINT_SCHE))
    {
        if(parser.getDecValAfterTargetStr((char*)COMMAND_STR_PRINT_SCHE, &lCmdVal))
        {
            if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) && gLcdAutoUI.percentDoneRecord != lCmdVal)
            {
                gLcdAutoUI.percentDoneRecord = lCmdVal;

                gLcdAutoUI.SwitchIcon((int)gLcdAutoUI.percentDoneRecord, ICONVAR_ADDR_PRINT_PROGRESS);
                gLcdAutoUI.DisplayData((int)gLcdAutoUI.percentDoneRecord, DATAVAR_ADDR_PRINT_PROGRESS);
            }
        }
    }
}

#endif
