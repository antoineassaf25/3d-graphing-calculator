/** @file Graph.cpp
 * @brief Class implementation for working with a graph of an equation using a height map.
 *
 * @author Antoine Assaf
 */

#include "Graph.hpp"
#include "exprtk.hpp"

#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>

#include <glad/glad.h>
#include <glm/glm.hpp>


float z_bound = 50.0f;

// Constructor loads a graph from an equation in form f(x,y), where z = f(x,y), and with a given dimension
Graph::Graph(std::string equation, unsigned int dimension, unsigned int id) {
  
    m_equation = equation;
    m_dimension = dimension;
    
    typedef exprtk::symbol_table<float> symbol_table_t;
    typedef exprtk::expression<float> expression_t;
    typedef exprtk::parser<float> parser_t;

    //initiate variables x & y
    float x;
    float y;

    symbol_table_t symbol_table;
    symbol_table.add_variable("x", x);
    symbol_table.add_variable("y", y);

    symbol_table.add_constant("e", 2.71828);
    symbol_table.add_constant("pi", 3.14159);
    symbol_table.add_constants();
    
    expression_t expression;
    expression.register_symbol_table(symbol_table);

    parser_t parser;
    parser.compile(equation, expression);

    // Map f(x,y) = [-5, 5] --> [0, 1]. Any point not in domain will be mapped to -1.
    m_heightData = new float[dimension*dimension];
    
    std::vector<float> positions;
    std::vector<float> colors;

    int index = -1;
    float last_height = 0.5f;
    for (y = -5.0; y <= 5.001; y += 10.0/(dimension-1.0)) {
        for (x = -5.0; x <= 5.001; x += 10.0/(dimension-1.0)) {
            index++;

            float z = expression.value();
            float alpha = 1.0f;

            float height = -1.0f;

            try {
                if (std::isnan(z)) {
                    height = -1.0f;
                } else if (z < -z_bound) {
                    height = -1.0f;
                    last_height = height;
                } else if (z > z_bound) {
                    height = -1.0f;
                    last_height = last_height;
                }
                else {
                    height = (z + z_bound)/(z_bound*2);
                    last_height = height;
                }
            }
            catch (...) {
                height = 0.5f;
                alpha = 0.0f;
            }
            m_heightData[index] = height;
            positions.push_back(x);
            positions.push_back(glm::clamp(height * (z_bound*2) - z_bound, -z_bound, z_bound));
            positions.push_back(y);


            float yellowTint = height*8 - 3.8f;

            if (yellowTint > 1.0f) {
                yellowTint = 1.0f;
            } else if (yellowTint < 0.0f) {
                yellowTint = 0.0f;
            }
           
            if (id == 1) {
                colors.push_back(0.0f);
                colors.push_back(0.0f);
                colors.push_back(1.0f - yellowTint);
            } else if (id == 2) {
                colors.push_back(0.0f);
                colors.push_back(1.0f - yellowTint);
                colors.push_back(0.0f);
            } else {
                colors.push_back(1.0f - yellowTint);
                colors.push_back(0.0f);
                colors.push_back(0.0f);
            }
            colors.push_back(glm::clamp(alpha - 0.1f, 0.0f, 1.0f));
        }
    }

    m_positions = positions;
    m_colors = colors;

    std::ofstream outFile;

    std::string filePath = "./generated/graph" + std::to_string(id) + ".ppm";

    outFile.open(filePath);

    outFile << "P3" << std::endl;
    outFile << "# Generated .ppm file from equation z = " << equation << std::endl;
    outFile << m_dimension << " " << m_dimension << std::endl;
    outFile << 255 << std::endl;

    for (int i = 0; i < dimension*dimension; i++) {
        float pixel = m_heightData[i];

        pixel = std::clamp(pixel, 0.0f, 10.0f);

        int rgb = (int) (pixel * 255.0f + 0.5f); // round to nearest pixel representation [0,255]

        outFile << std::to_string(rgb) << " " << std::to_string(rgb) << " " << std::to_string(rgb) << std::endl;
    }

    outFile.close();

    m_heightTexture = new Texture();
    m_heightTexture->LoadTexture(filePath);
    
    Graph::calculateNormals();
    Graph::updateBuffers();        
}

//Destructor clears any allocated memory
Graph::~Graph() {
    if(m_heightData!=nullptr){
        delete[] m_heightData;
    }
    if (m_heightTexture != nullptr) {
        delete m_heightTexture;
    }
}

// Returns the texture of the graph
Texture Graph::getTexture() const {
    return *m_heightTexture;
}

// Returns the vector buffer object of points in x,y,z triplets
std::vector<float> Graph::getVBO() const {
    return m_VBO;
}

// Returns the index buffer object of the points
std::vector<unsigned int> Graph::getIBO() const {
    return m_IBO;
}


//Sets up the values for m_VBO and m_IBO for the Graph
void Graph::updateBuffers() {
    std::vector<float> VBO;
    std::vector<unsigned int> IBO;

    for (int i = 0; i < m_positions.size()/3; i++) {
        float x = m_positions[i*3 + 0];
        float y = m_positions[i*3 + 1];
        float z = m_positions[i*3 + 2];

        float xn = m_normals[i*3 + 0];
        float yn = m_normals[i*3 + 1];
        float zn = m_normals[i*3 + 2];

        float r = m_colors[i*4 + 0];
        float g = m_colors[i*4 + 1];
        float b = m_colors[i*4 + 2];
        float a = m_colors[i*4 + 3];


        VBO.push_back(x);
        VBO.push_back(y);
        VBO.push_back(z);

        VBO.push_back(xn);
        VBO.push_back(yn);
        VBO.push_back(zn);

        VBO.push_back(r);
        VBO.push_back(g);
        VBO.push_back(b);
        VBO.push_back(a);

        VBO.push_back(0);
        VBO.push_back(0);

    }

    for (int y = 0; y < m_dimension - 1; y++) {
        for (int x = 0; x < m_dimension - 1; x++) {
            unsigned int curr = x + y * m_dimension;
            
            if (m_heightData[curr] >= 0.0f && m_heightData[curr + 1] >= 0.0f && m_heightData[curr + m_dimension] >= 0.0f){
                //triangle 1
                IBO.push_back(curr);
                IBO.push_back(curr + 1);
                IBO.push_back(curr + m_dimension);
            }
            
            if (m_heightData[curr + m_dimension + 1] >= 0.0f && m_heightData[curr + 1] >= 0.0f && m_heightData[curr + m_dimension] >= 0.0f) {
                //triangle 2
                IBO.push_back(curr + 1);
                IBO.push_back(curr + m_dimension + 1);
                IBO.push_back(curr + m_dimension);
           }
        }
    }

    m_VBO = VBO;
    m_IBO = IBO;
}

void Graph::calculateNormals() {
    std::vector<float> normals;
    for (int y = 0; y < m_dimension; y++) {
        for (int x = 0; x < m_dimension; x++) {
            unsigned int curr = x + y * m_dimension;
            
            if (x == 0 || x >= m_dimension - 1 || y == 0 || y >= m_dimension - 1) {
                normals.push_back(0);
                normals.push_back(0);
                normals.push_back(0);
                continue;
            }

            // PARTIAL DERIVATIVE - RESPECT TO X
            float leftPoint = m_heightData[(x - 1) + (y) * m_dimension];
            float rightPoint = m_heightData[(x + 1) + (y) * m_dimension];

            if (leftPoint < 0 || rightPoint < 0) {
                normals.push_back(0);
                normals.push_back(0);
                normals.push_back(0);
                continue;    
            }


            leftPoint = (leftPoint * (z_bound * 2)) - z_bound;
            rightPoint = (rightPoint * (z_bound * 2)) - z_bound;

            float partial_x = (rightPoint-leftPoint)/(2*10.0/(m_dimension-1.0));

            // PARTIAL DERIVATIVE - RESPECT TO Y
            float downPoint = m_heightData[(x) + (y - 1) * m_dimension];
            float upPoint = m_heightData[(x) + (y + 1) * m_dimension];

            if (downPoint < 0 || upPoint < 0) {
                normals.push_back(0);
                normals.push_back(0);
                normals.push_back(0);
                continue;
            }

            downPoint = (downPoint * (z_bound * 2)) - z_bound;
            upPoint = (upPoint * (z_bound * 2)) - z_bound;

            float partial_y = (upPoint-downPoint)/(2*10.0/(m_dimension-1.0));

            glm::vec3 vector_x(1, partial_x, 0);
            glm::vec3 vector_y(0, partial_y, 1);

            glm::vec3 normal_vec3 = glm::cross(vector_y,vector_x);

            normal_vec3 = glm::normalize(normal_vec3);

            normals.push_back(normal_vec3.x);
            normals.push_back(normal_vec3.y);
            normals.push_back(normal_vec3.z);

        }
    }
    m_normals = normals;
}
