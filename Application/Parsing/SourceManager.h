#pragma once
#include "StringBuffer.h"
#include <StrongPointer.h>
#include <map>

namespace Nome
{

using tc::sp;

class CSourceManager;

class CSourceFile
{
	friend class CSourceManager;

public:
	CSourceFile(CStringBuffer* content) : Content(content) {}

	const std::string& GetAsString() const;
	const std::string& GetPath() const { return Path; }

private:
	std::string Path;
	sp<CStringBuffer> Content;
};

class CSourceLocation
{
	friend class CSourceManager;

public:
	CSourceLocation();
	CSourceLocation(CStringBuffer::CLocation inLocation);

	bool operator==(const CSourceLocation& rhs) const;
	bool operator<(const CSourceLocation& rhs) const;
	bool operator<=(const CSourceLocation& rhs) const;

	bool IsValid() const;

private:
	CStringBuffer::CLocation Location;
};

class CSourceRange
{
	friend class CSourceManager;

public:
	CSourceRange();
	CSourceRange(CSourceLocation begin, CSourceLocation end);

	bool IsValid() const;
	bool Intersects(const CSourceRange& rhs) const;
	CSourceRange Merge(const CSourceRange& rhs) const;

private:
	CSourceLocation Begin;
	CSourceLocation End;
};

class CSourceManager : public tc::TLightRefBase<CSourceManager>
{
public:
    CSourceManager();

	CSourceFile* Open(const std::string& filePath);
	void Close(CSourceFile* file);
	void Save(CSourceFile* file);
	CSourceLocation GetLocation(CSourceFile* file, size_t offset);
	void ReplaceRange(CSourceFile* file, CSourceRange range, const std::string& string);
	void WriteLine(CSourceFile* file, const std::string& line);

private:
	std::map<std::string, CSourceFile*> SourceFiles;
};

}
