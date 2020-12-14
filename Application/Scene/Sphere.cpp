#include "Sphere.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSphere)
{
    BindPositionalArgument(&CSphere::Radius, 1, 0);
    BindPositionalArgument(&CSphere::ThetaMax, 1, 1);
    BindPositionalArgument(&CSphere::PhiMin, 1, 2);
    BindPositionalArgument(&CSphere::PhiMax, 1, 3);
    BindPositionalArgument(&CSphere::ThetaSegs, 1, 4);
    BindPositionalArgument(&CSphere::PhiSegs, 1, 5);
}

void CSphere::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    int n = (int)PhiSegs.GetValue(6.0f);
    float radius = Radius.GetValue(1.0f);
    int numCrossSections = (int)ThetaSegs.GetValue(6.0f);
    int maxTheta = (int)ThetaMax.GetValue(6.0f);
    int minPhi = (int)PhiMin.GetValue(6.0f) + 90;
    int maxPhi = (int)PhiMax.GetValue(6.0f) + 90;

    float startPhi = minPhi / 180.f * (float)tc::M_PI;

    float width = 0;
    for (int j = 0; j <= numCrossSections; j++) {
      float rotationTheta = (float)j / numCrossSections * (maxTheta / 360.f) * 2.f * (float)tc::M_PI;
      for (int i = 0; i <= n; i++) {
          float phi = startPhi + ((float)i / n * ((maxPhi - minPhi) / 180.f) * (float)tc::M_PI);
          float x = radius * cosf(phi);
          float y = radius * sinf(phi);
          float z = 0;
          float rotatedX = y * cosf(rotationTheta);
          float rotatedY = y * sinf(rotationTheta);
          float rotatedZ = x;
          AddVertex("v" + std::to_string(j) + "-" + std::to_string(i), {rotatedX, rotatedY, rotatedZ});
          if (j == 0) {
            if (i == 0) {
              width += rotatedZ;
              AddVertex("top", { 0, 0, rotatedZ});
            } else if (i == n) {
              width += rotatedZ;
              AddVertex("bottom", { 0, 0, rotatedZ});
            }
          }
      }
    }

    // Create faces
    for (int k = 0; k <= numCrossSections; k++) {
        for (int c = 0; c < n; c++) {
            // CCW winding
            int next_k = (k + 1) % (numCrossSections + 1);
            int next_c = (c + 1) % (n + 1);
            std::vector<std::string> upperFace = {
                // CCW
                "v" + std::to_string(next_k) + "-" + std::to_string(next_c),
                "v" + std::to_string(next_k) + "-" + std::to_string(c),
                "v" + std::to_string(k) + "-" + std::to_string(c),
                "v" + std::to_string(k) + "-" + std::to_string(next_c),
            };
            if (k == numCrossSections) {
              if (maxTheta == 360) {
                AddFace("f" + std::to_string(k) + "-" + std::to_string(c), upperFace);
              }
            } else {
              AddFace("f" + std::to_string(k) + "-" + std::to_string(c), upperFace);
            }
        }
    }

    // add top face
    int i = 0;
    for (int j = 0; j <= numCrossSections; j++) {
      if (j != numCrossSections || maxTheta == 360) {
        std::vector<std::string> face = {
          "v" + std::to_string(j) + "-" + std::to_string(i),
          "v" + std::to_string((j + 1) % (numCrossSections + 1)) + "-" + std::to_string(i),
          "top",
        };
        AddFace("top-face" + std::to_string(j), face);
      }
    }

    // add bottom face
    i = n;
    for (int j = 0; j <= numCrossSections; j++) {
      if (j != numCrossSections || maxTheta == 360) {
        std::vector<std::string> face = {
          "bottom",
          "v" + std::to_string((j + 1) % (numCrossSections + 1)) + "-" + std::to_string(i),
          "v" + std::to_string(j) + "-" + std::to_string(i),
        };
        AddFace("bottom-face" + std::to_string(j), face);
      }
    }

    AddVertex("center", { 0, 0, width / 2 });

    // add slice faces
    if (maxTheta != 360) {
      // add slice face 1
      int j = 0;
      for (int i = 0; i < n; i++) {
        std::vector<std::string> face = {
        "v" + std::to_string(j) + "-" + std::to_string((i + 1) % (n + 1)),
        "v" + std::to_string(j) + "-" + std::to_string(i),
        "center",
        };
        AddFace("slice-face" + std::to_string(j) + "-" + std::to_string(i), face);
      }
      std::vector<std::string> slice_face_top = {
      "top",
      "center",
      "v" + std::to_string(j) + "-" + std::to_string(0),
      };
      AddFace("slice-face-side-top" + std::to_string(j), slice_face_top);
      std::vector<std::string> slice_face_bottom = {
        "v" + std::to_string(j) + "-" + std::to_string(n),
        "center",
        "bottom",
      };
      AddFace("slice-face-side-bottom" + std::to_string(j), slice_face_bottom);

      // add slice face 2
      j = numCrossSections;
      for (int i = 0; i < n; i++) {
        std::vector<std::string> face = {
          "center",
          "v" + std::to_string(j) + "-" + std::to_string(i),
          "v" + std::to_string(j) + "-" + std::to_string((i + 1) % (n + 1)),
        };
        AddFace("slice-face" + std::to_string(j) + "-" + std::to_string(i), face);
      }
      slice_face_top = {
        "v" + std::to_string(j) + "-" + std::to_string(0),
        "center",
        "top",
      };
      AddFace("slice-face-side-top" + std::to_string(j), slice_face_top);
      slice_face_bottom = {
        "bottom",
        "center",
        "v" + std::to_string(j) + "-" + std::to_string(n),
      };
      AddFace("slice-face-side-bottom" + std::to_string(j), slice_face_bottom);

    }


  }



}
