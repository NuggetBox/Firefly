#pragma once
#include <fstream>
#include <vector>
#include <filesystem>
namespace Firefly
{
	inline std::vector<uint8_t> ReadBinary(const std::filesystem::path& aPath)
	{
		if (!std::filesystem::exists(aPath))
		{
			return std::vector<uint8_t>();
		}
		std::vector<uint8_t> bytes;
		std::ifstream fin(aPath, std::ios::binary | std::ios::ate);
		fin.seekg(0, std::ios::end);
		bytes.resize(fin.tellg());
		fin.seekg(0, std::ios::beg);
		fin.read((char*)bytes.data(), bytes.size());
		return bytes;
	}
	inline bool WriteBinary(const std::filesystem::path& aPath, std::vector<uint8_t> aData)
	{
		if (std::filesystem::is_directory(aPath))
		{
			return false;
		}

		if (!std::filesystem::is_directory(aPath.parent_path()))
		{
			std::filesystem::create_directories(aPath.parent_path());
		}

		std::ofstream fout(aPath, std::ios::out | std::ios::binary);
		if (fout.bad())
		{
			return false;
		}

		fout.write((char*)aData.data(), aData.size() * sizeof(uint8_t));
		return true;
	}
}