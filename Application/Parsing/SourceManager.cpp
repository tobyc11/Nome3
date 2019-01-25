#include "SourceManager.h"
#include <fstream>
#include <streambuf>

namespace Nome
{

const std::string& CSourceFile::GetAsString() const
{
	return Content->GetAsString();
}

CSourceLocation::CSourceLocation()
{
}

CSourceLocation::CSourceLocation(CStringBuffer::CLocation inLocation) : Location(inLocation)
{
}

bool CSourceLocation::operator==(const CSourceLocation& rhs) const
{
	if (!IsValid())
	{
		if (!rhs.IsValid())
			return true; //both are invalid
		else
			return false;
	}
	else
	{
		if (!rhs.IsValid())
			return false;
		else
			return *Location == *rhs.Location;
	}
}

bool CSourceLocation::operator<(const CSourceLocation& rhs) const
{
	return *Location < *rhs.Location;
}

bool CSourceLocation::operator<=(const CSourceLocation& rhs) const
{
	return operator==(rhs) || operator<(rhs);
}

bool CSourceLocation::IsValid() const
{
	return Location.IsValid();
}

CSourceRange::CSourceRange()
{
}

CSourceRange::CSourceRange(CSourceLocation begin, CSourceLocation end)
	: Begin(begin), End(end)
{
}

bool CSourceRange::IsValid() const
{
	return Begin.IsValid() && End.IsValid() && Begin <= End;
}

bool CSourceRange::Intersects(const CSourceRange& rhs) const
{
	assert(IsValid() && rhs.IsValid());

	if (Begin < rhs.Begin)
	{
		if (rhs.Begin < End)
			return true;
		else
			return false;
	}
	else
	{
		if (Begin < rhs.End)
			return true;
		else
			return false;
	}
}

CSourceRange CSourceRange::Merge(const CSourceRange& rhs) const
{
	if (!Intersects(rhs))
		return CSourceRange();
	if (Begin < rhs.Begin)
	{
		if (End < rhs.End)
			return CSourceRange(Begin, rhs.End);
		else
			return CSourceRange(Begin, End);
	}
	else
	{
		if (End < rhs.End)
			return CSourceRange(rhs.Begin, rhs.End);
		else
			return CSourceRange(rhs.Begin, End);
	}
}

CSourceManager::CSourceManager()
{
}

CSourceFile* CSourceManager::Open(const std::string& filePath)
{
	//Read the file into a string
	std::ifstream t(filePath);
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	auto* sb = new CStringBuffer(str);
	auto* sourceFile = new CSourceFile(sb);
	sourceFile->Path = filePath;
	SourceFiles.emplace(filePath, sourceFile);
	return sourceFile;
}

void CSourceManager::Close(CSourceFile* file)
{
	SourceFiles.erase(file->Path);
	delete file;
}

void CSourceManager::Save(CSourceFile* file)
{
	std::ofstream ofs(file->Path);
	ofs << file->Content->GetAsString();
}

CSourceLocation CSourceManager::GetLocation(CSourceFile* file, size_t offset)
{
	return CSourceLocation(file->Content->GetLocation(offset));
}

void CSourceManager::ReplaceRange(CSourceFile* file, CSourceRange range, const std::string& string)
{
	file->Content->ReplaceRange(range.Begin.Location, range.End.Location, string);
}

void CSourceManager::WriteLine(CSourceFile* file, const std::string& line)
{
	file->Content->WriteLine(line);
}

}
