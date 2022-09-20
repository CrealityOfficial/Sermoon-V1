/*
* @Author: 103066
* @Date:   2021-05-27 14:42:08
* @Last Modified by:   103066
* @Last Modified time: 2022-03-02 19:15:55
*/
#include <stdio.h>
#include <arduino.h>
#include <wstring.h>
#include "lcdAutoUI.h"
#include "LCD_RTS.h"
#include "../../inc/MarlinConfig.h"
#include "../../../Version.h"
// #include "../../Marlin.h"
#include "../../sd/cardreader.h"
#include "../../module/temperature.h"
#include "../../module/planner.h"
#include "../../module/stepper.h"
#include "../../module/settings.h"
#include "../../module/printcounter.h"
#include "../../module/probe.h"
#include "../../feature/babystep.h"
// #include "../../feature/power_loss_recovery.h"
#include "../../gcode/gcode.h"
#include "../../feature/bedlevel/mbl/mesh_bed_leveling.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "../../libs/duration_t.h"
#include "../../../Configuration.h"
#include "../../libs/BL24CXX.h"
#include "../../feature/powerloss.h"
#include "../../module/motion.h"

/* global variable of the Automatic-style-UI module */
LcdAutoUIStruct_t gLcdAutoUI;

/* touch-response functions */
static void TouVarConfirmFuncPic001(void);
static void TouVarConfirmFuncPic004(void);
static void TouVarConfirmFuncPic005(void);
static void TouVarConfirmFuncPic006(void);
static void TouVarPrintFileFuncPic008(void);
static void TouVarPrintModeFuncPic008(void);
static void TouVarPrintSetFuncPic008(void);
static void TouVarDevInfoFuncPic008(void);
static void TouVarHotendTempFuncPic008(void);
static void TouVarBedTempFuncPic008(void);
static void TouVarReturn(void);
static void TouVarFileSelect(void);
static void TouVarFileNextPage(void);
static void TouVarFileLastPage(void);
static void TouVarPrintSetPic014(void);
static void TouVarPrintPausePic014(void);
static void TouVarPrintStopPic014(void);
static void TouVarPrintFinishPic015(void);
static void TouVarBoxFanPic016(void);
static void TouVarBoxLedPic016(void);
static void TouVarWifiLedPic016(void);
static void TouVarDoorPausePic016(void);
static void TouVarPrintStopConfirmPic017(void);
static void TouVarPrintStopCancelPic017(void);
static void TouVarPrintPauseRecoPic018(void);
static void TouVarPrintStopPic018(void);
static void TouVarPrintInOuMatPic018(void);
static void TouVarConfirmPrintPic019(void);
static void TouVarCancelPrintPic019(void);
static void TouVarPLASelectPic020(void);
static void TouVarABSSelectPic020(void);
#if 0
static void TouVarUserDefPic020(void);
#endif
static void TouVarDisableHeatPic020(void);
static void TouVarInOutMatPic020(void);
static void TouVarInMatPic021(void);
static void TouVarOutMatPic021(void);
static void TouVarOutMatConfirmPic023(void);
static void TouVarSetHotendFanPic024(void);
static void TouVarSetBoxFanPic024(void);
static void TouVarBedCaliPic025(void);
static void TouVarAxisMovePic025(void);
static void TouVarStepUnitMM1(void);
static void TouVarStepUnitMM01(void);
static void TouVarXBackward(void);
static void TouVarXForward(void);
static void TouVarYForward(void);
static void TouVarYBackward(void);
static void TouVarZUp(void);
static void TouVarZDown(void);
static void TouVarGoHome(void);
static void TouVarStepUnitMM10(void);
static void TouVarLangSelectPic031(void);
static void TouVarDevResetPic031(void);
#if 1
static void TouVarLangTypePic032(void);
#endif
static void TouVarConfirmResetPic033(void);
static void TouVarCancelResetPic033(void);
static void TouVarContinuePrintPic036(void);
static void TouVarCancelPrintPic036(void);
static void TouVarBedCaliUpPic027(void);
static void TouVarBedCaliDownPic027(void);
static void TouVarBedCaliSavePic027(void);
static void TouVarStartPrint(void);
static void TouVarConfirmLoadMatPic039(void);
static void TouVarStopPrintPic039(void);
static void TouVarContinuePrintPic040(void);
static void TouVarStopPrintPic040(void);
static void TouVarSaveSelectPic024(void);
static void TouVarConfStopHeatPic042(void);
static void TouVarCancStopHeatPic042(void);
static void TouVarUserDefHotendPic024(void);
static void TouVarUserDefBedPic024(void);
static void TouVarKeybrdInputX(void);
static void TouVarKeybrdInputY(void);
static void TouVarKeybrdInputZ(void);
static void TouVarFeedrateFunc(void);
static void TouVarConfirmPic044(void);
static void TouVarConfirmPic045(void);
static void TouVarCancelPic045(void);

AutoUITouVarResponse_t gAutoUITouVarResponseArray[TOUVAR_NUM] =
{
    { TOUVAR_PIC001_CONFIRM,            TouVarConfirmFuncPic001     },
    { TOUVAR_PIC004_CONFIRM,            TouVarConfirmFuncPic004     },
    { TOUVAR_PIC005_CONFIRM,            TouVarConfirmFuncPic005     },
    { TOUVAR_PIC006_CONFIRM,            TouVarConfirmFuncPic006     },
    { TOUVAR_PIC008_PRINTFILE,          TouVarPrintFileFuncPic008   },
    { TOUVAR_PIC008_PRINTMODE,          TouVarPrintModeFuncPic008   },
    { TOUVAR_PIC008_PRINTSET,           TouVarPrintSetFuncPic008    },
    { TOUVAR_PIC008_DEVINFO,            TouVarDevInfoFuncPic008     },
    { TOUVAR_PIC008_HOTENDTEMP,         TouVarHotendTempFuncPic008  },
    { TOUVAR_PIC008_BEDTEMP,            TouVarBedTempFuncPic008     },
    { TOUVAR_PIC_RETURN,                TouVarReturn                },
    { TOUVAR_PIC_FILE_SELECT,           TouVarFileSelect            },
    { TOUVAR_PIC_NEXT_PAGE,             TouVarFileNextPage          },
    { TOUVAR_PIC_LAST_PAGE,             TouVarFileLastPage          },
    { TOUVAR_PIC014_PRINT_SET,          TouVarPrintSetPic014        },
    { TOUVAR_PIC014_PRINT_PAUSE,        TouVarPrintPausePic014      },
    { TOUVAR_PIC014_PRINT_STOP,         TouVarPrintStopPic014       },
    { TOUVAR_PIC015_FINISH,             TouVarPrintFinishPic015     },
    { TOUVAR_PIC016_BOX_FAN,            TouVarBoxFanPic016          },
    { TOUVAR_PIC016_BOX_LED,            TouVarBoxLedPic016          },
    { TOUVAR_PIC016_WIFI_LED,           TouVarWifiLedPic016         },
    { TOUVAR_PIC016_DOOR_PAUSE,         TouVarDoorPausePic016       },
    { TOUVAR_PIC017_CONFIRM,            TouVarPrintStopConfirmPic017},
    { TOUVAR_PIC017_CANCEL,             TouVarPrintStopCancelPic017 },
    { TOUVAR_PIC018_CONTINUE,           TouVarPrintPauseRecoPic018  },
    { TOUVAR_PIC018_STOP,               TouVarPrintStopPic018       },
    { TOUVAR_PIC018_IN_OUT_MAT,         TouVarPrintInOuMatPic018    },
    { TOUVAR_PIC019_CONFIRM,            TouVarConfirmPrintPic019    },
    { TOUVAR_PIC019_CANCEL,             TouVarCancelPrintPic019     },
    { TOUVAR_PIC020_PLA_SELECT,         TouVarPLASelectPic020       },
    { TOUVAR_PIC020_ABS_SELECT,         TouVarABSSelectPic020       },
    // { TOUVAR_PIC020_USER_DEF,           TouVarUserDefPic020         },
    { TOUVAR_PIC020_DISABLE_HEAT,       TouVarDisableHeatPic020     },
    { TOUVAR_PIC020_IN_OUT_MAT,         TouVarInOutMatPic020        },
    { TOUVAR_PIC021_IN_MAT,             TouVarInMatPic021           },
    { TOUVAR_PIC021_OUT_MAT,            TouVarOutMatPic021          },
    { TOUVAR_PIC023_OUTMAT_CONF,        TouVarOutMatConfirmPic023   },
    { TOUVAR_PIC024_HOTEND_FAN,         TouVarSetHotendFanPic024    },    /* invalid */
    { TOUVAR_PIC_START_PRINT,           TouVarStartPrint            },
    { TOUVAR_PIC025_BED_CALI,           TouVarBedCaliPic025         },
    { TOUVAR_PIC025_AXIS_MOVE,          TouVarAxisMovePic025        },
    { TOUVAR_PIC_UNIT_1MM,              TouVarStepUnitMM1           },
    { TOUVAR_PIC_UNIT_01MM,             TouVarStepUnitMM01          },
    { TOUVAR_PIC_X_BACKWARD,            TouVarXBackward             },
    { TOUVAR_PIC_X_FORWARD,             TouVarXForward              },
    { TOUVAR_PIC_Y_FORWARD,             TouVarYForward              },
    { TOUVAR_PIC_Y_BACKWARD,            TouVarYBackward             },
    { TOUVAR_PIC_Z_UP,                  TouVarZUp                   },
    { TOUVAR_PIC_Z_DOWN,                TouVarZDown                 },
    { TOUVAR_PIC_GO_HOME,               TouVarGoHome                },
    { TOUVAR_PIC_UNIT_10MM,             TouVarStepUnitMM10          },
    { TOUVAR_PIC031_LANG_SELECT,        TouVarLangSelectPic031      },
    { TOUVAR_PIC031_DEV_RESET,          TouVarDevResetPic031        },
    { TOUVAR_PIC032_LANG_TYPE,          TouVarLangTypePic032        },
    { TOUVAR_PIC033_CONFIRM,            TouVarConfirmResetPic033    },
    { TOUVAR_PIC033_CANCEL,             TouVarCancelResetPic033     },
    { TOUVAR_PIC036_CONTINUE,           TouVarContinuePrintPic036   },
    { TOUVAR_PIC036_CANCEL,             TouVarCancelPrintPic036     },
    { TOUVAR_PIC027_BEDCALI_UP,         TouVarBedCaliUpPic027       },
    { TOUVAR_PIC027_BEDCALI_DOWN,       TouVarBedCaliDownPic027     },
    { TOUVAR_PIC027_BEDCALI_SAVE,       TouVarBedCaliSavePic027     },
    { TOUVAR_PIC024_BOX_FAN,            TouVarSetBoxFanPic024       },    /* invalid */
    { TOUVAR_PIC039_CONFIRM,            TouVarConfirmLoadMatPic039  },
    { TOUVAR_PIC039_STOPPRINT,          TouVarStopPrintPic039       },
    { TOUVAR_PIC040_CONTIPRINT,         TouVarContinuePrintPic040   },
    { TOUVAR_PIC040_STOPPRINT,          TouVarStopPrintPic040       },
    { TOUVAR_PIC024_SAVE_SELECT,        TouVarSaveSelectPic024      },
    { TOUVAR_PIC042_CONFIRM,            TouVarConfStopHeatPic042    },
    { TOUVAR_PIC042_CANCEL,             TouVarCancStopHeatPic042    },
    { TOUVAR_PIC_USERDEF_HOTEND,        TouVarUserDefHotendPic024   },
    { TOUVAR_PIC_USERDEF_BED,           TouVarUserDefBedPic024      },
    { TOUVAR_PIC_X_KBD_INPUT,           TouVarKeybrdInputX          },
    { TOUVAR_PIC_Y_KBD_INPUT,           TouVarKeybrdInputY          },
    { TOUVAR_PIC_Z_KBD_INPUT,           TouVarKeybrdInputZ          },
    { TOUVAR_PIC_PRINT_FEESRATE,        TouVarFeedrateFunc          },
    { TOUVAR_PIC044_CONFIRM,            TouVarConfirmPic044         },
    { TOUVAR_PIC045_CONFIRM,            TouVarConfirmPic045         },
    { TOUVAR_PIC045_CANCEL,             TouVarCancelPic045          }
};

/**
 * PLA print mode parameters' value
 */
const AutoUIPrintModeParas_t PLAMode =
{
    .pmpHotendTemp             = 200,
    .pmpBedTemp                = 60,
    .pmpHotendFanFlag          = true,
    .pmpBoxFanFlag             = true
};

/**
 * ABS print mode parameters' value
 */
const AutoUIPrintModeParas_t ABSMode =
{
    .pmpHotendTemp             = 240,
    .pmpBedTemp                = 80,
    .pmpHotendFanFlag          = false,
    .pmpBoxFanFlag             = false
};

/**
 * user-define print mode parameters' value
 */
AutoUIPrintModeParas_t userDefMode =
{
    .pmpHotendTemp             = 0,
    .pmpBedTemp                = 0,
    .pmpHotendFanFlag          = false,
    .pmpBoxFanFlag             = false
};

/***********************************************************************************************************************************
                                                          common functions
***********************************************************************************************************************************/
/**
 * [LcdAutoUISetBedLeveling :control function of bed-leveling]
 * @Author Creality
 * @Time   2021-07-14
 * @param  _en        [true:turn on, false:turn off]
 */
void LcdAutoUISetBedLeveling(bool _en)
{
    if(_en)
    {
        set_bed_leveling_enabled(true);
        queue.enqueue_one_now(PSTR("M420 S1"));
    }
    else
    {
        set_bed_leveling_enabled(false);
        queue.enqueue_one_now(PSTR("M420 S0"));
    }
}

/**
 * [LcdAutoUISetHotendTemp :set hotend temperature]
 * @Author Creality
 * @Time   2021-06-01
 * @param  _targetTemp [value of target temperature]
 */
void LcdAutoUISetTempHotend(int16_t _targetTemp)
{
    int16_t lTargetTemp = _targetTemp > MAX_TEMP_HOTEND ? MAX_TEMP_HOTEND : _targetTemp;

    thermalManager.temp_hotend[0].target = lTargetTemp;
    thermalManager.setTargetHotend(thermalManager.temp_hotend[0].target, 0);
}

/**
 * [LcdAutoUISetTempHotendBlock :set hotend temperature and wait for it to be reached]
 * @Author Creality
 * @Time   2021-06-10
 * @param  _targetTemp [value of target temperature, must bigger than 100]
 */
void LcdAutoUISetTempHotendBlock(int16_t _targetTemp)
{
    // char lCmdBuf[20] = {0};

    if(_targetTemp > 0)
    {
        /* set hotend target temperature */
        LcdAutoUISetTempHotend(_targetTemp);

        /* wait current hotend temperature to traget */
        thermalManager.wait_for_hotend(0, _targetTemp);
        // sprintf(lCmdBuf, "M109 S%d", _targetTemp);
        // queue.enqueue_one_now(lCmdBuf);
    } 
}

/**
 * [LcdAutoUIGetTempHotend :get current hotend temperature]
 * @Author Creality
 * @Time   2021-06-02
 * @return            [value of hotend temperature]
 */
float LcdAutoUIGetTempHotend(void)
{
    return thermalManager.temp_hotend[0].celsius;
}

/**
 * [LcdAutoUIGetTarTempHotend :get target hotend temperature]
 * @Author Creality
 * @Time   2021-08-11
 * @return            [description]
 */
float LcdAutoUIGetTarTempHotend(void)
{
    return thermalManager.temp_hotend[0].target;
}

/**
 * [LcdAutoUISetTempBed :set bed temperature]
 * @Author Creality
 * @Time   2021-06-01
 * @param  _targetTemp [value of target temperature]
 */
void LcdAutoUISetTempBed(int16_t _targetTemp)
{
    int16_t lTargetTemp = _targetTemp > MAX_TEMP_BED ? MAX_TEMP_BED : _targetTemp;
    
    thermalManager.temp_bed.target = lTargetTemp;
    thermalManager.setTargetBed(thermalManager.temp_bed.target);
}

/**
 * [LcdAutoUISetTempBedBlock :set bed temperature and wait for it to be reached]
 * @Author Creality
 * @Time   2021-06-10
 * @param  _targetTemp [value of target temperature, must bigger than 100]
 */
void LcdAutoUISetTempBedBlock(int16_t _targetTemp)
{
    // char lCmdBuf[20] = {0};

    if(_targetTemp > 0)
    {
        /* set bed target temperature */
        LcdAutoUISetTempBed(_targetTemp);

        /* wait current bed temperature to target */
        thermalManager.wait_for_bed(_targetTemp);
        // sprintf(lCmdBuf, "M190 S%d", _targetTemp);
        // queue.enqueue_one_now(lCmdBuf);
    }
}

/**
 * [LcdAutoUISetTempBed :get current bed temperature]
 * @Author Creality
 * @Time   2021-06-04
 * @return             [value of bed temperature]
 */
float LcdAutoUIGetTempBed(void)
{
    return thermalManager.temp_bed.celsius;
}

/**
 * [LcdAutoUIGetTarTempBed :get target bed temperature]
 * @Author Creality
 * @Time   2021-08-11
 * @return            [description]
 */
float LcdAutoUIGetTarTempBed(void)
{
    return thermalManager.temp_bed.target;
}

/**
 * [LcdAutoUISetFeedrate :set printing feedrate]
 * @Author Creality
 * @Time   2021-08-11
 * @return            [description]
 */
void LcdAutoUISetFeedrate(int16_t _feedrate)
{
    feedrate_percentage = _feedrate;

    //这两个备份值也同步赋值，使得在“执行G28期间修改速率不生效”问题解决
    saved_feedrate_mm_s = _feedrate;
    saved_feedrate_percentage = _feedrate;
    /* updata printting feedate */
    gLcdAutoUI.DisplayData(feedrate_percentage, DATAVAR_ADDR_FEEDRARE);
}

/**
 * [LcdAutoUIMoveX :X-Axis moving control]
 * @Author Creality
 * @Time   2021-06-07
 * @param  _distance  [distance that need to move or position that need to arrive]
 * @param  _moveType  [moving type]
 */
void LcdAutoUIMoveX(float _distance, AutoUIAxisMoveType_t _moveType)
{
    float lDistance = _distance > X_BED_SIZE ? X_BED_SIZE : _distance;

    /* if device have not gone home before, and absolute-motion is selected. Do not allow to move axis */
    if((_moveType == AXIS_MOVE_ABSO) && (!gLcdAutoUI.GetHaGoHoBeFlag()))
        return;

    do
    {
        if(_moveType == AXIS_MOVE_RELA)
            current_position[X_AXIS] += lDistance;
        else if(_moveType == AXIS_MOVE_ABSO)
            current_position[X_AXIS] = lDistance;
        else
            break;

        RTS_line_to_current(X_AXIS);
    }while(0);
}

/**
 * [LcdAutoUIMoveY :Y-Axis moving control]
 * @Author Creality
 * @Time   2021-06-07
 * @param  _distance  [distance that need to move or position that need to arrive]
 * @param  _moveType  [moving type]
 */
void LcdAutoUIMoveY(float _distance, AutoUIAxisMoveType_t _moveType)
{
    float lDistance = _distance > Y_BED_SIZE ? Y_BED_SIZE : _distance;

    /* if device have not gone home before, and absolute-motion is selected. Do not allow to move axis */
    if((_moveType == AXIS_MOVE_ABSO) && (!gLcdAutoUI.GetHaGoHoBeFlag()))
        return;

    do
    {
        if(_moveType == AXIS_MOVE_RELA)
            current_position[Y_AXIS] += lDistance;
        else if(_moveType == AXIS_MOVE_ABSO)
            current_position[Y_AXIS] = lDistance;
        else
            break;

        RTS_line_to_current(Y_AXIS);
    }while(0);
}

/**
 * [LcdAutoUIMoveXY :get to the given postion(x, y), block program running]
 * @Author Creality
 * @Time   2021-06-08
 * @param  _posX      [X coordinate]
 * @param  _posY      [Y coordinate]
 */
void LcdAutoUIMoveXYBlock(float _posX, float _posY)
{
    float lPosX = _posX > X_BED_SIZE ? X_BED_SIZE : _posX;
    float lPosY = _posY > Y_BED_SIZE ? Y_BED_SIZE : _posY;

    do_blocking_move_to_xy(lPosX, lPosY);
}

/**
 * [LcdAutoUIMoveXYCmd :get to the given postion(x, y) by inserting command]
 * @Author Creality
 * @Time   2021-07-13
 * @param  _posX      [X coordinate]
 * @param  _posY      [Y coordinate]
 */
void LcdAutoUIMoveXYCmd(int _posX, int _posY)
{
    char lCmdBuf[64];
    
    memset(lCmdBuf, 0, sizeof(lCmdBuf));
    sprintf(lCmdBuf, "%s%s%d%s%d%s", "G1 ", "X", _posX, " Y", _posY, " F2000");
    queue.enqueue_one_now(lCmdBuf);
}

/**
 * [LcdAutoUIMoveZ :Z-Axis moving control, not block program running]
 * @Author Creality
 * @Time   2021-06-07
 * @param  _distance  [distance that need to move or position that need to arrive]
 * @param  _moveType  [moving type]
 */
void LcdAutoUIMoveZ(float _distance, AutoUIAxisMoveType_t _moveType)
{
    float lDistance = _distance > Z_MAX_POS ? Z_MAX_POS : _distance;

    /* if device have not gone home before, and absolute-motion is selected. Do not allow to move axis */
    if((_moveType == AXIS_MOVE_ABSO) && (!gLcdAutoUI.GetHaGoHoBeFlag()))
        return;

    do
    {
        if(_moveType == AXIS_MOVE_RELA)
            current_position[Z_AXIS] += lDistance;
        else if(_moveType == AXIS_MOVE_ABSO)
            current_position[Z_AXIS] = lDistance;
        else
            break;

        RTS_line_to_current(Z_AXIS);
    }while(0);
}

/**
 * [LcdAutoUIMoveZBlock :get to the given postion(z), block program running]
 * @Author Creality
 * @Time   2021-06-08
 * @param  _posZ      [Z coordinate]
 */
void LcdAutoUIMoveZBlock(float _posZ)
{
    float lPosZ = _posZ > Z_MAX_POS ? Z_MAX_POS : _posZ;

    do_blocking_move_to_z(lPosZ);
}

/**
 * [LcdAutoUIMoveZCmd :get to the given postion(z) by inserting command]
 * @Author Creality
 * @Time   2021-07-13
 * @param  _posZ      [Z coordinate]
 */
void LcdAutoUIMoveZCmd(int _posZ)
{
    char lCmdBuf[32];
    
    memset(lCmdBuf, 0, sizeof(lCmdBuf));
    sprintf(lCmdBuf, "%s%s%d%s", "G1 ", "Z", _posZ, " F160");
    queue.enqueue_now_P(lCmdBuf);
}

/**
 * [LcdAutoUIMoveXYZBlock :get to the given postion(x, y, z) by inserting command]
 * @Author Creality
 * @Time   2021-07-20
 * @param  _posX      [X coordinate]
 * @param  _posY      [Y coordinate]
 * @param  _posZ      [Z coordinate]
 */
void LcdAutoUIMoveXYZBlock(float _posX, float _posY, float _posZ)
{
    xy_pos_t lPosXY;
    float lPosZ;

    lPosXY.x = _posX > X_BED_SIZE ? X_BED_SIZE : _posX;
    lPosXY.y = _posY > Y_BED_SIZE ? Y_BED_SIZE : _posY;
    lPosZ = _posZ > Z_MAX_POS ? Z_MAX_POS : _posZ;

    do_blocking_move_to_xy_z(lPosXY, lPosZ);
}

/**
 * [LcdAutoUIMoveE :E-Axis moving control, default: relative motion]
 * @Author Creality
 * @Time   2021-06-01
 * @param  _distance  [moving distance]
 * @param  _feedRate  [moving speed]
 */
void LcdAutoUIMoveE(float _distance, uint32_t _feedRate)
{
    current_position[E_AXIS] += _distance;
    line_to_current_position(feedRate_t(_feedRate));
}

void LcdAutoUIMoveECmd_G0(int _posE)
{
    char lCmdBuf[50];
    
    memset(lCmdBuf, 0, sizeof(lCmdBuf));
    sprintf(lCmdBuf, "%s%s%d%s", "G0 ", "E", _posE, " F160");
    queue.enqueue_now_P(lCmdBuf);
}

/**
 * [LcdAutoUISetECmd :Set E-Axis current position by G-Code]
 * @Author Creality
 * @Time   2021-08-14
 * @param  _PosE      [E-Axis position that need to be set]
 */
void LcdAutoUISetECmd(float _PosE)
{
    char lCmdBuf[32];
    
    memset(lCmdBuf, 0, sizeof(lCmdBuf));
    sprintf(lCmdBuf, "%s%d", "G92.9 E", (int)(_PosE));
    queue.enqueue_now_P(lCmdBuf);
}

/**
 * [LcdAutoUIGetTimeMs :get current time]
 * @Author Creality
 * @Time   2021-06-04
 * @return            [current time(Ms)]
 */
uint32_t LcdAutoUIGetTimeMs(void)
{
    return millis();
}

/**
 * [SaveWifiMac :save MAC Address of wifi board to EEPROM]
 * @Author Creality
 * @Time   2021-10-10
 */
void SaveWifiMac(void)
{
    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_WIFIMAC_OFFSET, (uint8_t*)gLcdAutoUI.wifiMacAddr, EEPROM_ADDR_WIFIMAC_SIZE);
}

/**
 * [ReadWifiMac :get MAC Address of wifi board from EEPROM]
 * @Author Creality
 * @Time   2021-10-10
 */
void ReadWifiMac(void)
{
    memset(gLcdAutoUI.wifiMacAddr, 0, sizeof(gLcdAutoUI.wifiMacAddr));
    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_WIFIMAC_OFFSET, (uint8_t*)gLcdAutoUI.wifiMacAddr, EEPROM_ADDR_WIFIMAC_SIZE);
}

/**
 * [SaveUserDefModeParas :save user define mode parameters to EEPROM]
 * @Author Creality
 * @Time   2021-06-19
 */
void SaveUserDefModeParas(void)
{
    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_UDMP_OFFSET, (uint8_t*)&userDefMode, EEPROM_ADDR_UDMP_SIZE);
}

/**
 * [ReadUserDefModeParas :get user define mode parameters from EEPROM]
 * @Author Creality
 * @Time   2021-06-19
 */
void ReadUserDefModeParas(void)
{
    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_UDMP_OFFSET, (uint8_t*)&userDefMode, EEPROM_ADDR_UDMP_SIZE);
}

/**
 * [SaveBedCaliZOffsetCenter :save bed calibration Z axis offset of center point to EEPROM]
 * @Author Creality
 * @Time   2021-07-21
 */
void SaveBedCaliZOffsetCenter(void)
{
    int lBedCaliZOffsetCenter = 0;

    /* Keep two decimal places */
    lBedCaliZOffsetCenter = (int)(100 * gLcdAutoUI.bedCaliZOffsetCenter);

    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_ZAXIS_OFFSET, (uint8_t*)&lBedCaliZOffsetCenter, EEPROM_ADDR_ZAXIS_SIZE);
}

/**
 * [ReadBedCaliZOffsetCenter :read bed calibration Z axis offset of center point from EEPROM]
 * @Author Creality
 * @Time   2021-07-21
 */
void ReadBedCaliZOffsetCenter(void)
{
    int lBedCaliZOffsetCenter = 0;

    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_ZAXIS_OFFSET, (uint8_t*)&lBedCaliZOffsetCenter, EEPROM_ADDR_ZAXIS_SIZE);

    /* Keep two decimal places */
    gLcdAutoUI.bedCaliZOffsetCenter = (float)(1.0 * lBedCaliZOffsetCenter) / 100.0;  
}

/**
 * [ClearBedCaliZOffsetCenter :clear bed calibration Z axis offset of center point and save in EEPROM]
 * @Author Creality
 * @Time   2021-07-21
 */
void ClearBedCaliZOffsetCenter(void)
{
    int lBedCaliZOffsetCenter = 0;

    gLcdAutoUI.bedCaliZOffsetCenter = 0.0;

    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_ZAXIS_OFFSET, (uint8_t*)&lBedCaliZOffsetCenter, EEPROM_ADDR_ZAXIS_SIZE);
}

/**
 * [GetSDCardGcodeFileNum :get gcode file number that read from SD card]
 * @Author Creality
 * @Time   2021-06-06
 * @return            [number of gcode file]
 */
int GetSDCardGcodeFileNum(void)
{
    return CardRecbuf.Filesum;
}

/**
 * [IsExistPowerOffDurPrint :does there exist sitution that power off during printing]
 * @Author Creality
 * @Time   2021-06-19
 * @return            [true: exist, false: not exist]
 */
bool IsExistPowerOffDurPrint(void)
{
    return recovery.info.recovery_flag;
}

/**
 * [ClearSDCardGcodeFileRecord :clear record about SD card]
 * @Author Creality
 * @Time   2021-06-08
 */
void ClearSDCardGcodeFileRecord(void)
{
    memset(&CardRecbuf, 0, sizeof(CardRecbuf));

    gLcdAutoUI.gcodeFileSelectedCur = VAULE_INVALIDE;
}

/**
 * [FileSelectBackground :display highlight background when file is selected]
 * @Author Creality
 * @Time   2021-06-06
 * @param  _fileSelected [ID of file selected]
 * @param  _bgStartAddr  [start address of background icon]
 */
uint16_t FileSelectBackground(uint16_t _fileSelectedID, uint16_t _bgStartAddr)
{
    uint16_t lRet = VAULE_INVALIDE;
    int lGcodeFileNum = GetSDCardGcodeFileNum() > GCODE_FILE_NUM_MAX ? GCODE_FILE_NUM_MAX : GetSDCardGcodeFileNum();

    for(int i=0; i<lGcodeFileNum; i++)
    {
        if(i == _fileSelectedID)
        {
            gLcdAutoUI.SwitchIcon(SELECTED_YES, _bgStartAddr + i);
            lRet = i;
        }
        else
        {
            gLcdAutoUI.SwitchIcon(SELECTED_NO, _bgStartAddr + i);
        }
    }

    return lRet;
}

/**
 * [LanguageSelect :get language selected ID according to key value and refresh language icons]
 * @Author Creality
 * @Time   2021-10-15
 * @param  _keyVal    [key value that get from touch-screen]
 * @return            [language selected ID]
 */
AutoUILanguage_t LanguageSelect(unsigned short _keyVal)
{
    unsigned short lKeyVal = 0;

    lKeyVal = _keyVal >= AUTOUI_LANGUAGE_TU ? AUTOUI_LANGUAGE_TU : _keyVal;

    for(uint8_t i=0; i<=AUTOUI_LANGUAGE_TU; i++)
    {
        if(i == lKeyVal)
            gLcdAutoUI.SwitchIcon(SELECTED_YES, ICONVAR_ADDR_LANG_SELECT_CH + i);
        else
            gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_LANG_SELECT_CH + i);
    }

    return (AutoUILanguage_t)lKeyVal;
}

/**
 * [ClearFileBackground :clear file selected highlight icon]
 * @Author Creality
 * @Time   2021-06-08
 */
void ClearFileBackground(void)
{
    for(int i=0; i<GCODE_FILE_NUM_MAX; i++)
        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_FILESELECT_BG00 + i);
}

/**
 * [IsSDCardStatusStable :to judge if SD Card Pin status is stable or not]
 * @Author Creality
 * @Time   2021-10-19
 * @return            [true: stable, false: not stable]
 */
bool IsSDCardStatusStable(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();
    static uint8_t lStaRecordInd = 0;
    static bool lStaRecord[6] = {0};

    if((LcdAutoUIGetTimeMs() - lMs) > 50)
    {
        lMs = LcdAutoUIGetTimeMs();

        if(lStaRecordInd < sizeof(lStaRecord))
        {
            lStaRecord[lStaRecordInd] = IS_SD_INSERTED();
            lStaRecordInd ++;
            return false;
        }
        else
        {
            lStaRecordInd = 0;

            for(uint8_t i=0; i<sizeof(lStaRecord)-1; i++)
            {
                if(lStaRecord[i] != lStaRecord[i+1])
                    return false;
            }
            return true;
        }
    }
    return false;
}

/**
 * [IsExistMaterial :if there exist material for printing]
 * @Author Creality
 * @Time   2021-06-17
 * @return            [true: exist, false: not exist]
 */
bool IsExistMaterial(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();
    static uint32_t checkMatweialCnt = 0;
    if((LcdAutoUIGetTimeMs() - lMs) > 1000)
    {
        lMs = LcdAutoUIGetTimeMs();
        checkMatweialCnt ++;
    }
    
    if(0 == READ(CHECK_MATWEIAL)) /* Low Level : exist material */
    {
        checkMatweialCnt = 0;
        return true;
    }
    else if (checkMatweialCnt >= 2) /* High Level : not exist material */
        return false;
    else /* High Level : Waiting for confirmation */
        return true;
}

/**
 * [IsDoorOpen :if door is open]
 * @Author Creality
 * @Time   2021-06-19
 * @return            [true: door open , false: door close]
 */
#if ENABLE_DOOR_OPEN_CHECK
bool IsDoorOpen(void)
{
    /* High Level : door open */
    if(1 == READ(CHECK_DOOR_PIN))
        return true;
    /* Low Level : door close */
    else
        return false;
}
#endif

/**
 * [SetHotendFan :hotend fan control]
 * @Author Creality
 * @Time   2021-06-16
 * @param  _en        [true: turn on, false: turn off]
 */
void SetHotendFan(bool _en)
{
    /* turn on */
    if(_en)
        thermalManager.fan_speed[0] = 0xFF;
    /* turn off */
    else
        thermalManager.fan_speed[0] = 0; 
}


/**
 * [SetBoxFan :box fan control]
 * @Author Creality
 * @Time   2021-06-16
 * @param  _en        [true: turn on, false: turn off]
 */
void SetBoxFan(bool _en)
{
    /* turn on */
    if(_en)
    {
        WRITE(BOX_FAN_PIN, AUTOUI_ENABLE);
        gLcdAutoUI.enableSwitches.swiBoxFan = true;
    }
    /* turn off */
    else
    {
        WRITE(BOX_FAN_PIN, AUTOUI_DISABLE);
        gLcdAutoUI.enableSwitches.swiBoxFan = false;
    }
}

/**
 * [SetBoxLed :box led control]
 * @Author Creality
 * @Time   2021-06-16
 * @param  _en        [true: turn on, false: turn off]
 */
void SetBoxLed(bool _en)
{
    /* turn on */
    if(_en)
    {
        digitalWrite(LED_CONTROL_PIN, AUTOUI_ENABLE);
        gLcdAutoUI.enableSwitches.swiBoxLed = true;
    }
    /* turn off */
    else
    {
        digitalWrite(LED_CONTROL_PIN, AUTOUI_DISABLE);
        gLcdAutoUI.enableSwitches.swiBoxLed = false;
    }
}

/**
 * [SaveSwitchesStatus :save box-fan/box-led/wifi-led/door-pause function status in EEPROM]
 * @Author Creality
 * @Time   2021-07-12
 */
void SaveSwitchesStatus(void)
{
    uint8_t lSwiSta = 0;

    /* record box-fan status */
    if(gLcdAutoUI.enableSwitches.swiBoxFan)
        lSwiSta = lSwiSta | 0x80;
    else
        lSwiSta = lSwiSta & 0x7F;

    /* record box-led status */
    if(gLcdAutoUI.enableSwitches.swiBoxLed)
        lSwiSta = lSwiSta | 0x40;
    else
        lSwiSta = lSwiSta & 0xBF;

    /* record wifi-led status */
    if(gLcdAutoUI.enableSwitches.swiWifiLed)
        lSwiSta = lSwiSta | 0x20;
    else
        lSwiSta = lSwiSta & 0xDF;

    /* record door-pause status */
    if(gLcdAutoUI.enableSwitches.swiDoorPause)
        lSwiSta = lSwiSta | 0x10;
    else
        lSwiSta = lSwiSta & 0xEF;

    /* save to EEPROM */
    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_BFLWD_OFFSET, &lSwiSta, EEPROM_ADDR_BFLWD_SIZE);
}

/**
 * [ReadSwitchesStatus :get box-fan/box-led/wifi-led/door-pause function status from EEPROM]
 * @Author Creality
 * @Time   2021-07-12
 */
void ReadSwitchesStatus(void)
{
    uint8_t lSwiSta = 0;

    /* read switches status from EEPROM */
    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_BFLWD_OFFSET, &lSwiSta, EEPROM_ADDR_BFLWD_SIZE);

    /* for the first time to read status */
    if(lSwiSta == 0xFF)
        return;

    /* set box-fan status */
    if(lSwiSta & 0x80)
        SetBoxFan(true);
    else
        SetBoxFan(false);

    /* set box-led status */
    if(lSwiSta & 0x40)
        SetBoxLed(true);
    else
        SetBoxLed(false);

    /* set wifi-led status */
    if(lSwiSta & 0x20)
        gLcdAutoUI.enableSwitches.swiWifiLed = true;
    else
        gLcdAutoUI.enableSwitches.swiWifiLed = false;

    /* set door-pause status */
    if(lSwiSta & 0x10)
        gLcdAutoUI.enableSwitches.swiDoorPause = true;
    else
        gLcdAutoUI.enableSwitches.swiDoorPause = false;
}

/**
 * [ClearSwitchesStatus :clear box-fan/box-led/wifi-led/door-pause function status and save in EEPROM]
 * @Author Creality
 * @Time   2021-07-23
 */
void ResetSwitchesStatus(void)
{
    SetBoxFan(false);
    SetBoxLed(false);
    gLcdAutoUI.enableSwitches.swiWifiLed = true;
    gLcdAutoUI.enableSwitches.swiDoorPause = true;

    /* save to EEPROM */
    SaveSwitchesStatus();
}

/**
 * [PrintParasSetByMode :set print parameters according to print mode]
 * @Author Creality
 * @Time   2021-06-16
 */
void PrintParasSetByMode(void)
{
    memset(&gLcdAutoUI.printModeParas, 0, sizeof(AutoUIPrintModeParas_t));

    if(gLcdAutoUI.printModeType == PRINT_MODE_INVAILD)
    {
        return;
    }
    /* PLA mode */
    else if(gLcdAutoUI.printModeType == PRINT_MODE_PLA)
    {
        memcpy(&gLcdAutoUI.printModeParas, &PLAMode, sizeof(AutoUIPrintModeParas_t));
    }
    /* ABS mode */
    else if(gLcdAutoUI.printModeType == PRINT_MODE_ABS)
    {
        memcpy(&gLcdAutoUI.printModeParas, &ABSMode, sizeof(AutoUIPrintModeParas_t));
    }
    /* user define */
    #if 0
    else if(gLcdAutoUI.printModeType == PRINT_MODE_USERDEF)
    {
        memcpy(&gLcdAutoUI.printModeParas, &userDefMode, sizeof(AutoUIPrintModeParas_t));
    }
    #endif

    /* device control by mode parameters */
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
    {
        LcdAutoUISetTempBedBlock(gLcdAutoUI.printModeParas.pmpBedTemp);
    }
    else
    {
        /* if stop printing during 'DEVSTA_HEATING' status */
        wait_for_heatup = false;
        return;
    }
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
    {
        LcdAutoUISetTempHotendBlock(gLcdAutoUI.printModeParas.pmpHotendTemp);
    }
    else
    {
        /* if stop printing during 'DEVSTA_HEATING' status */
        wait_for_heatup = false;
        return;
    }
    SetHotendFan(gLcdAutoUI.printModeParas.pmpHotendFanFlag);
    SetBoxFan(gLcdAutoUI.printModeParas.pmpBoxFanFlag);

    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
    /* change device status to 'DEVSTA_PRINTING' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_PRINTING);
    gLcdAutoUI.SetStaGoingHome(GO_HOME_IDLE);
}

/**
 * [RefreshUserDefParasDispaly :refresh user define mode parameter and display on DWIN]
 * @Author Creality
 * @Time   2021-06-19
 */
void RefreshUserDefParas(void)
{
    /* get user define mode parameters from EEPROM */
    ReadUserDefModeParas();

    /* refresh hotend and bed temperature */
    gLcdAutoUI.DisplayData(userDefMode.pmpHotendTemp, DATAVAR_ADDR_USERDEF_HOTEND_TEMP);
    gLcdAutoUI.DisplayData(userDefMode.pmpBedTemp, DATAVAR_ADDR_USERDEF_BED_TEMP);

    /* refresh hotend fan status */
    if(userDefMode.pmpHotendFanFlag)
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_HOTEND_FAN_STA);
    else
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_HOTEND_FAN_STA);

    /* refresh box fan status */
    if(userDefMode.pmpBoxFanFlag)
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_FAN_STA);
    else
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_FAN_STA);
}

/**
 * [RefreshDevInfo :refresh device information]
 * @Author Creality
 * @Time   2021-06-21
 */
void RefreshDevInfo(void)
{
    /* hardware version */
    gLcdAutoUI.DisplayText((char*)HARDWAREVERSION, TEXTVAR_ADDR_HARDWARE_VER);
    /* firmware version */
    gLcdAutoUI.DisplayText((char*)SOFTVERSION, TEXTVAR_ADDR_FIRMWARE_VER);
    /* screen project version */
    // gLcdAutoUI.DisplayText((char*)SCREENVERSION, TEXTVAR_ADDR_SCREEN_VER);
    /* wifi board MAC */
    gLcdAutoUI.DisplayText((char*)WIFI_BOARD_MAC, TEXTVAR_ADDR_WIFI_BOARD_MAC);
    /* machine size */
    gLcdAutoUI.DisplayText((char*)MACHINE_SIZE, TEXTVAR_ADDR_PRINT_SIZE);
    /* machine name */
    gLcdAutoUI.DisplayText((char*)MACHINE_NAME_DWIN, TEXTVAR_ADDR_DEVICE_NAME);
}

/**
 * [RefreshAllTemperature :refresh device all temperature display]
 * @Author Creality
 * @Time   2021-06-09
 */
void RefreshAllTemperature(void)
{
    static int lTempRecoHotendCur = 0;
    static int lTempRecoHotendTar = 0;
    static int lTempRecoBedCur = 0;
    static int lTempRecoBedTar = 0;

    /* refresh current hotend temperature */
    if(lTempRecoHotendCur != LcdAutoUIGetTempHotend())
    {
        lTempRecoHotendCur = LcdAutoUIGetTempHotend();
        gLcdAutoUI.DisplayData(lTempRecoHotendCur, DATAVAR_ADDR_HOTENDTEMP_CUR);
    }
    /* refresh target hotend temperature */
    if(lTempRecoHotendTar != thermalManager.degTargetHotend(0))
    {
        lTempRecoHotendTar = thermalManager.degTargetHotend(0);
        gLcdAutoUI.DisplayData(lTempRecoHotendTar, DATAVAR_ADDR_HOTENDTEMP_GIVEN);
    }
    /* refresh current bed temperature */
    if(lTempRecoBedCur != LcdAutoUIGetTempBed())
    {
        lTempRecoBedCur = LcdAutoUIGetTempBed();
        gLcdAutoUI.DisplayData(lTempRecoBedCur, DATAVAR_ADDR_BEDTEMP_CUR);
    }
    /* refresh target bed temperature */
    if(lTempRecoBedTar != thermalManager.degTargetBed())
    {
        lTempRecoBedTar = thermalManager.degTargetBed();
        gLcdAutoUI.DisplayData(lTempRecoBedTar, DATAVAR_ADDR_BEDTEMP_GIVEN);
    }
}

/**
 * [RefreshCurPrintProgress :refresh current printing progress when device is at 'DEVSTA_PRINTING']
 * @Author Creality
 * @Time   2021-06-11
 */
void RefreshCurPrintProgress(void)
{
    if(card.isPrinting() && gLcdAutoUI.percentDoneRecord != card.percentDone())
    {
        gLcdAutoUI.percentDoneRecord = card.percentDone();

        gLcdAutoUI.SwitchIcon((int)gLcdAutoUI.percentDoneRecord, ICONVAR_ADDR_PRINT_PROGRESS);
        gLcdAutoUI.DisplayData((int)gLcdAutoUI.percentDoneRecord, DATAVAR_ADDR_PRINT_PROGRESS);
    }
}

/**
 * [RefreshCurPrintProgress :refresh current printing progress when device is at 'DEVSTA_PRINTING']
 * @Author Creality
 * @Time   2021-06-11
 */
void RefreshCurPrintFeedrate(void)
{
    static int16_t lFeedratePercentageRecord = 0;

    if(lFeedratePercentageRecord != feedrate_percentage)
    {
        lFeedratePercentageRecord = feedrate_percentage;
        gLcdAutoUI.DisplayData(feedrate_percentage, DATAVAR_ADDR_FEEDRARE);

        /* notify to wifi board */
        SERIAL_ECHOPAIR("FR:", feedrate_percentage);
        SERIAL_CHAR('%');
        SERIAL_EOL();
    }
}

/**
 * [RefreshOutMatReturnKey :Select whether to display the return key]
 * true: display return key
 * false : The return key is not displayed
 * @Author Creality
 * @Time   2021-06-11
 */
void RefreshOutMatReturnKey(bool _dispaly)
{
    gLcdAutoUI.SetStaDynamicIcon(_dispaly, ICONVAR_ADDR_OUT_MAT_RETURN_KEY);
}

/**
 * [ClearCurPrintProgress :clear current printing progress]
 * @Author Creality
 * @Time   2021-06-15
 */
void ClearCurPrintProgress(void)
{
    gLcdAutoUI.SwitchIcon(0, ICONVAR_ADDR_PRINT_PROGRESS);
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_PROGRESS);
}

/**
 * [RefreshCurPrintTime :refresh current printing time(how long the gcode have printed)]
 * @Author Creality
 * @Time   2021-06-11
 */
void RefreshCurPrintTime(void)
{
    duration_t elapsed = print_job_timer.duration();

    /* time limit 99H:59M:59S */
    elapsed.value = elapsed.value <= 359999 ? elapsed.value : 359999;

    gLcdAutoUI.DisplayData(elapsed.value/3600, DATAVAR_ADDR_PRINT_TIME_H);
    gLcdAutoUI.DisplayData((elapsed.value%3600)/60, DATAVAR_ADDR_PRINT_TIME_M);

    /* record the printing time once print */
    gLcdAutoUI.printingTimeOnce = (float)elapsed.value/3600.0;
}

/**
 * [ClearCurPrintTime :clear current printing time]
 * @Author Creality
 * @Time   2021-06-15
 */
void ClearCurPrintTime(void)
{
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_TIME_H);
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_PRINT_TIME_M);
}

/**
 * [RefreshCurPosDisplay :refresh current position that display on screen]
 * @Author Creality
 * @Time   2021-06-16
 */
void RefreshCurPosDisplay(void)
{
    static float lXPosRecord = 0;
    static float lYPosRecord = 0;
    static float lZPosRecord = 0;

    /* refresh current X position that display on screen */
    if(lXPosRecord != current_position.x)
    {
        lXPosRecord = current_position.x;
        gLcdAutoUI.DisplayData(lXPosRecord * DWIN_REALVAL_TIMES, DATAVAR_ADDR_CURPOS_X);
    }
    /* refresh current Y position that display on screen */
    if(lYPosRecord != current_position.y)
    {
        lYPosRecord = current_position.y;
        gLcdAutoUI.DisplayData(lYPosRecord * DWIN_REALVAL_TIMES, DATAVAR_ADDR_CURPOS_Y);
    }
    /* refresh current Z position that display on screen */
    if(lZPosRecord != current_position.z)
    {
        lZPosRecord = current_position.z;
        gLcdAutoUI.DisplayData(lZPosRecord * DWIN_REALVAL_TIMES, DATAVAR_ADDR_CURPOS_Z);
    }
}

/**
 * [SynchronizeCurPosWithApp :Synchronize currrent position display with App]
 * @Author Creality
 * @Time   2021-11-24
 */
void SynchronizeCurPosWithApp(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if(((gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_10) || (gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_1) \
        || (gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_01))  && (!gLcdAutoUI.movAxisLocalControlFlag))
    {
        if((LcdAutoUIGetTimeMs() - lMs) > 2000)
        {
            lMs = LcdAutoUIGetTimeMs();

            /* refresh current position that display on screen */
            RefreshCurPosDisplay();
            /* refresh move-axis-picture position */
            memcpy(&gLcdAutoUI.movAxisPicPos, &current_position, sizeof(current_position));
        }
    }
}

/**
 * [RefreshCurPosAccordingToSet :moving axis according to set when background is at 'AUTOUI_AXISCONTR_##']
 * @Author Creality
 * @Time   2021-10-23
 */
void RefreshCurPosAccordingToSet(void)
{
    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    if(((gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_10) || (gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_1) \
        || (gLcdAutoUI.curPicID == AUTOUI_AXISCONTR_01)) && (gLcdAutoUI.movAxisLocalControlFlag))
    {
        if((LcdAutoUIGetTimeMs() - gLcdAutoUI.movAxisPicPosCurTime) > 300)
        {
            if(current_position.x != gLcdAutoUI.movAxisPicPos.x)
            {
                current_position.x = gLcdAutoUI.movAxisPicPos.x;

                LcdAutoUIMoveX(current_position.x, AXIS_MOVE_ABSO);
            }

            if(current_position.y != gLcdAutoUI.movAxisPicPos.y)
            {
                current_position.y = gLcdAutoUI.movAxisPicPos.y;

                LcdAutoUIMoveY(current_position.y, AXIS_MOVE_ABSO);
            }

            if(current_position.z != gLcdAutoUI.movAxisPicPos.z)
            {
                current_position.z = gLcdAutoUI.movAxisPicPos.z;

                LcdAutoUIMoveZ(current_position.z, AXIS_MOVE_ABSO);
            }

            gLcdAutoUI.movAxisLocalControlFlag = false;
        }
    }
}

/**
 * [RefreshCurSwiIconStatus :refresh current icon status of enable switches]
 * @Author Creality
 * @Time   2021-06-18
 */
void RefreshCurSwiIconStatus(void)
{
    static bool lSwiBoxFanRecord    = false;
    static bool lSwiBoxLedRecord    = false;
    static bool lSwiWifiLedRecord   = false;
    static bool lSwiDoorPauseRecord = false;

    /* refresh box fan icon status */
    if((lSwiBoxFanRecord == false) && (gLcdAutoUI.enableSwitches.swiBoxFan == true))
    {
        lSwiBoxFanRecord = true;
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_FAN);
        SaveSwitchesStatus();
    }
    else if((lSwiBoxFanRecord == true) && (gLcdAutoUI.enableSwitches.swiBoxFan == false))
    {
        lSwiBoxFanRecord = false;
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_FAN);
        SaveSwitchesStatus();
    }

    /* refresh box led icon status */
    if((lSwiBoxLedRecord == false) && (gLcdAutoUI.enableSwitches.swiBoxLed == true))
    {
        lSwiBoxLedRecord = true;
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_LED);
        SaveSwitchesStatus();
    }
    else if((lSwiBoxLedRecord == true) && (gLcdAutoUI.enableSwitches.swiBoxLed == false))
    {
        lSwiBoxLedRecord = false;
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_LED);
        SaveSwitchesStatus();
    }

    /* refresh WIFI led icon status */
    if((lSwiWifiLedRecord == false) && (gLcdAutoUI.enableSwitches.swiWifiLed == true))
    {
        lSwiWifiLedRecord = true;
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_WIFI_LED);
        SaveSwitchesStatus();
    }
    else if((lSwiWifiLedRecord == true) && (gLcdAutoUI.enableSwitches.swiWifiLed == false))
    {
        lSwiWifiLedRecord = false;
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_WIFI_LED);
        SaveSwitchesStatus();
    }

    /* refresh door-open-pause icon status */
    if((lSwiDoorPauseRecord == false) && (gLcdAutoUI.enableSwitches.swiDoorPause == true))
    {
        lSwiDoorPauseRecord = true;
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_DOOR_OPEN_PAUSE);
        SaveSwitchesStatus();
    }
    else if((lSwiDoorPauseRecord == true) && (gLcdAutoUI.enableSwitches.swiDoorPause == false))
    {
        lSwiDoorPauseRecord = false;
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_DOOR_OPEN_PAUSE);
        SaveSwitchesStatus();
    }
}

/**
 * [RefreshHeatingSta :refresh current hotend/bed heating status]
 * @Author Creality
 * @Time   2021-08-10
 */
void RefreshHeatingStatus(void)
{
    static float lHotendTempRecord = LcdAutoUIGetTarTempHotend();
    static float lBedTempRecord = LcdAutoUIGetTarTempBed();

    /* refresh hotend heating status */
    if((lHotendTempRecord == 0) && (LcdAutoUIGetTarTempHotend() != 0))
    {
        lHotendTempRecord = LcdAutoUIGetTarTempHotend();

        gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_HEATING_HOTEND);
    }
    else if((lHotendTempRecord != 0) && (LcdAutoUIGetTarTempHotend() == 0))
    {
        lHotendTempRecord = LcdAutoUIGetTarTempHotend();

        gLcdAutoUI.SetStaDynamicIcon(false, ICONVAR_ADDR_HEATING_HOTEND);
    }

    /* refresh bed heating status */
    if((lBedTempRecord == 0) && (LcdAutoUIGetTarTempBed() != 0))
    {
        lBedTempRecord = LcdAutoUIGetTarTempBed();

        gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_HEATING_BED);
    }
    else if((lBedTempRecord != 0) && (LcdAutoUIGetTarTempBed() == 0))
    {
        lBedTempRecord = LcdAutoUIGetTarTempBed();

        gLcdAutoUI.SetStaDynamicIcon(false, ICONVAR_ADDR_HEATING_BED);
    }
}

/**
 * [RefreshWifiMacAddress :if get wifi board MAC from wifi board, display on screen]
 * @Author Creality
 * @Time   2021-10-13
 */
void RefreshWifiMacAddress(void)
{
    if(gLcdAutoUI.getWifiMacAddrFlag)
    {
        gLcdAutoUI.getWifiMacAddrFlag = false;

        /* clear dsiplay area */
        for(uint32_t i=0; i<EEPROM_ADDR_WIFIMAC_SIZE; i++)
            gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_WIFI_BOARD_MAC + i);
        /* display WIFI board MAC*/
        gLcdAutoUI.DisplayText(gLcdAutoUI.wifiMacAddr, TEXTVAR_ADDR_WIFI_BOARD_MAC);
    }
}

/**
 * [GetTPTFromEEPROM :get total printing time from EEPROM]
 * @Author Creality
 * @Time   2021-06-16
 */
void GetTPTFromEEPROM(void)
{
    uint32_t lTotalTimeInt = 0;
    float lTotalTime = 0.0;

    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_TPT_OFFSET, (uint8_t*)&lTotalTimeInt, EEPROM_ADDR_TPT_SIZE);

    lTotalTime = (float)(lTotalTimeInt / 100.0);

    gLcdAutoUI.SetPrintTimeTotal(lTotalTime);
    /* refresh on screen */
    gLcdAutoUI.RefreshPrintTimeTotal();
}

/**
 * [ClearPrintTotalTimeRecord :clear printing-total-time]
 * @Author Creality
 * @Time   2021-06-21
 */
void ClearPrintTotalTimeRecord(void)
{
    uint32_t lTotalTimeInt = 0;

    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_TPT_OFFSET, (uint8_t*)&lTotalTimeInt, EEPROM_ADDR_TPT_SIZE);

    gLcdAutoUI.SetPrintTimeTotal((float)lTotalTimeInt);
    /* refresh on screen */
    gLcdAutoUI.RefreshPrintTimeTotal();
}

/**
 * [CloHeaTmrStart :start the timer to close all heater]
 * @Author Creality
 * @Time   2021-08-11
 * @param  _howLong   [how long does the timer to time]
 */
void CloHeaTmrStart(uint32_t _howLong)
{
    gLcdAutoUI.cloHeaTmr.cltSwi = true;
    gLcdAutoUI.cloHeaTmr.cltStartTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.cloHeaTmr.cltHowLongTime = _howLong;
}

/**
 * [CloHeaTmrStop :stop the timer of closing all heater]
 * @Author Creality
 * @Time   2021-08-11
 */
void CloHeaTmrStop(void)
{
    gLcdAutoUI.cloHeaTmr.cltSwi = false;
    gLcdAutoUI.cloHeaTmr.cltStartTime = 0;
    gLcdAutoUI.cloHeaTmr.cltHowLongTime = 0;
}

/**
 * [CloHeaTmrProcess :process of closing-all-heater-timer]
 * @Author Creality
 * @Time   2021-08-11
 */
void CloHeaTmrProcess(void)
{
    if(gLcdAutoUI.cloHeaTmr.cltSwi)
    {
        if((LcdAutoUIGetTimeMs() - gLcdAutoUI.cloHeaTmr.cltStartTime) > gLcdAutoUI.cloHeaTmr.cltHowLongTime)
        {
            CloHeaTmrStop();

            /*disable hotend heater */
            LcdAutoUISetTempHotend(0);
        }
    }
}

/**
 * [AutoUIPrintStart :start print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIPrintStart(void)
{
    char lCmdBuf[30] = {0};
    // char* lPLetter = NULL;

    /* 1.no file selected; 2.no SD card */
    if((gLcdAutoUI.gcodeFileSelectedCur == VAULE_INVALIDE) || !IS_SD_INSERTED())
        return;

    /* material check */
    if(!IsExistMaterial())
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LACKMATPW);
        return;
    }

    /* clear command in queue */
    queue.clear();
    /* clear current printing progress and time */
    ClearCurPrintProgress();
    ClearCurPrintTime();

    /* change device status to 'DEVSTA_HEATING' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_HEATING);
    /* switch picture */
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);

    /* make command */
    sprintf_P(lCmdBuf, PSTR("M23 %s"), CardRecbuf.Cardfilename[gLcdAutoUI.gcodeFileSelectedCur]);
    // for (lPLetter = &lCmdBuf[4]; *lPLetter; lPLetter++) *lPLetter = tolower(*lPLetter);
    /* excute the command to start print */
    // queue.enqueue_one_now(lCmdBuf);
    queue.enqueue_now_P(lCmdBuf);
    queue.enqueue_now_P(PSTR("M24"));

    /* clear printing-filename and display */
    for(uint32_t i=0; i<GCODE_FILE_BYTELEN; i++)
        gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_FILENAME_PRINTING + i);
    gLcdAutoUI.DisplayText(CardRecbuf.Cardshowfilename[gLcdAutoUI.gcodeFileSelectedCur], TEXTVAR_ADDR_FILENAME_PRINTING);

    /* set device action by mode parameters */
    PrintParasSetByMode();

    /* remove file that record information print-job recovery */
    #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
      card.removeJobRecoveryFile();
    #endif
}

/**
 * [AutoUIStopPrint :stop print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIPrintStop(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
    if(gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)
    {
        gLcdAutoUI.evtStPrWhGoHomeFlag = true;
        return;
    }

    gLcdAutoUI.SetStaGoingHome(GO_HOME_IDLE);
    /* means that device need to stop print and refuse any other operations */
    gLcdAutoUI.eventPriFlags.epfStopPrint = true;

    /* close sd card file */
    #if ENABLED(SDSUPPORT)
      wait_for_heatup = wait_for_user = false;
      card.flag.abort_sd_printing = true;
    #endif
    /* clear command in queue */
    queue.clear();
    /* quick stop all motors' moving */
    quickstop_stepper();
    /* stop print-job timer */
    print_job_timer.stop();
    /* close all heater */
    thermalManager.disable_all_heaters();
    /* close fans */
    thermalManager.zero_fan_speeds();
    /* go home */
    queue.inject_P(PSTR(EVENT_GCODE_SD_ABORT));
    /* remove file that record information print-job recovery */
    #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
      card.removeJobRecoveryFile();
    #endif

    /* clear current printing progress and time */
    ClearCurPrintProgress();
    ClearCurPrintTime();
    /* refresh total printing time */
    gLcdAutoUI.RefreshPrintTimeTotal();
    /* change device status to 'DEVSTA_IDLE' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
    /* reset printing feedrate */
    LcdAutoUISetFeedrate(100);
}

/**
 * [AutoUIPrintStopDelay :wait for finishing going home when need to stop print task, then stop print]
 * @Author Creality
 * @Time   2021-10-29
 */
void AutoUIPrintStopDelay(void)
{
    if(gLcdAutoUI.evtStPrWhGoHomeFlag)
    {
        if(gLcdAutoUI.GetStaGoingHome() == GO_HOME_DONE)
        {
            gLcdAutoUI.evtStPrWhGoHomeFlag = false;
            
            /* means that device need to stop print and refuse any other operations */
            gLcdAutoUI.eventPriFlags.epfStopPrint = true;
            /* close sd card file */
            #if ENABLED(SDSUPPORT)
              wait_for_heatup = wait_for_user = false;
              card.flag.abort_sd_printing = true;
            #endif
            /* clear command in queue */
            queue.clear();
            /* quick stop all motors' moving */
            quickstop_stepper();
            /* stop print-job timer */
            print_job_timer.stop();
            /* close all heater */
            thermalManager.disable_all_heaters();
            /* close fans */
            thermalManager.zero_fan_speeds();
            /* remove file that record information print-job recovery */
            #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
              card.removeJobRecoveryFile();
            #endif
            /* clear current printing progress and time */
            ClearCurPrintProgress();
            ClearCurPrintTime();
            /* refresh total printing time */
            gLcdAutoUI.RefreshPrintTimeTotal();
            /* change device status to 'DEVSTA_IDLE' */
            gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
            /* reset printing feedrate */
            LcdAutoUISetFeedrate(100);
        }
    }
}

/**
 * [AutoUIPrintPause :pause print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIPrintPause(void)
{
    /* change device status to 'DEVSTA_PRINT_PAUSE' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_PRINT_PAUSE);
    
    /* record some device status */
    memset(&gLcdAutoUI.pauseStaRecord, 0, sizeof(gLcdAutoUI.pauseStaRecord));
    gLcdAutoUI.pauseStaRecord.tempHotend = thermalManager.degTargetHotend(0) > EXTRUDE_MINTEMP ?\
                                           thermalManager.degTargetHotend(0) : EXTRUDE_MINTEMP;
    gLcdAutoUI.pauseStaRecord.tempBed = thermalManager.degTargetBed();
    
    /* close all heater */
    #if ENABLE_DOOR_OPEN_CHECK
    if(gLcdAutoUI.enableSwitches.swiDoorPause && IsDoorOpen())
        thermalManager.disable_all_heaters();
    #endif
    
    /* pause SD card print */
    queue.inject_P(PSTR("M25"));
}

/**
 * [AutoUIPrintPause :pause app-print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIAppPrintPause(void)
{
    /* change device status to 'DEVSTA_APP_PRTPAUSE' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTPAUSE);

    /* record some device status */
    memset(&gLcdAutoUI.pauseStaRecord, 0, sizeof(gLcdAutoUI.pauseStaRecord));
    gLcdAutoUI.pauseStaRecord.tempHotend = thermalManager.degTargetHotend(0) > EXTRUDE_MINTEMP ?\
                                           thermalManager.degTargetHotend(0) : EXTRUDE_MINTEMP;
    gLcdAutoUI.pauseStaRecord.tempBed = thermalManager.degTargetBed();
}

/**
 * [AutoUIPrintRecovery :recovery print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIPrintRecovery(void)
{
    if(card.isMounted())
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
        /* change device status to 'DEVSTA_HEATING' */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_HEATING);

        /* recovery temperature that device pause before */
        LcdAutoUISetTempHotendBlock(gLcdAutoUI.pauseStaRecord.tempHotend);
        LcdAutoUISetTempBedBlock(gLcdAutoUI.pauseStaRecord.tempBed);
        
        /* recovery position */
        if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
        {
            current_position = gLcdAutoUI.pauseStaRecord.posPauseBefore;
            LcdAutoUIMoveXYZBlock(current_position.x, current_position.y, current_position.z);
            memset(&gLcdAutoUI.pauseStaRecord, 0, sizeof(gLcdAutoUI.pauseStaRecord));
        }
        /* recovery to print */
        if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
        {
            /* recover print */
            queue.inject_P(PSTR("M24"));

            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
            /* change device status to 'DEVSTA_PRINTING' */
            gLcdAutoUI.AutoUIToStatus(DEVSTA_PRINTING);
        }
    }
}

/**
 * [AutoUIPrintRecovery :recovery app-print]
 * @Author Creality
 * @Time   2021-06-09
 */
void AutoUIAppPrintRecovery(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
    /* change device status to 'DEVSTA_APP_PRTHEAT' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTHEAT);
    
    /* recovery temperature that device pause before */
    LcdAutoUISetTempHotendBlock(gLcdAutoUI.pauseStaRecord.tempHotend);
    LcdAutoUISetTempBedBlock(gLcdAutoUI.pauseStaRecord.tempBed);

    /* recovery position */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT) || (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING))
    {
        current_position.z = gLcdAutoUI.pauseStaRecord.posPauseBefore.z;
        LcdAutoUIMoveXYZBlock(current_position.x, current_position.y, current_position.z);
        memset(&gLcdAutoUI.pauseStaRecord, 0, sizeof(gLcdAutoUI.pauseStaRecord));
    }

    SERIAL_ECHOLN(MSG_OK);
}

/**
 * [AutoUIPowerOffRecoveryCheck :check if device need to excute power-off-recovery]
 * @Author Creality
 * @Time   2021-10-28
 */
void AutoUIPowerOffRecoveryCheck(void)
{
    static bool lTryRecoveryFlag = true; /* true:need to try recovery print, false:no need */
    static uint8_t lCheckCnt = 0;        /* count of trying to recovery */

    if(IsExistPowerOffDurPrint() && lTryRecoveryFlag && IS_SD_INSERTED() && card.isMounted() && (lCheckCnt <= 20))
    {
        /* judge that if there exist file that is printing before power off in the SD card */
        for(uint32_t i=0; i<(uint32_t)GetSDCardGcodeFileNum(); i++)
        {
            if(!strcmp(CardRecbuf.Cardfilename[i], &recovery.info.sd_filename[1]))
            {
                lTryRecoveryFlag = false;

                /* notify to wifi board */
                SERIAL_ECHOLN(CMDSTR_LOCALPRT_POWOFFREC);

                /* popup window, device was power off when printing */
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INTERCONTPW);

                /* clear printing-filename and display */
                for(uint32_t j=0; j<GCODE_FILE_BYTELEN; j++)
                    gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_FILENAME_INTERRUPT + j);
                gLcdAutoUI.DisplayText(CardRecbuf.Cardshowfilename[i], TEXTVAR_ADDR_FILENAME_INTERRUPT);

                break;
            }
        }
        
        lCheckCnt ++;

        if(lCheckCnt >= 20)
            SERIAL_ECHOLN("Warning : exist event that power off during printing but failed to recovery! Can't get the right file name."); 
    }
}

/**
 * [AutoUIPowerOffRecovery :click 'yes' to recovery to print]
 * @Author Creality
 * @Time   2021-06-19
 */
void AutoUIPowerOffRecovery(void)
{
    if(card.isMounted())
    {
        /* clear printing-filename and display */
        for(uint32_t i=0; i<GCODE_FILE_BYTELEN; i++)
            gLcdAutoUI.DisplayData(0, TEXTVAR_ADDR_FILENAME_PRINTING + i);
        /* find the file name that is printing before power off */
        for(uint32_t i=0; i<(uint32_t)GetSDCardGcodeFileNum(); i++)
        {
            if(!strcmp(CardRecbuf.Cardfilename[i], &recovery.info.sd_filename[1]))
            {
                gLcdAutoUI.DisplayText(CardRecbuf.Cardshowfilename[i], TEXTVAR_ADDR_FILENAME_PRINTING);
                break;
            }
        }

        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
        /* change device status to 'DEVSTA_HEATING' */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_HEATING);

        recovery.resume(); 
    }
}

/**
 * [AutoUINoMatRecovery :No material recovery - waiting going home to continue printing when having replaced material]
 * @Author Creality
 * @Time   2021-12-14
 */
void AutoUINoMatRecoveryProcess(void)
{
    if(gLcdAutoUI.evtRepMatToConPriFlag && (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DONE))
    {
        gLcdAutoUI.evtRepMatToConPriFlag = false;

        if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT)
            gLcdAutoUI.evtWaitEAxisRecovery = true;
        else
            AutoUIPrintRecovery();
    }
}

/**
 * [AutoUINoMatAppPrintProcess :time to notify WIFI board that there is no material]
 * @Author Creality
 * @Time   2022-01-06
 */
void AutoUINoMatAppPrintProcess(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 2000)
    {
        lMs = LcdAutoUIGetTimeMs();

        #if ENABLE_MATERIAL_CHECK
            if(gLcdAutoUI.evtNoMatWhenAppPrintFlag)
                SERIAL_ECHOLN(MSG_NO_MATERIAL);
        #endif
    }
}

/**
 * [AutoUIDoorOpenAppPrintProcess :time to notify WIFI board that door is open]
 * @Author Creality
 * @Time   2022-01-06
 */
void AutoUIDoorOpenAppPrintProcess(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 2000)
    {
        lMs = LcdAutoUIGetTimeMs();

        #if ENABLE_DOOR_OPEN_CHECK
            if(gLcdAutoUI.evtDoorOpenWhenAppPrintFlag)
                SERIAL_ECHOLN(MSG_DOOR_OPEN);
        #endif
    }
}

/**
 * [AutoUIPrintFinished :if print finished or not, and what the device need to do if finished]
 * @Author Creality
 * @Time   2021-06-15
 */
void AutoUIPrintFinished(void)
{
    if(gLcdAutoUI.GetPrintFinishFlag() && !planner.has_blocks_queued() && (gLcdAutoUI.percentDoneRecord >= 99))
    {
        gLcdAutoUI.percentDoneRecord = 0;
        gLcdAutoUI.SetPrintFinishFlag(false);

        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTFINISHED);
        gLcdAutoUI.SwitchIcon(100, ICONVAR_ADDR_PRINT_PROGRESS);
        gLcdAutoUI.DisplayData(100, DATAVAR_ADDR_PRINT_PROGRESS);
        /* clear command in queue */
        queue.clear();
        /* stop print-job timer */
        print_job_timer.stop();
        /* close all heater */
        wait_for_heatup = false;
        thermalManager.disable_all_heaters();
        /* close fans */
        thermalManager.zero_fan_speeds();
        /* Z axis go to the max position */
        LcdAutoUIMoveZ(Z_MAX_POS, AXIS_MOVE_ABSO);
        /* XY axis go to the special position */
        LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
        LcdAutoUIMoveY(Y_BED_SIZE, AXIS_MOVE_ABSO);
        /* remove file that record information print-job recovery */
        #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
          card.removeJobRecoveryFile();
        #endif
  
        /* refresh total printing time */
        gLcdAutoUI.RefreshPrintTimeTotal();
        /* change device status to 'DEVSTA_IDLE' */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
        /* reset printing feedrate */
        LcdAutoUISetFeedrate(100);
        /* notify wifi board that local-print has finished */
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_FINISH);
    }
}

/**
 * [SDCardLostAtPrintingProcess :process of handling event that SD card lost at printing]
 * @Author Creality
 * @Time   2021-10-18
 */
void SDCardLostAtPrintingProcess(void)
{
    static bool lSDCardInsertFlagRecord = false;        /* false: no sd card(record), true: exist sd card(record) */
    static bool lSDCardLostAtPrintingEvent = false;     /* false: no event, true: exist event that SD card lost at printing */

    if(!IsSDCardStatusStable()) return;

    /* SD card is inserted but do not know that if SD card is mounted or not */
    if(!lSDCardInsertFlagRecord && IS_SD_INSERTED())
    {
        lSDCardInsertFlagRecord = IS_SD_INSERTED();

        if(gLcdAutoUI.removeSDCardAtPrintingFlag)
        {
            lSDCardLostAtPrintingEvent = true;
        }
    }
    /* SD card is removed */
    else if(lSDCardInsertFlagRecord && !IS_SD_INSERTED())
    {
        lSDCardInsertFlagRecord = IS_SD_INSERTED();

        if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING)
        {
            /* pause SD card print */
            queue.inject_P(PSTR("M25"));
            gLcdAutoUI.removeSDCardAtPrintingFlag = true;
        }
        else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
        {
            /* clear command in queue */
            queue.clear();
            /* stop printing */
            AutoUIPrintStop();
        }
    }

    /* handle event that SD card lost at printing */
    if(lSDCardLostAtPrintingEvent && IS_SD_INSERTED())
    {
        card.mount();
        if(card.isMounted())
        {
            rtscheck.RTS_SDCardInit();

            for(uint32_t i=0; i<(uint32_t)GetSDCardGcodeFileNum(); i++)
            {
                if(!strcmp(CardRecbuf.Cardfilename[i], &recovery.info.sd_filename[1]))
                {
                    /* recovery print */
                    queue.inject_P(PSTR("M24"));
                    /* clear flags */
                    lSDCardLostAtPrintingEvent = false;
                    gLcdAutoUI.removeSDCardAtPrintingFlag = false;

                    break;
                }
            }
        }
    }
}

/**
 * [MaterialCheckProcess :process of checking material status and response to status]
 * @Author Creality
 * @Time   2021-06-17
 */
void MaterialCheckProcess(void)
{
    static uint8_t lNoMatCnt = 0;

    /* Prevent misjudgment */
    if(IsExistMaterial())
    {
        lNoMatCnt = 0;
        return;
    }
    else
    {
        lNoMatCnt ++;
    }

    /* app printing: 1.lack of material; */
    if(!IsExistMaterial() && (lNoMatCnt >= 2))
    {
        lNoMatCnt = 0;

        /* current status: app printing */
        if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
        {
            /* record that there is no materials when app printing */
            gLcdAutoUI.evtNoMatWhenAppPrintFlag = true;
            gLcdAutoUI.matRunoutAtPrintingFlag = true;

            AutoUIAppPrintPause();

            return;
        }
        /* local printing: 1.lack of material; 2.is printing; */
        else if(card.isPrinting())
        {
            /* current status: local printing */
            AutoUIPrintPause();
            gLcdAutoUI.matRunoutAtPrintingFlag = true;
        }
    } 
}

/**
 * [DoorOpenPauseProcess :process of checking door-open status and response to status]
 * @Author Creality
 * @Time   2021-06-19
 */
#if ENABLE_DOOR_OPEN_CHECK
void DoorOpenPauseProcess(void)
{
    static uint8_t lDoorOpenCnt = 0;

    /* Prevent misjudgment */
    if(!IsDoorOpen())
    {
        lDoorOpenCnt = 0;
        return;
    }
    else
    {
        lDoorOpenCnt ++;
    }

    /* 1.door is open; 2.is printing; */
    if(IsDoorOpen() && (card.isPrinting() || gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) && (lDoorOpenCnt >= 2))
    {
        lDoorOpenCnt = 0;

        /* current status: app printing */
        if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
        {
            gLcdAutoUI.evtDoorOpenWhenAppPrintFlag = true;

            AutoUIAppPrintPause();
        }
        /* current status: local printing */
        else
        {
            AutoUIPrintPause();
        }
    }
}
#endif

/**
 * [ErrorCheckHeating :heating error check]
 * @Author Creality
 * @Time   2021-07-06
 */
void ErrorCheckHeating(void)
{
    static uint32_t lHotendTempErrorCnt = 0;
    static uint32_t lBedTempErrorCnt = 0;

    if(LcdAutoUIGetTempHotend() < 0)
        lHotendTempErrorCnt ++;
    else
        lHotendTempErrorCnt = 0;

    if(LcdAutoUIGetTempBed() < 0)
        lBedTempErrorCnt ++;
    else
        lBedTempErrorCnt = 0;

    if((lHotendTempErrorCnt > 3) || (lBedTempErrorCnt > 3))
    {
        lHotendTempErrorCnt = 0;
        lBedTempErrorCnt = 0;

        /* close all heater */
        thermalManager.disable_all_heaters();
        /* switch picture to give warning */
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_ERRORTIPS);
    }
}

/**
 * [ErrorsCheck :process of checking device errors]
 * @Author Creality
 * @Time   2021-07-06
 */
void ErrorsCheckProcess(void)
{
    /* hotend/bed heating error check */
    ErrorCheckHeating();
}

/***********************************************************************************************************************************
                                              communication with wifi board functions
***********************************************************************************************************************************/
/**
 * [CWWBIndLedSet :set wifi indicator led]
 * @Author Creality
 * @Time   2021-06-23
 * @param  _en        [true:turn on, false:turn off]
 */
void CWWBIndLedSet(bool _en)
{
    queue.inject_P(PSTR("M115"));
}

/**
 * [AppWaitForPreheating :App print - waiting for preheating]
 * @Author Creality
 * @Time   2021-11-18
 */
void AppWaitForPreheating(void)
{
    if(gLcdAutoUI.AutoUIGetStatus() != DEVSTA_APP_PRTHEAT) return;

    if((gLcdAutoUI.curPicID == AUTOUI_HEATING) && gLcdAutoUI.appPrint.flags.flgPreheatWaiting && \
       (LcdAutoUIGetTarTempHotend() != 0))
    {
        if((LcdAutoUIGetTempBed() >= (LcdAutoUIGetTarTempBed() - 1.0)) && \
           (LcdAutoUIGetTempHotend() >= (LcdAutoUIGetTarTempHotend() - 1.0)))
           
        {
            gLcdAutoUI.appPrint.flags.flgPreheatWaiting = false;

            /* change device status to "DEVSTA_APP_PRINTING" */
            gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRINTING);
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
        }
    }
}

/***********************************************************************************************************************************
                                                      touch-response functions
***********************************************************************************************************************************/
/**
 * [TouVarConfirmFuncPic001 :confirm touch response function on picture 001]
 * @Author Creality
 * @Time   2021-06-07
 */
static void TouVarConfirmFuncPic001(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BOOTGUIDE_02);
    /* enable Dynamic icon */
    gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_DYNAHOMING);
    /* make sure base-point */
    queue.enqueue_now_P(PSTR("G28"));
}

static void TouVarConfirmFuncPic004(void)
{
    /* do nothing */
}

static void TouVarConfirmFuncPic005(void)
{
    if (gLcdAutoUI.matRunoutAtPrintingFlag == true)
        gLcdAutoUI.flowStepCnt.fscInOutMat = 1;
    else
        /* change device status to 'DEVSTA_IN_OUT_MAT' */
        gLcdAutoUI.AutoUIToStatus(DEVSTA_IN_OUT_MAT);
    
    SERIAL_ECHOLNPAIR("\r\n mat runout ,and start remove mat ,flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
    
    /* select in-material */
    gLcdAutoUI.inOutMatFlag = true;

    gLcdAutoUI.SetStaDynamicIcon(false, ICONVAR_ADDR_HEATING_BED);
}

static void TouVarConfirmFuncPic006(void)
{
    /* feed material finished */
    gLcdAutoUI.flowStepCnt.fscInOutMat = 3;
    gLcdAutoUI.feedFinishedFlag = true;
    /* switch picture */
    if (gLcdAutoUI.matRunoutAtPrintingFlag == true)
    {
        if (IsExistMaterial()) 
        {
            gLcdAutoUI.matRunoutAtPrintingFlag = false;
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LOADMATPW);
            RefreshOutMatReturnKey(true);
        }
        else 
        {
            // gLcdAutoUI.flowStepCnt.fscInOutMat = 4;
            // gLcdAutoUI.feedFinishedFlag = false;
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LACKMATPW);
        }
        
        //gLcdAutoUI.AutoUIToStatus(DEVSTA_PRINT_PAUSE);
        // SERIAL_ECHOLNPAIR("\r\n ok!!! continue!!! run!!!, flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
        return;
    } 

    if(gLcdAutoUI.devStatusLastRecord == DEVSTA_STARTUP_FIR)
    {
        /* startup for first time */
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BOOTGUIDE_07);
    }
    else if((gLcdAutoUI.devStatusLastRecord == DEVSTA_PRINT_PAUSE) || (gLcdAutoUI.devStatusLastRecord == DEVSTA_APP_PRTPAUSE))
    {
        if(IsExistMaterial())
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LOADMATPW);
        else
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LACKMATPW);
    }
    else
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INOUTMAT);
    }
}

static void TouVarPrintFileFuncPic008(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_01);
}

static void TouVarPrintModeFuncPic008(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);
}

static void TouVarPrintSetFuncPic008(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSETTING);
}

static void TouVarDevInfoFuncPic008(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_DEVINFO);
}

static void TouVarHotendTempFuncPic008(void)
{
    int16_t lHotendTempToBeSet = rtscheck.recdat.data[0];

    /* when device is at 'DEVSTA_PRINTING' or 'DEVSTA_HEATING' */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING) || (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING))
    {
        lHotendTempToBeSet = lHotendTempToBeSet < EXTRUDE_MINTEMP ? \
                                    EXTRUDE_MINTEMP : lHotendTempToBeSet;
    }

    LcdAutoUISetTempHotend(lHotendTempToBeSet);
}

static void TouVarBedTempFuncPic008(void)
{
    LcdAutoUISetTempBed(rtscheck.recdat.data[0]);
}

static void TouVarFeedrateFunc(void)
{
    LcdAutoUISetFeedrate(rtscheck.recdat.data[0]);
}

/**
 * [TouVarReturn :response to "return" touch-variable]
 * @Author Creality
 * @Time   2021-06-06
 */
static void TouVarReturn(void)
{
    switch(rtscheck.recdat.data[0])
    {
        /* picture 009,010,011,012,013 */
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
            break;

        case 0x05:
            if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
            else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
            else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
            else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
            break;

        case 0x06:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
            break;

        case 0x07:
            if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINT_PAUSE)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
            else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE)
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
            else
                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);
            break;

        case 0x08:
            if (gLcdAutoUI.matRunoutAtPrintingFlag == true)
            {
                SERIAL_ECHOLNPAIR("\r\n cancel in mat when heating, flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
                break;
            }
            /* stop feed(out) material */
            gLcdAutoUI.flowStepCnt.fscInOutMat = 3;
            gLcdAutoUI.feedFinishedFlag = true;
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INOUTMAT);
            break;

        case 0x09:
            break;

        case 0x0A:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);
            break;

        case 0x0B:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
            break;

        case 0x0C:
            gLcdAutoUI.bedCaliZOffset = 0;
            LcdAutoUISetBedLeveling(true);
            gLcdAutoUI.AutoUIToStatus(DEVSTA_IDLE);
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSETTING);
            gLcdAutoUI.SetFlowStep(&gLcdAutoUI.flowStepCnt.fscBedCali, 0);
            break;

        case 0x0D:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSETTING);
            break;

        case 0x0E:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
            break;

        case 0x0F:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_DEVINFO);
            break;

        default:
            break;
    }
}

static void TouVarFileSelect(void)
{
    /* rtscheck.recdat.data[0] : mean index of file */
    gLcdAutoUI.gcodeFileSelectedCur = FileSelectBackground(rtscheck.recdat.data[0], ICONVAR_ADDR_FILESELECT_BG00);
}

static void TouVarFileNextPage(void)
{
    switch(rtscheck.recdat.data[0])
    {
        case 0:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_02);
            break;

        case 1:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_03);
            break;

        case 2:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_04);
            break;

        case 3:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_05);
            break;

        default:
            break;
    }
}

static void TouVarFileLastPage(void)
{
    switch(rtscheck.recdat.data[0])
    {
        case 0:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_01);
            break;

        case 1:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_02);
            break;

        case 2:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_03);
            break;
        
        case 3:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FILESELECT_04);
            break;

        default:
            break;
    }
}

static void TouVarPrintSetPic014(void)
{
    if((gLcdAutoUI.AutoUIGetStatus() != DEVSTA_PRINT_PAUSE) && (gLcdAutoUI.AutoUIGetStatus() != DEVSTA_APP_PRTPAUSE))
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTADJUST);
}

static void TouVarPrintPausePic014(void)
{
    /* device is at APP-Print */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)  || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE))
    {
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_PAUSE);
        return;
    }

    if(!gLcdAutoUI.removeSDCardAtPrintingFlag)
    {
        if(gLcdAutoUI.AutoUIGetStatus() != DEVSTA_PRINT_PAUSE)
        {
            SERIAL_ECHOLN(CMDSTR_LOCALPRT_PAUSE);
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
            AutoUIPrintPause();
        }
    } 
}

static void TouVarPrintStopPic014(void)
{
    if(gLcdAutoUI.AutoUIGetStatus() != DEVSTA_PRINT_PAUSE)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSTOPPW);
}

static void TouVarPrintFinishPic015(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);

    /* clear current printing progress and time */
    ClearCurPrintProgress();
    ClearCurPrintTime();
}

static void TouVarBoxFanPic016(void)
{
    if(gLcdAutoUI.enableSwitches.swiBoxFan)
    {
        gLcdAutoUI.enableSwitches.swiBoxFan = false;

        SetBoxFan(false);
        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_FAN);
    }
    else
    {
        gLcdAutoUI.enableSwitches.swiBoxFan = true;

        SetBoxFan(true);
        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_FAN);
    }
}

static void TouVarBoxLedPic016(void)
{
    if(gLcdAutoUI.enableSwitches.swiBoxLed)
    {
        gLcdAutoUI.enableSwitches.swiBoxLed = false;

        SetBoxLed(false);

        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_LED);
    }
    else
    {
        gLcdAutoUI.enableSwitches.swiBoxLed = true;

        SetBoxLed(true);

        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_LED);
    }
}

static void TouVarWifiLedPic016(void)
{
    if(gLcdAutoUI.enableSwitches.swiWifiLed)
    {
        gLcdAutoUI.enableSwitches.swiWifiLed = false;

        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_WIFI_LED);  
    }
    else
    {
        gLcdAutoUI.enableSwitches.swiWifiLed = true;

        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_WIFI_LED);
    }

    CWWBIndLedSet(gLcdAutoUI.enableSwitches.swiWifiLed);
}

static void TouVarDoorPausePic016(void)
{
    if(gLcdAutoUI.enableSwitches.swiDoorPause)
    {
        gLcdAutoUI.enableSwitches.swiDoorPause = false;

        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_DOOR_OPEN_PAUSE);
    }
    else
    {
        gLcdAutoUI.enableSwitches.swiDoorPause = true;

        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_DOOR_OPEN_PAUSE);
    }
}

static void TouVarPrintStopConfirmPic017(void)
{
    /* device is at APP-Print */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)  || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE))
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
        queue.enqueue_one_now(PSTR(CMDSTR_LOCALPRT_STOP));
        return;
    }

    SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
    /* clear flag that removing SD card at printing */
    gLcdAutoUI.removeSDCardAtPrintingFlag = false;
    /* stop print */
    AutoUIPrintStop();
}

static void TouVarPrintStopCancelPic017(void)
{
    /* device is at APP-Print */
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
    /* device is at Local-Print */
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_HEATING)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINTING)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTING);
    else if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_PRINT_PAUSE)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
}

static void TouVarPrintPauseRecoPic018(void)
{   
    /* device is at APP-Print */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)  || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE))
    {
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_RESUME);
        return;
    }

    /* notify to wifi board */
    SERIAL_ECHOLN(CMDSTR_LOCALPRT_RESUME);
    /* recovery to print */
    AutoUIPrintRecovery();
}

static void TouVarPrintStopPic018(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSTOPPW);
}

static void TouVarPrintInOuMatPic018(void)
{
    /* current status: app printing */
    if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
    {
        /* Temporary block */
        // return;
    }

    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INOUTMAT);
}

static void TouVarConfirmPrintPic019(void)
{
    #if ENABLE_DOOR_OPEN_CHECK
        if(!IsDoorOpen())
        {
            /* current status: app printing */
            if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
            {
                gLcdAutoUI.evtDoorOpenWhenAppPrintFlag = false;

                AutoUIAppPrintRecovery();
            }
            /* current status: local printing */
            else
            {
                AutoUIPrintRecovery();
            }
        }
    #else
        return;
    #endif
}

static void TouVarCancelPrintPic019(void)
{
    /* current status: app printing */
    if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
    {
        gLcdAutoUI.evtDoorOpenWhenAppPrintFlag = false;

        SERIAL_ECHOLN(MSG_OK);

        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
        queue.enqueue_one_now(PSTR(CMDSTR_LOCALPRT_STOP));
    }
    /* current status: local printing */
    else
    {
        AutoUIPrintStop();
    }
}

static void TouVarPLASelectPic020(void)
{
    /* current print mode is 'PRINT_MODE_INVAILD' or 'PRINT_MODE_ABS' */
    if(gLcdAutoUI.printModeType == PRINT_MODE_INVAILD || gLcdAutoUI.printModeType == PRINT_MODE_ABS)
    {
        gLcdAutoUI.printModeType = PRINT_MODE_PLA;

        gLcdAutoUI.SwitchIcon(SELECTED_YES, ICONVAR_ADDR_PLA_SELECTED);
        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_ABS_SELECTED);
    }
    /* current print mode is 'PRINT_MODE_PLA' */
    else if(gLcdAutoUI.printModeType == PRINT_MODE_PLA)
    {
        gLcdAutoUI.printModeType = PRINT_MODE_INVAILD;

        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_PLA_SELECTED);
        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_ABS_SELECTED);
    }
}

static void TouVarABSSelectPic020(void)
{
    /* current print mode is 'PRINT_MODE_INVAILD' or 'PRINT_MODE_PLA' */
    if(gLcdAutoUI.printModeType == PRINT_MODE_INVAILD || gLcdAutoUI.printModeType == PRINT_MODE_PLA)
    {
        gLcdAutoUI.printModeType = PRINT_MODE_ABS;

        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_PLA_SELECTED);
        gLcdAutoUI.SwitchIcon(SELECTED_YES, ICONVAR_ADDR_ABS_SELECTED);
    }
    /* current print mode is 'PRINT_MODE_ABS' */
    else if(gLcdAutoUI.printModeType == PRINT_MODE_ABS)
    {
        gLcdAutoUI.printModeType = PRINT_MODE_INVAILD;

        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_PLA_SELECTED);
        gLcdAutoUI.SwitchIcon(SELECTED_NO, ICONVAR_ADDR_ABS_SELECTED);
    }
}

#if 0
static void TouVarUserDefPic020(void)
{
    ReadUserDefModeParas();
    
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MODEUSERDEF);
}
#endif

static void TouVarDisableHeatPic020(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_CLOSEHEATPW);
}

static void TouVarInOutMatPic020(void)
{
    gLcdAutoUI.SwitchIcon(VAULE_INVALIDE, ICONVAR_ADDR_INOUTMAT_TIP);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_INOUTMAT);
}

static void TouVarInMatPic021()
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BOOTGUIDE_05);
    gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_IN_MAT);  
}

static void TouVarOutMatPic021()
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OUTMAT_01);
    /* change device status to 'DEVSTA_IN_OUT_MAT' */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_IN_OUT_MAT);
    /* select out-material */
    gLcdAutoUI.inOutMatFlag = false;
}

static void TouVarOutMatConfirmPic023(void)
{
    if (gLcdAutoUI.matRunoutAtPrintingFlag == true)
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BOOTGUIDE_05);
        gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_IN_MAT);
        SERIAL_ECHOLNPAIR("\r\n continue!!!! start to in mat, flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
    } 
    else
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
        /* feed(out) material finished */
        gLcdAutoUI.feedFinishedFlag = true;
    }
}

static void TouVarSetHotendFanPic024(void)
{
    if(userDefMode.pmpHotendFanFlag)
    {
        userDefMode.pmpHotendFanFlag = false;

        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_HOTEND_FAN_STA);
    }
    else
    {
        userDefMode.pmpHotendFanFlag = true;

        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_HOTEND_FAN_STA);
    }
}

static void TouVarSetBoxFanPic024(void)
{
    if(userDefMode.pmpBoxFanFlag)
    {
        userDefMode.pmpBoxFanFlag = false;

        gLcdAutoUI.SwitchIcon(AUTOUI_DISABLE, ICONVAR_ADDR_BOX_FAN_STA);
    }
    else
    {
        userDefMode.pmpBoxFanFlag = true;

        gLcdAutoUI.SwitchIcon(AUTOUI_ENABLE, ICONVAR_ADDR_BOX_FAN_STA);
    }
}

static void TouVarBedCaliPic025(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BED_CALI_PW);
}

static void TouVarAxisMovePic025(void)
{
    /* refresh current position that display on screen */
    RefreshCurPosDisplay();
    /* refresh move-axis-picture position */
    memcpy(&gLcdAutoUI.movAxisPicPos, &current_position, sizeof(current_position));

    gLcdAutoUI.SetAxisStepUnit(STEP_UNIT_10MM);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_AXISCONTR_10);
}

static void TouVarStepUnitMM1(void)
{
    gLcdAutoUI.SetAxisStepUnit(STEP_UNIT_1MM);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_AXISCONTR_1);
}

static void TouVarStepUnitMM01(void)
{
    gLcdAutoUI.SetAxisStepUnit(STEP_UNIT_01MM);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_AXISCONTR_01);
}

static void TouVarXBackward(void)
{
    int lPosX = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.x = ((gLcdAutoUI.movAxisPicPos.x * STEP_UNIT_TIMES) - gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* X axis lower limit */
    gLcdAutoUI.movAxisPicPos.x = gLcdAutoUI.movAxisPicPos.x < 0 ? 0 : gLcdAutoUI.movAxisPicPos.x;

    lPosX = gLcdAutoUI.movAxisPicPos.x * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosX, DATAVAR_ADDR_CURPOS_X);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarXForward(void)
{
    int lPosX = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.x = ((gLcdAutoUI.movAxisPicPos.x * STEP_UNIT_TIMES) + gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* X axis upper limit */
    gLcdAutoUI.movAxisPicPos.x = gLcdAutoUI.movAxisPicPos.x > X_BED_SIZE ? X_BED_SIZE : gLcdAutoUI.movAxisPicPos.x;

    lPosX = gLcdAutoUI.movAxisPicPos.x * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosX, DATAVAR_ADDR_CURPOS_X);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarYForward(void)
{
    int lPosY = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.y = ((gLcdAutoUI.movAxisPicPos.y * STEP_UNIT_TIMES) + gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* Y axis upper limit */
    gLcdAutoUI.movAxisPicPos.y = gLcdAutoUI.movAxisPicPos.y > Y_BED_SIZE ? Y_BED_SIZE : gLcdAutoUI.movAxisPicPos.y;

    lPosY = gLcdAutoUI.movAxisPicPos.y * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosY, DATAVAR_ADDR_CURPOS_Y);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarYBackward(void)
{
    int lPosY = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.y = ((gLcdAutoUI.movAxisPicPos.y * STEP_UNIT_TIMES) - gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* Y axis lower limit */
    gLcdAutoUI.movAxisPicPos.y = gLcdAutoUI.movAxisPicPos.y < 0 ? 0 : gLcdAutoUI.movAxisPicPos.y;

    lPosY = gLcdAutoUI.movAxisPicPos.y * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosY, DATAVAR_ADDR_CURPOS_Y);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarZUp(void)
{
    int lPosZ = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.z = ((gLcdAutoUI.movAxisPicPos.z * STEP_UNIT_TIMES) - gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* Z axis lower limit */
    gLcdAutoUI.movAxisPicPos.z = gLcdAutoUI.movAxisPicPos.z < 0 ? 0 : gLcdAutoUI.movAxisPicPos.z;

    lPosZ = gLcdAutoUI.movAxisPicPos.z * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosZ, DATAVAR_ADDR_CURPOS_Z);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarZDown(void)
{
    int lPosZ = 0;

    if(!gLcdAutoUI.GetHaGoHoBeFlag() || (gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING)) return;

    gLcdAutoUI.movAxisPicPos.z = ((gLcdAutoUI.movAxisPicPos.z * STEP_UNIT_TIMES) + gLcdAutoUI.GetAxisStepUnit()) / STEP_UNIT_TIMES;

    /* Z axis upper limit */
    gLcdAutoUI.movAxisPicPos.z = gLcdAutoUI.movAxisPicPos.z > Z_MAX_POS ? Z_MAX_POS : gLcdAutoUI.movAxisPicPos.z;

    lPosZ = gLcdAutoUI.movAxisPicPos.z * DWIN_REALVAL_TIMES;
    gLcdAutoUI.DisplayData(lPosZ, DATAVAR_ADDR_CURPOS_Z);

    gLcdAutoUI.movAxisPicPosCurTime = LcdAutoUIGetTimeMs();
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarGoHome(void)
{
    if(gLcdAutoUI.GetStaGoingHome() == GO_HOME_DOING) return;

    /* make sure base-point */
    queue.enqueue_now_P(PSTR("G28"));
    /* refresh current postion display */
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_CURPOS_X);
    gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_CURPOS_Y);
    gLcdAutoUI.DisplayData((Z_MAX_POS - gLcdAutoUI.bedCaliZOffsetCenter) * DWIN_REALVAL_TIMES, DATAVAR_ADDR_CURPOS_Z);
    /* refresh move-axis-picture position */
    current_position.x = 0;
    current_position.y = 0;
    current_position.z = Z_MAX_POS - gLcdAutoUI.bedCaliZOffsetCenter;
    gLcdAutoUI.movAxisPicPos.x = 0;
    gLcdAutoUI.movAxisPicPos.y = 0;
    gLcdAutoUI.movAxisPicPos.z = Z_MAX_POS - gLcdAutoUI.bedCaliZOffsetCenter;
}

static void TouVarStepUnitMM10(void)
{
    gLcdAutoUI.SetAxisStepUnit(STEP_UNIT_10MM);
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_AXISCONTR_10);
}

static void TouVarLangSelectPic031(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LANGSELECT);
}

static void TouVarDevResetPic031(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_FACTORRESETPW);
}

#if 1
static void TouVarLangTypePic032(void)
{
    gLcdAutoUI.SetSaveCurLang(LanguageSelect(rtscheck.recdat.data[0]));
    // gLcdAutoUI.SetSaveCurLang((AutoUILanguage_t)rtscheck.recdat.data[0]);
}
#endif

static void TouVarConfirmResetPic033(void)
{
    /* factory reset  */
    /* get to the status that device has never used */
    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_STTF_OFFSET, 0, EEPROM_ADDR_STTF_SIZE);
    /* default language: Chinese */
    // gLcdAutoUI.SetSaveCurLang(AUTOUI_LANGUAGE_CH);
    /* clear printing-total-time */
    ClearPrintTotalTimeRecord();
    /* clear switches status */
    ResetSwitchesStatus();
    /* clear bed calibration Z axis offset at center point and save in EEPROM */
    // ClearBedCaliZOffsetCenter();
    /* reset all parameters */
    settings.reset();
    settings.save();

    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_DEVINFO);
}

static void TouVarCancelResetPic033(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_DEVINFO);
}

static void TouVarContinuePrintPic036(void)
{
    /* recovery local-print */
    if(recovery.info.recovery_flag)
    {
        AutoUIPowerOffRecovery();
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_RESUME);
    }
    /* recovery app-print */
    else
    {
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_RESUME);
    }
    
}

static void TouVarCancelPrintPic036(void)
{
    /* stop local-print */
    if(recovery.info.recovery_flag)
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        /* remove file that record information print-job recovery */
        #if ENABLED(SDSUPPORT) && ENABLED(POWER_LOSS_RECOVERY)
          card.removeJobRecoveryFile();
        #endif

        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
    }
    /* stop app-print */
    else
    {
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
    }
}

static void TouVarBedCaliUpPic027(void)
{
    /* move Z Axis and refresh current Z coordinate */
    LcdAutoUIMoveZ((-1)*BED_CALI_STEPUNIT, AXIS_MOVE_RELA);
    gLcdAutoUI.bedCaliZOffset -= BED_CALI_STEPUNIT;

    if(gLcdAutoUI.bedCaliZOffset < 0)
        gLcdAutoUI.DisplayData((int)((1000*gLcdAutoUI.bedCaliZOffset - 1)/10), DATAVAR_ADDR_ZOFFSET);
    else
        gLcdAutoUI.DisplayData((int)((1000*gLcdAutoUI.bedCaliZOffset + 1)/10), DATAVAR_ADDR_ZOFFSET);
    gLcdAutoUI.DisplayData((int)(100*((-1)*gLcdAutoUI.bedCaliZOffset + Z_MAX_POS)), DATAVAR_ADDR_ZHEIGHT);
}

static void TouVarBedCaliDownPic027(void)
{
    /* move Z Axis and refresh current Z coordinate */
    LcdAutoUIMoveZ(BED_CALI_STEPUNIT, AXIS_MOVE_RELA);
    gLcdAutoUI.bedCaliZOffset += BED_CALI_STEPUNIT;
    
    if(gLcdAutoUI.bedCaliZOffset < 0)
        gLcdAutoUI.DisplayData((int)((1000*gLcdAutoUI.bedCaliZOffset - 1)/10), DATAVAR_ADDR_ZOFFSET);
    else
        gLcdAutoUI.DisplayData((int)((1000*gLcdAutoUI.bedCaliZOffset + 1)/10), DATAVAR_ADDR_ZOFFSET);
    gLcdAutoUI.DisplayData((int)(100*((-1)*gLcdAutoUI.bedCaliZOffset + Z_MAX_POS)), DATAVAR_ADDR_ZHEIGHT);
}

static void TouVarBedCaliSavePic027(void)
{
    /* Central point Z offset, display the offset on screen */
    if((gLcdAutoUI.pointCntCaliCur - 1) == 0)
    {
        /* reset the value of bed calibration */
        mbl.reset();
        settings.reset();
        /* bed calibration Z axis offset at center point, save in EEPROM */
        gLcdAutoUI.bedCaliZOffsetCenter = gLcdAutoUI.bedCaliZOffset;
        SaveBedCaliZOffsetCenter();

        gLcdAutoUI.DisplayData((int)(100*gLcdAutoUI.bedCaliZOffsetCenter), DATAVAR_ADDR_CENTRAL_Z_OFFSET);

        /* set current position Z axis 0 */
        current_position.z = 0;
        queue.enqueue_one_now(PSTR("G92 Z0.0"));
    }
    /* go to next point */
    queue.inject_P(PSTR("G29 S2"));

    /* clear bed calibration Z offset */
    gLcdAutoUI.bedCaliZOffset = 0;

    if(gLcdAutoUI.pointCntCaliCur >= BED_CALI_POINT_NUM)
    {
        /* bed calibration finished */
        gLcdAutoUI.bedCaliFinishedFlag = true;
    } 
    else
    {
        gLcdAutoUI.DisplayData(0, DATAVAR_ADDR_ZOFFSET);
        gLcdAutoUI.DisplayData((int)(100 * Z_MAX_POS), DATAVAR_ADDR_ZHEIGHT);
        /* next point(display by dynamic icon) */
        gLcdAutoUI.SwitchIcon(gLcdAutoUI.pointCntCaliCur++, ICONVAR_ADDR_BEDCALI_CURPOINT);
    }
}

static void TouVarStartPrint(void)
{
    /* device is at APP printing, refuse local-print */
    if((gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRINTING) || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTHEAT)  || \
       (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_APP_PRTPAUSE)) return;
    /* start local print, need to notify wifi board */
    else SERIAL_ECHOLN(CMDSTR_LOCALPRT_START);
    
    #if 1
    AutoUIPrintStart();
    #endif
}

static void TouVarConfirmLoadMatPic039(void)
{
    if (gLcdAutoUI.matRunoutAtPrintingFlag == true)
    {
        RefreshOutMatReturnKey(false);
        TouVarOutMatPic021();
        SERIAL_ECHOLNPAIR("\r\n mat runout ,and start remove mat ,flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
    }
    else 
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_BOOTGUIDE_05);
        gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_IN_MAT);
    }
    CloHeaTmrStop();
}

static void TouVarStopPrintPic039(void)
{
    /* current status: app printing */
    if(((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM)) || \
       ((gLcdAutoUI.devStatusLastRecord >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.devStatusLastRecord < DEVSTA_NUM) && (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT)))
    {
        gLcdAutoUI.evtNoMatWhenAppPrintFlag = false;

        SERIAL_ECHOLN(MSG_OK);

        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
        queue.enqueue_one_now(PSTR(CMDSTR_LOCALPRT_STOP));

        /* close all heater */
        thermalManager.disable_all_heaters();

        return;
    }

    /* notify to WIFI board */
    SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);

    if (gLcdAutoUI.matRunoutAtPrintingFlag == true) // ??????Ϸ??
    {
        gLcdAutoUI.matRunoutAtPrintingFlag = false;
        SERIAL_ECHOLNPAIR("\r\n cancel mat runout deal,flag = ", gLcdAutoUI.matRunoutAtPrintingFlag);
    }
    if(gLcdAutoUI.AutoUIGetStatus() != DEVSTA_IDLE)
    {
        /* stop print */
        AutoUIPrintStop();
    }
    else
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
    }  
}

static void TouVarContinuePrintPic040(void)
{
    /* current status: app printing */
    if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
    {
        /* record that materials have replaced */
        gLcdAutoUI.evtNoMatWhenAppPrintFlag = false;

        AutoUIAppPrintRecovery();

        return;
    }
    else if((gLcdAutoUI.devStatusLastRecord >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.devStatusLastRecord < DEVSTA_NUM) && (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT))
    {
        /* record that materials have replaced */
        gLcdAutoUI.evtNoMatWhenAppPrintFlag = false;

        /* waiting for In/Out material finish */
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
        gLcdAutoUI.evtWaitEAxisRecovery = true;

        return;
    }

    /* local printing */
    if(((LcdAutoUIGetTimeMs() - gLcdAutoUI.evtRepMatToConPriTmr) > (1000 * DEFAULT_STEPPER_DEACTIVE_TIME)) && \
       (gLcdAutoUI.pauseType == PAUSE_NO_MAT))
    {
        /* waiting for In/Out material finish */
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);

        queue.enqueue_now_P(PSTR("G28"));
        gLcdAutoUI.SetStaGoingHome(GO_HOME_DOING);
        gLcdAutoUI.evtRepMatToConPriFlag = true;
    }
    else
    {
        if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT)
        {
            /* waiting for In/Out material finish */
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
            gLcdAutoUI.evtWaitEAxisRecovery = true;
        }
        else
        {
            AutoUIPrintRecovery();
        }
    }
}

static void TouVarStopPrintPic040(void)
{
    /* current status: app printing */
    if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
    {
        gLcdAutoUI.evtNoMatWhenAppPrintFlag = false;

        SERIAL_ECHOLN(MSG_OK);

        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
        queue.enqueue_one_now(PSTR(CMDSTR_LOCALPRT_STOP));

        /* close all heater */
        thermalManager.disable_all_heaters();

        return;
    }
    else if((gLcdAutoUI.devStatusLastRecord >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.devStatusLastRecord < DEVSTA_NUM) && (gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT))
    {
        gLcdAutoUI.evtRepMatToStopPriFlag = true;

        return;
    }

    /* local print */
    if(gLcdAutoUI.AutoUIGetStatus() == DEVSTA_IN_OUT_MAT)
    {
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
        gLcdAutoUI.evtRepMatToStopPriFlag = true;
    }
    else
    {
        /* notify to WIFI board */
        SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
        /* stop print */
        AutoUIPrintStop();
    }
}

static void TouVarSaveSelectPic024(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);

    gLcdAutoUI.printModeType = PRINT_MODE_USERDEF;

    SaveUserDefModeParas();
}

static void TouVarConfStopHeatPic042(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);
    
    /* close all heater */
    thermalManager.disable_all_heaters();
    /* enable all fans */
    SetHotendFan(true);
    SetBoxFan(true);
}

static void TouVarCancStopHeatPic042(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTMODE);
}

static void TouVarUserDefHotendPic024(void)
{
    userDefMode.pmpHotendTemp = rtscheck.recdat.data[0];
}

static void TouVarUserDefBedPic024(void)
{
    userDefMode.pmpBedTemp = rtscheck.recdat.data[0];
}

static void TouVarKeybrdInputX(void)
{ 
    gLcdAutoUI.movAxisPicPos.x = rtscheck.recdat.data[0]/DWIN_REALVAL_TIMES;
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarKeybrdInputY(void)
{
    gLcdAutoUI.movAxisPicPos.y = rtscheck.recdat.data[0]/DWIN_REALVAL_TIMES;
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarKeybrdInputZ(void)
{
    gLcdAutoUI.movAxisPicPos.z = rtscheck.recdat.data[0]/DWIN_REALVAL_TIMES;
    gLcdAutoUI.movAxisLocalControlFlag = true;
}

static void TouVarConfirmPic044(void)
{
    if(gLcdAutoUI.lastPicID == gLcdAutoUI.curPicID)
        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
    else
        gLcdAutoUI.SwitchBackgroundPic(gLcdAutoUI.lastPicID);
}

static void TouVarConfirmPic045(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_OPERATING);
    /* quick stop all motors' moving */
    quickstop_stepper();
    /* change device status - 'DEVSTA_BED_CALI', start bed calibration flow */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_BED_CALI);
}

static void TouVarCancelPic045(void)
{
    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTSETTING);
}
/***********************************************************************************************************************************
                                                 functions for 'LcdAutoUIStruct_t'
***********************************************************************************************************************************/
/**
 * [LcdAutoUIStruct_t :Initialize Automatic-style-UI struct]
 * @Author Creality
 * @Time   2021-05-27
 */
LcdAutoUIStruct_t::LcdAutoUIStruct_t(void)
{
    /* default:Execute the guidance process */
    startupFirstTimeFlag = false;
    /* default:display in Chinese */
    curLanguage = (AutoUILanguage_t)AUTOUI_LANGUAGE_CH;
    /* default device status */
    devStatus = DEVSTA_INIT;
    goHomeSta = GO_HOME_IDLE;
    printModeType = PRINT_MODE_INVAILD;
    gcodeFileSelectedCur = VAULE_INVALIDE;
    memset(&flowStepCnt, 0, sizeof(flowStepCnt));
    memset(&enableSwitches, 0, sizeof(enableSwitches));
    bedCaliZOffset = 0.0;
    getWifiMacAddrFlag = false;
    evtWaitEAxisRecovery = false;
    /* default: Initialization of material breaking status*/
    matRunoutAtPrintingFlag = false;
    /* about card-read-error data initialization */
    memset(&cardReadErr, 0, sizeof(cardReadErr));
}

/**
 * [SetStartupFirstTimeFlag :set StartupFirstTimeFlag]
 * @Author Creality
 * @Time   2021-05-27
 * @param  _flag      [true:startup for first time, false:other situation]
 */
void LcdAutoUIStruct_t::SetStartupFirstTimeFlag(bool _flag)
{
    startupFirstTimeFlag = _flag;
}

/**
 * [GetStartupFirstTimeFlag :get the value of StartupFirstTimeFlag]
 * @Author Creality
 * @Time   2021-05-27
 */
bool LcdAutoUIStruct_t::GetStartupFirstTimeFlag(void)
{
    return startupFirstTimeFlag;
}

/**
 * [GetCurLangFromEEPROM :get current language that record in EEPROM]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::GetCurLangFromEEPROM(void)
{
    BL24CXX::read(FONT_EEPROM, (uint8_t*)&curLanguage, 1);
    if(curLanguage < AUTOUI_LANGUAGE_CH || curLanguage >= AUTOUI_LANGUAGE_NUM )
    {
        /* 'curLanguage' is out of range or loading language firstly, so display deault language */
        curLanguage = (AutoUILanguage_t)DEFAULT_LANGUAGE;
        /* record language type that display now in EEPROM at address of 'FONT_EEPROM' */
        BL24CXX::write(FONT_EEPROM, (uint8_t*)&curLanguage, 1);
    }
    else
    {
        /* display language by value of 'curLanguage' that load from 'EEPROM' */
    }

    #if 0
    curLanguage = (AutoUILanguage_t)DEFAULT_LANGUAGE;
    BL24CXX::write(FONT_EEPROM, (uint8_t*)&curLanguage, 1);
    #endif
}

/**
 * [LcdAutoUIStruct_t::SetSaveCurLang :set current language and save in EEPROM]
 * @Author Creality
 * @Time   2021-06-17
 * @param  _language  [language to be set]
 */
void LcdAutoUIStruct_t::SetSaveCurLang(AutoUILanguage_t _language)
{
    curLanguage = _language;

    BL24CXX::write(FONT_EEPROM, (uint8_t*)&curLanguage, 1);

    RefreshCurLanguage();
}

/**
 * [LcdAutoUIStruct_t::GetCurLanguage :get current language]
 * @Author Creality
 * @Time   2021-06-21
 * @return            [current language]
 */
AutoUILanguage_t LcdAutoUIStruct_t::GetCurLanguage(void)
{
    return curLanguage;
}

/**
 * [GetStaFirTimFlagFromEEPROM :get 'StartupFirstTimeFlag' that record in EEPROM]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::GetStaFirTimFlagFromEEPROM(void)
{
    uint8_t lFlagVal = 0;

    /* device startup for first time or not */
    BL24CXX::read(FONT_EEPROM + EEPROM_ADDR_STTF_OFFSET, &lFlagVal, EEPROM_ADDR_STTF_SIZE);
    if(lFlagVal != STARTUP_FIR_TIME_VAL)
    {
        /* startup for first time */
        SetStartupFirstTimeFlag(true);
        lFlagVal = STARTUP_FIR_TIME_VAL;
        BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_STTF_OFFSET, &lFlagVal, EEPROM_ADDR_STTF_SIZE);
        /* wait for finish writing EEPROM */
        delay(5);
    }
}

/**
 * [SetStaGoingHome :Set status of going home]
 * @Author Creality
 * @Time   2021-06-01
 * @param  _sta       [status of going home]
 */
void LcdAutoUIStruct_t::SetStaGoingHome(AutoUIGoHomeSta_t _sta)
{
    goHomeSta = _sta;
}

/**
 * [LcdAutoUIStruct_t::GetStaGoingHome :get status of going home]
 * @Author Creality
 * @Time   2021-06-16
 * @return            [status of going home]
 */
AutoUIGoHomeSta_t LcdAutoUIStruct_t::GetStaGoingHome(void)
{
    return goHomeSta;
}

/**
 * [LcdAutoUIStruct_t::GetCurPrintMode :get current print-mode]
 * @Author Creality
 * @Time   2021-06-23
 * @return            [current print-mode]
 */
AutoUIPrintModeType_t LcdAutoUIStruct_t::GetCurPrintMode(void)
{
    return printModeType;
}

/**
 * [LcdAutoUIStruct_t::AutoUISetBoxLed :set box led]
 * @Author Creality
 * @Time   2021-06-22
 * @param  _en        [true:turn on, false:turn off]
 */
void LcdAutoUIStruct_t::AutoUISetBoxLed(bool _en)
{
    SetBoxLed(_en);
}

/**
 * [LcdAutoUIStruct_t::SetHaGoHoBeFlag :set flag of 'haveGoneHomeBeforeFlag']
 * @Author Creality
 * @Time   2021-06-11
 * @param  _flag      [true: have gone home before, false:have not gone home]
 */
void LcdAutoUIStruct_t::SetHaGoHoBeFlag(bool _flag)
{
    haveGoneHomeBeforeFlag = _flag;
}

/**
 * [LcdAutoUIStruct_t::GetHaGoHoBeFlag :get flag of 'haveGoneHomeBeforeFlag']
 * @Author Creality
 * @Time   2021-06-11
 * @return            [true: have gone home before, false:have not gone home]
 */
bool LcdAutoUIStruct_t::GetHaGoHoBeFlag(void)
{
    return haveGoneHomeBeforeFlag;
}

/**
 * [LcdAutoUIStruct_t::SetPrintFinishFlag :set flag of 'printFinishFlag']
 * @Author Creality
 * @Time   2021-07-10
 * @param  _flag      [true:print has finished, false:other situation]
 */
void LcdAutoUIStruct_t::SetPrintFinishFlag(bool _flag)
{
    printFinishFlag = _flag;
}

/**
 * [LcdAutoUIStruct_t::GetPrintFinishFlag :get flag of 'printFinishFlag']
 * @Author Creality
 * @Time   2021-07-10
 * @param  _flag      [true:print has finished, false:other situation]
 */
bool LcdAutoUIStruct_t::GetPrintFinishFlag(void)
{
    return printFinishFlag;
}

/**
 * [LcdAutoUIStruct_t::GetNoMatWhenAppPrintFlag :get flag of 'evtNoMatWhenAppPrintFlag']
 * @Author Creality
 * @Time   2021-12-30
 * @return            [true: Out of materials when app printing, false: other situations]
 */
bool LcdAutoUIStruct_t::GetNoMatWhenAppPrintFlag(void)
{
    return evtNoMatWhenAppPrintFlag;
}

/**
 * [LcdAutoUIStruct_t::GetDoorOpenWhenAppPrintFlag :get flag of 'evtDoorOpenWhenAppPrintFlag']
 * @Author Creality
 * @Time   2021-12-30
 * @return            [true: door is open when app printing, false: other situations]
 */
bool LcdAutoUIStruct_t::GetDoorOpenWhenAppPrintFlag(void)
{
    return evtDoorOpenWhenAppPrintFlag;
}

/**
 * [LcdAutoUIStruct_t::SetFlowStep :set current step of flow]
 * @Author Creality
 * @Time   2021-06-07
 * @param  _flowCntType [type of flow-count, refer to variable 'gLcdAutoUI.flowStepCnt']
 * @param  _stepVal     [description]
 */
void LcdAutoUIStruct_t::SetFlowStep(uint8_t* _flowCntType, uint8_t _stepVal)
{
    *_flowCntType = _stepVal;
}

/**
 * [LcdAutoUIStruct_t::SetAxisStepUnit :set axis move step unit]
 * @Author Creality
 * @Time   2021-06-10
 * @param  _uint      [value of unit]
 */
void LcdAutoUIStruct_t::SetAxisStepUnit(AutoUIAxisStepUnit_t _uint)
{
    axisStepUnit = _uint;
}

/**
 * [LcdAutoUIStruct_t::GetAxisStepUnit :get axis move step unit]
 * @Author Creality
 * @Time   2021-06-10
 * @return            [current step unit]
 */
int LcdAutoUIStruct_t::GetAxisStepUnit(void)
{
    return (int)axisStepUnit;
}

/**
 * [SwitchBackgroundPic :switch background picture]
 * @Author Creality
 * @Time   2021-05-27
 * @param  _picId     [ID of background-picture]
 */
void LcdAutoUIStruct_t::SwitchBackgroundPic(AutoUIPictureID_t _picId)
{
    if(_picId == curPicID) return;
    rtscheck.RTS_SndData(ExchangePageBase + (uint8_t)_picId, ExchangepageAddr);
    /* record current and last picture ID */
    lastPicID = curPicID;
    curPicID = _picId;
}

/**
 * [SwitchIcon :switch icon]
 * @Author Creality
 * @Time   2021-05-27
 * @param  _iconId     [ID of icon]
 * @param  _iconAdress [adress of icon]
 */
void LcdAutoUIStruct_t::SwitchIcon(int _iconId, unsigned long _iconAdress)
{
    rtscheck.RTS_SndData(_iconId, _iconAdress);
}

/**
 * [LcdAutoUIStruct_t::DisplayData :display data]
 * @Author Creality
 * @Time   2021-06-04
 * @param  _data       [value of data]
 * @param  _dataAdress [address of data]
 */
void LcdAutoUIStruct_t::DisplayData(int _data, unsigned long _dataAdress)
{
    rtscheck.RTS_SndData(_data, _dataAdress);
}

/**
 * [LcdAutoUIStruct_t::DisplayText :display text]
 * @Author Creality
 * @Time   2021-06-09
 * @param  _str        [text string]
 * @param  _dataAdress [address of text variable]
 */
void LcdAutoUIStruct_t::DisplayText(char* _str, unsigned long _dataAdress)
{
    rtscheck.RTS_SndData(_str, _dataAdress);
}

/**
 * [SetStaDynamicIcon :enable/disable dynamic icon]
 * @Author Creality
 * @Time   2021-06-01
 * @param  _en         [true: start to loop icon, false: stop ]
 * @param  _iconAdress [adress of dynamic-icon]
 */
void LcdAutoUIStruct_t::SetStaDynamicIcon(bool _en, unsigned long _iconAdress)
{
    rtscheck.RTS_SndData(_en, _iconAdress);
}

/**
 * [RefreshCurLanguage :refresh current language according to 'curLanguage']
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::RefreshCurLanguage(void)
{
    LanguageSelect((unsigned short)curLanguage);

    SwitchIcon(curLanguage, ICONVAR_ADDR_LANGUAGE_SWI);

    /* official website */
    if(GetCurLanguage() == AUTOUI_LANGUAGE_CH)
        DisplayText((char*)CORP_WEBSITE_C, TEXTVAR_ADDR_OFFICIAL_WEB);
    else
        DisplayText((char*)CORP_WEBSITE_E, TEXTVAR_ADDR_OFFICIAL_WEB);
}
/**
 * [LcdAutoUIStruct_t::RefreshCurDisplayData :refresh data that display currently]
 * @Author Creality
 * @Time   2021-06-10
 */
void LcdAutoUIStruct_t::RefreshCurDisplayData(void)
{
    DisplayData(LcdAutoUIGetTempHotend(), DATAVAR_ADDR_HOTENDTEMP_CUR);
    DisplayData(LcdAutoUIGetTempBed(), DATAVAR_ADDR_BEDTEMP_CUR);
    DisplayData(0, DATAVAR_ADDR_HOTENDTEMP_GIVEN);
    DisplayData(0, DATAVAR_ADDR_BEDTEMP_GIVEN);
    /* display bed calibration Z axis offset at center point */
    DisplayData((int)(100*bedCaliZOffsetCenter), DATAVAR_ADDR_CENTRAL_Z_OFFSET);
    /* display default print-feedrate-percentage */
    DisplayData(feedrate_percentage, DATAVAR_ADDR_FEEDRARE);
}

/**
 * [LcdAutoUIStruct_t::RefreshCurDisplayIcon :refresh icon that display currently]
 * @Author Creality
 * @Time   2021-06-16
 */
void LcdAutoUIStruct_t::RefreshCurDisplayIcon(void)
{
    SwitchIcon(SELECTED_NO, ICONVAR_ADDR_PLA_SELECTED);
    SwitchIcon(SELECTED_NO, ICONVAR_ADDR_ABS_SELECTED);
    SwitchIcon(VAULE_INVALIDE_8BIT, ICONVAR_ADDR_DOOR_OPEN_TIP);
}

/**
 * [LcdAutoUIStruct_t::RefreshPrintTimeTotal :refresh current total printing time]
 * @Author Creality
 * @Time   2021-06-15
 */
void LcdAutoUIStruct_t::RefreshPrintTimeTotal(void)
{
    uint32_t lTotalTimeInt = 0;

    /* app print */
    if((gLcdAutoUI.AutoUIGetStatus() >= DEVSTA_APP_PRINTING) && (gLcdAutoUI.AutoUIGetStatus() < DEVSTA_NUM))
    {
        printingTimeTotal += appPrint.apPrintTime;
        appPrint.apPrintTime = 0.0;
    }
    /* local print */
    else
    {
        printingTimeTotal += printingTimeOnce;
        printingTimeOnce = 0.0;
    }
    
    /* two decimal fraction was set in DWIN project, need to zoom in ten times */
    rtscheck.RTS_SndData((unsigned int)(100 * printingTimeTotal), DATAVAR_ADDR_PRINTTIME_TOTAL);

    /* record total printing time in EEPROM */
    lTotalTimeInt = (uint32_t)(100 * printingTimeTotal);
    BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_TPT_OFFSET, (uint8_t*)&lTotalTimeInt, EEPROM_ADDR_TPT_SIZE);
}

/**
 * [LcdAutoUIStruct_t::SetPrintTimeTotal :set current total printing time]
 * @Author Creality
 * @Time   2021-06-16
 */
void LcdAutoUIStruct_t::SetPrintTimeTotal(float _totalTime)
{
    printingTimeTotal = _totalTime;
}

/**
 * [LcdAutoUIStruct_t::GetPrintTimeTotal :get current total printing time]
 * @Author Creality
 * @Time   2021-06-16
 */
float LcdAutoUIStruct_t::GetPrintTimeTotal(void)
{
    return printingTimeTotal;
}

/**
 * [DevicePowerOnProgressBar :power on progress bar]
 * @Author Creality
 * @Time   2021-05-27
 */
void LcdAutoUIStruct_t::DevicePowerOnProgressBar(void)
{
    SwitchBackgroundPic(AUTOUI_STARTING);
    for(int i=0; i<=100; i++)
    {
        SwitchIcon(i, ICONVAR_ADDR_PROGRESSBAR);
        delay(30);
        watchdog_refresh();
    }
}

/**
 * [LcdAutoUIStruct_t::SDCardUpdate :refresh gcode file on DWIN screen]
 * @Author Creality
 * @Time   2021-06-04
 */
void LcdAutoUIStruct_t::SDCardUpdate(void)
{
    static bool lSDCardStatus = false;
    char lFileName[GCODE_FILE_BYTELEN] = {0};

    /* sd card inserted */
    if(!lSDCardStatus && card.isMounted())
    {
        lSDCardStatus = card.isMounted();

        /* sd card init */
        rtscheck.RTS_SDCardInit();
        /* display gocde file name on DWIN screen */
        for(uint16_t i=0; i<GetSDCardGcodeFileNum(); i++) 
        {
            delay(3);
            memcpy(lFileName, CardRecbuf.Cardshowfilename[i], GCODE_FILE_BYTELEN);
            DisplayText(lFileName, TEXTVAR_ADDR_FILENAME_00 + GCODE_FILE_BYTELEN * i);
        }
    }
    /* sd card removed */
    else if(lSDCardStatus && !card.isMounted())
    {
        lSDCardStatus = card.isMounted();

        /* clean all file name */
        for(uint16_t i=0; i<GCODE_FILE_NUM_MAX; i++) 
        {
            delay(1);
            for(uint32_t j=0; j<GCODE_FILE_BYTELEN; j++)
                DisplayData(0, TEXTVAR_ADDR_FILENAME_00 + GCODE_FILE_BYTELEN * i + j);
        }
        /* clear file selected highlight icon */
        ClearFileBackground();
        /* clear record about SD card */
        ClearSDCardGcodeFileRecord();
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIToStatus :change device status]
 * @Author Creality
 * @Time   2021-06-02
 * @param  _sta       [status to change]
 */
void LcdAutoUIStruct_t::AutoUIToStatus(AutoUIDevStatus_t _sta)
{
    if(_sta == devStatus)
    {
        SwitchBackgroundPic(AUTOUI_OPERATING);
        devStaCon.dscFlag = true;
        devStaCon.dscDevStaRecord = _sta;
        return;
    }
    /* record current device status */
    devStatusLastRecord = devStatus;
    /* set status */
    devStatus = _sta;

    switch(devStatus)
    {
        case DEVSTA_INIT:
            
            break;

        case DEVSTA_STARTUP_FIR:
            
            break;

        case DEVSTA_IDLE:
            
            break;

        case DEVSTA_HEATING:
            /* set preheatFinishFlags status to "heating" */
            memset(&gLcdAutoUI.preheatFinishFlags, 0, sizeof(gLcdAutoUI.preheatFinishFlags));
            break;

        case DEVSTA_IN_OUT_MAT:
            feedFinishedFlag = false;
            SetFlowStep(&flowStepCnt.fscInOutMat, 0);
            break;

        case DEVSTA_PRINTING:

            break;

        case DEVSTA_PRINT_PAUSE:
            if(gLcdAutoUI.devStatusLastRecord != DEVSTA_IN_OUT_MAT)
                SetFlowStep(&flowStepCnt.fscPrintPause, 0);
            break;

        case DEVSTA_BED_CALI:
            bedCaliFinishedFlag = false;
            SetFlowStep(&flowStepCnt.fscBedCali, 0);
            break;
        
        case DEVSTA_APP_PRINTING:

            break;

        case DEVSTA_APP_PRTHEAT:
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_HEATING);
            gLcdAutoUI.appPrint.flags.flgPreheatWaiting = true;
            break;

        case DEVSTA_APP_PRTPAUSE:
            if(gLcdAutoUI.devStatusLastRecord != DEVSTA_IN_OUT_MAT)
                SetFlowStep(&flowStepCnt.fscAppPrintPause, 0);
            break;

        case DEVSTA_ERROR:

            break;
        default:
            break;
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIDelayToStatus :delay to change device status because of status conflict]
 * @Author Creality
 * @Time   2021-12-27
 * @param  _sta       [status to change]
 */
void LcdAutoUIStruct_t::AutoUIDelayToStatus(AutoUIDevStatus_t _sta)
{
    if(devStaCon.dscFlag)
    {
        if(_sta != devStatus)
        {
            devStaCon.dscFlag = false;
            AutoUIToStatus(_sta);
        }
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIGetStatus :get device current status]
 * @Author Creality
 * @Time   2021-06-09
 * @return            [device current status]
 */
AutoUIDevStatus_t LcdAutoUIStruct_t::AutoUIGetStatus(void)
{
    return devStatus;
}

/**
 * [AutoUIVirKeyEvent :Simulate sending touch key information]
 * @Author Creality
 * @Time   2021-12-23
 * @param  _touVarAddr [virtual key address]
 * @param  _keyVal     [virtual key value]
 */
void LcdAutoUIStruct_t::AutoUIVirKeyEvent(AutoUITouVarAddr_t _touVarAddr, uint16_t _keyVal)
{
    if(!virKeyInfo.virKeyFlag)
    {
        virKeyInfo.virKeyFlag = true;
        virKeyInfo.virKeyAddr = _touVarAddr;
        virKeyInfo.virKeyValue = _keyVal;
    }
    else
    {
        SERIAL_ECHOLN("<warning>Last virtual key haven't handled");
    }
}

/**
 * [AutoUIRecDataHandle :receive data handle]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::AutoUIRecDataHandle(void)
{
    /* virtual key handle */
    if(virKeyInfo.virKeyFlag)
    {
        rtscheck.recdat.addr = virKeyInfo.virKeyAddr;
        rtscheck.recdat.data[0] = virKeyInfo.virKeyValue;

        memset(&virKeyInfo, 0, sizeof(virKeyInfo));
    }

    for(uint32_t i=0; i<TOUVAR_NUM; i++)
    {
        #if 1
        switch(rtscheck.recdat.addr)
        {
            case TOUVAR_PIC033_CONFIRM:
                /* factory reset  */
                /* get to the status that device has never used */
                BL24CXX::write(FONT_EEPROM + EEPROM_ADDR_STTF_OFFSET, 0, EEPROM_ADDR_STTF_SIZE);
                /* clear printing-total-time */
                ClearPrintTotalTimeRecord();
                /* clear switches status */
                ResetSwitchesStatus();
                /* reset all parameters */
                settings.reset();
                settings.save();

                gLcdAutoUI.SwitchBackgroundPic(AUTOUI_DEVINFO);

                rtscheck.recdat.addr = 0;
                rtscheck.recdat.data[0] = 0;
                return;

            #if 0
            case TOUVAR_PIC_START_PRINT:
                AutoUIPrintStart();
                
                rtscheck.recdat.addr = 0;
                rtscheck.recdat.data[0] = 0;
                return;
            #endif

            default:
                break;
        }
        #endif


        if(gAutoUITouVarResponseArray[i].touchVarAddr == rtscheck.recdat.addr)
        {
            rtscheck.recdat.addr = 0;
            if(gAutoUITouVarResponseArray[i].touchVarFunc != (void*)0)
            {
                gAutoUITouVarResponseArray[i].touchVarFunc();

                rtscheck.recdat.data[0] = 0;
            }
            return;
        }
    }
}

/**
 * [AutoUIResponseToDWIN :response to DWIN touch]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::AutoUIResponseToDWIN(void)
{
    if((rtscheck.RTS_RecData() > 0) || virKeyInfo.virKeyFlag)
    {
        #if 0
        /* variable-address got from DWIN */
        SERIAL_ECHO("rtscheck.recdat.addr: 0x");
        SERIAL_PRINTLN(rtscheck.recdat.addr, HEX);
        /* variable-address got from DWIN */
        SERIAL_ECHO("rtscheck.recdat.data[0]: 0x");
        SERIAL_PRINTLN(rtscheck.recdat.data[0], HEX);
        #endif

        AutoUIRecDataHandle();
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIAppManPauRecovey :manual pause recovery when app-print]
 * @Author Creality
 * @Time   2022-01-17
 */
void LcdAutoUIStruct_t::AutoUIAppManPauRecovey(void)
{
    /* change device status to "DEVSTA_APP_PRTHEAT" */
    gLcdAutoUI.AutoUIToStatus(DEVSTA_APP_PRTHEAT);
    
    /* recovery position */
    if(devStatusLastRecord == DEVSTA_APP_PRTPAUSE)
    {
        /* only recovery Z-axis */
        current_position.z = gLcdAutoUI.pauseStaRecord.posPauseBefore.z;
        LcdAutoUIMoveXYZBlock(current_position.x, current_position.y, current_position.z);
        memset(&gLcdAutoUI.pauseStaRecord, 0, sizeof(gLcdAutoUI.pauseStaRecord));
    }
}

/**
 * [AutoUIInit :startup process]
 * @Author Creality
 * @Time   2021-05-27
 */
void LcdAutoUIStruct_t::AutoUIInit(void)
{
    GetCurLangFromEEPROM();

    GetStaFirTimFlagFromEEPROM();

    ReadSwitchesStatus();

    ReadBedCaliZOffsetCenter();

    RefreshUserDefParas();

    /* wait for DWIN LCD ready to communication */
    delay(500);

    DevicePowerOnProgressBar();

    RefreshCurLanguage();

    RefreshDevInfo();

    RefreshCurDisplayData();

    RefreshCurDisplayIcon();

    /* report wifi status */
    CWWBIndLedSet(gLcdAutoUI.enableSwitches.swiWifiLed);

    RefreshOutMatReturnKey(true);

    /* default : enable bed leveling */
    LcdAutoUISetBedLeveling(true);

    /* startup for first time or not */
    if(GetStartupFirstTimeFlag())
    {
        /* yes, change status to 'DEVSTA_STARTUP_FIR' */
        AutoUIToStatus(DEVSTA_STARTUP_FIR);

        /* Initial value(total-printing-time stored in EEPROM) setting */
        RefreshPrintTimeTotal();

        SwitchBackgroundPic(AUTOUI_BOOTGUIDE_01);
    }  
    else
    {
        /* no, change status to 'DEVSTA_IDLE' */
        AutoUIToStatus(DEVSTA_IDLE);

        SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
    }

    GetTPTFromEEPROM();
}

/**
 * [AutoUIStartupFirstTime :logic flow of 'DEVSTA_STARTUP_FIR' status]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::AutoUIStartupFirstTimeFlow(void)
{
    switch(flowStepCnt.fscStartupFir)
    {
        case 0:
            if(goHomeSta == GO_HOME_DONE)
            {
                SetStaGoingHome(GO_HOME_IDLE);

                /* disable Dynamic icon */
                gLcdAutoUI.SetStaDynamicIcon(false, ICONVAR_ADDR_DYNAHOMING);

                SwitchBackgroundPic(AUTOUI_BOOTGUIDE_05);
                gLcdAutoUI.SetStaDynamicIcon(true, ICONVAR_ADDR_IN_MAT);

                flowStepCnt.fscStartupFir ++;
            }
            break;

        case 1:
            /* wait for inserting SD card */
            if((IS_SD_INSERTED()) && (curPicID == AUTOUI_BOOTGUIDE_07))
            {
                SwitchBackgroundPic(AUTOUI_FILESELECT_01);

                AutoUIToStatus(DEVSTA_IDLE);

                flowStepCnt.fscStartupFir ++;
            }
            break;

        default:
            break;
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIIdleFlow :logic flow of 'DEVSTA_IDLE' status]
 * @Author Creality
 * @Time   2021-06-04
 */
void LcdAutoUIStruct_t::AutoUIIdleFlow(void)
{
    static uint32_t lMs1000 = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs1000) > 1000)
    {
        lMs1000 = LcdAutoUIGetTimeMs();

        RefreshAllTemperature();

        RefreshCurSwiIconStatus();

        RefreshHeatingStatus();

        RefreshCurPrintFeedrate();

        if(eventPriFlags.epfStopPrint && (GetStaGoingHome() == GO_HOME_DONE))
        {
            eventPriFlags.epfStopPrint = false;
            SetStaGoingHome(GO_HOME_IDLE);
            gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
        }
    }

    /* check if device need to excute power-off-recovery */
    AutoUIPowerOffRecoveryCheck();
}

/**
 * [LcdAutoUIStruct_t::AutoUIHeatFlow :logic flow of 'DEVSTA_HEATING' status]
 * @Author Creality
 * @Time   2021-07-13
 */
void LcdAutoUIStruct_t::AutoUIHeatFlow(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 1000)
    {
        lMs = LcdAutoUIGetTimeMs();

        RefreshAllTemperature();

        RefreshCurSwiIconStatus();

        RefreshHeatingStatus();

        RefreshCurPrintFeedrate();
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIInOutMatFlow :logic flow of 'DEVSTA_IN_OUT_MAT' status]
 * @Author Creality
 * @Time   2021-06-02
 */
void LcdAutoUIStruct_t::AutoUIInOutMatFlow(void)
{
    static float lTempTarHotend = 0.0;
    static uint32_t lMs1000 = LcdAutoUIGetTimeMs();
    /* record the count of no-command-in-queue */
    static uint8_t lNoCmdInQueueCnt = 0;

    if((LcdAutoUIGetTimeMs() - lMs1000) > 1000)
    {
        lMs1000 = LcdAutoUIGetTimeMs();
        RefreshAllTemperature();
    }

    switch(flowStepCnt.fscInOutMat)
    {
        case 0:
            /* record current hotend temperature */
            if(!flowStepCnt.fscInOutMat)
                lTempTarHotend = LcdAutoUIGetTarTempHotend();
            flowStepCnt.fscInOutMat = 1;
            /* Hotend heating up to feeding-material-temperature */
            LcdAutoUISetTempHotend(FEEDING_TEMP);
            break;

        case 1:
            /* if the temperature is satisfied */
            if(LcdAutoUIGetTempHotend() >= FEEDING_TEMP)
            {
                flowStepCnt.fscInOutMat ++;
            }
            else
            {
                /* be sure that feeding-material-temperature was set to 'FEEDING_TEMP' */
                if(LcdAutoUIGetTarTempHotend() != FEEDING_TEMP)
                    LcdAutoUISetTempHotend(FEEDING_TEMP);
            }
            break;

        case 2:
            if(inOutMatFlag)
            {
                /* in material, start E-Axis moving */
                LcdAutoUIMoveE(FEEDING_DEF_DISTANCE, FEEDING_DEF_SPEED);
            }
            else
            {
                /* out material */
                LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS1, FEEDING_DEF_OUT_MAT_SPE1);
                LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS2, FEEDING_DEF_OUT_MAT_SPE2);

                SwitchBackgroundPic(AUTOUI_OUTMAT_02);
            }

            flowStepCnt.fscInOutMat ++;
            break;

        case 3:
            if(feedFinishedFlag)
            {
                feedFinishedFlag = false;
                /* stop All Axis moving */
                quickstop_stepper();
                /* recovery the E-Axis position */
                if((devStatusLastRecord == DEVSTA_PRINT_PAUSE) || (devStatusLastRecord == DEVSTA_APP_PRTPAUSE))
                    LcdAutoUISetECmd(pauseStaRecord.posPauseBefore.e + PRINT_RECOVER_EXTERA_E_POS);
                /* recovery hotend temperature */
                LcdAutoUISetTempHotend(lTempTarHotend);
                for(uint8_t i=0; i<5; i++)
                {
                    /* be sure that the hotend temperature has returned to its original temperature */
                    if(LcdAutoUIGetTarTempHotend() != lTempTarHotend)
                        LcdAutoUISetTempHotend(lTempTarHotend);
                    else
                        break;
                }
                /* clear hotend temperature record */
                lTempTarHotend = 0.0;

                flowStepCnt.fscInOutMat ++;
            }
            break;

        case 4:
            if(planner.has_blocks_queued())
                break;

            /* be sure that there is no commands in queue */
            if(lNoCmdInQueueCnt < 3)
            {
                lNoCmdInQueueCnt ++;
                break;
            }
            else
            {
                lNoCmdInQueueCnt = 0;
            }

            /* out material finish */
            if(!inOutMatFlag)
                SwitchBackgroundPic(AUTOUI_INOUTMAT);
            /* return the device status to the last */
            AutoUIToStatus(devStatusLastRecord);

            SetFlowStep(&flowStepCnt.fscInOutMat, 0);

            if(evtWaitEAxisRecovery)
            {
                evtWaitEAxisRecovery = false;
                if((devStatus >= DEVSTA_APP_PRINTING) && (devStatus < DEVSTA_NUM))
                    AutoUIAppPrintRecovery();
                else
                    AutoUIPrintRecovery();
            }
            else if(evtRepMatToStopPriFlag)
            {
                evtRepMatToStopPriFlag = false;
                if((devStatus >= DEVSTA_APP_PRINTING) && (devStatus < DEVSTA_NUM))
                {
                    gLcdAutoUI.evtNoMatWhenAppPrintFlag = false;

                    SERIAL_ECHOLN(MSG_OK);

                    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_MAIN_WINDOW);
                    SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
                    queue.enqueue_one_now(PSTR(CMDSTR_LOCALPRT_STOP));

                    /* close all heater */
                    thermalManager.disable_all_heaters();
                }
                else
                {
                    /* notify to WIFI board */
                    SERIAL_ECHOLN(CMDSTR_LOCALPRT_STOP);
                    /* stop print */
                    AutoUIPrintStop();
                }
            }
            break;

        default:
            break;
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIPrintingFlow :logic flow of 'DEVSTA_PRINTING' status]
 * @Author Creality
 * @Time   2021-06-09
 */
void LcdAutoUIStruct_t::AutoUIPrintingFlow(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 1000)
    {
        lMs = LcdAutoUIGetTimeMs();

        RefreshAllTemperature();

        RefreshCurPrintProgress();

        RefreshCurPrintTime();

        RefreshCurSwiIconStatus();

        RefreshHeatingStatus();

        RefreshCurPrintFeedrate();

        #if ENABLE_MATERIAL_CHECK
            if(GetStaGoingHome() != GO_HOME_DOING)
                MaterialCheckProcess();
        #endif

        #if ENABLE_DOOR_OPEN_CHECK
            if(enableSwitches.swiDoorPause)
                if(GetStaGoingHome() != GO_HOME_DOING)
                    DoorOpenPauseProcess();
        #endif

        AutoUIPrintFinished();
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIPrintPauseFlow :logic flow of 'DEVSTA_PRINT_PAUSE' status]
 * @Author Creality
 * @Time   2021-06-09
 */
void LcdAutoUIStruct_t::AutoUIPrintPauseFlow(void)
{
    /* record the count of no-command-in-queue */
    static uint8_t lNoCmdInQueueCnt = 0;

    switch(flowStepCnt.fscPrintPause)
    {
        case 0:
            /* device is going home */
            if(GetStaGoingHome() == GO_HOME_DOING)
                break;

            pauseType = PAUSE_MANUAL;

            /* material check */
            #if ENABLE_MATERIAL_CHECK
            if(!IsExistMaterial())
                pauseType = PAUSE_NO_MAT;
            #endif

            /* door open check */
            #if ENABLE_DOOR_OPEN_CHECK
            if(enableSwitches.swiDoorPause && IsDoorOpen())
                pauseType = PRINT_DOOR_OPEN;
            #endif

            if(!planner.has_blocks_queued())
            {
                /* be sure that there is no commands in queue */
                if(lNoCmdInQueueCnt < 3)
                {
                    lNoCmdInQueueCnt ++;
                    break;
                }
                else
                {
                    lNoCmdInQueueCnt = 0;
                }

				/* record current position */
                pauseStaRecord.posPauseBefore = current_position;
                /* pause SD card print */
                queue.inject_P(PSTR("M25"));

                /* get to pause position */
                switch(pauseType)
                {
                    case PAUSE_MANUAL:
                        LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveY(Y_BED_SIZE, AXIS_MOVE_ABSO);
                        break;

                    /* material check */
                    #if ENABLE_MATERIAL_CHECK
                    case PAUSE_NO_MAT:
                        LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveY(0, AXIS_MOVE_ABSO);
                        /* record current tick */
                        evtRepMatToConPriTmr = LcdAutoUIGetTimeMs();
                        break;
                    #endif

                    /* door open check */
                    #if ENABLE_DOOR_OPEN_CHECK
                    case PRINT_DOOR_OPEN:
                        if(enableSwitches.swiDoorPause)
                        {
                            // LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                            // LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
                            // LcdAutoUIMoveY(Y_BED_SIZE, AXIS_MOVE_ABSO);
                        }
                        break;
                    #endif

                    default:
                        break;
                }
                flowStepCnt.fscPrintPause ++;
            }
            break;

        case 1:
            if((pauseType == PAUSE_NO_MAT) && (planner.has_blocks_queued()))
                break;

            switch(pauseType)
            {
                case PAUSE_MANUAL:
                    SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
                    break;

                /* material check */
                #if ENABLE_MATERIAL_CHECK
                case PAUSE_NO_MAT:
                    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LACKMATPW);
                    /* start close-heater-timer */
                    CloHeaTmrStart(CLOHEATMR_TIME);
                    /* out material */
                    // LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS1, FEEDING_DEF_OUT_MAT_SPE1);
                    // LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS2, FEEDING_DEF_OUT_MAT_SPE2);
                    break;
                #endif

                /* door open check */
                #if ENABLE_DOOR_OPEN_CHECK
                case PRINT_DOOR_OPEN:
                    if(enableSwitches.swiDoorPause)
                    {
                        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTDOOROPEN);
                        gLcdAutoUI.SwitchIcon(VAULE_INVALIDE_8BIT, ICONVAR_ADDR_DOOR_OPEN_TIP);
                    }
                    break;
                #endif

                default:
                    break;
            }

            flowStepCnt.fscPrintPause ++;
            break;

        default:
            break;
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIBedCaliFlow :logic flow of 'DEVSTA_BED_CALI' status]
 * @Author Creality
 * @Time   2021-06-07
 */
void LcdAutoUIStruct_t::AutoUIBedCaliFlow(void)
{
    /* true:have switched background picture, false: have not */
    static bool lSwiBackgroundPicFlag = false;

    switch(flowStepCnt.fscBedCali)
    {
        case 0:
            /* device is going home */
            if(GetStaGoingHome() == GO_HOME_DOING)
            {
                if(!lSwiBackgroundPicFlag)
                {
                    lSwiBackgroundPicFlag = true;
                    /* switch picture to wait for going home */
                    SwitchBackgroundPic(AUTOUI_OPERATING);
                    /* clear command in queue */
                    queue.clear();
                    break;
                } 
                else
                {
                    break;
                }
            }
            else
            {
                /* switch picture to wait for going home */
                SwitchBackgroundPic(AUTOUI_OPERATING);
            }       

            lSwiBackgroundPicFlag = false;
            /* set go-home status - 'GO_HOME_IDLE' */
            SetStaGoingHome(GO_HOME_IDLE);
            /* start bed calibration */
            queue.inject_P(PSTR("G29 S1"));

            LcdAutoUISetBedLeveling(false);

            flowStepCnt.fscBedCali ++;

            break;

        case 1:
            /* wait for going home */
            if(goHomeSta == GO_HOME_DONE)
            {
                /* set go-home status - 'GO_HOME_IDLE' */
                SetStaGoingHome(GO_HOME_IDLE);
                /* get to the center(x, y), block */
                LcdAutoUIMoveXYZBlock((float)(X_BED_SIZE/2), (float)(Y_BED_SIZE/2), (current_position.z - Z_MAX_POS));
                /* turn on the LED */
                SetBoxLed(true);

                SwitchBackgroundPic(AUTOUI_BED_CALI);

                pointCntCaliCur = 0;
                SwitchIcon(pointCntCaliCur++, ICONVAR_ADDR_BEDCALI_CURPOINT);

                /* init display of Z-offset and Z-height */
                DisplayData((int)(100*bedCaliZOffset), DATAVAR_ADDR_ZOFFSET);
                DisplayData((int)(100*(bedCaliZOffset + Z_MAX_POS)), DATAVAR_ADDR_ZHEIGHT);

                flowStepCnt.fscBedCali ++;  
            }
            break;

        case 2:
            if(bedCaliFinishedFlag)
            {
                bedCaliFinishedFlag = false;

                SwitchBackgroundPic(AUTOUI_PRINTSETTING);

                /* turn off the LED */
                SetBoxLed(false);

                LcdAutoUISetBedLeveling(true);

                AutoUIToStatus(DEVSTA_IDLE);

                SetFlowStep(&flowStepCnt.fscBedCali, 0);
            }
            break;

        default:
            break;
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIAppPrintingFlow :flow of 'DEVSTA_APP_PRINTING' status]
 * @Author Creality
 * @Time   2021-11-16
 */
void LcdAutoUIStruct_t::AutoUIAppPrintingFlow(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 1000)
    {
        lMs = LcdAutoUIGetTimeMs();

        RefreshAllTemperature();

        RefreshCurSwiIconStatus();

        RefreshHeatingStatus();

        RefreshCurPrintFeedrate();

        #if ENABLE_MATERIAL_CHECK
            if(GetStaGoingHome() != GO_HOME_DOING)
                MaterialCheckProcess();
        #endif

        #if ENABLE_DOOR_OPEN_CHECK
            if(enableSwitches.swiDoorPause)
                if(GetStaGoingHome() != GO_HOME_DOING)
                    DoorOpenPauseProcess();
        #endif
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIAppPrintHeatFlow :flow of 'DEVSTA_APP_PRTHEAT' status]
 * @Author Creality
 * @Time   2021-11-24
 */
void LcdAutoUIStruct_t::AutoUIAppPrintHeatFlow(void)
{
    static uint32_t lMs = LcdAutoUIGetTimeMs();

    if((LcdAutoUIGetTimeMs() - lMs) > 1000)
    {
        lMs = LcdAutoUIGetTimeMs();

        RefreshAllTemperature();

        RefreshCurSwiIconStatus();

        RefreshHeatingStatus();

        RefreshCurPrintFeedrate();

        AppWaitForPreheating();
    }
}

/**
 * [LcdAutoUIStruct_t::AutoUIAppPrintPauseFlow :flow of 'DEVSTA_APP_PRTPAUSE' status]
 * @Author Creality
 * @Time   2021-11-19
 */
void LcdAutoUIStruct_t::AutoUIAppPrintPauseFlow(void)
{
    /* record the count of no-command-in-queue */
    static uint8_t lNoCmdInQueueCnt = 0;

    switch(flowStepCnt.fscAppPrintPause)
    {
        case 0:
            /* device is going home */
            if(GetStaGoingHome() == GO_HOME_DOING)
                break;

            pauseType = PAUSE_MANUAL_APP;

            /* material check */
            #if ENABLE_MATERIAL_CHECK
            if(!IsExistMaterial())
                pauseType = PAUSE_NO_MAT_APP;
            #endif

            /* door open check */
            #if ENABLE_DOOR_OPEN_CHECK
            if(enableSwitches.swiDoorPause && IsDoorOpen())
                pauseType = PRINT_DOOR_OPEN_APP;
            #endif

            if(!planner.has_blocks_queued())
            {
                /* be sure that there is no commands in queue */
                if(lNoCmdInQueueCnt < 3)
                {
                    lNoCmdInQueueCnt ++;
                    break;
                }
                else
                {
                    lNoCmdInQueueCnt = 0;
                }

                /* record current position */
                pauseStaRecord.posPauseBefore = current_position;

                /* get to pause position */
                switch(pauseType)
                {
                    case PAUSE_MANUAL_APP:
                        LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                        break;

                    /* material check */
                    #if ENABLE_MATERIAL_CHECK
                    case PAUSE_NO_MAT_APP:
                        LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
                        LcdAutoUIMoveY(0, AXIS_MOVE_ABSO);
                        /* record current tick */
                        evtRepMatToConPriTmr = LcdAutoUIGetTimeMs();
                        break;
                    #endif

                    /* door open check */
                    #if ENABLE_DOOR_OPEN_CHECK
                    case PRINT_DOOR_OPEN_APP:
                        if(enableSwitches.swiDoorPause)
                        {
                            // LcdAutoUIMoveZ(current_position.z + PAUSE_POS_Z_HEIGHT_DIF, AXIS_MOVE_ABSO);
                            // LcdAutoUIMoveX(0, AXIS_MOVE_ABSO);
                            // LcdAutoUIMoveY(Y_BED_SIZE, AXIS_MOVE_ABSO);
                        }
                        break;
                    #endif

                    default:
                        break;
                }
                flowStepCnt.fscAppPrintPause ++;
            }
            break;

        case 1:
            if((pauseType == 1) && (planner.has_blocks_queued()))
                break;

            switch(pauseType)
            {
                #if 0
                case PAUSE_MANUAL_APP:
                    SwitchBackgroundPic(AUTOUI_PRINTPAUSE);
                    break;
                #endif

                /* material check */
                #if ENABLE_MATERIAL_CHECK
                case PAUSE_NO_MAT_APP:
                    gLcdAutoUI.SwitchBackgroundPic(AUTOUI_LACKMATPW);
                    /* start close-heater-timer */
                    CloHeaTmrStart(CLOHEATMR_TIME);
                    /* out material */
                    // LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS1, FEEDING_DEF_OUT_MAT_SPE1);
                    // LcdAutoUIMoveE(FEEDING_DEF_OUT_MAT_DIS2, FEEDING_DEF_OUT_MAT_SPE2);
                    break;
                #endif

                /* door open check */
                #if ENABLE_DOOR_OPEN_CHECK
                case PRINT_DOOR_OPEN_APP:
                    if(enableSwitches.swiDoorPause)
                    {
                        gLcdAutoUI.SwitchBackgroundPic(AUTOUI_PRINTDOOROPEN);
                        gLcdAutoUI.SwitchIcon(VAULE_INVALIDE_8BIT, ICONVAR_ADDR_DOOR_OPEN_TIP);
                    }
                    break;
                #endif

                default:
                    break;
            }

            flowStepCnt.fscAppPrintPause ++;
            break;

        default:
            break;
    }
}

/**
 * [AutoUIMainProcess :AutoUI main process]
 * @Author Creality
 * @Time   2021-05-28
 */
void LcdAutoUIStruct_t::AutoUIMainProcess(void)
{
    /* sd card status check and refresh gcode file name */
    SDCardUpdate();
    /* response to DWIN-touch */
    AutoUIResponseToDWIN();
    /* process of close-heater-timer */
    CloHeaTmrProcess();
    /* errors check */
    ErrorsCheckProcess();
    /* process of handling event that SD card lost at printing */
    SDCardLostAtPrintingProcess();
    /* material check */
    IsExistMaterial();
    /* refresh wifi board MAC */
    RefreshWifiMacAddress();
    /* Synchronize currrent position display with App */
    SynchronizeCurPosWithApp();
    /* moving axis according to set */
    RefreshCurPosAccordingToSet();
    /* delay to stop print */
    AutoUIPrintStopDelay();
    /* check: waiting going home to continue printing when having replaced material */
    AutoUINoMatRecoveryProcess();
	/* time to notify WIFI board that there is no material when device is at app-printing */
    AutoUINoMatAppPrintProcess();
    /* time to notify WIFI board that door is open when device is at app-printing */
    AutoUIDoorOpenAppPrintProcess();
    /* delay to change device status */
    AutoUIDelayToStatus(devStaCon.dscDevStaRecord);

    switch(devStatus)
    {
        case DEVSTA_INIT:
            AutoUIInit();
            break;

        case DEVSTA_STARTUP_FIR:
            AutoUIStartupFirstTimeFlow();
            break;

        case DEVSTA_IDLE:
            AutoUIIdleFlow();
            break;

        case DEVSTA_HEATING:
            AutoUIHeatFlow();
            break;

        case DEVSTA_IN_OUT_MAT:
            AutoUIInOutMatFlow();
            break;

        case DEVSTA_PRINTING:
            AutoUIPrintingFlow();
            break;

        case DEVSTA_PRINT_PAUSE:
            AutoUIPrintPauseFlow();
            break;

        case DEVSTA_BED_CALI:
            AutoUIBedCaliFlow();
            break;

        case DEVSTA_ERROR:

            break;

        case DEVSTA_APP_PRINTING:
            AutoUIAppPrintingFlow();
            break;

        case DEVSTA_APP_PRTHEAT:
            AutoUIAppPrintHeatFlow();
            break;

        case DEVSTA_APP_PRTPAUSE:
            AutoUIAppPrintPauseFlow();
            break;
        default:
            break;
    }
}
