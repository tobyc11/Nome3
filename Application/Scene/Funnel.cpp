#include "Funnel.h"
#include <cmath>
#include <iostream>

#undef M_PI 

namespace Nome::Scene
{

typedef struct
{
    float x;
    float y;
    float z;
} Point;
Point points;
Point translation; ///randy added


float rotationMatrix[4][4];
float inputMatrix[4][1] = { 0.0, 0.0, 0.0, 0.0 };
float outputMatrix[4][1] = { 0.0, 0.0, 0.0, 0.0 };

void multiplyMatrix()
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 1; j++)
        {
            outputMatrix[i][j] = 0;
            for (int k = 0; k < 4; k++)
            {
                outputMatrix[i][j] += rotationMatrix[i][k] * inputMatrix[k][j];
            }
        }
    }
}
void setUpRotationMatrix(float angle, float u, float v, float w)
{
    float L = (u * u + v * v + w * w);
    angle = angle * (float)tc::M_PI / 180.0; // converting to radian value
    float u2 = u * u;
    float v2 = v * v;
    float w2 = w * w;

    rotationMatrix[0][0] = (u2 + (v2 + w2) * cos(angle)) / L;
    rotationMatrix[0][1] = (u * v * (1 - cos(angle)) - w * sqrt(L) * sin(angle)) / L;
    rotationMatrix[0][2] = (u * w * (1 - cos(angle)) + v * sqrt(L) * sin(angle)) / L;
    rotationMatrix[0][3] = 0.0;

    rotationMatrix[1][0] = (u * v * (1 - cos(angle)) + w * sqrt(L) * sin(angle)) / L;
    rotationMatrix[1][1] = (v2 + (u2 + w2) * cos(angle)) / L;
    rotationMatrix[1][2] = (v * w * (1 - cos(angle)) - u * sqrt(L) * sin(angle)) / L;
    rotationMatrix[1][3] = 0.0;

    rotationMatrix[2][0] = (u * w * (1 - cos(angle)) - v * sqrt(L) * sin(angle)) / L;
    rotationMatrix[2][1] = (v * w * (1 - cos(angle)) + u * sqrt(L) * sin(angle)) / L;
    rotationMatrix[2][2] = (w2 + (u2 + v2) * cos(angle)) / L;
    rotationMatrix[2][3] = 0.0;

    rotationMatrix[3][0] = 0.0;
    rotationMatrix[3][1] = 0.0;
    rotationMatrix[3][2] = 0.0;
    rotationMatrix[3][3] = 1.0;
}
void CFunnel::UpdateEntity()
{

    if (!IsDirty())
        return;


    // Clear mesh
    Super::UpdateEntity();

    int num_phi = static_cast<int>(VerticesPerRing.GetValue(16.0f));
    int num_theta = Radius.GetValue(1.0f);
    float major_radius = Ratio.GetValue(0.0f);
    float minor_radius = Height.GetValue(1.0f);
    //float ri = num_theta * (1 + major_radius);

	for (int i = 0; i < num_phi; i++) //create initial circle
    {

        // for each point on initial circle, rotate it num_theta amount of times
        float theta = (float)i / num_phi * 2.f * (float)tc::M_PI;
        float x = minor_radius * cosf(theta);
        float y = minor_radius * sinf(theta);
        float z = 0; //this used to be nonzero and was causing an issue

		float x_trans = major_radius;
        float y_trans = 0;
        float z_trans = 0;
        for (int k = 0; k < num_theta; k++) // start at the first rotation
        {
            points.x = x;
            points.y = y;
            points.z = z;

			translation.x = x_trans;
            translation.y = y_trans;	
			translation.z = z_trans;

            inputMatrix[0][0] = points.x;
            inputMatrix[1][0] = points.y;
            inputMatrix[2][0] = points.z;
            inputMatrix[3][0] = 1.0;
            //std::cout << points.x << std::endl;
            //std::cout << points.y << std::endl;
            //std::cout << points.z << std::endl;
            //std::cout << k * (360 / num_theta) <<std::endl;
            setUpRotationMatrix(k * (360 / num_theta), 0, 1, 0);
            multiplyMatrix();
            points.x = outputMatrix[0][0];
            points.y = outputMatrix[1][0];
            points.z = outputMatrix[2][0];

			
            inputMatrix[0][0] = translation.x;
            inputMatrix[1][0] = translation.y;
            inputMatrix[2][0] = translation.z;
            inputMatrix[3][0] = 1.0;
            // std::cout << points.x << std::endl;
            // std::cout << points.y << std::endl;
            // std::cout << points.z << std::endl;
            // std::cout << k * (360 / num_theta) <<std::endl;
            setUpRotationMatrix(k * (360 / num_theta), 0, 1, 0);
            multiplyMatrix();
            translation.x = outputMatrix[0][0];
			translation.y = outputMatrix[1][0];
            translation.z = outputMatrix[2][0];
            
			points.x = points.x + translation.x;
            points.y = points.y + translation.y;
            points.z = points.z + translation.z;
			//std::cout << points.x << std::endl;
            //std::cout << points.y << std::endl;
            //std::cout << points.z << std::endl;
            AddVertex("v" + std::to_string(k + 1) + "_" + std::to_string(i),
                      { points.x, points.y, points.z });
        }
	}
   
    // Create faces
 
    for (int k = 0; k < num_theta-2; k++)
    {    
    for (int i = 0; i < num_phi; i++)
    {
        // CCW winding
        // v1_next v1_i
        // v2_next v2_i
        int next = (i + 1) % num_phi;
        int next_k = (k + 2) % num_theta;
        std::vector<std::string> upperFace = {
            "v" + std::to_string(k + 1) + "_" + std::to_string(next),
                                               "v" + std::to_string(k + 1)  + "_" + std::to_string(i),
                                               "v" + std::to_string(next_k) + "_" + std::to_string(i),
                                               "v" + std::to_string(next_k) + "_"
                                                   + std::to_string(next) };
        AddFace("f1_" + std::to_string(i), upperFace);
    }
	}

    // Two caps
    // std::vector<std::string> upperCap, lowerCap;
    // for (int i = 0; i < n; i++)
    //{
    //    upperCap.push_back("v1_" + std::to_string(i));
    //    lowerCap.push_back("v2_" + std::to_string(n - 1 - i));
    //}
    // AddFace("top", upperCap);
    // AddFace("bottom", lowerCap);
}
}