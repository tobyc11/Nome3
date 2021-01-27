#include "LineMesh.h"
#include <vector>
#include <cmath>
#include "../Parsing/exprtk.hpp"

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CLineMesh)
{
    BindPositionalArgument(&CLineMesh::input1, 1, 0);
}

void CLineMesh::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    float n = (float)input1.GetValue(100.0f);
    std::string filename = this->GetName();
    std::cout << filename;

//    typedef exprtk::symbol_table<double> symbol_table_t;
//    typedef exprtk::expression<double>     expression_t;
//    typedef exprtk::parser<double>             parser_t;
//
//    std::string expression_str_x = "abs((2 * x)  - 1)";
//    std::string expression_str_y = "sin(y)";
//
//    double x = 0.0;
//    double y = 0.0;
//
//    // Register x with the symbol_table
//    symbol_table_t symbol_table;
//    symbol_table.add_variable("x",x);
//    symbol_table.add_variable("y",y);
//
//    // Instantiate expression and register symbol_table
//    expression_t expression_x;
//    expression_x.register_symbol_table(symbol_table);
//
//    // Instantiate parser and compile the expression
//    parser_t parser_x;
//    parser_x.compile(expression_str_x,expression_x);
//
//    // Instantiate expression and register symbol_table
//    expression_t expression_y;
//    expression_y.register_symbol_table(symbol_table);
//
//    // Instantiate parser and compile the expression
//    parser_t parser_y;
//    parser_y.compile(expression_str_y,expression_y);
//
//    // Instantiate vector of points
//    float point_x = 0.0;
//    float point_y = 0.0;
//    std::vector<CMeshImpl::VertexHandle> handles;
//
//    // add points to vector
//    for (double i = -10.0; i <= 10.0; i += 0.2) {
//        x = i;
//        y = i;
//        point_x = (float)expression_x.value();
//        point_y = (float)expression_y.value();
//        handles.push_back(AddVertex("v_" + std::to_string(i), { point_x, point_y, 0.0f } ));
//    }
//
//    // turn vector into line strip
//    AddLineStrip("line_0", handles);
}

}
