/*
* @Author: 103066
* @Date:   2021-10-10 16:49:15
* @Last Modified by:   103066
* @Last Modified time: 2021-11-18 17:15:55
*/
#include "stdio.h"
#include "string.h"
#include "../gcode.h"
#include "../parser.h"
#include "../../core/serial.h"
#include "../../inc/MarlinConfig.h"
#include "../../lcd/dwin/lcdAutoUI.h"
#include "../../../Configuration.h"
#include "../../libs/BL24CXX.h"

#if WIFI_BOARD_SUPPORT

#define COMMAND_STR_FILENAME      "M72 "

void GcodeSuite::M72()
{
    /* get print-file-name from WIFI board */
    if(parser.str_contain((char*)COMMAND_STR_FILENAME))
    {
        if(parser.getStrAfterTargetStr((char*)COMMAND_STR_FILENAME, gLcdAutoUI.appPrint.apFileName))
        {
            memset(gLcdAutoUI.appPrint.apFileName, 0, sizeof(gLcdAutoUI.appPrint.apFileName));
            parser.getStrAfterTargetStr((char*)COMMAND_STR_FILENAME, gLcdAutoUI.appPrint.apFileName);
            SERIAL_ECHOLN(gLcdAutoUI.appPrint.apFileName);

            /* clear printing-filename and display */
            for(uint32_t i=0; i<GCODE_FILE_BYTELEN; i++)
                gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_FILENAME_PRINTING + i);
            gLcdAutoUI.DisplayText(gLcdAutoUI.appPrint.apFileName, TEXTVAR_ADDR_FILENAME_PRINTING);
        }
        else
        {
            SERIAL_ECHOLN("get print file name failed");
        }
    }
}

#endif
