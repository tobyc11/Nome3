#include "TorusKnot.h"
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


DEFINE_META_OBJECT(CTorusKnot)
{ 
    BindPositionalArgument(&CTorusKnot::P_Val, 1, 0); // equivalent to "symm" in the language reference
    BindPositionalArgument(&CTorusKnot::Q_Val, 1, 1); // equivalent to "turns" in the language reference
    BindPositionalArgument(&CTorusKnot::MajorRadius, 1, 2);
    BindPositionalArgument(&CTorusKnot::MinorRadius, 1, 3);
    BindPositionalArgument(&CTorusKnot::TubeRadius, 1, 4); // equivalent to "minor radius"
    BindPositionalArgument(&CTorusKnot::VerticesPerRing, 1, 5); // "circle_segs" 
    BindPositionalArgument(&CTorusKnot::Segments, 1, 6);// "sweep_segs"
}
void CTorusKnot::UpdateEntity()
{
    if (!IsDirty())
        return;

    // Clear mesh
    Super::UpdateEntity();

    // Initialize torus knot parameters from document 
    int _p = P_Val.GetValue(1.0f);
    int _q = Q_Val.GetValue(1.0f);
    int numPhi = static_cast<int>(VerticesPerRing.GetValue(16.0f));
    float minorRadius = MinorRadius.GetValue(1.0f);
    float majorRadius = MajorRadius.GetValue(1.0f);
    float tubeRadius = TubeRadius.GetValue(1.0f);
    int numSegments = Segments.GetValue(0.0f); // number of circles basically

    const float epsilon = 1e-4;
    const float dt = (2.0f * (float)tc::M_PI) / (numSegments);
    const float du = (2.0f * (float)tc::M_PI) / numPhi;
    
    // https://mathworld.wolfram.com/Torus.html and http://makerhome.blogspot.com/2014/01/day-150-trefoil-torus-knots.html
    for (int i = 0; i < numSegments; i++) // Create torus knot, creating one cross section at each iteration
    {
        float t0 = i * dt;
        float r0 = (majorRadius + minorRadius * cosf(_q * t0)); // * 0.5;
        
        Point p0 = { r0 * cosf(_p * t0), r0 * sinf(_p * t0), minorRadius * sinf(_q * t0) }; 
        // Point p0 = { sinf(t0), cos(t0), 0}; // uncomment this to do torus instead    . naive circle method
        
        // Below, we'll work on approximating the Frenet frame { T, N, B } for the curve at the current point

        float t1 = t0 + epsilon;
        float r1 = (majorRadius + minorRadius * cosf(_q * t1)); //* 0.5;

        // p1 is p0 advanced infinitesimally along the curve
        Point p1 = { r1 * cosf(_p * t1), r1 * sinf(_p * t1), minorRadius * sinf(_q * t1) };
        //Point p1 = { sinf(t1), cos(t1), 0 }; //uncomment this to do torus instead 

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
        for (int j = 0; j <= numPhi; ++j)
        {
            float u = j * du;

            // compute position of circle point
            float x = tubeRadius * cosf(u);
            float y = tubeRadius * sinf(u);

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
    for (int k = 0; k < numSegments; k++)
    {
        for (int i = 0; i < numPhi; i++)
        {
            // CCW winding
            int next = (i + 1) % numPhi;
            int next_k = (k + 1) % numSegments;
            std::vector<std::string> upperFace = {
                "v" + std::to_string(next_k + 1) + "_" + std::to_string(next), 
                "v" + std::to_string(next_k + 1) + "_" + std::to_string(i),
                "v" + std::to_string(k + 1) + "_" + std::to_string(i),
                "v" + std::to_string(k + 1) + "_" + std::to_string(next)

            };
            AddFace("f1_" + std::to_string(i), upperFace);
        }
    }

}
}
