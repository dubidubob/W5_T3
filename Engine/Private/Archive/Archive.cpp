#include "pch.h"
#include "Archive/Archive.h"

FWindowsBinWriter::FWindowsBinWriter(const path& FilePath)
{
	FileStream.open(FilePath, std::ios::binary | std::ios::out);
}

FWindowsBinWriter::~FWindowsBinWriter()
{
	if (FileStream.is_open()) { FileStream.close(); }
}

FArchive& FWindowsBinWriter::Serialize(void* V, size_t Length)
{
	FileStream.write(static_cast<const char*>(V), Length);
	return *this;
}

FWindowsBinReader::FWindowsBinReader(const path& FilePath)
{
	FileStream.open(FilePath, std::ios::binary | std::ios::in);
}

FWindowsBinReader::~FWindowsBinReader()
{
	if (FileStream.is_open()) { FileStream.close(); }
}

FArchive& FWindowsBinReader::Serialize(void* V, size_t Length)
{
	FileStream.read(static_cast<char*>(V), Length);
	return *this;
}
