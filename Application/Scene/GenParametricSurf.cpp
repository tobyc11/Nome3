#include "GenParametricSurf.h"
#include "../Parsing/exprtk.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#undef M_PI

namespace Nome::Scene
{

DEFINE_META_OBJECT(CGenParametricSurf)
{
    BindPositionalArgument(&CGenParametricSurf::u_start, 1, 0);
    BindPositionalArgument(&CGenParametricSurf::u_end, 1, 1);
    BindPositionalArgument(&CGenParametricSurf::v_start, 1, 2);
    BindPositionalArgument(&CGenParametricSurf::v_end, 1, 3);
    BindPositionalArgument(&CGenParametricSurf::u_segs, 1, 4);
    BindPositionalArgument(&CGenParametricSurf::v_segs, 1, 5);
}

void CGenParametricSurf::UpdateEntity()
{
    if (!IsDirty())
        return;

    Super::UpdateEntity();

    double uStart = (double)u_start.GetValue(0.0f);
    double uEnd = (double)u_end.GetValue(0.0f);
    double vStart = (double)v_start.GetValue(0.0f);
    double vEnd = (double)v_end.GetValue(0.0f);
    double uSegs = (double)u_segs.GetValue(0.0f);
    double vSegs = (double)v_segs.GetValue(0.0f);

    std::string funcConcat = this->GetName(); // ex. funcConcat := x(u,v)|y(u,v)|z(u,v) = "cos(u)*sin(v)|sin(u)*sin(v)|cos(v)"
    funcConcat.erase(std::remove(funcConcat.begin(), funcConcat.end(), '"'), funcConcat.end());
    size_t numFuncsInString = std::count(funcConcat.begin(), funcConcat.end(), '|');
    if (numFuncsInString != 2) {
        try
        {
            throw 20;
        }
        catch (int e)
        {
            std::cout << "Wrong number of parametric functions specified in Parametric generator!";
        }
    }

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(funcConcat);
    while (std::getline(tokenStream, token, '|')) {
        tokens.push_back(token);
    }
    std::string funcX = tokens[0];
    std::string funcY = tokens[1];
    std::string funcZ = tokens[2];


    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double>     expression_t;
    typedef exprtk::parser<double>             parser_t;

    double u = 0.0;
    double v = 0.0;

    // Register symbols with the symbol_table
    symbol_table_t symbol_table;
    symbol_table.add_variable("u",u);
    symbol_table.add_variable("v",v);

    // Instantiate expressions and register symbol_table
    expression_t expression_x;
    expression_x.register_symbol_table(symbol_table);
    expression_t expression_y;
    expression_y.register_symbol_table(symbol_table);
    expression_t expression_z;
    expression_z.register_symbol_table(symbol_table);

    // Instantiate parsers and compile the expression
    parser_t parser_x;
    parser_x.compile(funcX,expression_x);
    parser_t parser_y;
    parser_y.compile(funcY,expression_y);
    parser_t parser_z;
    parser_z.compile(funcZ,expression_z);

    // set bounds
    double uIncrement = (uEnd - uStart) / uSegs;
    double vIncrement = (vEnd - vStart) / vSegs;

    // add points to vector
    double xi = 0.0;
    double yi = 0.0;
    double zi = 0.0;
    int uCounter = 0;
    int vCounter = 0;
    for (double ui = uStart; ui <= uEnd + (uIncrement/5); ui += uIncrement) { // dividing by 5 for rounding error
        u = ui;
        vCounter = 0;
        for (double vi = vStart; vi <= vEnd + (vIncrement/5); vi += vIncrement) { // dividing by 5 for rounding error
            v = vi;
//            printf("\n%f", yi);
            xi = expression_x.value();
            yi = expression_y.value();
            zi = expression_z.value();
            AddVertex("v_" + std::to_string(uCounter) + "_" + std::to_string(vCounter), // name ex. "v_0_5"
                      { (float)xi, (float)yi, (float)zi } );
            vCounter++;
        }
        uCounter++;
    }

    // add faces
    int faceCounter = 0;
    for (int ui = 0; ui + 1 < uCounter; ui++) {
        for (int vi = 0; vi < vCounter - 1; vi++) {
            std::vector<std::string> face;
            face.push_back("v_" + std::to_string(ui) + "_" + std::to_string(vi));
            face.push_back("v_" + std::to_string(ui+1) + "_" + std::to_string(vi));
            face.push_back("v_" + std::to_string(ui+1) + "_" + std::to_string(vi+1));
            face.push_back("v_" + std::to_string(ui) + "_" + std::to_string(vi+1));

            AddFace("f1_" + std::to_string(faceCounter++), face);
        }
    }

}

}
