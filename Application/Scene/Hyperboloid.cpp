#include "Hyperboloid.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CHyperboloid)
{
    BindPositionalArgument(&CHyperboloid::Segments, 1, 0);
    BindPositionalArgument(&CHyperboloid::Radius, 1, 1);
    BindPositionalArgument(&CHyperboloid::B, 1, 2);
    BindPositionalArgument(&CHyperboloid::C, 1, 3);
    BindPositionalArgument(&CHyperboloid::Sheet, 1, 4);
    BindPositionalArgument(&CHyperboloid::Theta, 1, 5);

}

void CHyperboloid::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    int n = (int)Segments.GetValue(6.0f);
    float a = Radius.GetValue(1.0f);
    float b = B.GetValue(1.0f);
    float c = C.GetValue(1.0f);
    int sheet = (int)Sheet.GetValue(1.0f);
    float angle = Theta.GetValue(1.0f);
    int crosssec = (int)c; 

    if (sheet == 0) {
        for (int j = -crosssec; j <= crosssec; j++) {
            for (int i = 0; i < n; i++)
            {
                float theta = (float)i / n * angle * (float)tc::M_PI;
                AddVertex("v" + std::to_string(i) + "*" + std::to_string(j), {a * (float)sqrt(j*j + 1) * cosf(theta), b * (float)sqrt(j*j + 1) * sinf(theta), (float)j});
            }
        }

        for (int j = -crosssec; j < crosssec; j++) {
            for (int i = 0; i < n; i++) {
                if (i == n - 1 && angle == 2) {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j + 1), "v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j + 1)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);                     

                } else if (i == n - 1 && angle != 2) {

                } else {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j + 1), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j + 1)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);      
                }   
            }
        }
    } else {
        for (int j = -crosssec; j <= crosssec; j++) {
            for (int i = 0; i < n; i++)
            {
                float theta = (float)i / n * angle * (float)tc::M_PI;
                AddVertex("v" + std::to_string(i) + "*" + std::to_string(j), {a * (float)sqrt(j*j - 1) * cosf(theta), b * (float)sqrt(j*j - 1) * sinf(theta), (float)j});
            }
        }
        for (int j = -crosssec; j < crosssec; j++) {
            for (int i = 0; i < n; i++) {
                if (i == n - 1 && angle == 2) {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j + 1), "v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j + 1)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);                     

                } else if (i == n - 1 && angle != 2) {

                } else {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j + 1), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j + 1)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);      
                }   
            }
        }

    }
}

}
