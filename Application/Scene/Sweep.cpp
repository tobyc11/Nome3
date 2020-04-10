#include "Sweep.h"
#include <vector>
#include "SweepControlPoint.h"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CSweep)
{
    BindPositionalArgument(&CSweep::Path, 1, 0);
    BindPositionalArgument(&CSweep::CrossSection, 1, 1);
    BindPositionalArgument(&CSweep::Azimuth, 1, 2);
    BindPositionalArgument(&CSweep::Twist, 1, 3);
}

// do cross pruduct with vectorA and vectorB
Vector3 crossProduct(Vector3 vectorA, Vector3 vectorB)
{
    return Vector3(vectorA.y * vectorB.z - vectorA.z * vectorB.y,
                   vectorA.z * vectorB.x - vectorA.x * vectorB.z,
                   vectorA.x * vectorB.y - vectorA.y * vectorB.x);
}

void CSweep::drawCrossSection(std::vector<Vector3> crossSection,
                              Vector3 center, Vector3 T, Vector3 N,
                              float angle, float scaleX, float scaleY,
                              float scaleN, int index)
{
    Vector3 B = crossProduct(T, N);

    N.Normalize();
    B.Normalize();

    for (size_t i = 0; i < crossSection.size(); i++)
    {
        // make rotations
        float x = crossSection[i].x * scaleX * cosf(angle) - crossSection[i].y * scaleY * sinf(angle);
        float y = crossSection[i].x * scaleX * sinf(angle) + crossSection[i].y * scaleY *cosf(angle);
        // do thransform
        Vector3 transformVector = N * x * scaleN + B * y ;
        // add offset
        Vector3 curVertex = center + transformVector;

        AddVertex("v" + std::to_string(index) + "_" + std::to_string(i), { curVertex.x, curVertex.y, curVertex.z });
    }
}

// get the angle of two vectors (unsigned)
float getAngle(Vector3 vectorA, Vector3 vectorB)
{
    float value = vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length();
    float epsilon = 1e-4;
    if (fabs(value - 1) < epsilon) { return 0; }
    if (fabs(value + 1) < epsilon) { return (float)tc::M_PI; }
    if (fabs(value) < epsilon) {return (float)tc::M_PI / 2; }

    return acosf(vectorA.DotProduct(vectorB) / vectorA.Length() / vectorB.Length()) ;
}

// get the perpendicular vector from vectorB to vectorA
Vector3 getPerpendicularVector(Vector3 vectorA, Vector3 vectorB)
{
    float epsilon = 1e-4;
    float scale = cosf(getAngle(vectorA, vectorB));

    if (fabs(scale) < epsilon) { return vectorA.Normalized(); }

    return (vectorA - vectorB.Normalized() * vectorA.Length() * cosf(getAngle(vectorA, vectorB))).Normalized();
}

// multiply a vector to a matrix
Vector3 vectorMultiplyMatrix(Vector3 vector, float matrix[3][3])
{
    float x = vector.x * matrix[0][0] + vector.y * matrix[1][0] + vector.z * matrix[2][0];
    float y = vector.x * matrix[0][1] + vector.y * matrix[1][1] + vector.z * matrix[2][1];
    float z = vector.x * matrix[0][2] + vector.y * matrix[1][2] + vector.z * matrix[2][2];

    return Vector3(x, y, z);
}

// calculate the rotation angle from vectorA to vectorB (signed)
float calculateRoatateAngle(Vector3 vectorA, Vector3 vectorB, Vector3 T)
{
    float angle = getAngle(vectorA, vectorB);
    T.Normalize();

    Vector3 perpendicular = getPerpendicularVector(vectorB, vectorA).Normalized();
    Vector3 direction = crossProduct(T, vectorA).Normalized();

    if (direction.DotProduct(perpendicular) >= 0)
    {
        return angle;
    }
    else {
        return -angle;
    }
}

void CSweep::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    // the last point is control arguments
    CPolylineInfo *pathInfo = Path.GetValue(nullptr);
    CPolylineInfo *crossSectionInfo = CrossSection.GetValue(nullptr);

    if (pathInfo == nullptr || crossSectionInfo == nullptr) { return; }

    // detect if is a closed polyline
    bool isClosed = pathInfo->IsClosed;
    size_t numPoints = pathInfo->Positions.size();
    // if the number of points cannot build a model, exit
    if ((!isClosed && numPoints < 2) || (isClosed && numPoints < 3) || crossSectionInfo->Positions.size() < 3) { return; }

    std::vector<Vector3> points;
    // Normal vectors of each paths
    std::vector<Vector3> Ns;
    // Rotation angles of each paths
    std::vector<float> angles;
    // Cross sections
    std::vector<Vector3> crossSection;
    // Scales
    std::vector<float> scaleX;
    std::vector<float> scaleY;

    std::string name = GetName();

    // Polygon's sides;
    float twist =  Twist.GetValue(0) * (float)tc::M_PI / 180 / (numPoints - 1);
    float azimuth = Azimuth.GetValue(0) * (float)tc::M_PI / 180;

    for (size_t i = 0; i < crossSectionInfo->Positions.size(); i++)
    {
        CVertexInfo* point = crossSectionInfo->Positions[i];
        crossSection.push_back(Vector3(point->Position.x, point->Position.y, point->Position.z));
    }

    for (size_t i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = pathInfo->Positions[i];

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
            angles.push_back(calculateRoatateAngle(Ns[i - 2], prevPerpendicular,
                                                   points[i - 1] - points[i - 2]) + twist);
            // set the current normal vector
            Ns.push_back(curPerpendicular);
        }

        scaleX.push_back(1.0f);
        scaleY.push_back(1.0f);

    }

    if (isClosed)
    {
        Vector3 prevVector = points[1] - points[0];
        Vector3 curVector = points[0] - points[numPoints - 2];

        Vector3 sumVector = (prevVector.Normalized() - curVector.Normalized());
        Vector3 curPerpendicular = getPerpendicularVector(sumVector, prevVector);
        Vector3 prevPerpendicular = getPerpendicularVector(sumVector, -curVector);

        angles.push_back(calculateRoatateAngle(Ns[numPoints - 2], prevPerpendicular,
                      points[numPoints - 1] - points[numPoints - 2]));
        // add the rotation angle of the closed joint
        angles[0] += calculateRoatateAngle(curPerpendicular, Ns[0], prevVector);
    }
    angles.push_back(twist);
    angles.push_back(twist);

    // get the result rotation angles
    for (size_t i = numPoints - 2; i >= 1; i--) { angles[i - 1] += angles[i]; }
    // add rotation
    for (size_t i = 0; i < numPoints; i++)
    {
        CVertexInfo* point = pathInfo->Positions[i];

        angles[i] += azimuth;

        for (size_t j = 0; j < point->ControlPoints.size(); j++)
        {
            CControlPointInfo* CI = point->ControlPoints[j];
            if (CI->OwnerName == name)
            {
                CSweepControlPointInfo* SI = dynamic_cast<CSweepControlPointInfo*>(CI);
                scaleX[i] *= SI->ScaleX;
                scaleY[i] *= SI->ScaleY;
                angles[i] += SI->Rotate * tc::M_PI / 180;
            }
        }
    }

    // the count of drawing segments
    int segmentCount = 0;

    Vector3 T, N;

    if (!isClosed)
    {
        T = points[1] - points[0];
        N = Ns[0];

        // generate points in a circle perpendicular to the curve at the current point
        drawCrossSection(crossSection, points[0], T, N, angles[0], scaleX[0],
                         scaleY[0], 1, ++segmentCount);
    }
    else {
        Vector3 prevVector = (points[1] - points[0]).Normalized();
        Vector3 curVector = (points[0] - points[numPoints - 2]).Normalized();

        T = prevVector + curVector;
        N = prevVector - curVector;

        drawCrossSection(crossSection, points[0], T, N, angles[0], scaleX[0],
                         scaleY[0], N.Length(), ++segmentCount);
    }

    for (size_t i = 1; i < numPoints; i++)
    {
        if (i == numPoints - 1)
        {
            if (isClosed)
            {
                Vector3 prevVector = (points[1] - points[0]).Normalized();
                Vector3 curVector = (points[0] - points[numPoints - 2]).Normalized();

                T = prevVector + curVector;
                N = prevVector - curVector;

                // 0 is perfect.
                drawCrossSection(crossSection, points[i], T, N, angles[i] - twist,
                                 scaleX[i], scaleY[i], N.Length(), ++segmentCount);
            }
            else {
                T = points[i] - points[i - 1];
                // add twist
                drawCrossSection(crossSection, points[i], T, Ns[i - 1], angles[i] - twist,
                                 scaleX[i], scaleY[i], 1, ++segmentCount);
            }
        }
        else {
            Vector3 prevVector = (points[i + 1] - points[i]).Normalized();
            Vector3 curVector = (points[i] - points[i - 1]).Normalized();

            T = prevVector + curVector;
            N = prevVector - curVector;
            drawCrossSection(crossSection, points[i], T, N, angles[i], scaleX[i],
                             scaleY[i], N.Length(), ++segmentCount);
        }
    }

    // Create faces
    for (int k = 0; k < segmentCount - 1; k++)
    {
        for (size_t i = 0; i < crossSection.size() - 1; i++)
        {
            // CCW winding
            // v1_next v1_i
            // v2_next v2_i
            int next = (i + 1) % crossSection.size();
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

}
