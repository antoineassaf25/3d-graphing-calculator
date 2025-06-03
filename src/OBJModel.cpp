/** @file OBJModel.cpp
 * @brief Class implementation for working with .obj models
 *
 * @author Antoine Assaf
 */

#include <OBJModel.hpp>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <fstream>

// VertexData structure for filling VBO
struct VertexData{
	float x,y,z;
	float s,t;

	VertexData(float _x, float _y, float _z, float _s, float _t): x(_x),y(_y),z(_z),s(_s),t(_t) { }

	// Tests if two VertexData are equal
	bool operator== (const VertexData &rhs) const{
		if( (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (s == rhs.s) && (t == rhs.t) ){
			return true;
		}
		return false;
	}
};

// Constructor loads a filename with the .obj extension
OBJModel::OBJModel(std::string fileName) {
    std::ifstream inFile;

    inFile.open(fileName);

    std::vector<float> Verts;
    std::vector<float> VertNorms;
    std::vector<float> VertTexts;

    std::vector<unsigned int> Faces;

    if (inFile.is_open()) {
        std::string line;

        while (getline(inFile, line)) {
            int index = line.find('#');

            if (index != std::string::npos) {
                line = line.substr(0, index);
            }

            std::stringstream stream(line);

            int lineType = -1;

            std::string chunk;

            while (stream >> chunk) {
                if (lineType == -1) {
                    if (chunk == "v") {
                        lineType = 0;
                    } else if (chunk == "vn") {
                        lineType = 1;
                    } else if (chunk == "f") {
                        lineType = 2;
                    } else if (chunk == "vt") {
                        lineType = 3;
                    } else if (chunk == "mtllib") {
                        lineType = 4;
                    } else {
                        break;
                    }
                } else if (lineType == 0) {
                    float value = stof(chunk);
                    Verts.push_back(value);
                } else if (lineType == 1) {
                    float value = stof(chunk);
                    VertNorms.push_back(value);
                } else if (lineType == 2) {
                    int slashesIndex = chunk.find("/");
                    
                    int vertIndex = stoi(chunk.substr(0, slashesIndex)) - 1;

                    int slashesIndex2 = chunk.substr(slashesIndex + 1, chunk.length()).find("/");

                    std::string part2 = chunk.substr(slashesIndex + 1, chunk.length());

                    int textureIndex = -1;

                    if (slashesIndex2 > 0) {
                        textureIndex = stoi(part2.substr(0, slashesIndex2)) - 1;
                    }

                    int normIndex = stoi(part2.substr(slashesIndex2 + 1, chunk.length())) - 1;

                    Faces.push_back(vertIndex);
                    Faces.push_back(textureIndex);
                    Faces.push_back(normIndex);
                } else if (lineType == 3) {
                    float value =stof(chunk);
                    VertTexts.push_back(value);
                } else if (lineType == 4) {
                    m_MaterialFile = chunk;
                }
            }
        }
    }

    inFile.close();

    m_Verts = Verts;
    m_VertTexts = VertTexts;
    m_VertNorms = VertNorms;
    m_Faces = Faces;

    int index = -1;
    for (int i = fileName.size() - 1; i >= 0; i--) {
        if (fileName.at(i) == *"/") {
            index = i + 1;
            break;
        }
    }

    m_ObjectPath = fileName.substr(0, index);
    m_MaterialFile = fileName.substr(0, index) + m_MaterialFile;

    OBJModel::populateBuffers();
}

//Destructor clears any allocated memory
OBJModel::~OBJModel() {
    //no memory allocated
}


// Returns the string of the file of the texture (map_Kd)
std::string OBJModel::getTexture() const {

    std::ifstream inFile;
    
    inFile.open(m_MaterialFile);

    if (inFile.is_open()) {
        std::string line;
        while (getline(inFile, line)) {
            int index = line.find('#');

            if (index != std::string::npos) {
                line = line.substr(0, index);
            }

            std::stringstream stream(line);

            int lineType = -1;

            std::string chunk;

            while (stream >> chunk) {
                if (lineType == -1) {
                    if (chunk == "map_Kd") {
                        lineType = 0;
                    } else {
                        break;
                    }
                } else if (lineType == 0) {
                    inFile.close();
                    return m_ObjectPath + chunk;
                }
            }
        }
    }

    inFile.close();
    return "";
}

// Returns the vector buffer object of points in x,y,z triplets followed by s, t texture coordinates
std::vector<float> OBJModel::getVBO() const {
    return m_VBO;
}

// Returns the index buffer object of the points (0 index)
std::vector<unsigned int> OBJModel::getIBO() const {
    return m_IBO;
}

void OBJModel::populateBuffers() {
    std::vector<float> VBO;
    std::vector<unsigned int> IBO;

    std::vector<VertexData> vertices;

    for (int i = 0; i < m_Faces.size(); i = i + 3) {
        unsigned int pos = m_Faces[i];
        unsigned int text = m_Faces[i + 1];
        unsigned int norm = m_Faces[i + 2];

        float x = m_Verts[pos*3 + 0];
        float y = m_Verts[pos*3 + 1];
        float z = m_Verts[pos*3 + 2];

        float s = m_VertTexts[text * 2 + 0];
        float t = m_VertTexts[text * 2 + 1]; 
        
        VertexData vertex(x, y, z, s, t);

        auto index = std::find(vertices.begin(), vertices.end(), vertex);

        if (index == vertices.end()) {
            VBO.push_back(x); // x
            VBO.push_back(y); // y
            VBO.push_back(z); // z
            VBO.push_back(0); // xn
            VBO.push_back(0); // yn
            VBO.push_back(0); // zn
            VBO.push_back(1); // r
            VBO.push_back(1); // g
            VBO.push_back(1); // b
            VBO.push_back(1); // a
            VBO.push_back(s); // s
            VBO.push_back(t); // t
            
            IBO.push_back(vertices.size());

            vertices.push_back(vertex);
        } else {
            int ind = index - vertices.begin();
            IBO.push_back(ind);
        }

    }

    m_VBO = VBO;
    m_IBO = IBO;
}
