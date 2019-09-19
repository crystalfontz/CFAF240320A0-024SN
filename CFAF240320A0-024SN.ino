//==============================================================================
//
//  CRYSTALFONTZ CFAF240320A0-024SN
//
//  Code written for Seeeduino v4.2 set to 3.3v (important!)
//
//  This code uses the 4-wire SPI mode of the display.
//
//  The controller is a Sitronix ST7789V:
//    https://www.crystalfontz.com/controllers/Sitronix/ST7789V
//
//  Seeeduino v4.2, an open-source 3.3v capable Arduino clone.
//    https://www.seeedstudio.com/Seeeduino-V4.2-p-2517.html
//    https://github.com/SeeedDocument/SeeeduinoV4/raw/master/resources/Seeeduino_v4.2_sch.pdf
//
//==============================================================================
//
//  2017-10-02 Brent A. Crosby / Crystalfontz
//  2018-08-22 Max Roberg / Crystalfontz
//  2019-09-09 Mark Williams / Crystalfontz
//
//==============================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//==============================================================================

//========= CONFIGURATION OPTIONS
#define WAIT_TIME		2000
#define DEMO_FONT		1
#define DEMO_LINES		1
#define DEMO_CIRCLES	1
#define DEMO_EXPANDING	1
#define DEMO_CHECKER	1
#define DEMO_BMPIMAGES	0 /* enabling also enables use of SD card */

//BMP images must be save in the root directory of the SD card.
//They must be exactly 240x320 pixels in size, and in 24-bit colour (3 bytes per pixel)

// DISPLAY SPI FREQUENCY
// try lowering this if you have display init or corruption issues
// default is 20Mhz
#define SPI_FREQ 20000000

//==============================================================================

#include <Arduino.h>
#include <SPI.h>
#include "font-ascii-12x16.h"

#if DEMO_BMPIMAGES
#include <SD.h>
//this uses the Adafruit SD Arduino library
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.cpp
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.h
#endif

//==============================================================================
// LCD SPI & control lines
//        
// -------------------------------------------------------------------------------
//    LCD Control Lines
// -----------+-------+-----------------------------------------------------------
//            |       |          4-WIRE SPI                     |
// -----------+-------+-----------------------------------------+-----------------
//   ARD      | Port  | LCD Pins    | Pin Function              |
// -----------+-------+-------------+---------------------------+-----------------
//  3.3V      |       | #3, 39      | POWER (+)                 |
//  GND       |       | #25, 27, 40 | POWER (-)                 |
//  #8/D8     |  PB0  | #32         | LCD_RS                    |
//  #9/D9     |  PB1  | #35         | LCD_RESET                 |
// #10/D10    |  PB2  | #34         | LCD_CS_NOT (or SPI SS)    |
// #11/D11    |  PB3  | #24         | LCD_MOSI   (hardware SPI) |
// #13/D13    |  PB5  | #33         | LCD_SCK    (hardware SPI) |
// GND        |       | #36         | LCD_IM0                   |
// 3.3V       |       | #37         | LCD_IM1                   |
// 3.3V       |       | #38         | LCD_IM2                   |
//
//
// -------------------------------------------------------------------------------
//    SD Control Lines
// -----------+-------+-------------+---------------------------------------------
//   ARD      | Port  | SD Pins     | Pin Function
// -----------+-------+-------------+---------------------------------------------
//  3.3V      |       |             | POWER (+)
//  GND       |       |             | POWER (-)
//  #7/D7     |  PD7  |             | SD_CS
// #11/D11    |  PB3  |             | SD_MOSI   (hardware SPI)
// #12/D12    |  PB3  |             | SD_MISO   (hardware SPI)
// #13/D13    |  PB5  |             | SD_SCK    (hardware SPI)
//==============================================================================

#define SPIPORT (PORTB)
#define SPITOGGLE (PINB)
// PB0 (0x01) is RS   (output) red    OLED pin 9
#define SPI_RS_PIN (8)
#define RS_MASK (0x01) // 0000 0001
#define CLR_RS (SPIPORT &= ~(RS_MASK))
#define SET_RS (SPIPORT |= (RS_MASK))
// PB1 (0x02) is RESET  (output) yellow OLED pin 7
#define SPI_RESET_PIN (9)
#define RESET_MASK (0x02) // 0000 0010
#define CLR_RESET (SPIPORT &= ~(RESET_MASK))
#define SET_RESET (SPIPORT |= (RESET_MASK))
// PB2 (0x04) is CS   (output) gray   OLED pin 15
#define SPI_CS_PIN (10)
#define CS_MASK (0x04) // 0000 0100
#define CLR_CS (SPIPORT &= ~(CS_MASK))
#define SET_CS (SPIPORT |= (CS_MASK))
// PB3 (0x08) is MOSI (output) violet OLED pin 14
#define SPI_MOSI_PIN (11)
#define MOSI_MASK (0x08) // 0000 1000
#define CLR_MOSI (SPIPORT &= ~(MOSI_MASK))
#define SET_MOSI (SPIPORT |= (MOSI_MASK))
// PB4 (0x10) is MISO (input)  blue   OLED pin 13
//(reference only, it is an input)
#define SPI_MISO_PIN (12)
#define MISO_MASK (0x10) // 0001 0000
#define CLR_MISO (SPIPORT &= ~(MISO_MASK))
#define SET_MISO (SPIPORT |= (MISO_MASK))
// PB5 (0x20) is CLK  (output) green  OLED pin 12
#define SPI_CLK_PIN (13)
#define CLK_MASK (0x20) // 0010 0000
#define CLR_CLK (SPIPORT &= ~(CLK_MASK))
#define SET_CLK (SPIPORT |= (CLK_MASK))

//==============================================================================
void SPI_sendCommand(uint8_t cmd)
{
	// Select the LCD controller
	CLR_CS;
	//Send the command via SPI:
	CLR_RS;
	SPI.transfer(cmd);
	// Deselect the LCD controller
	SET_CS;
}
//==============================================================================
void SPI_sendData(uint8_t data)
{
	// Select the LCD controller
	CLR_CS;
	//Send the data via SPI:
	SET_RS;
	SPI.transfer(data);
	// Deselect the LCD controller
	SET_CS;
}
//==============================================================================
// Defines for the ST7789 registers.
// ref: https://www.crystalfontz.com/controllers/Sitronix/ST7789V/
#define ST7789_00_NOP (0x00)
#define ST7789_01_SWRESET (0x01)
#define ST7789_04_RDDID (0x04)
#define ST7789_09_RDDST (0x09)
#define ST7789_0A_RDDPM (0x0A)
#define ST7789_0B_RDDMADCTL (0x0B)
#define ST7789_0C_RDDCOLMOD (0x0C)
#define ST7789_0D_RDDIM (0x0D)
#define ST7789_0E_RDDSM (0x0E)
#define ST7789_0F_RDDSDR (0x0F)
#define ST7789_10_SLPIN (0x10)
#define ST7789_11_SLPOUT (0x11)
#define ST7789_12_PTLON (0x12)
#define ST7789_13_NORON (0x13)
#define ST7789_20_INVOFF (0x20)
#define ST7789_21_INVON (0x21)
#define ST7789_26_GAMSET (0x26)
#define ST7789_28_DISPOFF (0x28)
#define ST7789_29_DISPON (0x29)
#define ST7789_2A_CASET (0x2A)
#define ST7789_2B_RASET (0x2B)
#define ST7789_2C_RAMWR (0x2C)
#define ST7789_2E_RAMRD (0x2E)
#define ST7789_30_PTLAR (0x30)
#define ST7789_33_VSCRDEF (0x33)
#define ST7789_34_TEOFF (0x34)
#define ST7789_35_TEON (0x35)
#define ST7789_36_MADCTL (0x36)
#define ST7789_37_VSCRSADD (0x37)
#define ST7789_38_IDMOFF (0x38)
#define ST7789_39_IDMON (0x39)
#define ST7789_3A_COLMOD (0x3A)
#define ST7789_3C_RAMWRC (0x3C)
#define ST7789_3E_RAMRDC (0x3E)
#define ST7789_44_TESCAN (0x44)
#define ST7789_45_RDTESCAN (0x45)
#define ST7789_51_WRDISBV (0x51)
#define ST7789_52_RDDISBV (0x52)
#define ST7789_53_WRCTRLD (0x53)
#define ST7789_54_RDCTRLD (0x54)
#define ST7789_55_WRCACE (0x55)
#define ST7789_56_RDCABC (0x56)
#define ST7789_5E_WRCABCMB (0x5E)
#define ST7789_5F_RDCABCMB (0x5F)
#define ST7789_68_RDABCSDR (0x68)
#define ST7789_B0_RAMCTRL (0xB0)
#define ST7789_B1_RGBCTRL (0xB1)
#define ST7789_B2_PORCTRL (0xB2)
#define ST7789_B3_FRCTRL1 (0xB3)
#define ST7789_B7_GCTRL (0xB7)
#define ST7789_BA_DGMEN (0xBA)
#define ST7789_BB_VCOMS (0xBB)
#define ST7789_C0_LCMCTRL (0xC0)
#define ST7789_C1_IDSET (0xC1)
#define ST7789_C2_VDVVRHEN (0xC2)
#define ST7789_C3_VRHS (0xC3)
#define ST7789_C4_VDVSET (0xC4)
#define ST7789_C5_VCMOFSET (0xC5)
#define ST7789_C6_FRCTR2 (0xC6)
#define ST7789_C7_CABCCTRL (0xC7)
#define ST7789_C8_REGSEL1 (0xC8)
#define ST7789_CA_REGSEL2 (0xCA)
#define ST7789_CC_PWMFRSEL (0xCC)
#define ST7789_D0_PWCTRL1 (0xD0)
#define ST7789_D2_VAPVANEN (0xD2)
#define ST7789_DA_RDID1 (0xDA)
#define ST7789_DB_RDID2 (0xDB)
#define ST7789_DC_RDID3 (0xDC)
#define ST7789_DF_CMD2EN (0xDF)
#define ST7789_E0_PVGAMCTRL (0xE0)
#define ST7789_E1_NVGAMCTRL (0xE1)
#define ST7789_E2_DGMLUTR (0xE2)
#define ST7789_E3_DGMLUTB (0xE3)
#define ST7789_E4_GATECTRL (0xE4)
#define ST7789_E8_PWCTRL2 (0xE8)
#define ST7789_E9_EQCTRL (0xE9)
#define ST7789_EC_PROMCTRL (0xEC)
#define ST7789_FA_PROMEN (0xFA)
#define ST7789_FC_NVMSET (0xFC)
#define ST7789_FE_PROMACT (0xFE)
//==============================================================================
void Initialize_LCD(void)
{
	//Reset the LCD controller
	CLR_RESET;
	delay(1); //10µS min
	SET_RESET;
	delay(150); //120mS max

	// SLPOUT (11h): Sleep Out ("Sleep Out"  is chingrish for "wake")
	// The DC/DC converter is enabled, Internal display oscillator
	// is started, and panel scanning is started.
	SPI_sendCommand(ST7789_11_SLPOUT);
	delay(120); //Delay 120ms

	// MADCTL (36h): Memory Data Access Control
	// Set the RGB vs BGR order to match a windows 24-bit BMP
	SPI_sendCommand(ST7789_36_MADCTL);
	SPI_sendData(0x08 | 0x80);
						// |||| ||||-- Unused: 0
						// |||| ||---- MH: Horizontal Refresh Order
						// |||| |        0 = left to right
						// |||| |        1 = right to left
						// |||| |----- RGB: RGB vs BGR Order
						// ||||          0 = RGB color filter panel
						// ||||          1 = BGR color filter panel
						// ||||------- ML: Vertical Refresh Order
						// |||           0 = top to bottom
						// |||           1 = bottom to top
						// |||-------- MV: Row / Column Exchange
						// ||--------- MX: Column Address Order  <<<<<
						// |---------- MY: Row Address Order

	// COLMOD (3Ah): Interface Pixel Format
	// * This command is used to define the format of RGB picture
	//   data, which is to be transferred via the MCU interface.
	SPI_sendCommand(ST7789_3A_COLMOD);
	SPI_sendData(0x06); // Default: 0x06 => 18-bit/pixel
						// IFPF[2:0] MCU Interface Color Format
						// IFPF[2:0] | Format
						//      000b | reserved
						//      001b | reserved
						//      010b | reserved
						//      011b | 12-bit/pixel
						//      100b | reserved
						//      101b | 16-bit/pixel
						//      110b | 18-bit/pixel   <<<<<
						//      111b | reserved

	// PORCTRL (B2h): Porch Setting
	SPI_sendCommand(ST7789_B2_PORCTRL);
	SPI_sendData(0x0C); //BPA[6:0]: Back porch setting in normal mode. The minimum setting is 0x01.
	SPI_sendData(0x0C); //FPA[6:0]: Front porch setting in normal mode. The minimum setting is 0x01.
	SPI_sendData(0x00); //Disable(0) / Enable (1) separate porch control
	SPI_sendData(0x33); //(high nibble) BPB[3:0]: Back porch setting in idle mode. The minimum setting is 0x01.
						//(low nibble)  FPB[3:0]: Front porch setting in idle mode. The minimum setting is 0x01.
	SPI_sendData(0x33); //(high nibble) BPB[3:0]: Back porch setting in partial mode. The minimum setting is 0x01.
						//(low nibble)  FPC[3:0]: Front porch setting in partial mode. The minimum setting is 0x01.

	// GCTRL (B7h): Gate Control
	SPI_sendCommand(ST7789_B7_GCTRL);
	SPI_sendData(0x35);

	//(high nibble) VGHS[2:0]: VGH Setting.
	// 0x0- => 12.20v
	// 0x1- => 12.54v
	// 0x2- => 12.89v
	// 0x3- => 13.26v <<<<<
	// 0x4- => 13.65v
	// 0x5- => 14.06v
	// 0x6- => 14.50v
	// 0x7- => 14.97v
	//(low nibble) VGLS[2:0]: VGL Setting.
	// 0x-0 =>  -7.16v
	// 0x-1 =>  -7.67v
	// 0x-2 =>  -8.23v
	// 0x-3 =>  -8.87v
	// 0x-4 =>  -9.60v
	// 0x-5 => -10.43v <<<<<
	// 0x-6 => -11.38v
	// 0x-7 => -12.50v

	// VCOMS (BBh): VCOM Setting
	SPI_sendCommand(ST7789_BB_VCOMS);
	SPI_sendData(0x2B);

	// VCOMS[5:0]
	// 0x00 => 0.100v
	// 0x01 => 0.125v
	// 0x02 => 0.150v
	// 0x03 => 0.175v
	// 0x04 => 0.200v
	// 0x05 => 0.225v
	// 0x06 => 0.250v
	// 0x07 => 0.275v
	// 0x08 => 0.300v
	// 0x09 => 0.325v
	// 0x0A => 0.350v
	// 0x0B => 0.375v
	// 0x0C => 0.400v
	// 0x0D => 0.425v
	// 0x0E => 0.450v
	// 0x0F => 0.475v
	// 0x10 => 0.500v
	// 0x11 => 0.525v
	// 0x12 => 0.550v
	// 0x13 => 0.575v
	// 0x14 => 0.600v
	// 0x15 => 0.625v
	// 0x16 => 0.650v
	// 0x17 => 0.675v
	// 0x18 => 0.700v
	// 0x19 => 0.725v
	// 0x1A => 0.750v
	// 0x1B => 0.775v
	// 0x1C => 0.800v <<<< V
	// 0x1D => 0.825v
	// 0x1E => 0.850v
	// 0x1F => 0.875v
	// 0x20 => 0.900v
	// 0x21 => 0.925v
	// 0x22 => 0.950v
	// 0x23 => 0.975v
	// 0x24 => 1.000v
	// 0x25 => 1.025v
	// 0x26 => 1.050v
	// 0x27 => 1.075v
	// 0x28 => 1.100v
	// 0x29 => 1.125v
	// 0x2A => 1.150v
	// 0x2B => 1.175v <<<< W
	// 0x2C => 1.200v
	// 0x2D => 1.225v
	// 0x2E => 1.250v
	// 0x2F => 1.275v
	// 0x30 => 1.300v
	// 0x31 => 1.325v
	// 0x32 => 1.350v
	// 0x33 => 1.375v
	// 0x34 => 1.400v
	// 0x35 => 1.425v <<<< X
	// 0x36 => 1.450v
	// 0x37 => 1.475v
	// 0x38 => 1.500v
	// 0x39 => 1.525v
	// 0x3A => 1.550v
	// 0x3B => 1.575v
	// 0x3C => 1.600v
	// 0x3D => 1.625v
	// 0x3E => 1.650v
	// 0x3F => 1.675v

	// LCMCTRL (C0h): LCM Control
	SPI_sendCommand(ST7789_C0_LCMCTRL);
	SPI_sendData(0x2C);
	// 0010 1100
	// |||| ||||--  GS: Gate scan inversion enable: 1: enable, 0: disable.
	// |||| |||---  XMX: XOR MX setting in command 36h.
	// |||| ||---- *XMV: XOR MV setting in command 36h.
	// |||| |----- *XMH: XOR RGB setting in command 36h, in RGB interface
	// ||||             without RAM mode can support column address order.
	// ||||-------  XREV: XOR inverse setting in command 21h.
	// |||-------- *XBGR: XOR RGB setting in command 36h.
	// ||---------  XMY: XOR MY setting in command 36h.
	// |----------  Unused: 0

	// VDVVRHEN (C2h): VDV and VRH Command Enable
	SPI_sendCommand(ST7789_C2_VDVVRHEN);
	SPI_sendData(0x01); // CMDEN=”0”: VDV and VRH register value comes from NVM.
						// CMDEN=”1”, VDV and VRH register value comes from command write.
	SPI_sendData(0xFF);

	// VRHS (C3h): VRH Set
	SPI_sendCommand(ST7789_C3_VRHS);
	SPI_sendData(0x20);

	// 0x00 => VAP(GVDD)(V) = +3.55 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) = -3.55 + (vcom + vcom_offset - 0.5vdv)
	// 0x01 => VAP(GVDD)(V) = +3.60 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) = -3.60 + (vcom + vcom_offset - 0.5vdv)
	// 0x02 => VAP(GVDD)(V) = +3.65 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) = -3.65 + (vcom + vcom_offset - 0.5vdv)
	// 0x03 => VAP(GVDD)(V) = +3.70 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.70 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x04 => VAP(GVDD)(V) = +3.75 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.75 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x05 => VAP(GVDD)(V) = +3.80 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.80 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x06 => VAP(GVDD)(V) = +3.85 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.85 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x07 => VAP(GVDD)(V) = +3.90 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.90 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x08 => VAP(GVDD)(V) = +3.95 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 3.95 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x09 => VAP(GVDD)(V) = +4.00 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.00 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x0A => VAP(GVDD)(V) = +4.05 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.05 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x0B => VAP(GVDD)(V) = +4.10 + (vcom + vcom_offset + 0.5vdv) <<<< V
	//         VAN(GVCL)(V) =- 4.10 + (vcom + vcom_offset-+ 0.5vdv) <<<< V
	// 0x0C => VAP(GVDD)(V) = +4.15 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.15 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x0D => VAP(GVDD)(V) = +4.20 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.20 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x0E => VAP(GVDD)(V) = +4.25 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.25 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x0F => VAP(GVDD)(V) = +4.30 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.30 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x10 => VAP(GVDD)(V) = +4.35 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.35 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x11 => VAP(GVDD)(V) = +4.40 + (vcom + vcom_offset + 0.5vdv) <<<< X
	//         VAN(GVCL)(V) =- 4.40 + (vcom + vcom_offset-+ 0.5vdv) <<<< X
	// 0x12 => VAP(GVDD)(V) = +4.45 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.45 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x13 => VAP(GVDD)(V) = +4.50 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.50 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x14 => VAP(GVDD)(V) = +4.55 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.55 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x15 => VAP(GVDD)(V) = +4.60 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.60 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x16 => VAP(GVDD)(V) = +4.65 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.65 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x17 => VAP(GVDD)(V) = +4.70 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.70 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x18 => VAP(GVDD)(V) = +4.75 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.75 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x19 => VAP(GVDD)(V) = +4.80 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.80 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1A => VAP(GVDD)(V) = +4.85 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.85 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1B => VAP(GVDD)(V) = +4.90 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.90 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1C => VAP(GVDD)(V) = +4.95 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 4.95 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1D => VAP(GVDD)(V) = +5.00 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.00 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1E => VAP(GVDD)(V) = +5.05 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.05 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x1F => VAP(GVDD)(V) = +5.10 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.10 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x20 => VAP(GVDD)(V) = +5.15 + (vcom + vcom_offset + 0.5vdv) <<<< W
	//         VAN(GVCL)(V) =- 5.15 + (vcom + vcom_offset-+ 0.5vdv) <<<< W
	// 0x21 => VAP(GVDD)(V) = +5.20 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.20 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x22 => VAP(GVDD)(V) = +5.25 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.25 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x23 => VAP(GVDD)(V) = +5.30 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.30 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x24 => VAP(GVDD)(V) = +5.35 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.35 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x25 => VAP(GVDD)(V) = +5.40 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.40 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x26 => VAP(GVDD)(V) = +5.45 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.45 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x27 => VAP(GVDD)(V) = +5.50 + (vcom + vcom_offset + 0.5vdv)
	//         VAN(GVCL)(V) =- 5.50 + (vcom + vcom_offset-+ 0.5vdv)
	// 0x28~0x3Fh => Reserved

	//VDVS (C4h): VDV Set
	SPI_sendCommand(ST7789_C4_VDVSET);
	SPI_sendData(0x20);
	// 0x00 => -0.800
	// 0x01 => -0.775
	// 0x02 => -0.750
	// 0x03 => -0.725
	// 0x04 => -0.700
	// 0x05 => -0.675
	// 0x06 => -0.650
	// 0x07 => -0.625
	// 0x08 => -0.600
	// 0x09 => -0.575
	// 0x0A => -0.550
	// 0x0B => -0.525
	// 0x0C => -0.500
	// 0x0D => -0.475
	// 0x0E => -0.450
	// 0x0F => -0.425
	// 0x10 => -0.400
	// 0x11 => -0.375
	// 0x12 => -0.350
	// 0x13 => -0.325
	// 0x14 => -0.300
	// 0x15 => -0.275
	// 0x16 => -0.250
	// 0x17 => -0.225
	// 0x18 => -0.200
	// 0x19 => -0.175
	// 0x1A => -0.150
	// 0x1B => -0.125
	// 0x1C => -0.100
	// 0x1D => -0.075
	// 0x1E => -0.050
	// 0x1F => -0.025
	// 0x20 => +0.000 <<<<<
	// 0x21 => +0.025
	// 0x22 => +0.050
	// 0x23 => +0.075
	// 0x24 => +0.100
	// 0x25 => +0.125
	// 0x26 => +0.150
	// 0x27 => +0.175
	// 0x28 => +0.200
	// 0x29 => +0.225
	// 0x2A => +0.250
	// 0x2B => +0.275
	// 0x2C => +0.300
	// 0x2D => +0.325
	// 0x2E => +0.350
	// 0x2F => +0.375
	// 0x30 => +0.400
	// 0x31 => +0.425
	// 0x32 => +0.450
	// 0x33 => +0.475
	// 0x34 => +0.500
	// 0x35 => +0.525
	// 0x36 => +0.550
	// 0x37 => +0.575
	// 0x38 => +0.600
	// 0x39 => +0.625
	// 0x3A => +0.650
	// 0x3B => +0.675
	// 0x3C => +0.700
	// 0x3D => +0.725
	// 0x3E => +0.750
	// 0x3F => +0.775

	//FRCTRL2 (C6h): Frame Rate Control in Normal Mode
	SPI_sendCommand(ST7789_C6_FRCTR2);
	SPI_sendData(0x0F);
	// 0000 1111
	// ||||-||||--  RTNA[4:0]
	// |||--------  NLA[2:0] : Inversion selection in normal mode
	//                000 = dot inversion
	//                111 = column inversion
	// RTNA[4:0] => FR in normal mode (Hz)
	// 0x00 => 119 Hz
	// 0x01 => 111 Hz
	// 0x02 => 105 Hz
	// 0x03 =>  99 Hz
	// 0x04 =>  94 Hz
	// 0x05 =>  90 Hz
	// 0x06 =>  86 Hz
	// 0x07 =>  82 Hz
	// 0x08 =>  78 Hz
	// 0x09 =>  75 Hz
	// 0x0A =>  72 Hz
	// 0x0B =>  69 Hz
	// 0x0C =>  67 Hz
	// 0x0D =>  64 Hz
	// 0x0E =>  62 Hz
	// 0x0F =>  60 Hz <<<<<
	// 0x10 =>  58 Hz
	// 0x11 =>  57 Hz
	// 0x12 =>  55 Hz
	// 0x13 =>  53 Hz
	// 0x14 =>  52 Hz
	// 0x15 =>  50 Hz
	// 0x16 =>  49 Hz
	// 0x17 =>  48 Hz
	// 0x18 =>  46 Hz
	// 0x19 =>  45 Hz
	// 0x1A =>  44 Hz
	// 0x1B =>  43 Hz
	// 0x1C =>  42 Hz
	// 0x1D =>  41 Hz
	// 0x1E =>  40 Hz
	// 0x1F =>  39 Hz

	// PWCTRL1 (D0h): Power Control 1
	SPI_sendCommand(ST7789_D0_PWCTRL1);
	SPI_sendData(0xA4); //Fixed vector
	SPI_sendData(0xA1); //AVDD=6.8V, AVCL=-4.8V, VDS=2.3V
						// DDCC --VV
						// |||| ||||--  VDS[1:0]:
						// ||||           00 = 2.19v
						// ||||           01 = 2.30v <<<<<
						// ||||           10 = 2.40v
						// ||||           11 = 2.51v
						// ||||-------  AVCL[1:0]:
						// ||             00 = -4.4v
						// ||             01 = -4.6v
						// ||             10 = -4.8v <<<<<
						// ||             11 = -5.0v
						// ||---------  AVDD[1:0]:
						//                00 = 6.4v
						//                01 = 6.6v
						//                10 = 6.8v <<<<<
						//                11 = Reserved

	// PVGAMCTRL (E0h): Positive Voltage Gamma Control
	SPI_sendCommand(ST7789_E0_PVGAMCTRL);
	// Pile of magic numbers :-(
	SPI_sendData(0xD0);
	SPI_sendData(0xCA);
	SPI_sendData(0x0E);
	SPI_sendData(0x08);
	SPI_sendData(0x09);
	SPI_sendData(0x07);
	SPI_sendData(0x2D);
	SPI_sendData(0x3B);
	SPI_sendData(0x3D);
	SPI_sendData(0x34);
	SPI_sendData(0x0A);
	SPI_sendData(0x0A);
	SPI_sendData(0x1B);
	SPI_sendData(0x28);


	// NVGAMCTRL (E1h): Negative Voltage Gamma Control
	SPI_sendCommand(ST7789_E1_NVGAMCTRL);

	// Pile of magic numbers :-(
	SPI_sendData(0xD0);
	SPI_sendData(0xCA);
	SPI_sendData(0x0F);
	SPI_sendData(0x08);
	SPI_sendData(0x08);
	SPI_sendData(0x07);
	SPI_sendData(0x2E);
	SPI_sendData(0x5C);
	SPI_sendData(0x40);
	SPI_sendData(0x34);
	SPI_sendData(0x09);
	SPI_sendData(0x0B);
	SPI_sendData(0x1B);
	SPI_sendData(0x28);

	// INVON (21h): Display Inversion On (correct for W)
	SPI_sendCommand(ST7789_21_INVON);

	// CASET (2Ah): Column Address Set
	SPI_sendCommand(ST7789_2A_CASET);
	SPI_sendData(0x00); //Start MSB Start = 0
	SPI_sendData(0x00); //Start LSB
	SPI_sendData(0x00); //End MSB End = 249
	SPI_sendData(0xEF); //End LSB

	// RASET (2Bh): Row Address Set
	SPI_sendCommand(ST7789_2B_RASET);
	SPI_sendData(0x00); //Start MSB Start = 0
	SPI_sendData(0x00); //Start LSB
	SPI_sendData(0x01); //End MSB End = 319
	SPI_sendData(0x3F); //End LSB

	// DISPON (29h): Display On
	SPI_sendCommand(ST7789_29_DISPON);
	delay(1);
}
//==============================================================================
void Set_LCD_for_write_at_X_Y(uint16_t x, uint16_t y)
{
	//CASET (2Ah): Column Address Set
	// * The value of XS [15:0] and XE [15:0] are referred when RAMWR
	//   command comes.
	// * Each value represents one column line in the Frame Memory.
	// * XS [15:0] always must be equal to or less than XE [15:0]
	SPI_sendCommand(ST7789_2A_CASET); //Column address set
	//Write the parameters for the "column address set" command
	SPI_sendData(x >> 8);	 //Start MSB = XS[15:8]
	SPI_sendData(x & 0x00FF); //Start LSB = XS[ 7:0]
	SPI_sendData(0);		  //End MSB   = XE[15:8] 240-1
	SPI_sendData(240);		  //End LSB   = XE[ 7:0]
	//Write the "row address set" command to the LCD
	//RASET (2Bh): Row Address Set
	// * The value of YS [15:0] and YE [15:0] are referred when RAMWR
	//   command comes.
	// * Each value represents one row line in the Frame Memory.
	// * YS [15:0] always must be equal to or less than YE [15:0]
	SPI_sendCommand(ST7789_2B_RASET); //Row address set
	//Write the parameters for the "row address set" command
	//Use 1st quadrant coordinates: 0,0 is lower left, 239,319 is upper right.
	y = 319 - y;
	SPI_sendData(y >> 8);	 //Start MSB = YS[15:8]
	SPI_sendData(y & 0x00FF); //Start LSB = YS[ 7:0]
	SPI_sendData(0x01);		  //End MSB   = YE[15:8] 320-1
	SPI_sendData(0x3F);		  //End LSB   = YE[ 7:0]
	//Write the "write data" command to the LCD
	//RAMWR (2Ch): Memory Write
	SPI_sendCommand(ST7789_2C_RAMWR); //write data
}
//==============================================================================
#if (0) //simple
void Fill_LCD(uint8_t R, uint8_t G, uint8_t B)
{
	uint32_t
		i;
	Set_LCD_for_write_at_X_Y(0, 319);

	//Fill display with a given RGB value
	for (i = 0; i < (320UL * 240UL); i++)
	{
		SPI_sendData(B); //Blue
		SPI_sendData(G); //Green
		SPI_sendData(R); //Red
	}
}
#else //faster, bigger (6 bytes)
void Fill_LCD(uint8_t R, uint8_t G, uint8_t B)
{
	uint32_t i;
	Set_LCD_for_write_at_X_Y(0, 319);

	// Select the LCD controller
	CLR_CS;
	// Select the LCD's data register
	SET_RS;

	//Fill display with a given RGB value
	for (i = 0; i < (320UL * 240UL); i++)
	{
		SPI.transfer(B); //Blue
		SPI.transfer(G); //Green
		SPI.transfer(R); //Red
	}
	// Deselect the LCD controller
	SET_CS;
}
#endif
//==============================================================================
#if (0) //simple
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
	Set_LCD_for_write_at_X_Y(x, y);
	//Write the single pixel's worth of data
	SPI_sendData(B); //Blue
	SPI_sendData(G); //Green
	SPI_sendData(R); //Red
}
#else //faster, bigger (78 bytes)
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
{
	// Select the LCD controller
	CLR_CS;
	//CASET (2Ah): Column Address Set
	// * The value of XS [15:0] and XE [15:0] are referred when RAMWR
	//   command comes.
	// * Each value represents one column line in the Frame Memory.
	// * XS [15:0] always must be equal to or less than XE [15:0]
	SPI_sendCommand(ST7789_2A_CASET); //Column address set
	//Write the parameters for the "column address set" command
	SPI_sendData(x >> 8);	 //Start MSB = XS[15:8]
	SPI_sendData(x & 0x00FF); //Start LSB = XS[ 7:0]
	SPI_sendData(0);		  //End MSB   = XE[15:8] 240-1
	SPI_sendData(240);		  //End LSB   = XE[ 7:0]
	//Write the "row address set" command to the LCD
	//RASET (2Bh): Row Address Set
	// * The value of YS [15:0] and YE [15:0] are referred when RAMWR
	//   command comes.
	// * Each value represents one row line in the Frame Memory.
	// * YS [15:0] always must be equal to or less than YE [15:0]
	SPI_sendCommand(ST7789_2B_RASET); //Row address set

	//Use 1st quadrant coordinates: 0,0 is lower left, 239,319 is upper right.
	y = 319 - y;
	//Write the parameters for the "row address set" command
	SPI_sendData(y >> 8);	 //Start MSB = YS[15:8]
	SPI_sendData(y & 0x00FF); //Start LSB = YS[ 7:0]
	SPI_sendData(0x01);		  //End MSB   = YE[15:8] 320-1
	SPI_sendData(0x3F);		  //End LSB   = YE[ 7:0]
	//Write the "write data" command to the LCD
	//RAMWR (2Ch): Memory Write
	SPI_sendCommand(ST7789_2C_RAMWR); //write data
	//Write the single pixel's worth of data
	SPI_sendData(B); //Blue
	SPI_sendData(G); //Green
	SPI_sendData(R); //Red
	// Deselect the LCD controller
	SET_CS;
}
#endif
//==============================================================================
// From: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void LCD_Circle(uint16_t x0, uint16_t y0, uint16_t radius,
				uint16_t R, uint16_t G, uint16_t B)
{
	uint16_t x = radius;
	uint16_t y = 0;
	int16_t radiusError = 1 - (int16_t)x;

	while (x >= y)
	{
		//11 O'Clock
		Put_Pixel(x0 - y, y0 + x, R, G, B);
		//1 O'Clock
		Put_Pixel(x0 + y, y0 + x, R, G, B);
		//10 O'Clock
		Put_Pixel(x0 - x, y0 + y, R, G, B);
		//2 O'Clock
		Put_Pixel(x0 + x, y0 + y, R, G, B);
		//8 O'Clock
		Put_Pixel(x0 - x, y0 - y, R, G, B);
		//4 O'Clock
		Put_Pixel(x0 + x, y0 - y, R, G, B);
		//7 O'Clock
		Put_Pixel(x0 - y, y0 - x, R, G, B);
		//5 O'Clock
		Put_Pixel(x0 + y, y0 - x, R, G, B);

		y++;
		if (radiusError < 0)
			radiusError += (int16_t)(2 * y + 1);
		else
		{
			x--;
			radiusError += 2 * (((int16_t)y - (int16_t)x) + 1);
		}
	}
}
//==============================================================================
#define mSwap(a, b, t) \
	{                  \
		t = a;         \
		a = b;         \
		b = t;         \
	}
//==============================================================================
void Fast_Horizontal_Line(uint16_t x0, uint16_t y, uint16_t x1,
						  uint8_t r, uint8_t g, uint8_t b)
{
	uint16_t temp;
	if (x1 < x0)
		mSwap(x0, x1, temp);
	Set_LCD_for_write_at_X_Y(x0, y);
	while (x0 <= x1)
	{
		//Write the single pixel's worth of data
		SPI_sendData(r); //Blue
		SPI_sendData(g); //Green
		SPI_sendData(b); //Red
		x0++;
	}
}
//==============================================================================
// From: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void LCD_Line(uint16_t x0, uint16_t y0,
			  uint16_t x1, uint16_t y1,
			  uint8_t r, uint8_t g, uint8_t b)
{
	int16_t dx;
	int16_t sx;
	int16_t dy;
	int16_t sy;
	int16_t err;
	int16_t e2;

	//General case
	if (y0 != y1)
	{
		dx = abs((int16_t)x1 - (int16_t)x0);
		sx = x0 < x1 ? 1 : -1;
		dy = abs((int16_t)y1 - (int16_t)y0);
		sy = y0 < y1 ? 1 : -1;
		err = (dx > dy ? dx : -dy) / 2;

		for (;;)
		{
			Put_Pixel(x0, y0, r, g, b);
			if ((x0 == x1) && (y0 == y1))
				break;
			e2 = err;
			if (e2 > -dx)
			{
				err -= dy;
				x0 = (uint16_t)((int16_t)x0 + sx);
			}
			if (e2 < dy)
			{
				err += dx;
				y0 = (uint16_t)((int16_t)y0 + sy);
			}
		}
	}
	else
	{
		//Optimized for LCD
		Fast_Horizontal_Line(x0, y0, x1, r, g, b);
	}
}
//==============================================================================
// This function transfers data, in one stream. Slightly
// optimized to do index operations during SPI transfers.
void SPI_send_pixels(uint8_t byte_count, uint8_t *data_ptr)
{
	uint8_t subpixel;

	// Select the LCD's data register
	SET_RS;
	// Select the LCD controller
	CLR_CS;

	//Load the first byte
	subpixel = *data_ptr;

	while (byte_count)
	{
		//Send the byte out.
		SPDR = subpixel;
		//do something that can happen while transmitting
		data_ptr++; //point to next byte
		//Load the next byte
		subpixel = *data_ptr;
		//count this byte
		byte_count--;
		//Now that we have done all we can do, wait for the transfer to finish.
		while (!(SPSR & _BV(SPIF)));
	}
	// Deselect the LCD controller
	SET_CS;
}
//==============================================================================
#if DEMO_BMPIMAGES
#define BMP_FLIP	1 /* enabling this draws BMP images the right way up */
void show_BMPs_in_root(void)
{
	File root_dir;
	root_dir = SD.open("/");
	if (0 == root_dir)
	{
		Serial.println("show_BMPs_in_root: Can't open \"root\"");
		return;
	}
	File bmp_file;

	while (1)
	{

		bmp_file = root_dir.openNextFile();
		if (0 == bmp_file)
		{
			// no more files, break out of while()
			// root_dir will be closed below.
			break;
		}
		//Skip directories (what about volume name?)
		if (0 == bmp_file.isDirectory())
		{
			//The file name must include ".BMP"
			if (0 != strstr(bmp_file.name(), ".BMP"))
			{
				//The BMP must be exactly 230456 long
				//(this is correct for 240x320, 24-bit)
				if (230456 == bmp_file.size())
				{
					//Jump over BMP header. BMP must be 240x320 24-bit
					bmp_file.seek(54);

					//Since we are limited in memory, break the line up from
					// 240*3 = 720 bytes into three chunks of 80 pixels
					// each 80*3 = 240 bytes.
					//Making this static speeds it up slightly (10ms)
					//Reduces flash size by 114 bytes, and uses 240 bytes.
					static uint8_t third_of_a_line[80 * 3];
					for (uint16_t line = 0; line < 320; line++)
					{
						//Set the LCD to the left of this line. BMPs store data
						//lowest line first -- bottom up.
#if BMP_FLIP						
						Set_LCD_for_write_at_X_Y(0, 320-line);
#else
						Set_LCD_for_write_at_X_Y(0, line);
#endif						
						for (uint8_t line_section = 0; line_section < 3; line_section++)
						{
							//Get a third of the line
							bmp_file.read(third_of_a_line, 80 * 3);
							//Now write this third to the TFT, doing the BGR -> RGB
							//color fixup interlaced with the SPI transfers.
							SPI_send_pixels(80 * 3, third_of_a_line);
						}
					}
			}
			}
		}
		//Release the BMP file handle
		bmp_file.close();

		delay(1000);
	}
	//Release the root directory file handle
	root_dir.close();
}
#endif
//==============================================================================
void F12x16_DrawChar(uint16_t x, uint16_t y, char c)
{
	uint8_t state;
	uint8_t xc, yc;

	//draw character to frame buffer
	if (c > FONT_ASCII_12X16_BASE - 1)
	{
		c -= FONT_ASCII_12X16_BASE;
		if (c > FONT_ASCII_12X16_CHARNUM)
			c = ' ';

		for (yc = 0; yc < FONT_ASCII_12X16_HEIGHT; yc++)
		{
			//12x16 is two bytes wide
			uint16_t w;
			uint8_t b;
			b = pgm_read_byte(&Font_ASCII_12X16[((c * FONT_ASCII_12X16_HEIGHT) + yc) * 2]);
			w = b << 8;
			b = pgm_read_byte(&Font_ASCII_12X16[(((c * FONT_ASCII_12X16_HEIGHT) + yc) * 2) + 1]);
			w |= b;

//			w = (Font_ASCII_12X16[((c * FONT_ASCII_12X16_HEIGHT) + yc) * 2] << 8) |
//				Font_ASCII_12X16[(((c * FONT_ASCII_12X16_HEIGHT) + yc) * 2) + 1];

			for (xc = 0; xc < FONT_ASCII_12X16_WIDTH; xc++)
			{
				state = (w >> (15 - xc)) & 1 ? 0xFF : 0x00;
				Put_Pixel(x+xc, y+yc, state, state, state);
			}
		}
	}
}
//==============================================================================
#define F12CENTREX(a) ((240-((a)*13))/2) /*center text*/
void F12x16_DrawString(uint16_t x, uint16_t y, const char *text)
{
	uint8_t c;

	c = 0;
	while (text[c] != 0)
	{
		x += FONT_ASCII_12X16_WIDTH;
		F12x16_DrawChar(x, y, text[c]);
		c++;
	}
}
//==============================================================================
void setup()
{
	//debug console
	Serial.begin(115200);
	Serial.println("setup()");

	//Set up ports B & C
	DDRB = 0x3F;
	DDRC = 0x0F;
	PORTB = 0x00;
	PORTC = 0x00;

	//Drive the ports to a reasonable starting state.
	CLR_RESET;
	CLR_RS;
	SET_CS;
	CLR_MOSI;
	CLR_CLK;

#if DEMO_BMPIMAGES
	//Initialize the SD card (if used)
	if (!SD.begin(7)) //use Arduino pin 7 for SS/CS.
		Serial.println("SD failed to initialize");
#endif

	//Initialize SPI
	SPI.begin();
	SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0));

	//Initialize the LCD controller
	Initialize_LCD();
}
//==============================================================================

void loop()
{
	uint8_t i;
	uint16_t x;
	uint16_t sub_x;
	uint16_t y;
	uint16_t sub_y;
	uint8_t r;
	uint8_t g;
	uint8_t b;

#if DEMO_FONT
	//text example
	Serial.println("fill LCD");
	Fill_LCD(0x00, 0x00, 0x00);
	Serial.println("font example");
	//												 123456789012345678901234567890123456789
	F12x16_DrawString(F12CENTREX(12),  5+(18*0),	"CRYSTALFONTZ");
	F12x16_DrawString(F12CENTREX(18), 10+(18*1),	"CFAF240320A0-024SN");
	F12x16_DrawString(F12CENTREX(19), 10+(18*3),	"240x320 TFT Display");
	F12x16_DrawString(F12CENTREX(16), 10+(18*4),	"ST7789V Cont. IC");
	F12x16_DrawString(F12CENTREX(14), 10+(18*8),	"Seeeduino 3.3V");
	F12x16_DrawString(F12CENTREX(19), 10+(18*9),	"Simple Display Demo");
	delay(WAIT_TIME);
#endif

#if DEMO_LINES
	//cheesy lines
	Serial.println("fill LCD");
	Fill_LCD(0x20, 0x20, 0x20);
	Serial.println("cheesy lines");
	r = 0xff; g = 0x00; b = 0x80;
	for (x = 0; x < 240; x++)
		LCD_Line(120, 160, x, 0, r++, g--, b += 2);
	for (y = 0; y < 320; y++)
		LCD_Line(120, 160, 239, y, r++, g += 4, b += 2);
	for (x = 239; 0 != x; x--)
		LCD_Line(120, 160, x, 319, r -= 3, g -= 2, b -= 1);
	for (y = 319; 0 != y; y--)
		LCD_Line(120, 160, 0, y, r + -3, g--, b++);
	delay(WAIT_TIME);
#endif

#if DEMO_CIRCLES
	//Fill display with a given RGB value
	Serial.println("fill LCD");
	Fill_LCD(0x00, 0x00, 0xFF);
	Serial.println("circles");
	//Draw a cyan circle
	LCD_Circle(120, 120 + 40, 119, 0x00, 0xFF, 0xFF);
	//Draw a white circle
	LCD_Circle(120, 120 + 40, 40, 0xFF, 0xFF, 0xFF);
	//Draw a green circle
	LCD_Circle(40, 120 + 40, 37, 0x00, 0xFF, 0x00);
	//Draw a red circle
	LCD_Circle(200, 120 + 40, 37, 0xFF, 0x00, 0x00);
	//Draw a purple circle
	LCD_Circle(120, 200 + 40, 32, 0xFF, 0x00, 0xFF);
	//Draw a orange circle
	LCD_Circle(120, 40 + 40, 28, 0xFF, 0xA5, 0x00);
	delay(WAIT_TIME);
#endif

#if DEMO_EXPANDING
	//expanding circles
	Serial.println("fill LCD");
	Fill_LCD(0x00, 0x00, 0x00);

	Serial.println("expanding circles");
	for (i = 2; i < 120; i += 2)
		LCD_Circle(i + 2, 160, i, i << 2, 0xff - (i << 2), 0xFF);
	delay(WAIT_TIME);
#endif

#if DEMO_CHECKER
	//Write a 16x16 checkerboard
	Serial.println("Checkerboard");
	for (x = 0; x < (240 / 16); x++)
		for (y = 0; y < (320 / 16); y++)
			for (sub_x = 0; sub_x <= 15; sub_x++)
				for (sub_y = 0; sub_y <= 15; sub_y++)
					if (((x & 0x01) && !(y & 0x01)) || (!(x & 0x01) && (y & 0x01)))
						Put_Pixel((x << 4) + sub_x, (y << 4) + sub_y, 0x00, 0x00, 0x00);
					else
						Put_Pixel((x << 4) + sub_x, (y << 4) + sub_y, 0xFF, 0xFF - (x << 4), 0xFF - (y << 4));
	delay(WAIT_TIME);
#endif

#if DEMO_BMPIMAGES
	//Slideshow of bitmap files on uSD card.
	Serial.println("BMPs");
	show_BMPs_in_root();
#endif
}
//==============================================================================
