/** @file OBJModel.hpp
 * @brief Class for working with .obj models
 *
 * @author Antoine Assaf
 */

#ifndef OBJModel_HPP
#define OBJModel_HPP

#include <string>

class OBJModel {
public:
    // Constructor loads a filename with the .obj extension
    OBJModel(std::string fileName);
    //Destructor clears any allocated memory
    ~OBJModel();
    // Returns the file of the texture of the object
    std::string getTexture() const;
    // Returns the vector buffer object of points in x,y,z triplets
    std::vector<float> getVBO() const;
    // Returns the index buffer object of the points
    std::vector<unsigned int> getIBO() const;
private:
    //Sets up the values for m_VBO and m_IBO
    void populateBuffers();

    std::vector<float> m_Verts; // v
    std::vector<float> m_VertTexts; // vt
    std::vector<float> m_VertNorms; // vn
    std::string m_MaterialFile; // the path of the material file
    std::string m_ObjectPath; // the path of the object
    std::vector<unsigned int> m_Faces; // face stored: v1 v1_n v2 v2_n v3 v3_n...
    std::vector<float> m_VBO; // the vertex buffer object for rendering
    std::vector<unsigned int> m_IBO; // the index buffer object for rendering
};

#endif
