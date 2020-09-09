#include "Rope.h"

namespace Nome
{

CRope::CRope() {}

CRope::CRope(std::string content) { Content.push_back(content); }

std::string CRope::Assemble() const
{
    std::string result;
    for (const auto& str : Content)
    {
        result.append(str);
    }
    return result;
}

size_t CRope::GetSize() const
{
    size_t result = 0;
    for (const auto& str : Content)
    {
        result += str.size();
    }
    return result;
}

CRope::CLocation CRope::GetLoc(size_t index, size_t offset) const
{
    auto indexIter = LocationMap.find(index);
    if (indexIter != LocationMap.end())
    {
        auto offsetIter = indexIter->second.find(offset);
        if (offsetIter != indexIter->second.end())
        {
            // Such location already exists
            return CLocation(offsetIter->second);
        }
    }

    RealLocRef ref = LocationPool.GetNewElement(tc::kAddToFront);
    ref->Index = index;
    ref->Offset = offset;
    ref->RefCount = 0;

    LocationMap[index][offset] = ref;

    return CLocation(ref);
}

CRope::CLocation CRope::GetLocFromOffset(size_t offset) const
{
    size_t prefixLen = 0;
    for (size_t currIndex = 0; currIndex < Content.size(); currIndex++)
    {
        if (prefixLen <= offset && offset < prefixLen + Content[currIndex].size())
        {
            return GetLoc(currIndex, offset - prefixLen);
        }
        prefixLen += Content[currIndex].size();
    }
    throw "Offset out of range";
}

CRope::CLocation CRope::AdvanceLoc(CLocation loc, size_t offset) const
{
    size_t newIndex = loc.Reference->Index, newOffset = loc.Reference->Offset;
    while (offset)
    {
        if (newIndex >= Content.size())
            throw "Index out of range";

        size_t rowLeft = Content[newIndex].size() - 1 - newOffset;
        if (offset <= rowLeft)
        {
            newOffset += offset;
            offset = 0;
        }
        else
        {
            newIndex++;
            offset -= rowLeft;
        }
    }
    return GetLoc(newIndex, newOffset);
}

void CRope::InsertAt(CRope::CLocation loc, const std::string& text)
{
    RealLocRef realLoc = loc.Reference;

    size_t textLen = text.size();

    // Move locations after loc by textLen
    auto indexIter = LocationMap.find(realLoc->Index);
    if (indexIter != LocationMap.end())
    {
        std::map<size_t, RealLocRef> newOffsetMap;
        auto& offsetMap = indexIter->second;
        for (auto offsetIter = offsetMap.begin(); offsetIter != offsetMap.end(); ++offsetIter)
        {
            if (offsetIter->second->Offset >= realLoc->Offset)
                offsetIter->second->Offset += textLen;
            newOffsetMap.insert({ offsetIter->second->Offset, offsetIter->second });
        }
        LocationMap.insert_or_assign(realLoc->Index, newOffsetMap);
    }

    // Actually modify the string
    Content[realLoc->Index].insert(realLoc->Offset, text);
}

void CRope::Replace(CRope::CLocation leftLoc, CRope::CLocation rightLoc, const std::string& text)
{
    throw "Unimplemented";
}

void CRope::GarbageCollectLocations() const
{
    for (auto iter = LocationPool.begin(); iter != LocationPool.end(); ++iter)
    {
        if (iter->RefCount == 0)
        {
            // Erase from map
            LocationMap.at(iter->Index).erase(iter->Offset);
            // Delete from pool
            LocationPool.FreeElement(iter);
        }
    }
}

void CRope::SplitRow(const CRope::CRealLocation& where) { throw "Unimplemented"; }

}
