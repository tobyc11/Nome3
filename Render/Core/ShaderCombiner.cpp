#include "ShaderCombiner.h"

#include <Hash.h>
#include <PathTools.h>
#include <ShaderModule.h>
#include <slang.h>

#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>

namespace Nome::Render
{

using namespace RHI;

struct VSSig
{
    std::string VertexShader;
    std::string SurfaceInteraction;
    std::string CompileDefs;

    bool operator==(const VSSig& r) const
    {
        return VertexShader == r.VertexShader && SurfaceInteraction == r.SurfaceInteraction && CompileDefs == r.CompileDefs;
    }

    friend std::size_t hash_value(const VSSig& r)
    {
        std::size_t h = 0;
        tc::hash_combine(h, r.VertexShader);
        tc::hash_combine(h, r.SurfaceInteraction);
        tc::hash_combine(h, r.CompileDefs);
        return h;
    }
};

struct PSSig
{
    std::string SurfaceInteraction;
    std::string Integrator;
    std::string Material;
    std::string CompileDefs;

    bool operator==(const PSSig& r) const
    {
        return SurfaceInteraction == r.SurfaceInteraction && Integrator == r.Integrator && Material == r.Material && CompileDefs == r.CompileDefs;
    }

    friend std::size_t hash_value(const PSSig& r)
    {
        std::size_t h = 0;
        tc::hash_combine(h, r.SurfaceInteraction);
        tc::hash_combine(h, r.Integrator);
        tc::hash_combine(h, r.Material);
        tc::hash_combine(h, r.CompileDefs);
        return h;
    }
};

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>>
class GenericCache
{
public:
    bool Contains(const TKey& key) const
    {
        auto iter = hashMap.find(key);
        return iter != hashMap.end();
    }

    TValue Get(const TKey& key) const
    {
        auto iter = hashMap.find(key);
        return iter->second;
    }

    void Put(const TKey& key, TValue value) { hashMap[key] = value; }

    void Clear() { hashMap.clear(); }

    // void Save(const std::string& path) const;
    // void Load(const std::string& path);

private:
    std::unordered_map<TKey, TValue, THasher> hashMap;
};

using VSCache = GenericCache<VSSig, sp<CShaderModule>, tc::hash<VSSig>>;
using PSCache = GenericCache<PSSig, sp<CShaderModule>, tc::hash<PSSig>>;

static VSCache VertexShaderCache;
static PSCache PixelShaderCache;

struct SlangGlobalSession
{
    SlangSession* Session;

    SlangGlobalSession()
    {
        Session = spCreateSession();
    }

    ~SlangGlobalSession()
    {
        spDestroySession(Session);
    }
};

static SlangGlobalSession MySlang;

sp<CShaderModule> InstanciateVS(const VSSig& signature, const std::set<std::string>& definitions)
{
    std::string targetFile = "D:/Dev/Nome3/Render/Shaders/" + signature.VertexShader + ".slang";
    auto targetDir = tc::FPathTools::StripFilename(targetFile, '/');
    targetDir += "/";

    SlangCompileRequest* request = spCreateCompileRequest(MySlang.Session);
    spSetCodeGenTarget(request, SLANG_DXBC);
    spAddSearchPath(request, targetDir.c_str());
    spAddPreprocessorDefine(request, "__SLANG__", "1");
    for (const std::string& def : definitions)
        spAddPreprocessorDefine(request, def.c_str(), "1");

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "");
    spAddTranslationUnitSourceFile(request, translationUnitIndex, targetFile.c_str());
    //SlangProfileID profileID = spFindProfile(session, "sm_4_0");

    int mainEntry = spAddEntryPoint(
        request, translationUnitIndex, (signature.VertexShader + "VSMain").c_str(), SLANG_STAGE_VERTEX);

    int anyErrors = spCompile(request);
    char const* diagnostics = spGetDiagnosticOutput(request);
    if (anyErrors)
    {
        std::cout << "Errors encountered when compiling shaders for VS " << signature.VertexShader << std::endl;
    }
    printf("%s", diagnostics);

    sp<CShaderModule> shader;
    if (anyErrors == 0)
    {
        size_t dataSize = 0;
        void const* data = spGetEntryPointCode(request, mainEntry, &dataSize);
        shader = new CShaderModule(data, dataSize, DXBCBlob());
    }

    spDestroyCompileRequest(request);
    return shader;
}

sp<CShaderModule> InstanciatePS(const PSSig& signature, const std::set<std::string>& definitions)
{
    std::string targetFile = "D:/Dev/Nome3/Render/Shaders/" + signature.Integrator + ".slang";
    auto targetDir = tc::FPathTools::StripFilename(targetFile, '/');
    targetDir += "/";

    SlangCompileRequest* request = spCreateCompileRequest(MySlang.Session);
    spSetCodeGenTarget(request, SLANG_DXBC);
    spAddSearchPath(request, targetDir.c_str());
    spAddPreprocessorDefine(request, "__SLANG__", "1");
    for (const std::string& def : definitions)
        spAddPreprocessorDefine(request, def.c_str(), "1");

    int translationUnitIndex = spAddTranslationUnit(request, SLANG_SOURCE_LANGUAGE_SLANG, "");
    spAddTranslationUnitSourceFile(request, translationUnitIndex, targetFile.c_str());
    //SlangProfileID profileID = spFindProfile(session, "sm_5_0");

    const char* globalArgs[] = { signature.SurfaceInteraction.c_str(), signature.Material.c_str() };
    spSetGlobalGenericArgs(request, 2, globalArgs);

    int mainEntry = spAddEntryPoint(
        request, translationUnitIndex, (signature.Integrator + "PSMain").c_str(), SLANG_STAGE_FRAGMENT);

    int anyErrors = spCompile(request);
    char const* diagnostics = spGetDiagnosticOutput(request);
    if (anyErrors)
    {
        std::cout << "Errors encountered when compiling shaders for PS " << signature.Integrator
                  << std::endl;
    }
    printf("%s", diagnostics);

    sp<CShaderModule> shader;
    if (anyErrors == 0)
    {
        size_t dataSize = 0;
        void const* data = spGetEntryPointCode(request, mainEntry, &dataSize);
        shader = new CShaderModule(data, dataSize, DXBCBlob());
    }

    spDestroyCompileRequest(request);
    return shader;
}

void CShaderCombiner::ClearShaderCache()
{
    VertexShaderCache.Clear();
    PixelShaderCache.Clear();
}

void CShaderCombiner::SetVertexShader(const std::string& type) { VertexShaderType = type; }

void CShaderCombiner::SetSurfaceInteraction(const std::string& type) { SurfaceInteractionType = type; }

void CShaderCombiner::SetIntegrator(const std::string& type) { IntegratorType = type; }

void CShaderCombiner::SetMaterial(const std::string& type) { MaterialType = type; }

void CShaderCombiner::AddCompileDefinition(const std::string& def)
{
    CompileDefs.insert(def);
}

void CShaderCombiner::BindToDrawTemplate(CDrawTemplate& drawTemplate)
{
    //combine all definitions into one string
    std::string aggregate;
    for (const auto& def : CompileDefs)
        aggregate += def;

    VSSig vsSig{ VertexShaderType, SurfaceInteractionType, aggregate };
    sp<CShaderModule> vsModule;
    if (!VertexShaderCache.Contains(vsSig))
    {
        std::cout << "VS Cache Miss: " << VertexShaderType << ", " << SurfaceInteractionType << ", " << aggregate << std::endl;
        vsModule = InstanciateVS(vsSig, CompileDefs);
        VertexShaderCache.Put(vsSig, vsModule);
    }
    else
    {
        vsModule = VertexShaderCache.Get(vsSig);
    }

    PSSig psSig{ SurfaceInteractionType, IntegratorType, MaterialType, aggregate };
    sp<CShaderModule> psModule;
    if (!PixelShaderCache.Contains(psSig))
    {
        std::cout << "PS Cache Miss: " << SurfaceInteractionType << ", " << IntegratorType << ", " << MaterialType << ", " << aggregate << std::endl;
        psModule = InstanciatePS(psSig, CompileDefs);
        PixelShaderCache.Put(psSig, psModule);
    }
    else
    {
        psModule = PixelShaderCache.Get(psSig);
    }

    drawTemplate.SetVertexShader(vsModule);
    drawTemplate.SetPixelShader(psModule);
}

} /* namespace Nome::Render */
