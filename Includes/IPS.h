/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: IPS.h
Open source lines: 110/110 (100.00%)
*****************************************************/

#pragma once
#include "CTRPluginFramework.hpp"
#include "malloc.h"

namespace CTRPluginFramework {
	class IPS {
	public:
		struct IFile {
			FILE* f = NULL;
			File* pfF = nullptr;
			void(*cb)(IPS::IFile&);
			u32 totProgress;
			u32 offset = 0;
			u32 fileSize = 0;
			u32 bufferOffset = 0;
			u8* bufferAppend = NULL;
			u8* bufferWrite = NULL;
			u32 bufferWritePage = 0;
			u8* bufferRead = NULL;
			u8* bufferSet = NULL;
			u8* copyDataBuf = NULL;
			u32 bufferPage = 0;
			bool isCTRPF = false;
			IFile(std::string path, bool isCTRPFM, std::string modeS, File::Mode modeM) {
				isCTRPF = isCTRPFM;
				if (isCTRPF) {
					pfF = new File(path, modeM);
					if (!pfF->IsOpen()) return;
					if ((modeM & File::CREATE) != 0) {
						u8 tmp = 0;
						pfF->Write(&tmp, 1);
					}
					fileSize = pfF->GetSize();
				}
				else {
					f = fopen(path.c_str(), modeS.c_str());
					if (!f) return;
					fseek(f, 0, SEEK_END);
					fileSize = ftell(f);
					fseek(f, 0, SEEK_SET);
				}
				offset = 0;
			}
			~IFile() {
				if (bufferAppend) {
					IFile* file = this;
					if (IFileIsOpen(*this)) IFileCommitToFile(*file, bufferAppend, bufferOffset);
					file = nullptr;
					free(bufferAppend);
					bufferAppend = NULL;
				}
				if (bufferWrite) {
					IFile* file = this;
					file->offset = bufferWritePage;
					u32 block = (file->offset + 0x10000 > file->fileSize) ? (file->fileSize - file->offset) : 0x10000;
					if (IFileIsOpen(*this)) IFileCommitToFile(*file, bufferWrite, block);
					file = nullptr;
					free(bufferWrite);
					bufferWrite = NULL;
				}
				if (bufferRead) {
					free(bufferRead);
					bufferRead = NULL;
				}
				if (bufferSet) {
					free(bufferSet);
					bufferSet = NULL;
				}
				if (copyDataBuf) {
					free(copyDataBuf);
					copyDataBuf = NULL;
				}
				if (!isCTRPF) {
					if (f) fclose(f);
				}
				else {
					if (pfF->IsOpen()) pfF->Flush();
					delete pfF;
				}
			}
		};
		using IFileCallBack = void(*)(IPS::IFile&);
		static bool applyIPSPatch(std::string dst, std::string ori, std::string ips, IFileCallBack cb = nullptr);
	private:
		static bool IFileIsOpen(IFile& f);
		static u32 IFileReadBytes(IFile& f, void* out, u32 size);
		static u32 IFIleSwitchReadPage(IFile& f);
		static u32 IFileGetFromFile(IFile& f, void* out, u32 size, u32 offset);
		static u32 IFileWriteBytes(IFile& f, void* in, u32 size);
		static bool IFileSwitchWritePage(IFile& f, u32 newPage);
		static u32 IFileAppendBytes(IFile& f, void* in, u32 size);
		static u32 IFileCommitToFile(IFile& f, void* in, u32 size);

		static bool WriteBlock(IFile& f, u8 val, u32 size);
		static bool WriteBlockNoAppend(IFile& f, u8 val, u32 size);
		static bool CopyData(IFile& dst, IFile& ori, u32 size);
		static bool CopyDataNoAppend(IFile& dst, IFile& ori, u32 size);

		static bool applyIPSPatch(std::string dstFile, std::string ipsFile, IFileCallBack cb);
	};
}