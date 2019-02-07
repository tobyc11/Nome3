#include "StringBuffer.h"

namespace Nome
{

CStringBuffer::CStringBuffer()
{
}

CStringBuffer::CStringBuffer(const std::string& content) : Buffer(content)
{
}

CStringBuffer::CLocation CStringBuffer::GetLocation(size_t offset)
{
    if (offset > Buffer.size())
        return {};

    for (auto iter = LocationTable.begin(); iter != LocationTable.end(); ++iter)
    {
        if (*iter == offset)
            return iter.ToReference();
    }

    CLocation loc = LocationTable.GetNewElement(tc::kAddToBack);
    *loc = offset;
    return loc;
}

const std::string& CStringBuffer::GetAsString() const
{
    return Buffer;
}

void CStringBuffer::ReplaceRange(size_t begin, size_t end, const std::string& content)
{
    assert(end >= begin);
    size_t lenBefore = end - begin;
    size_t lenAfter = content.size();
    ptrdiff_t deltaLen = (ptrdiff_t)lenAfter - lenBefore;
    //Now move all locations after end (inclusive) by deltaLen
    if (deltaLen != 0)
    {
        for (auto& loc : LocationTable)
        {
            if (loc >= end)
                loc += deltaLen;
        }
    }
    //Now replace the actual content
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

}
