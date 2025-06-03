#version 410 core
#
// From Vertex Buffer Object (VBO)
// The only thing that can come 'in', that is
// what our shader reads, the first part of the
// graphics pipeline.
layout(location=0) in vec3 position;
layout(location=1) in vec3 normals;
layout(location=2) in vec4 vertexColors;
layout(location=3) in vec2 textureCoordinates;

// Uniform variables
uniform mat4 u_ModelMatrix;
uniform mat4 u_ViewMatrix;
// We'll use a perspective projection
uniform mat4 u_Projection; 

uniform int u_coloring;
uniform int u_highlight;

// Pass vertex colors into the fragment shader
out vec4 v_vertexColors;
// Pass texture coordinates to the fragment shader
out vec3 v_normals;
out vec2 v_textureCoordinates;
// Highlight based on position
out float highlight;
out vec3 coloring;

void main()
{
    v_normals = normals;	
    v_vertexColors 	 = vertexColors;
    v_textureCoordinates = textureCoordinates;
    
    highlight = 1.0f;
    vec3 m_coloring = vec3(-1.0f,-1.0f,-1.0f);

    // is this texture part of a graph?
    if (textureCoordinates.x <= 0.01f && textureCoordinates.y <= .01f) {
        if (position.x - floor(position.x) < .01f || position.x - floor(position.x) > .99f) { 
            highlight = 1.0f + 0.2f * u_highlight;
        }
        if (position.z - floor(position.z) < .01f || position.z - floor(position.z) > .99f) {
            highlight = 1.0f + 0.2f * u_highlight;
        }
        if (u_coloring == 1) {
            m_coloring.x = abs(normals.x);
            m_coloring.y = abs(normals.z);
            m_coloring.z = abs(normals.y);
        }
    }

    coloring = m_coloring;

    vec4 newPosition = u_Projection * u_ViewMatrix * u_ModelMatrix * vec4(position,1.0f);
                                                                    // Don't forget 'w'
	gl_Position = vec4(newPosition.x, newPosition.y, newPosition.z, newPosition.w);
}
