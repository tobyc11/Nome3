#pragma once

#include "Document.h"

namespace Nome
{

namespace Scene
{
class CPoint;
} /* namespace Scene */

class CDocumentBuilder
{
public:
    ///Create a static point
    Scene::CPoint* Point(float x, float y, float z);

    ///Create a point from expressions
    Scene::CPoint* Point();

    ///Create a polyline from a list of points
    void Polyline(bool closed = false);

    ///Create a face from a list of points
    void Face();

private:
    CDocument* Document;
};

} /* namespace Nome */
