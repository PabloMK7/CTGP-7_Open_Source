/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: CrashReport.hpp
Open source lines: 91/91 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "QrCode.hpp"

namespace CTRPluginFramework {
	class CrashReport
	{
	public:
		static constexpr const u32 QRREPORTMAGIC = 0x43375200;
		static constexpr const char BINARYREPORTVERSION = '0';
		static constexpr const char TEXTREPORTVERSION = 'A';
		enum ExceptionType
		{
			EXTYPE_PREFETCH = 0,
			EXTYPE_DATA = 1,
			EXTYPE_UNDINS = 2,
			EXTYPE_ABORT = 3,
			EXTYPE_CUSTOM = 4,
			EXTYPE_UNKNOWN = 5,
		};
		enum StateID
		{
			STATE_UNINITIALIZED = 0,
			STATE_PATCHPROCESS = 1,
			STATE_MAIN = 2,
			STATE_MENU = 3,
			STATE_RACE = 4,
			STATE_TROPHY = 5
		};
		struct QRData {
			struct BinaryException {
				struct RegisterInfo {
				u32 sp;
				u32 pc;
				u32 lr;
				u32 far;
				u32 callStack[6];
				} registerInfo;

				struct GameState {
					u32 stateArgs0;
					u32 stateArgs1;
				} gameState;
			};

			u32 magic;
			u32 ctgp7ver;
			union
			{
				BinaryException binaryData;
				char textData[sizeof(BinaryException)];
			};			
			u8 regRev;
			u8 exceptionType;
			u8 padding[2];
		};
		struct CrashTaskData
		{
			ERRF_ExceptionInfo* excep;
			CpuRegisters* regs;
		};
		
		static StateID stateID;
		static CpuRegisters abortRegs;
		static ERRF_ExceptionInfo abortExcep;
		static bool fromAbort;
		static const char* textCrash;
		static Process::ExceptionCallbackState CTGPExceptCallback(ERRF_ExceptionInfo* excep, CpuRegisters* regs);

		static u32 SaveStackPointer() NAKED;
		static void OnAbort() NORETURN;
		void populateReportData(const ERRF_ExceptionInfo* excep, const CpuRegisters* regs);
		void DrawTopScreen(Screen& scr);
		QRData reportData;
		static void Initialize();
	private:
		static Sound* oofSound;
		static const char** getRandomCrahMsg();
		void populateRegisterInfo(const CpuRegisters* regs);
		static void DrawQRCode(Screen& scr, QRData* qrdata);
	};
}