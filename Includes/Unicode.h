/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: Unicode.h
Open source lines: 45/45 (100.00%)
*****************************************************/

#ifndef UNICODE_H
#define UNICODE_H

// HID Symbols
#define FONT_A      "\uE000" // System Font A button
#define FONT_B      "\uE001" // System Font B button
#define FONT_X      "\uE002" // System Font X button
#define FONT_Y      "\uE003" // System Font Y button
#if CITRA_MODE == 1
#define FONT_L      "(L)" // System Font L button
#define FONT_R      "(R)" // System Font R button
#define FONT_ZL     "(ZL)" // System Font ZL button
#define FONT_ZR     "(ZR)" // System Font ZR button
#define FONT_DU     "DPAD UP" // System Font D-Pad Up button
#define FONT_DD     "DPAD DOWN" // System Font D-Pad Down button
#define FONT_DL     "DPAD LEFT" // System Font D-Pad Left button
#define FONT_DR     "DPAD RIGHT" // System Font D-Pad Right button
#define FONT_DUD    "DPAD" // System Font D-Pad Up and Down button
#define FONT_DLR    "DPAD" // System Font D-Pad Left and Right button
#define FONT_CP     "CPAD" // System Font Circle Pad button
#define FONT_T      "Touch Screen" // System Font Touch button
#else
#define FONT_L      "\uE052" // System Font L button
#define FONT_R      "\uE053" // System Font R button
#define FONT_ZL     "\uE054" // System Font ZL button
#define FONT_ZR     "\uE055" // System Font ZR button
#define FONT_DU     "\uE079" // System Font D-Pad Up button
#define FONT_DD     "\uE07A" // System Font D-Pad Down button
#define FONT_DL     "\uE07B" // System Font D-Pad Left button
#define FONT_DR     "\uE07C" // System Font D-Pad Right button
#define FONT_DUD    "\uE07D" // System Font D-Pad Up and Down button
#define FONT_DLR    "\uE07E" // System Font D-Pad Left and Right button
#define FONT_CP     "\uE077" // System Font Circle Pad button
#define FONT_T      "\uE058" // System Font Touch button
#endif
#endif