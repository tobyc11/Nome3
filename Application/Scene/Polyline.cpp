#include "Polyline.h"
#include <iostream>
#include <vector>

namespace Nome::Scene
{

void CPolyline::drawCircle(Vector3 center, Vector3 T, Vector3 N, float radius, float angle, float scale, int num_phi, int index)
{

    Vector3 B = Vector3((-1.0f * N.y) * T.z + T.y * N.z,N.x * T.z - T.x * N.z,(-1.0f * N.x) * T.y + T.x * N.y);

    N.Normalize();
    B.Normalize();

    const float du = (2.0f * M_PI) / num_phi;

    for (int j = 0; j < num_phi; ++j)
    {
        float u = angle + j * du;

        // compute position of circle point
        // https://math.stackexchange.com/questions/1958939/parametric-equation-for-rectangular-tubing-with-corner-radius

        float x = radius * cosf(u) * scale;
        float y = radius * sinf(u);

        Vector3 tansformVector = N * x + B * y;

        Vector3 currVertex = center + tansformVector;

        AddVertex("v" + std::to_string(index) + "_" + std::to_string(j),
                  { currVertex.x, currVertex.y, currVertex.z });
    }
}

float getAngle(Vector3 vectorA, Vector3 vectorB)
{
    float value = vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length();
    float epsilon = 1e-4;
    if (fabs(value - 1) < epsilon) { return 0; }
    if (fabs(value + 1) < epsilon) { return M_PI; }
    if (fabs(value) < epsilon) {return M_PI / 2; }

    return acosf(vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length()) ;
}

Vector3 getPerpendicularVector(Vector3 vectorA, Vector3 vectorB)
{
    float epsilon = 1e-4;
    float scale = cosf(getAngle(vectorA, vectorB));

    if (fabs(scale) < epsilon) { return vectorA.Normalized(); }

    return (vectorA - vectorB.Normalized() * vectorA.Length() * cosf(getAngle(vectorA, vectorB))).Normalized();
}

Vector3 vectorMultiplyMatrix(Vector3 vector, float matrix[3][3])
{
    float x = vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0];
    float y = vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1];
    float z = vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2];

    return Vector3(x, y, z);
}

Vector3 crossProduct(Vector3 vectorA, Vector3 vectorB)
{
    return Vector3(vectorA.y * vectorB.z - vectorA.z * vectorB.y, vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                  vectorA.x * vectorB.y - vectorA.y * vectorB.x);
}

float calculateRoatateAngle(Vector3 vectorA, Vector3 vectorB, Vector3 T)
{
    float angle = getAngle(vectorA, vectorB);
    T.Normalize();

    Vector3 perpendicular = getPerpendicularVector(vectorB, vectorA).Normalized();
    Vector3 direction = crossProduct(T, vectorA).Normalized();

    if (direction.DotProduct(perpendicular) >= 0) {
        return angle;
    } else {
        return -angle;
    }
}

void CPolyline::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    // the last point is control arguments
    auto numPoints = Points.GetSize() - 1;

    // if there's just one point, exit
    if (numPoints < 2) { return; }

    const float radius = 1; //hard coded

    std::vector<Vector3> points;
    // Normal vectors of each paths
    std::vector<Vector3> Ns;
    // Rotation angles of each paths
    std::vector<float> angles;

    // get control arguments;
    CVertexInfo* controls = Points.GetValue(numPoints, nullptr);

    // Polygon's sides;
    int num_phi = int(controls->Position.x);
    float twist = controls->Position.y * M_PI / 180 / (numPoints - 1);
    float forget_the_name = controls->Position.z * M_PI / 180;

    for (unsigned long i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = Points.GetValue(i, nullptr);

        points.push_back(Vector3(point->Position.x, point->Position.y, point->Position.z)); // current point

        if (i > 1)
        {
            Vector3 prevVector = points[i] - points[i - 1];
            Vector3 curVector = points[i - 1] - points[i - 2];

            Vector3 sumVector = (prevVector.Normalized() - curVector.Normalized());
            Vector3 curPerpendicular = getPerpendicularVector(sumVector, prevVector);
            Vector3 prevPerpendicular = getPerpendicularVector(sumVector, -curVector);

            /* let the normal vector be the prevPerpendicular vector
             * in this case, the rotation angle is 0 */
            if (i == 2) { Ns.push_back(prevPerpendicular); }
            // calculate the rotaion angle of each joint
            angles.push_back(calculateRoatateAngle(Ns[i - 2], prevPerpendicular, points[i - 1] - points[i - 2]) + twist);
            // set the current normal vector
            Ns.push_back(curPerpendicular);
        }
    }

    // detect if is a closed polyline
    bool isClosed = (points[0] == points[numPoints - 1]);
    // closed polyline should have at least 3 points
    if (isClosed && numPoints < 3) { return; }

    if (isClosed)
    {
        Vector3 prevVector = points[1] - points[0];
        Vector3 curVector = points[0] - points[numPoints - 2];

        Vector3 sumVector = (prevVector.Normalized() - curVector.Normalized());
        Vector3 curPerpendicular = getPerpendicularVector(sumVector, prevVector);
        Vector3 prevPerpendicular = getPerpendicularVector(sumVector, -curVector);

        angles[numPoints - 2] = calculateRoatateAngle(Ns[numPoints - 2], prevPerpendicular, points[numPoints - 1] - points[numPoints - 2]);
        // add the rotation angle of the closed joint
        angles[0] += calculateRoatateAngle(curPerpendicular, Ns[0], prevVector);
    } else {
        angles[numPoints - 2] = 0;
    }
    angles[numPoints - 2] += twist;

    // get the result rotation angles
    for (unsigned long i = numPoints - 2; i >= 1; i--) { angles[i - 1] += angles[i]; }
    // add rotation
    for (unsigned long i = 0; i < numPoints - 1; i++) { angles[i] += forget_the_name; }

//    for (unsigned long i = 0; i < numPoints - 1; i++)
//    {
//        std::cout << "Path " << i << ":\n";
//        std::cout << "\t N: " << Ns[i].x << ' ' << Ns[i].y << ' ' << Ns[i].z << '\n';
//        std::cout << "\t Angle: " << ": " << angles[i] * 180 / M_PI << std::endl;
//    }

    // the count of drawing segments
    int segmentCount = 0;

    Vector3 T, N;

    if (!isClosed)
    {
        T = points[1] - points[0];
        N = Ns[0];

        // generate points in a circle perpendicular to the curve at the current point
        drawCircle(points[0], T, N, radius, angles[0], 1, num_phi, ++segmentCount);
    } else
    {
        Vector3 prevVector = (points[1] - points[0]).Normalized();
        Vector3 curVector = (points[0] - points[numPoints - 2]).Normalized();

        T = prevVector + curVector;
        N = prevVector - curVector;

        drawCircle(points[0], T, N, radius, angles[0], N.Length(), num_phi, ++segmentCount);
    }

    for (unsigned long i = 1; i < numPoints; i++)
    {
        if (i == numPoints - 1)
        {
            if (isClosed) {
                Vector3 prevVector = (points[1] - points[0]).Normalized();
                Vector3 curVector = (points[0] - points[numPoints - 2]).Normalized();

                T = prevVector + curVector;
                N = prevVector - curVector;

                // 0 is perfect.
                drawCircle(points[numPoints - 1], T, N, radius, angles[i - 1] - twist, N.Length(), num_phi, ++segmentCount);
            } else {
                T = points[i] - points[i - 1];
                // add twist
                drawCircle(points[i], T, Ns[i - 1], radius, angles[i - 1] - twist, 1, num_phi, ++segmentCount);
            }
        } else
        {
            Vector3 prevVector = (points[i + 1] - points[i]).Normalized();
            Vector3 curVector = (points[i] - points[i - 1]).Normalized();

            T = prevVector + curVector;
            N = prevVector - curVector;
            drawCircle(points[i], T, N, radius, angles[i], N.Length(), num_phi, ++segmentCount);
        }
    }

    // Create faces
    for (int k = 0; k < segmentCount - 1; k++)
    {
        for (int i = 0; i < num_phi; i++)
        {
            // CCW winding
            // v1_next v1_i
            // v2_next v2_i
            int next = (i + 1) % num_phi;
            int next_k = (k + 1) % segmentCount;
            std::vector<std::string> upperFace = {
                "v" + std::to_string(next_k + 1) + "_" + std::to_string(next),
                "v" + std::to_string(next_k + 1) + "_" + std::to_string(i),
                "v" + std::to_string(k + 1) + "_" + std::to_string(i),
                "v" + std::to_string(k + 1) + "_" + std::to_string(next),
            };
            AddFace("f" + std::to_string(k) + "_" + std::to_string(i), upperFace);
        }
    }
}

void CPolyline::SetClosed(bool closed)
{
    bClosed = closed;
    MarkDirty();
}

}
