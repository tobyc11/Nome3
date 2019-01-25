#pragma once
#include <LightRefBase.h>
#include <VectorPool.h>
#include <string>
#include <vector>
#include <map>

namespace Nome
{

//A data structure that holds a list of strings to allow fast update
//Note: this is not yet a proper implementation of the rope ADT. Will change in the future.
class CRope : public tc::TLightRefBase<CRope>
{
    class CRealLocation
    {
    public:
        //Select which string
        size_t Index;
        //An offset into the string
        size_t Offset;

        int32_t RefCount = 0;
    };

    typedef tc::TVectorPool<CRealLocation>::VectorPoolReference RealLocRef;

public:
    class CLocation
    {
    public:
		CLocation() {}

        CLocation(RealLocRef ref) : Reference(ref)
        {
            if (Reference.IsValid())
                Reference->RefCount++;
        }

        ~CLocation()
        {
            if (Reference.IsValid())
                Reference->RefCount--;
        }

		bool IsValid() const
		{
			return Reference.IsValid();
		}

    private:
        friend class CRope;
        RealLocRef Reference;
    };

    CRope();
    CRope(std::string content);

    std::string Assemble() const;
    size_t GetSize() const;

    CLocation GetLoc(size_t index, size_t offset) const;
    CLocation GetLocFromOffset(size_t offset) const;
    CLocation AdvanceLoc(CLocation loc, size_t offset) const;

    void InsertAt(CLocation loc, const std::string& text);
    void Replace(CLocation leftLoc, CLocation rightLoc, const std::string& text);

    void GarbageCollectLocations() const;

private:
    void SplitRow(const CRealLocation& where);

    std::vector<std::string> Content;
    mutable tc::TVectorPool<CRealLocation> LocationPool;
    mutable std::map<size_t, std::map<size_t, RealLocRef>> LocationMap;
};

}
