#include "BSpline.h"

namespace Nome::Scene
{

Matrix3 CBSplineMath::FrenetFrameAt(float t)
{
    //TODO
    //tangent of curve at t (approximate)
    return Matrix3();
}

std::vector<float> CBSplineMath::GetDefaultKnots()
{
    std::vector<float> result = {0.0f, 0.5f, 1.0f};
    return result;
}

std::vector<float> CBSpline::GetDefKnots() {
    float numKnots = Order.GetValue(3) + ((float) ControlPoints.GetSize());
    std::vector<float> result = {};
    for (int i = 0; i < numKnots; i++) {
        result.push_back(i/numKnots - 1);
    }
    return result;
}



void CBSpline::SetClosed(bool closed)
{
    bClosed = closed;
}


int CBSpline::GetKnotInt(float x, std::vector<float> knots) {
    int index = -1;
    for (int i = 1; i < knots.size(); i++) {
        if (x < knots[i]) {
            index = i-1;
            break;
        } else if (x == knots[knots.size() - 1]) {
            index = knots.size() - 1;
        }
    }
    return index;
}

int CBSpline::NFactor(int i, int j, float t) {
    std::vector<float> knots = GetDefKnots();
    if (j == 0) {
        std::vector<float> knots = GetDefKnots();
        if (knots[i] <= t && t < knots[i+1] && knots[i] < knots[i+1]) {
            return 1;
        } else {
            return 0;
        }
    }
    return (((t - knots[i])/(knots[i+j] - knots[i]))*NFactor(i, j-1, t)) + (((knots[i+j+1] - t)/(knots[i+j+1] - knots[i+1]))*NFactor(i+1, j-1,t));
}

Vector3 CBSpline::FindPoint(int degree, float t) {
    Vector3 ret = {0,0,0};
    for (int i = 0; i < ControlPoints.GetSize(); i++) {
        ret += ControlPoints.GetValue(i, nullptr)->Position*NFactor(i, degree, t);
    }
}

Vector3 CBSpline::deBoor(int k , int degree, int i, double x, std::vector<float> knots) {
    if (k == 0) {
        CVertexInfo* vinfo =  ControlPoints.GetValue(i, nullptr);
        return vinfo->Position;
    }
    double alpha = (x - knots[i])/(knots[i + degree + 1 - k] - knots[i]);
    Vector3 rec1 = deBoor(k-1,degree, i-1, x, knots);
    Vector3 rec2 = deBoor(k-1,degree, i, x, knots);
    return (rec1*(1-alpha )) + (rec2*alpha);

}

Vector3 CBSpline::deBoor1(int k,  double x, std::vector<float> knots, int p) {
    std::vector<Vector3> d;
    for (int j = 0; j <= p; j++) {
        d[j] = ControlPoints.GetValue(j + k - p, nullptr)->Position;
    }
    for (int r = 1; r <= p; r++) {
        for (int j = p; j >= r-1;j--) {
            double alpha = (x - knots[j + k - p])/(knots[j + 1 + k - r] - knots[j + k - p]);
            d[j] = (1 - alpha)*d[j-1] + alpha*d[j];
        }
    }
    return d[p];
}

void CBSpline::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    int n = (int) Segments.GetValue(8.0f);
    size_t howMany = ControlPoints.GetSize();
    CVertexInfo* vInfo = ControlPoints.GetValue(0, nullptr);
    std::vector<float> knots = GetDefKnots();
    float order = Order.GetValue(3);

    // TODO: starter code
    for (float i = 0; i < n; i++)
    {
        //Sample some dummy values
        float x = i/n;
        int index = GetKnotInt(x, knots);
        Vector3 pt = FindPoint(order, x);
        SamplePositions.emplace_back(pt.x, pt.y, pt.z);
    }

    std::vector<CMeshImpl::VertexHandle> handles;
    handles.reserve(n + 1);
    for (int i = 0; i < n + 1; i++)
    {
        handles.push_back(AddVertex("v" + std::to_string(i), SamplePositions[i]));
    }
    AddLineStrip("curve", handles);
}

void CBSpline::Draw(IDebugDraw* draw)
{
    tc::Color c1 = tc::Color::YELLOW;
    tc::Color c2 = tc::Color::RED;
    auto iter = SamplePositions.begin();
    while (iter != SamplePositions.end() - 1)
    {
        const auto& p0 = *iter;
        const auto& p1 = *(++iter);
        draw->LineSegment(p0, c1, p1, c2);
        std::swap(c1, c2);
    }
}

}
