/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

// Source: https://github.com/stm32duino/Arduino_Core_STM32/blob/master/variants/ST3DP001_EVAL/variant.cpp

/**
 * HOW TO COMPILE
 *
 * PlatformIO - Use the STM32F401VE_STEVAL environment (or the "Auto Build Marlin" extension).
 *
 * Arduino - Tested with 1.8.10
 *      Install library per https://github.com/stm32duino/Arduino_Core_STM32
 *      Make the following selections under the TOOL menu in the Arduino IDE
 *          Board: "3D printer boards"
 *          Board part number: "STEVAL-3DP001V1"
 *          U(S)ART support: "Enabled (generic "Serial")"
 *          USB support (if available): "CDC (no generic "Serial")"
 *          Optimize: "Smallest (-Os default)"
 *          C Runtime Library: "newlib Nano (default)"
 */

#pragma once

#ifndef STM32F4
  #error "Oops! Select an STM32F4 board in 'Tools > Board.'"
#endif

#define BOARD_INFO_NAME "Sermoon V1"

#ifndef MACHINE_NAME
  #define MACHINE_NAME "CREALITY_F401"
#endif

//
// EEPROM
//

#if NO_EEPROM_SELECTED
  // FLASH
  // #define FLASH_EEPROM_EMULATION
  // #define SDCARD_EEPROM_EMULATION
  // #define USE_WIRED_EEPROM
  // I2C
  #define IIC_BL24CXX_EEPROM                      // EEPROM on I2C-0 used only for display settings
  #if ENABLED(IIC_BL24CXX_EEPROM)
    #define IIC_EEPROM_SDA                  PA11
    #define IIC_EEPROM_SCL                  PA12
    #define MARLIN_EEPROM_SIZE             0x800  // 2Kb (24C16)
  #else
    #define SDCARD_EEPROM_EMULATION               // SD EEPROM until all EEPROM is BL24CXX
    #define MARLIN_EEPROM_SIZE             0x800  // 2Kb
  #endif

  // SPI
  //#define SPI_EEPROM                            // EEPROM on SPI-0
  //#define SPI_CHAN_EEPROM1  ?
  //#define SPI_EEPROM1_CS    ?

  // 2K EEPROM
  //#define SPI_EEPROM2_CS    ?

  // 32Mb FLASH
  //#define SPI_FLASH_CS      ?
#endif


//
// Limit Switches
//
#define X_MAX_PIN          PC4
#define Y_MIN_PIN          PB13

#ifdef BLTOUCH
  #define Z_MIN_PIN        PC3  // BLTouch IN PIN
  #define SERVO0_PIN       PC2  // BLTouch OUT PIN
#elif ENABLED(FIX_MOUNTED_PROBE)
  #define Z_MIN_PIN        PA4
  #define COM_PIN          PA5
#else
  #define Z_MAX_PIN        PB3
#endif

//
// Steppers
//
#define X_ENABLE_PIN       PB8
#define X_STEP_PIN         PA7
#define X_DIR_PIN          PA4

#define Y_ENABLE_PIN       PB8
#define Y_STEP_PIN         PB0
#define Y_DIR_PIN          PB10

#define Z_ENABLE_PIN       PB8
#define Z_STEP_PIN         PB7
#define Z_DIR_PIN          PB6

#define E0_ENABLE_PIN      PB8
#define E0_STEP_PIN        PB1
#define E0_DIR_PIN         PB12


#if HAS_TMC220x

  //
  // TMC2208 mode
  //
  // #define TMC2208_STANDALONE

  #define X_HARDWARE_SERIAL  MSerial2
  #define Y_HARDWARE_SERIAL  MSerial2
  #define Z_HARDWARE_SERIAL  MSerial2
  #define E0_HARDWARE_SERIAL MSerial2

  //
  // TMC2208 Software serial
  //
  // #define HAVE_SW_SERIAL

  // Reduce baud rate to improve software serial reliability
  // #define TMC_BAUD_RATE 19200

#endif
//
// Temperature Sensors
//
#define TEMP_0_PIN                         PC1   // TH1
#define TEMP_BED_PIN                       PC0   // TB1

//
// Heaters / Fans
//
#define HEATER_0_PIN                        PC5  // HEATER1
#define HEATER_BED_PIN                      PB9   // HOT BED

#define FAN_PIN                             PA5   // FAN
#define FAN_SOFT_PWM
//
// Onboard SD support
//
//
// SD Card
//
#define SD_DETECT_PIN                       PC7
#define SDCARD_CONNECTION                  ONBOARD
#define ON_BOARD_SPI_DEVICE                  1
// #define ONBOARD_SD_CS_PIN                   PA4   // SDSS
#define SDIO_SUPPORT
#define NO_SD_HOST_DRIVE

#define SDIO_D0_PIN                           PC8  // PC8   SDIO_D0
#define SDIO_D1_PIN                           PC9  // PC9   SDIO_D1
//#define SD_CARD_DETECT_PIN                    PC7  // PC7  SD_CARD_DETECT
#define SDIO_D2_PIN                           PC10  // PC10  SDIO_D2
#define SDIO_D3_PIN                           PC11  // PC11  SDIO_D3
#define SDIO_CK_PIN                           PC12  // PC12  SDIO_CK
#define SDIO_CMD_PIN                          PD2   // PD2   SDIO_CMD

#ifndef SDCARD_CONNECTION
  #define SDCARD_CONNECTION              ONBOARD
#endif

#if SD_CONNECTION_IS(ONBOARD)
  #define SDIO_SUPPORT                            // Use SDIO for onboard SD

  #ifndef SDIO_SUPPORT
    #define SOFTWARE_SPI                          // Use soft SPI for onboard SD
    #undef SDSS
    #define SDSS                     SDIO_D3_PIN
    #define SCK_PIN                  SDIO_CK_PIN
    #define MISO_PIN                 SDIO_D0_PIN
    #define MOSI_PIN                SDIO_CMD_PIN
  #endif
#endif

// #define RET6_12864_LCD
#if ENABLED(CR10_STOCKDISPLAY) && NONE(RET6_12864_LCD, VET6_12864_LCD)
  #error "Define RET6_12864_LCD or VET6_12864_LCD to select pins for CR10_STOCKDISPLAY with the Creality V4 controller."
#endif

#if ENABLED(RET6_12864_LCD)

  // RET6 12864 LCD
  #define LCD_PINS_RS                       PB12
  #define LCD_PINS_ENABLE                   PB15
  #define LCD_PINS_D4                       PB13

  #define BTN_ENC                           PB2
  #define BTN_EN1                           PB10
  #define BTN_EN2                           PB14

  #define BEEPER_PIN                        PC6

#elif ENABLED(VET6_12864_LCD)

  // VET6 12864 LCD
  #define LCD_PINS_RS                       PA4
  #define LCD_PINS_ENABLE                   PA7
  #define LCD_PINS_D4                       PA5

  #define BTN_ENC                           PC5
  #define BTN_EN1                           PB10
  #define BTN_EN2                           PA6

#elif ENABLED(DWIN_CREALITY_LCD)

  // RET6 DWIN ENCODER LCD
  #define BTN_ENC                           PB14
  #define BTN_EN1                           PB15
  #define BTN_EN2                           PB12

  //#define LCD_LED_PIN                     PB2
  #ifndef BEEPER_PIN
    #define BEEPER_PIN                      PB13
    #undef SPEAKER
  #endif

#elif ENABLED(DWIN_VET6_CREALITY_LCD)

  // VET6 DWIN ENCODER LCD
  #define BTN_ENC                           PA6
  #define BTN_EN1                           PA7
  #define BTN_EN2                           PA4

  #define BEEPER_PIN                        PA5

#endif
 
/* SD card detect */
#define SD_DETECT_PIN      PC7

#define LED_CONTROL_PIN    PC14

#define CHECK_MATWEIAL     PC6

// Box Fan
#define BOX_FAN_PIN        PC15

#if ENABLE_DOOR_OPEN_CHECK
  /* Open the door to detect (is high) */
  #define CHECK_DOOR_PIN      PB14 // (J24 - RSV3)
#endif
