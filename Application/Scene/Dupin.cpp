#include "Dupin.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CDupin)
{
    BindPositionalArgument(&CDupin::A, 1, 0);
    BindPositionalArgument(&CDupin::B, 1, 1);
    BindPositionalArgument(&CDupin::C, 1, 2);
    BindPositionalArgument(&CDupin::D, 1, 3);
    BindPositionalArgument(&CDupin::U, 1, 4);
    BindPositionalArgument(&CDupin::V, 1, 5);
    BindPositionalArgument(&CDupin::Crosssec, 1, 6);


}

void CDupin::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    float a = A.GetValue(1.0f);
    float b = B.GetValue(1.0f);
    float c = C.GetValue(1.0f);
    float d = D.GetValue(1.0f);
    float u = U.GetValue(1.0f);
    float v = V.GetValue(1.0f);
    int crossec = (int)Crosssec.GetValue(6.0f);

    for (int i = 0; i < crossec; i++) {
        for (int j = 0; j < crossec; j++) {
            float theta1 = (float)i / crossec * u * (float)tc::M_PI;
            float theta2 = (float)j / crossec * v * (float)tc::M_PI;
            AddVertex("v" + std::to_string(i) + "*" + std::to_string(j), {(d*(c - a * cosf(theta1)*cosf(theta2)) + b*b*cosf(theta1))/(a - c*cosf(theta1)*cosf(theta2)),(b*sinf(theta1)*(a-d*cosf(theta2)))/(a-c*cosf(theta1)*cosf(theta2)), (b*sinf(theta2)*(c*cosf(theta1)-d))/(a - c*cosf(theta1)*cosf(theta2))});
        }
    }
    for (int i = 0; i < crossec; i++) {
        for (int j = 0; j < crossec; j++) { 
            if (j == crossec - 1 && i == crossec - 1) {

            } else if (j == crossec - 1) {
                if (v == 2) {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(0), "v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(0)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(0)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);    
                } else {
                }     
            } else if (i == crossec - 1) {
                if (u == 2) {
                    std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                    std::vector<std::string> face2 = {"v" + std::to_string(0) + "*" + std::to_string(j), "v" + std::to_string(0) + "*" + std::to_string(j + 1), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                    AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);    
                } else {

                }
            }
            else {
                std::vector<std::string> face1 = {"v" + std::to_string(i) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i) + "*" + std::to_string(j + 1)};
                AddFace("f1_" + std::to_string(i) + "-" + std::to_string(j), face1);
                std::vector<std::string> face2 = {"v" + std::to_string(i) + "*" + std::to_string(j + 1), "v" + std::to_string(i + 1) + "*" + std::to_string(j), "v" + std::to_string(i + 1) + "*" + std::to_string(j + 1)};
                AddFace("f2_" + std::to_string(i) + "-" + std::to_string(j), face2);      
            }
        }
    }
}

}
