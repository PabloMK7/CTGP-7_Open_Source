/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: IPS.cpp
Open source lines: 352/352 (100.00%)
*****************************************************/

#include "IPS.h"
#include "3ds/romfs.h"
#include "cheats.hpp"
#include "OSDManager.hpp"

namespace CTRPluginFramework {
	bool IPS::IFileIsOpen(IFile& f)
	{
		if (f.isCTRPF) {
			return f.pfF->IsOpen();
		}
		else {
			return f.f != NULL;
		}
	}

	u32 IPS::IFileGetFromFile(IFile& f, void* out, u32 size, u32 offset)
	{
		u32 bytesRead;
		if (f.isCTRPF) {
			f.pfF->Seek(offset, File::SeekPos::SET);
			u32 old = f.pfF->Tell();
			f.pfF->Read(out, size);
			u32 newV = f.pfF->Tell();
			bytesRead = newV - old;
		}
		else {
			fseek(f.f, offset, SEEK_SET);
			bytesRead = fread(out, 1, size, f.f);
		}
		return bytesRead;
	}

	u32 IPS::IFileWriteBytes(IFile& f, void* in, u32 size)
	{
		if (!f.bufferWrite) {
			f.bufferWrite = (u8*)memalign(0x1000, 0x10000);
			f.bufferWritePage = f.offset & ~0xFFFF;
			u32 block = (f.bufferWritePage + 0x10000 > f.fileSize) ? (f.fileSize - f.bufferWritePage) : 0x10000;
			IFileGetFromFile(f, f.bufferWrite, block, f.bufferWritePage);
		}
		u32 bytesWritten = 0;
		u32 pageStart = f.offset & ~0xFFFF;
		u32 offset = f.offset & 0xFFFF;
		while (size != 0) {
			if (!IFileSwitchWritePage(f, pageStart)) return -1;
			u32 toWrite = (offset + size > 0x10000) ? toWrite = 0x10000 - offset : toWrite = size;
			memcpy(f.bufferWrite + offset, in, toWrite);
			offset = 0;
			in = (u8*)in + toWrite;
			size -= toWrite;
			pageStart += 0x10000;
			bytesWritten += toWrite;
		}
		f.offset += bytesWritten;
		return bytesWritten;
	}

	bool IPS::IFileSwitchWritePage(IFile& f, u32 newPage)
	{
		if (f.bufferWritePage == newPage) return true;
		u32 oldOffset = f.offset;
		f.offset = f.bufferWritePage;
		u32 block = (f.offset + 0x10000 > f.fileSize) ? (f.fileSize - f.offset) : 0x10000;
		if (IFileCommitToFile(f, f.bufferWrite, block) != block) return false;
		f.bufferWritePage = newPage;
		block = (f.bufferWritePage + 0x10000 > f.fileSize) ? (f.fileSize - f.bufferWritePage) : 0x10000;
		if (IFileGetFromFile(f, f.bufferWrite, block, f.bufferWritePage) != block) return false;
		f.offset = oldOffset;
		return true;
	}

	u32 IPS::IFIleSwitchReadPage(IFile& f)
	{
		u32 block = (f.bufferPage + 0x10000 > f.fileSize) ? (f.fileSize - f.bufferPage) : 0x10000;
		if (IFileGetFromFile(f, f.bufferRead, block, f.bufferPage) != block) return -1;
		return 0;
	}

	u32 IPS::IFileReadBytes(IFile& f, void* out, u32 size)
	{
		if (f.offset > f.fileSize) return -1;
		if ((f.offset + size) > f.fileSize) size = f.fileSize - f.offset;
		u32 pageStart = f.offset & ~0xFFFF;
		u32 pageEnd = (f.offset + size) & ~0xFFFF;
		if (!f.bufferRead) {
			f.bufferRead = (u8*)memalign(0x1000, 0x10000);
			f.bufferPage = pageStart;
			if (IFIleSwitchReadPage(f)) return -1;
		}
		if (pageStart != f.bufferPage) {
			f.bufferPage = pageStart;
			if (IFIleSwitchReadPage(f)) return -1;
		}
		if (pageStart != pageEnd) {
			u32 startSize = 0x10000 - (f.offset & 0xFFFF);
			u32 endSize = size - startSize;
			memcpy(out, f.bufferRead + (f.offset & 0xFFFF), startSize);
			f.bufferPage = pageEnd;
			if (IFIleSwitchReadPage(f)) return -1;
			memcpy((u8*)out + startSize, f.bufferRead, endSize);
		}
		else {
			memcpy(out, f.bufferRead + (f.offset & 0xFFFF), size);
		}
		f.offset += size;
		return size;
	}

	u32 IPS::IFileCommitToFile(IFile& f, void* in, u32 size) {
		if (f.offset > f.fileSize) f.offset = f.fileSize;
		u32 bytesWritten;
		if (f.cb) {
			f.cb(f);
		}
		if (f.isCTRPF) {
			f.pfF->Seek(f.offset, File::SeekPos::SET);
			u32 old = f.pfF->Tell();
			f.pfF->Write(in, size);
			u32 newV = f.pfF->Tell();
			bytesWritten = newV - old;
			f.fileSize = f.pfF->GetSize();
		}
		else {
			fseek(f.f, f.offset, SEEK_SET);
			bytesWritten = fwrite(in, 1, size, f.f);
			fseek(f.f, 0, SEEK_END);
			f.fileSize = ftell(f.f);
		}
		f.offset += bytesWritten;
		return bytesWritten;
	}

	u32 IPS::IFileAppendBytes(IFile& f, void* in, u32 size)
	{
		if (size > 0x10000) return -1;
		if (!f.bufferAppend) f.bufferAppend = (u8*)memalign(0x1000, 0x10000);
		if (f.bufferOffset + size > 0x10000) {
			if (IFileCommitToFile(f, f.bufferAppend, f.bufferOffset) != f.bufferOffset) return -1;
			f.bufferOffset = 0;
		}
		memcpy(f.bufferAppend + f.bufferOffset, in, size);
		f.bufferOffset += size;
		return size;
	}
	u32 g_ipsPreviousVal = 0x1000, g_ipsPreviusSize = 0;
	bool IPS::WriteBlock(IFile& f, u8 val, u32 size)
	{
		if (!f.bufferSet) {
			f.bufferSet = (u8*)memalign(0x1000, 0x10000);
			g_ipsPreviousVal = 0x1000;
			g_ipsPreviusSize = 0;
		}
		u32 setSize = (size > 0x10000) ? 0x10000 : size;
		if (!(g_ipsPreviousVal == val && g_ipsPreviusSize >= size)) {
			memset(f.bufferSet, val, setSize);
			g_ipsPreviousVal = val;
			g_ipsPreviusSize = size;
		}
		while (size != 0) {
			u32 block = (size < 0x10000) ? size : 0x10000;
			size -= block;
			if (IFileAppendBytes(f, f.bufferSet, block) != block) {
				return false;
			}
		}
		return true;
	}

	bool IPS::WriteBlockNoAppend(IFile& f, u8 val, u32 size)
	{
		if (!f.bufferSet) {
			f.bufferSet = (u8*)memalign(0x1000, 0x10000);
			g_ipsPreviousVal = 0x1000;
			g_ipsPreviusSize = 0;
		}
		u32 setSize = (size > 0x10000) ? 0x10000 : size;
		if (!(g_ipsPreviousVal == val && g_ipsPreviusSize >= size)) {
			memset(f.bufferSet, val, setSize);
			g_ipsPreviousVal = val;
			g_ipsPreviusSize = size;
		}
		while (size != 0) {
			u32 block = (size < 0x10000) ? size : 0x10000;
			size -= block;
			if (IFileWriteBytes(f, f.bufferSet, block) != block) {
				return false;
			}
		}
		return true;
	}

	bool IPS::CopyData(IFile& dst, IFile& ori, u32 size)
	{
		if (!dst.copyDataBuf) dst.copyDataBuf = (u8*)memalign(0x1000, 0x10000);
		while (size != 0)
		{
			u32 block = (size < 0x10000) ? size : 0x10000;
			size -= block;
			if (IFileReadBytes(ori, dst.copyDataBuf, block) != block) {
				return false;
			}
			if (IFileAppendBytes(dst, dst.copyDataBuf, block) != block) {
				return false;
			}
		}
		return true;
	}

	bool IPS::CopyDataNoAppend(IFile& dst, IFile& ori, u32 size)
	{
		if (!dst.copyDataBuf) dst.copyDataBuf = (u8*)memalign(0x1000, 0x10000);
		while (size != 0)
		{
			u32 block = (size < 0x10000) ? size : 0x10000;
			size -= block;
			if (IFileReadBytes(ori, dst.copyDataBuf, block) != block) {
				return false;
			}
			if (IFileWriteBytes(dst, dst.copyDataBuf, block) != block) {
				return false;
			}
		}
		return true;
	}

	bool IPS::applyIPSPatch(std::string dstFile, std::string ipsFile, IFileCallBack cb)
	{
		if (!File::Exists(ipsFile)) return false;
		{
			IFile dst(dstFile, true, "", File::RW);
			IFile ips(ipsFile, true, "", File::READ);

			if (!IFileIsOpen(dst) || !IFileIsOpen(ips)) goto exitError;

			dst.cb = cb;
			dst.totProgress = dst.fileSize;

			u8 magic[5];
			if (IFileReadBytes(ips, magic, 5) != 5 || memcmp(magic, "PATCH", 5) != 0) goto exitError;

			while (true) {
				u8 ipsEntry[5];
				u32 bytesRead = IFileReadBytes(ips, ipsEntry, 5);
				if (bytesRead == 3) {
					if (memcmp(ipsEntry, "EOF", 3) == 0) {
						break;
					}
					else goto exitError;
				}
				else if (bytesRead == 5) {
					u32 offset = (ipsEntry[0] << 16) | (ipsEntry[1] << 8) | (ipsEntry[2]);
					u32 size = (ipsEntry[3] << 8) | ipsEntry[4];

					if (offset > dst.fileSize) goto exitError;

					if (size == 0) { // RLE
						u8 rle[3];
						if (IFileReadBytes(ips, &rle, 3) != 3) goto exitError;
						dst.offset = offset;
						if (!WriteBlockNoAppend(dst, rle[2], (rle[0] << 8) | rle[1])) goto exitError;
					}
					else { // Normal
						dst.offset = offset;
						if (!CopyDataNoAppend(dst, ips, size)) goto exitError;
					}

				}
			}
		}
		return true;
	exitError:
		File::Remove(dstFile);
		return false;
	}

	bool IPS::applyIPSPatch(std::string dstFile, std::string oriFile, std::string ipsFile, IFileCallBack cb)
	{
		if (!File::Exists(ipsFile)) return false;
		if (oriFile.compare("") == 0) {
			return applyIPSPatch(dstFile, ipsFile, cb);
		}
		bool romfs = false;
		if (oriFile.find("romfs:") == 0) {
			romfsInit();
			romfs = true;
		}
		{
			File::Remove(dstFile);

			IFile dst(dstFile, true, "", File::RWC);
			IFile ori(oriFile, !romfs, "rb", File::READ);
			IFile ips(ipsFile, true, "", File::READ);

			if (!IFileIsOpen(dst) || !IFileIsOpen(ori) || !IFileIsOpen(ips)) goto exitError;

			dst.cb = cb;
			dst.totProgress = ori.fileSize;

			u8 magic[5];
			if (IFileReadBytes(ips, magic, 5) != 5 || memcmp(magic, "PATCH", 5) != 0) goto exitError;

			while (true) {
				u8 ipsEntry[5];
				u32 bytesRead = IFileReadBytes(ips, ipsEntry, 5);
				if (bytesRead == 3) {
					if (memcmp(ipsEntry, "EOF", 3) == 0) {
						if (!CopyData(dst, ori, ori.fileSize - ori.offset)) goto exitError;
						break;
					}
					else goto exitError;
				}
				else if (bytesRead == 5) {
					u32 offset = (ipsEntry[0] << 16) | (ipsEntry[1] << 8) | (ipsEntry[2]);
					u32 size = (ipsEntry[3] << 8) | ipsEntry[4];

					int toAdvance = offset - ori.offset;
					if (offset > ori.fileSize || toAdvance < 0) goto exitError;
					if (!CopyData(dst, ori, (u32)toAdvance)) goto exitError;

					if (size == 0) { // RLE
						u8 rle[3];
						if (IFileReadBytes(ips, &rle, 3) != 3) goto exitError;
						if (!WriteBlock(dst, rle[2], (rle[0] << 8) | rle[1])) goto exitError;
						ori.offset += (rle[0] << 8) | rle[1];
					}
					else { // Normal
						if (!CopyData(dst, ips, size)) goto exitError;
						ori.offset += size;
					}

				}
			}
		}
		if (romfs) romfsExit();
		return true;
		
	exitError:
		if (romfs) romfsExit();
		File::Remove(dstFile);
		return false;
	}
}