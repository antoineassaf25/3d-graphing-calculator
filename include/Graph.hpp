/** @file Graph.hpp
 * @brief Class for working with a graph of an equation using a height map.
 *
 * @author Antoine Assaf
 */

#ifndef Graph_HPP
#define Graph_HPP

#include <string>
#include "Texture.hpp"

class Graph {
public:
    // Constructor loads a graph from an equation in form f(x,y), where z = f(x,y), and with a given dimension
    Graph(std::string equation, unsigned int dimension, unsigned int id);
    //Destructor clears any allocated memory
    ~Graph();
    // Returns the texture of the graph
    Texture getTexture() const;
    // Returns the vector buffer object of points in x,y,z triplets
    std::vector<float> getVBO() const;
    // Returns the index buffer object of the points
    std::vector<unsigned int> getIBO() const;
private:
    //Sets up the values for m_VBO and m_IBO for the Graph
    void updateBuffers();

    //Calculates the approximated NORAMLIZED normal vectors for m_normals
    void calculateNormals();

    std::vector<float> m_positions; // stored x y z
    std::vector<float> m_colors; // stored r g b a
    
    std::string m_equation; // string in the form z = f(x,y)

    Texture* m_heightTexture; // the path of the material file
    float* m_heightData; // stores the values at given f(x,y) 
    std::vector<float> m_normals; // store the NORMALIZED normal values at (x,y)

    unsigned int m_dimension; // dimension of the graph

    std::vector<float> m_VBO; // the vertex buffer object for rendering
    std::vector<unsigned int> m_IBO; // the index buffer object for rendering
};

#endif
