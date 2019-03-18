#include "ImageLibrary.h"
#include <Device.h>
#include <RHIInstance.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Nome::Render
{

void CImageHolder::CreateImage2D()
{
    RHI::EFormat fmt = RHI::EFormat::R8G8B8A8_UNORM;
    auto* device = RHI::CInstance::Get().GetCurrDevice();
    Image2D = device->CreateImage2D(fmt, RHI::EImageUsageFlags::Sampled, Width, Height);
    Image2D->CopyFrom(Data.get());

    RHI::CImageViewDesc desc;
    desc.Format = fmt;
    desc.Type = RHI::EImageViewType::View2D;
    desc.Range.BaseMipLevel = 0;
    desc.Range.LevelCount = 1;
    ImageView2D = device->CreateImageView(desc, Image2D);
}

tc::sp<RHI::CImage> CImageHolder::GetImage2D() const
{
    return Image2D;
}

tc::sp<RHI::CImageView> CImageHolder::GetImageView2D() const
{
    return ImageView2D;
}

CImageLibrary& CImageLibrary::Get()
{
    static CImageLibrary sLib;
    return sLib;
}

CImageLibrary::CImageLibrary()
{
}

CImageLibrary::~CImageLibrary()
{
    for (auto pair : Images)
    {
        delete pair.second;
    }
    Images.clear();
}

CImageHolder* CImageLibrary::Load(const std::string& name, const std::string& filename)
{
    if (Images.find(name) != Images.end())
        return Images[name];

    int x, y, n;
    unsigned char* data = stbi_load(filename.c_str(), &x, &y, &n, 4);
    if (data)
    {
        size_t sz = 4 * x * y; //1 byte per channel
        CImageHolder* holder = new CImageHolder();
        holder->Width = x;
        holder->Height = y;
        holder->Channels = 4;
        holder->Data = std::make_unique<unsigned char[]>(sz);
        memcpy(holder->Data.get(), data, sz);
        stbi_image_free(data);

        holder->CreateImage2D();
        Images.emplace(name, holder);
        return holder;
    }
    return nullptr;
}

CImageHolder* CImageLibrary::Load(const std::string& name)
{
    if (Images.find(name) != Images.end())
        return Images[name];

    int x, y, n;
    unsigned char* data = stbi_load(name.c_str(), &x, &y, &n, 4);
    if (data)
    {
        size_t sz = 4 * x * y; //1 byte per channel
        CImageHolder* holder = new CImageHolder();
        holder->Width = x;
        holder->Height = y;
        holder->Channels = 4;
        holder->Data = std::make_unique<unsigned char[]>(sz);
        memcpy(holder->Data.get(), data, sz);
        stbi_image_free(data);

        holder->CreateImage2D();
        Images.emplace(name, holder);
        return holder;
    }
    return nullptr;
}

} /* namespace Nome::Render */
