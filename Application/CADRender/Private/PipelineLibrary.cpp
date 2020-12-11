#include "PipelineLibrary.h"
#include <sys/stat.h>

static std::unordered_map<EShaderStage, std::string> SuffixMap = { { EShaderStage::Vertex, ".vert.glsl" },
                                                                   { EShaderStage::Fragment, ".frag.glsl" } };

static std::string ReadFile(const std::string& under, const std::string& name)
{
    auto path = under + '/' + name;
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
    {
        FILE* fp = fopen(path.c_str(), "r");
        if (!fp)
            return {};
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        std::string dataBuf;
        dataBuf.resize(size);
        fread(dataBuf.data(), 1, size, fp);
        return dataBuf;
    }
    return {};
}

template <typename T, int size> int GetArrLength(T (&)[size]) { return size; }

std::shared_ptr<CShaderObject> CPipelineLibrary::FindShader(CGLThread* device, EShaderStage type,
                                                            const std::string& name)
{
    std::string topDirs[] = { "./", "../", "../../", "../../../" };
    std::string subDirs[] = { "Shaders", "Application/CADRender/Shaders" };
    // Find file first
    auto fullName = name + SuffixMap[type];
    for (int i = 0; i < GetArrLength(topDirs); i++)
        for (int j = 0; j < GetArrLength(subDirs); j++)
        {
            auto path = topDirs[i] + subDirs[j];
            auto dataBuf = ReadFile(path, fullName);
            if (!dataBuf.empty())
                return std::make_shared<CShaderObject>(device, type, dataBuf);
        }

    // Potentially we also provide a way to bundle up shaders into a blob
    return {};
}
