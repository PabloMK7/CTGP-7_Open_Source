/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: MiscUtils.cpp
Open source lines: 62/62 (100.00%)
*****************************************************/

#include "MiscUtils.hpp"

namespace CTRPluginFramework {
    bool MiscUtils::CopyFile(const std::string &dst, const std::string &ori)
	{
        constexpr ssize_t copyBufferSize = 0x2000;
        std::unique_ptr<u8[]> copyBuffer = std::make_unique_for_overwrite<u8[]>(copyBufferSize);

		File::Remove(dst);
		File oriFile(ori, File::READ);
		File dstFile(dst, File::RWC);
		if (!oriFile.IsOpen() || !dstFile.IsOpen()) return false;
		ssize_t fileSize = oriFile.GetSize();
        if (fileSize < 0) return false;
		ssize_t currSize = 0, totSize = fileSize;

		while (fileSize != 0) {
			ssize_t block = (fileSize > copyBufferSize) ? copyBufferSize : fileSize;
			if (oriFile.Read(copyBuffer.get(), block) < 0) return false;
			if (dstFile.Write(copyBuffer.get(), block) < 0) return false;
			currSize += block;
			fileSize -= block;
		}
        return true;
	}

    bool MiscUtils::CopyDirectory(const std::string& dst, const std::string& ori, bool recursive)
	{
		if (!Directory::IsExists(dst)) Directory::Create(dst);
		Directory oriDir(ori);
		if (!oriDir.IsOpen()) return false;
		std::vector<std::string> dirs;
		std::vector<std::string> files;
		if (recursive) if (oriDir.ListDirectories(dirs) < 0) return false; // This fails if there are no subdirs
		if (oriDir.ListFiles(files) < 0) return false;

		for (int i = 0; i < files.size(); i++) {
			std::string l = dst << "/" << files[i];
			std::string r = ori << "/" << files[i];
            if (!CopyFile(l, r)) return false;
		}
		files.clear();

		for (int i = 0; i < dirs.size(); i++) {
			if (!CopyDirectory(dst << "/" << dirs[i], ori << "/" << dirs[i], true)) return false;
		}

        return true;
	}
}



