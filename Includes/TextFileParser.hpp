/*****************************************************
This file is part of the CTGP-7 Open Source project.
Please see README.md for the project license.
(Some files may be sublicensed, please check below.)

File: TextFileParser.hpp
Open source lines: 58/58 (100.00%)
*****************************************************/

#pragma once

#include "CTRPluginFramework.hpp"
#include <map>
#include <string>
#include <vector>
#include <tuple>

namespace CTRPluginFramework {
	class TextFileParser
	{
	public:
		using TextMap = std::map<std::string, std::vector<std::string>>;
		using TextMapIter = TextMap::iterator;
		using TextMapConstIter = TextMap::const_iterator;

		TextFileParser();
		bool Parse(const std::string& filename, const std::string& separator = "::");
		bool ParseLine(const std::string& line, const std::string& separator = "::");
		bool ParseLines(const std::string& lines, const std::string& separator = "::");
		const std::vector<std::string>& getEntries(const std::string& key);
		const std::string& getEntry(const std::string& key, u32 element = 0);
		TextMapIter begin();
		TextMapIter end();
		TextMapConstIter cbegin();
		TextMapConstIter cend();
		TextMapConstIter erase(TextMapConstIter& iter);

		~TextFileParser();

		static std::string& Trim(std::string& str);
		static void ProcessText(std::string& str);

		static bool IsNumerical(const std::string& str, bool isHex, bool isU64 = false);

		enum class NumberType {
			INVALID,
			DECIMAL,
			HEXADECIMAL
		};
		static NumberType IsValidNumber(const std::string& str);
		static std::vector<std::string> Split(std::string str, const std::string& delimiter = "::");

	private:
		TextMap dataMap;
		const std::vector<std::string>& _FindKey(const std::string& key);
		void ParseLineImpl(std::string& key, std::vector<std::string>& args, std::string& line, const std::string& separator = "::", bool firstLine = false);
	};
}