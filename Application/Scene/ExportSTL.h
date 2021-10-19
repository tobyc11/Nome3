#pragma once
#include "Mesh.h"
#include <vector>

namespace Nome::Scene
{

class CExportSTL
{
public:
    CExportSTL();

    void Process(const CMeshInstance& mesh);

    std::vector<uint8_t>&& Take() { return std::move(Buffer); }

private:
    std::vector<uint8_t> Buffer;
    uint32_t TriangleCount;
};

} // namespace Nome::Scene
