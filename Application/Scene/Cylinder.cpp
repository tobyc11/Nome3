#include "Cylinder.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CCylinder)
{
    BindPositionalArgument(&CCylinder::Radius, 1, 0);
    BindPositionalArgument(&CCylinder::Height, 1, 1);
    BindPositionalArgument(&CCylinder::ThetaMax, 1, 2);
    BindPositionalArgument(&CCylinder::ThetaSegs, 1, 3);
}

void CCylinder::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    float radius = Radius.GetValue(1.0f);
    float height = Height.GetValue(1.0f);
    int maxTheta = (int)ThetaMax.GetValue(6.0f);
    int numSegs = (int)ThetaSegs.GetValue(6.0f);

    for (int j = 0; j < 2; j++) {
      for (int i = 0; i < numSegs; i++) {
          float theta = (float)i / numSegs * ((float) maxTheta / 360.f) * 2.f * (float)tc::M_PI;
          float x = radius * cosf(theta);
          float y = radius * sinf(theta);
          float z = j * height;
          AddVertex("v" + std::to_string(j) + "-" + std::to_string(i),
                                    { x, y, z });
      }
    }

    AddVertex("top-center", {0, 0, 0});
    AddVertex("bottom-center", {0, 0, height});

    // add side faces
    for (int c = 0; c < numSegs; c++) {
      // CCW winding
      int next_c = (c + 1) % numSegs;
      std::vector<std::string> face = {
          // CCW
          "v" + std::to_string(1) + "-" + std::to_string(next_c),
          "v" + std::to_string(1) + "-" + std::to_string(c),
          "v" + std::to_string(0) + "-" + std::to_string(c),
          "v" + std::to_string(0) + "-" + std::to_string(next_c),
      };
      if (c == numSegs - 1) {
        if (maxTheta == 360) {
          AddFace("f-" + std::to_string(c), face);
        }
      } else {
        AddFace("f-" + std::to_string(c), face);
      }
    }

    // add wedge faces
    if (maxTheta != 360) {
      std::vector<std::string> face1 = {
        "top-center",
        "bottom-center",
        "v" + std::to_string(1) + "-" + std::to_string(numSegs - 1),
        "v" + std::to_string(0) + "-" + std::to_string(numSegs - 1),
      };
      AddFace("wedge1", face1);
      std::vector<std::string> face2 = {
        "v" + std::to_string(0) + "-" + std::to_string(0),
        "v" + std::to_string(1) + "-" + std::to_string(0),
        "bottom-center",
        "top-center",
      };
      AddFace("wedge2", face2);
    }


    // add top and bottom faces
    int j = 0;
    std::vector<std::string> face = {};
    if (maxTheta != 360) {
      face.push_back("top-center");
    }
    for (int i = numSegs - 1; i >= 0; i--) {
      face.push_back("v" + std::to_string(j) + "-" + std::to_string(i));
    }
    AddFace("cap-face" + std::to_string(j), face);

    j = 1;
    face = {};
    if (maxTheta != 360) {
      face.push_back("bottom-center");
    }
    for (int i = 0; i < numSegs; i++) {
        face.push_back("v" + std::to_string(j) + "-" + std::to_string(i));
    }
    AddFace("cap-face" + std::to_string(j), face);


}



}
