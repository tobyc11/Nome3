#include "Torus.h"
#include <cmath>

#undef M_PI

namespace Nome::Scene
{

typedef struct
{
    float x;
    float y;
    float z;
} Point;

DEFINE_META_OBJECT(CTorus)
{
    BindPositionalArgument(&CTorus::VerticesPerRing, 1, 0);
    BindPositionalArgument(&CTorus::MajorRadius, 1, 1);
    BindPositionalArgument(&CTorus::MinorRadius, 1, 2);
    BindPositionalArgument(&CTorus::ThetaMax, 1, 3);
    // BindPositionalArgument(&CTorus::PhiMin, 1, 4);
    BindPositionalArgument(&CTorus::PhiMax, 1, 4);
    BindPositionalArgument(&CTorus::Segments, 1, 5);
}
void CTorus::UpdateEntity()
{

    if (!IsDirty())
        return;

    // Clear mesh
    Super::UpdateEntity();


    // Initialize torus parameters from document 
    int numPhi = static_cast<int>(VerticesPerRing.GetValue(16.0f));
    float majorRadius = MajorRadius.GetValue(1.0f);
    float minorRadius = MinorRadius.GetValue(1.0f);
    float thetaMax = ThetaMax.GetValue(1.0f);
    // float phiMin = PhiMin.GetValue(1.0f);
    float phiMax= PhiMax.GetValue(1.0f);

    // number of circles or cross sections
    int numSegments = Segments.GetValue(1.0f); 

    const float epsilon = 1e-4;
    const float dt = (thetaMax*(float)tc::M_PI/180.0f) / (numSegments);
    const float du = (phiMax * (float)tc::M_PI / 180.0f) / numPhi;



    // Create torus, creating one cross section at each iteration
    for (int i = 0; i < numSegments + 1; i++) // numSegments + 1; for some reason numSegments was outputting an off by one torus...
    {
        float t0 = i * dt;

        Point p0 = {majorRadius*cos(t0), majorRadius*sinf(t0), 0}; 

        // Below, we'll work on approximating the Frenet frame { T, N, B } for the curve at the current point

        float t1 = t0 + epsilon;


        // p1 is p0 advanced infinitesimally along the curve

        Point p1 = {majorRadius*cos(t1), majorRadius*sinf(t1), 0 }; //uncomment this to do torus instead 

        // compute approximate tangent as vector connecting p0 to p1
        Point T = { p1.x - p0.x, p1.y - p0.y, p1.z - p0.z };

        // rough approximation of normal vector
        Point N = { p1.x + p0.x, p1.y + p0.y, p1.z + p0.z };

        //>> Cross({x1, y1, z1}, {x2, y2, z2})
        //>> { -y2 * z1 + y1 * z2, x2 * z1 - x1 * z2, -x2* y1 + x1* y2 }
        // compute binormal of curve
        Point B = { (-1.0f * N.y) * T.z + T.y * N.z, N.x * T.z - T.x * N.z,
                    (-1.0f * N.x) * T.y + T.x * N.y }; // vector_cross(T, N);

        // refine normal vector by Graham-Schmidt
        N = {
            (-1.0f * T.y) * B.z + B.y * T.z, T.x * B.z - B.x * T.z, (-1.0f * T.x) * B.y + B.x * T.y
        }; // N = vector_cross(B, T); FORGOT TO CHANGE -1*N to -1*T for the second one.

        // B = vector_normalize(B); https://github.com/Jam3/math-as-code
        float B_squaredlength = B.x * B.x + B.y * B.y + B.z * B.z;
        float B_length = sqrt(B_squaredlength);
        B = { B.x / B_length, B.y / B_length, B.z / B_length };

        // N = vector_normalize(N);
        float N_squaredlength = N.x * N.x + N.y * N.y + N.z * N.z;
        float N_length = sqrt(N_squaredlength);
        N = { N.x / N_length, N.y / N_length, N.z / N_length };

        // generate points in a circle perpendicular to the curve at the current point
        for (int j = 0; j < numPhi; ++j)
        {
            float u = j * du;

            // compute position of circle point
            float x = minorRadius * cosf(u);
            float y = minorRadius * sinf(u);

            Point p2 = { x * N.x + y * B.x, x * N.y + y * B.y, x * N.z + y * B.z };

            Point curr_vertex;
            curr_vertex.x = p0.x + p2.x;
            curr_vertex.y = p0.y + p2.y;
            curr_vertex.z = p0.z + p2.z;

            AddVertex("v" + std::to_string(i + 1) + "_" + std::to_string(j),
                      { curr_vertex.x, curr_vertex.y, curr_vertex.z });
        }
    }
    // Create faces
    if (thetaMax == 360)
    {

        for (int k = 0; k < numSegments + 1; k++)
        {
            for (int i = 0; i < numPhi; i++)
            {
                // CCW winding
                // v1_next v1_i
                // v2_next v2_i
                int next = (i + 1) % numPhi;
                int next_k = (k + 1) % numSegments;
                std::vector<std::string> upperFace = {
                    "v" + std::to_string(k + 1) + "_" + std::to_string(next),
                    "v" + std::to_string(k + 1) + "_" + std::to_string(i),
                    "v" + std::to_string(next_k + 1) + "_" + std::to_string(i),
                    "v" + std::to_string(next_k + 1) + "_" + std::to_string(next)
                };
                AddFace("f1_" + std::to_string(i), upperFace);
            }
        }
    }
    else
    {
        for (int k = 0; k < numSegments; k++) // numSegments instead of numsegments + 1 because we don't to connect the last segment with the first segment
        {
            for (int i = 0; i < numPhi; i++)
            {
                // CCW winding
                // v1_next v1_i
                // v2_next v2_i
                int next = (i + 1) % numPhi;
                int next_k = (k + 1) % (numSegments + 1);
                std::vector<std::string> upperFace = {
                    "v" + std::to_string(k + 1) + "_" + std::to_string(next),
                    "v" + std::to_string(k + 1) + "_" + std::to_string(i),
                    "v" + std::to_string(next_k + 1) + "_" + std::to_string(i),
                    "v" + std::to_string(next_k + 1) + "_" + std::to_string(next)
                };
                AddFace("f1_" + std::to_string(i), upperFace);
            }
        }
    }

}
}
