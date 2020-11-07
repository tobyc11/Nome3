#include "MobiusStrip.h"
#include <vector>
#include <cmath>

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CMobiusStrip)
{
    BindPositionalArgument(&CMobiusStrip::N, 1, 0);
    BindPositionalArgument(&CMobiusStrip::Radius, 1, 1);
    BindPositionalArgument(&CMobiusStrip::NumTwists, 1, 2);
    BindPositionalArgument(&CMobiusStrip::NumCuts, 1, 3);
}

void CMobiusStrip::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    // load in arguments to Mobius Strip generator
    float n = (float)N.GetValue(100.0f); // number of individual points on each band
    float radius = (float)Radius.GetValue(1.0f); // total radius
    int numTwists = (int)ceil(NumTwists.GetValue(1.0f)); // number of twists
    int numCuts = (int)ceil(NumCuts.GetValue(0.0f)); // number of times surface is cut
    float bandwidth = 2*radius/((numCuts*2) + 1); // radius of each band

    // create vertices
    float uIncrement = (1.0f/n)*(float)tc::M_PI;
    int uCounter = 0;
    for (float u = 0.0f; u < 2.f * (float)tc::M_PI + uIncrement/3; u += uIncrement)
    { // uIncrement/3 allows n+1 total vertices, accounting for rounding error (n+1th vertex == 0th vertex)
        int vCounter = 0;
        for (float v = -1*radius; v <= radius + bandwidth/3; v += bandwidth)
        { // bandwidth/3 accounts for rounding error
            float x = (1+(v/2.0f)*cosf((numTwists*u)/2.0f))*cosf(u);
            float y = (1+(v/2.0f)*cosf((numTwists*u)/2.0f))*sinf(u);
            float z = (v/2.0f)*sinf((numTwists*u)/2.0f);
            AddVertex("v_" + std::to_string(uCounter) + "_" + std::to_string(vCounter), // name ex. "v_0_5"
                      { x, y, z } );
            vCounter++;
        }
        uCounter++;
    }

    // add faces
    for (int uFaceCounter = 0; uFaceCounter + 1 < uCounter; uFaceCounter++)
    {
        for (int cut = 0; cut <= numCuts; cut++)
        {
            std::vector<std::string> face;

            face.push_back("v_" + std::to_string(uFaceCounter) + "_" + std::to_string(2*cut)); //2*cut
            face.push_back("v_" + std::to_string(uFaceCounter + 1) + "_" + std::to_string(2*cut));
            face.push_back("v_" + std::to_string(uFaceCounter + 1) + "_" + std::to_string(2*cut+1)); //2*cut+1
            face.push_back("v_" + std::to_string(uFaceCounter) + "_" + std::to_string(2*cut+1));

            AddFace("f1_" + std::to_string(uFaceCounter) + "_" + std::to_string(cut), face);
        }
    }
}

}
