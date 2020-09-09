#pragma once
#include <Image.h>
#include <ImageView.h>

#include <string>
#include <unordered_map>

namespace Nome::Render
{

class CImageHolder
{
    friend class CImageLibrary;
public:
    tc::sp<RHI::CImage> GetImage2D() const;
    tc::sp<RHI::CImageView> GetImageView2D() const;

private:
    void CreateImage2D();

    std::unique_ptr<unsigned char[]> Data;
    int Width, Height, Channels;

    tc::sp<RHI::CImage> Image2D;
    tc::sp<RHI::CImageView> ImageView2D;
};

class CImageLibrary
{
public:
    static CImageLibrary& Get();

    CImageLibrary();
    ~CImageLibrary();

    CImageHolder* Load(const std::string& name, const std::string& filename);
    CImageHolder* Load(const std::string& name);

private:
    std::unordered_map<std::string, CImageHolder*> Images;
};

} /* namespace Nome::Render */
