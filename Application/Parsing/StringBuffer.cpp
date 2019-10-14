#include "StringBuffer.h"

namespace Nome
{

CStringBuffer::CStringBuffer() {}

CStringBuffer::CStringBuffer(const std::string& content)
    : Buffer(content)
{
}

CStringBuffer::CLocation CStringBuffer::GetLocation(size_t offset)
{
    if (offset > Buffer.size())
        return {};

    auto iter = LocationMap.find(offset);
    if (iter != LocationMap.end())
        return iter->second;

    CLocation loc = LocationTable.GetNewElement(tc::kAddToBack);
    LocationMap[offset] = loc;
    *loc = offset;
    return loc;
}

const std::string& CStringBuffer::GetAsString() const { return Buffer; }

void CStringBuffer::ReplaceRange(size_t begin, size_t end, const std::string& content)
{
    assert(end >= begin);
    size_t lenBefore = end - begin;
    size_t lenAfter = content.size();
    ptrdiff_t deltaLen = (ptrdiff_t)lenAfter - lenBefore;
    // Now move all locations after end (inclusive) by deltaLen
    if (deltaLen != 0)
    {
        for (auto& loc : LocationTable)
        {
            if (loc >= end)
                loc += deltaLen;
        }
    }
    RebuildLocationMap();

    // Now replace the actual content
    Buffer.replace(begin, lenBefore, content);
}

void CStringBuffer::ReplaceRange(CLocation begin, CLocation end, const std::string& content)
{
    ReplaceRange(*begin, *end, content);
}

void CStringBuffer::WriteLine(const std::string& what)
{
    Buffer += what;
    Buffer += '\n';
}

void CStringBuffer::RebuildLocationMap()
{
    LocationMap.clear();
    for (auto iter = LocationTable.begin(); iter != LocationTable.end(); ++iter)
    {
        LocationMap[*iter] = iter.ToReference();
    }
}

}
