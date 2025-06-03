// ==================== Libraries ==================
// Depending on the operating system we use
// The paths to SDL are actually different.
// The #define statement should be passed in
// when compiling using the -D argument.
// This gives an example of how a programmer
// may support multiple platforms with different
// dependencies.
#if defined(LINUX) || defined(MINGW)
    #include <SDL2/SDL.h>
#else // This works for Mac
    #include <SDL.h>
#endif

// Third Party Libraries
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp> 

// C++ Standard Template Library (STL)
#include <iostream>
#include <vector>
#include <OBJModel.hpp>
#include <Camera.hpp>
#include <Texture.hpp>
#include <Graph.hpp>
#include <fstream>

// vvvvvvvvvvvvvvvvvvvvvvvvvv Globals vvvvvvvvvvvvvvvvvvvvvvvvvv
// Globals generally are prefixed with 'g' in this application.

// Screen Dimensions
int gScreenWidth 						= 640*2;
int gScreenHeight 						= 480*2;
SDL_Window* gGraphicsApplicationWindow 	= nullptr;
SDL_GLContext gOpenGLContext			= nullptr;

// Main loop flag
bool gQuit = false; // If this is quit = 'true' then the program terminates.

// shader
// The following stores the a unique id for the graphics pipeline
// program object that will be used for our OpenGL draw calls.
GLuint gGraphicsPipelineShaderProgram	= 0;

// OpenGL Objects
// Vertex Array Object (VAO)
// Vertex array objects encapsulate all of the items needed to render an object.
// For example, we may have multiple vertex buffer objects (VBO) related to rendering one
// object. The VAO allows us to setup the OpenGL state to render that object using the
// correct layout and correct buffers with one call after being setup.
GLuint gVertexArrayObject					= 0;
// Vertex Buffer Object (VBO)
// Vertex Buffer Objects store information relating to vertices (e.g. positions, normals, textures)
// VBOs are our mechanism for arranging geometry on the GPU.
GLuint 	gVertexBufferObject					= 0;
GLuint  gIndexBufferObject                  = 0;

unsigned int equationCount = 0;


std::vector<std::string> gEquations;

int gFaceCount = 0;
int gDrawMode = 0;
int gRESOLUTION = 401; // 401x401

float g_CameraRadius = 15.0f;
float g_RotateTheta = 45.0f;
float g_RotatePhi = 30.0f;

float g_CenterX = 0.0f;
float g_CenterY = 0.0f;
float g_CenterZ = 0.0f;

int u_coloring = 0;
int u_highlight = 0;

Camera gCamera;

Texture gTexture;
// ^^^^^^^^^^^^^^^^^^^^^^^^ Globals ^^^^^^^^^^^^^^^^^^^^^^^^^^^


static void GLClearAllErrors(){
    while(glGetError() != GL_NO_ERROR){
    }
}

// Returns true if we have an error
static bool GLCheckErrorStatus(const char* function, int line){
    while(GLenum error = glGetError()){
        std::cout << "OpenGL Error:" << error 
                  << "\tLine: " << line 
                  << "\tfunction: " << function << std::endl;
        return true;
    }
    return false;
}

#define GLCheck(x) GLClearAllErrors(); x; GLCheckErrorStatus(#x,__LINE__);



// loads the shader in as a string from the glsl file (episode 12 Mike Shah Intro to OpenGL series)
std::string ShaderToString(const std::string& filename) {
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()) {
        while (std::getline(myFile, line)) {
            result += line + '\n';
        }
        myFile.close();
    }
    
    return result;
}


/**
* CompileShader will compile any valid vertex, fragment, geometry, tesselation, or compute shader.
* e.g.
*	    Compile a vertex shader: 	CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
*       Compile a fragment shader: 	CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
*
* @param type We use the 'type' field to determine which shader we are going to compile.
* @param source : The shader source code.
* @return id of the shaderObject
*/
GLuint CompileShader(GLuint type, const std::string& source){
	// Compile our shaders
	GLuint shaderObject;

	// Based on the type passed in, we create a shader object specifically for that
	// type.
	if(type == GL_VERTEX_SHADER){
		shaderObject = glCreateShader(GL_VERTEX_SHADER);
	}else if(type == GL_FRAGMENT_SHADER){
		shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	}

	const char* src = source.c_str();
	// The source of our shader
	glShaderSource(shaderObject, 1, &src, nullptr);
	// Now compile our shader
	glCompileShader(shaderObject);

	// Retrieve the result of our compilation
	int result;
	// Our goal with glGetShaderiv is to retrieve the compilation status
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &result);

	if(result == GL_FALSE){
		int length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		char* errorMessages = new char[length]; // Could also use alloca here.
		glGetShaderInfoLog(shaderObject, length, &length, errorMessages);

		if(type == GL_VERTEX_SHADER){
			std::cout << "ERROR: GL_VERTEX_SHADER compilation failed!\n" << errorMessages << "\n";
		}else if(type == GL_FRAGMENT_SHADER){
			std::cout << "ERROR: GL_FRAGMENT_SHADER compilation failed!\n" << errorMessages << "\n";
		}
		// Reclaim our memory
		delete[] errorMessages;

		// Delete our broken shader
		glDeleteShader(shaderObject);

		return 0;
	}

  return shaderObject;
}



/**
* Creates a graphics program object (i.e. graphics pipeline) with a Vertex Shader and a Fragment Shader
*
* @param vertexShaderSource Vertex source code as a string
* @param fragmentShaderSource Fragment shader source code as a string
* @return id of the program Object
*/
GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    // Create a new program object
    GLuint programObject = glCreateProgram();

    // Compile our shaders
    GLuint myVertexShader   = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // Link our two shader programs together.
	// Consider this the equivalent of taking two .cpp files, and linking them into
	// one executable file.
    glAttachShader(programObject,myVertexShader);
    glAttachShader(programObject,myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);

    // Once our final program Object has been created, we can
	// detach and then delete our individual shaders.
    glDetachShader(programObject,myVertexShader);
    glDetachShader(programObject,myFragmentShader);
	// Delete the individual shaders once we are done
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}


/**
* Create the graphics pipeline
*
* @return void
*/
void CreateGraphicsPipeline(){
	
    std::string vertexShaderSource = ShaderToString("./shaders/vert.glsl");
    std::string fragmentShaderSource = ShaderToString("./shaders/frag.glsl");
    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource,fragmentShaderSource);


}


/**
* Initialization of the graphics application. Typically this will involve setting up a window
* and the OpenGL Context (with the appropriate version)
*
* @return void
*/
void InitializeProgram(){
	// Initialize SDL
	if(SDL_Init(SDL_INIT_VIDEO)< 0){
		std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}
	
	// Setup the OpenGL Context
	// Use OpenGL 4.1 core or greater
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	// We want to request a double buffer for smooth updating.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	// Create an application window using OpenGL that supports SDL
	gGraphicsApplicationWindow = SDL_CreateWindow( "OpenGL: 3D Graphing Calculator",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													gScreenWidth,
													gScreenHeight,
													SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );

	// Check if Window did not create.
	if( gGraphicsApplicationWindow == nullptr ){
		std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Create an OpenGL Graphics Context
	gOpenGLContext = SDL_GL_CreateContext( gGraphicsApplicationWindow );
	if( gOpenGLContext == nullptr){
		std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << "\n";
		exit(1);
	}

	// Initialize GLAD Library
	if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
		std::cout << "glad did not initialize" << std::endl;
		exit(1);
	}
	
}

/**
* Create the geometry of the .obj (OBJModel) based on gFilePath
* Assumes there is a texture and mtl file
* @return void
*/
void VertexSpecification(){

    OBJModel commandObject("./objects/grid/grid.obj");
    
    std::string diffuse = commandObject.getTexture();
    
    gTexture.LoadTexture(diffuse);
    
    std::vector<std::vector<float>> VBOs;
    std::vector<std::vector<unsigned int>> IBOs;

    for (int i = 0; i < gEquations.size(); i++) {
        Graph g(gEquations[i], gRESOLUTION, i + 1);
        VBOs.push_back(g.getVBO());
        IBOs.push_back(g.getIBO());
    }
    
    gFaceCount = commandObject.getIBO().size();

    for (int i = 0; i < gEquations.size(); i++) {
        gFaceCount += IBOs[i].size();
    }

    gFaceCount = gFaceCount/3;

    std::vector<GLfloat> vertexData = commandObject.getVBO();
    
    for (int i = 0; i < gEquations.size(); i++) {
        std::vector<GLfloat> vertexDataGraph = VBOs[i];
        vertexData.insert(vertexData.end(), vertexDataGraph.begin(), vertexDataGraph.end());
    }

	glGenVertexArrays(1, &gVertexArrayObject);
	glBindVertexArray(gVertexArrayObject);

	glGenBuffers(1, &gVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, 								// Kind of buffer we are working with 
																// (e.g. GL_ARRAY_BUFFER or GL_ELEMENT_ARRAY_BUFFER)
				 vertexData.size() * sizeof(GL_FLOAT), 	// Size of data in bytes
				 vertexData.data(), 						// Raw array of data
				 GL_STATIC_DRAW);								// How we intend to use the data


    std::vector<GLuint> indexBufferData = commandObject.getIBO();
    
    int offset = commandObject.getVBO().size()/12;
    
    for (int j = 0; j < gEquations.size(); j++) {
        std::vector<GLuint> IBOGraph = IBOs[j];
        for (int i = 0; i < IBOGraph.size(); i++) {
            IBOGraph[i] = IBOGraph[i] + offset;
        }
        offset += VBOs[j].size()/12;
        indexBufferData.insert(indexBufferData.end(), IBOGraph.begin(), IBOGraph.end());
    }
    
    glGenBuffers(1, &gIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indexBufferData.size() * sizeof(GLuint),
                 indexBufferData.data(),
                 GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,  		// Attribute 0 corresponds to the enabled glEnableVertexAttribArray
							  		// In the future, you'll see in our vertex shader this also correspond
							  		// to (layout=0) which selects these attributes.
                          3,  		// The number of components (e.g. x,y,z = 3 components)
                          GL_FLOAT, // Type
                          GL_FALSE, // Is the data normalized
                          sizeof(GL_FLOAT)*12, 		// Stride
                          (void*)0	// Offset
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,
                          3, // xn,yx,zn
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*12,
                          (GLvoid*)(sizeof(GL_FLOAT)*3));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,
                          4, // r,g,b,a
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*12,
                          (GLvoid*)(sizeof(GL_FLOAT)*6));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,
                          2, // s,t
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(GL_FLOAT)*12,
                          (GLvoid*)(sizeof(GL_FLOAT)*10));

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}


/**
* PreDraw
* Typically we will use this for setting some sort of 'state'
* Note: some of the calls may take place at different stages (post-processing) of the
* 		 pipeline.
* @return void
*/
void PreDraw(){
	// Disable depth test and face culling.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
		// Enable texture mapping
	glEnable(GL_TEXTURE_2D);
    glCullFace(GL_FRONT_AND_BACK);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Initialize clear color
    // This is the background of the screen.
    glViewport(0, 0, gScreenWidth, gScreenHeight);
    glClearColor( 0.1f, 0.1f, 0.1f, 1.f );

    //Clear color buffer and Depth Buffer
  	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   	glUseProgram(gGraphicsPipelineShaderProgram);

    // Spherical Coordinates
    float x = g_CameraRadius * glm::sin(glm::radians(g_RotatePhi)) * glm::cos(glm::radians(g_RotateTheta));
    float z = g_CameraRadius * glm::sin(glm::radians(g_RotatePhi)) * glm::sin(glm::radians(g_RotateTheta));
    float y = g_CameraRadius * glm::cos(glm::radians(g_RotatePhi));

    glm::mat4 model = glm::lookAt(glm::vec3(x,y,z),glm::vec3(0,0,0),glm::vec3(0,1,0));

    model = glm::translate(model, glm::vec3(g_CenterX, g_CenterY, g_CenterZ));
    // Retrieve our location of our Model Matrix
    GLint u_ModelMatrixLocation = glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_ModelMatrix");
    if(u_ModelMatrixLocation >=0){
        glUniformMatrix4fv(u_ModelMatrixLocation,1,GL_FALSE,&model[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    // Update the View Matrix
    GLint u_ViewMatrixLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"u_ViewMatrix");
    if(u_ViewMatrixLocation>=0){
        glm::mat4 viewMatrix = gCamera.GetViewMatrix();
        glUniformMatrix4fv(u_ViewMatrixLocation,1,GL_FALSE,&viewMatrix[0][0]);
    }else{
        std::cout << "Could not find u_ModelMatrix, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }


    // Projection matrix (in perspective) 
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f),
                                             (float)gScreenWidth/(float)gScreenHeight,
                                             0.1f,
                                             30.0f);

    // Retrieve our location of our perspective matrix uniform 
    GLint u_ProjectionLocation= glGetUniformLocation( gGraphicsPipelineShaderProgram,"u_Projection");
    if(u_ProjectionLocation>=0){
        glUniformMatrix4fv(u_ProjectionLocation,1,GL_FALSE,&perspective[0][0]);
    }else{
        std::cout << "Could not find u_Perspective, maybe a mispelling?\n";
        exit(EXIT_FAILURE);
    }

    
    // Bind our texture to slot number 0
    gTexture.Bind(0);

    // Setup our uniform for our texture
    GLint u_textureLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram,"u_DiffuseTexture");
    //std::cout << u_textureLocation << std::endl;
	if(u_textureLocation>=0){
		// Setup the slot for the texture
		glUniform1i(u_textureLocation,0);
	}else{
		std::cout << "Could not find u_DiffuseTexture, maybe a misspelling?" << std::endl;
    	exit(EXIT_FAILURE);
	}

    GLint u_colorLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_coloring");

    if (u_colorLocation >= 0) {
        glUniform1i(u_colorLocation, u_coloring);
    } else {
        std::cout << "Could not find u_coloring, maybe a misspelling?" << std::endl;
        exit(EXIT_FAILURE);
    }

    GLint u_highlightLocation = glGetUniformLocation(gGraphicsPipelineShaderProgram, "u_highlight");

    if (u_highlightLocation >= 0) {
        glUniform1i(u_highlightLocation, u_highlight);
    } else {
        std::cout << "Could not find u_highlight, maybe a misspelling?" << std::endl;
        exit(EXIT_FAILURE);
    }


}



/**
* Draw
* The render function gets called once per loop.
* Typically this includes 'glDraw' related calls, and the relevant setup of buffers
* for those calls.
*
* @return void
*/
void Draw(){
    // Enable our attributes
	glBindVertexArray(gVertexArrayObject);

	// Select the vertex buffer object we want to enable
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);

    if (gDrawMode == 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //Render data
    glDrawElements(GL_TRIANGLES, gFaceCount*3, GL_UNSIGNED_INT, 0);

	// Stop using our current graphics pipeline
	// Note: This is not necessary if we only have one graphics pipeline.
    glUseProgram(0);
}


/**
* Helper Function to get OpenGL Version Information
*
* @return void
*/
void getOpenGLVersionInfo(){
  std::cout << "Vendor: " << glGetString(GL_VENDOR) << "\n";
  std::cout << "Renderer: " << glGetString(GL_RENDERER) << "\n";
  std::cout << "Version: " << glGetString(GL_VERSION) << "\n";
  std::cout << "Shading language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
}


/**
* Function called in the main application loop to handle user input
*
* The user can press 
*   use the arrow keys to move the camera around origin
*   H to add highlights to graphs
*   N to visualize normals of graphs
* @return void
*/
void Input(){
	// Event handler that handles various events in SDL
	// that are related to input and output
	SDL_Event e;
	//Handle events on queue
	while(SDL_PollEvent( &e ) != 0){
		if(e.type == SDL_QUIT){
			std::cout << "Goodbye! (Leaving MainApplicationLoop())" << std::endl;
			gQuit = true;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_h) {
                u_highlight = (u_highlight + 1)%2;
            } else if (e.key.keysym.sym == SDLK_n) {
                u_coloring = (u_coloring + 1)%2;
            }
        }
        
        
        //additional global movement
        const Uint8 *state = SDL_GetKeyboardState(NULL);
        
        float SPEED_T = 0.125f;
        float SPEED_R = 2.5f;
           

        //ARROW KEYS - Rotate Theta and Phi
        if (state[SDL_SCANCODE_LEFT]) {
            g_RotateTheta = (g_RotateTheta + SPEED_R);
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            g_RotateTheta = (g_RotateTheta - SPEED_R);
        }
        if (state[SDL_SCANCODE_UP]) {
            g_RotatePhi = glm::clamp(g_RotatePhi - SPEED_R, 0.0f + SPEED_R, 180.0f - SPEED_R);
        }
        if (state[SDL_SCANCODE_DOWN]) {
            g_RotatePhi = glm::clamp(g_RotatePhi + SPEED_R, 0.0f + SPEED_R, 180.0f - SPEED_R);
        }
	}
}


/**
* Main Application Loop
* This is an infinite loop
*
* @return void
*/
void MainLoop(){

	// While application is running
	while(!gQuit){
		// Handle Input
		Input();
		// Setup anything (i.e. OpenGL State) that needs to take
		// place before draw calls
		PreDraw();
		// Draw Calls in OpenGL
		Draw();
		//Update screen of our specified window
		SDL_GL_SwapWindow(gGraphicsApplicationWindow);
	}
}



/**
* The last function called in the program
* This functions responsibility is to destroy any global
* objects in which we have create dmemory.
*
* @return void
*/
void CleanUp(){
	//Destroy our SDL2 Window
	SDL_DestroyWindow(gGraphicsApplicationWindow );
	gGraphicsApplicationWindow = nullptr;

    // Delete our OpenGL Objects
    glDeleteBuffers(1, &gVertexBufferObject);
    glDeleteVertexArrays(1, &gVertexArrayObject);

	// Delete our Graphics pipeline
    glDeleteProgram(gGraphicsPipelineShaderProgram);

	//Quit SDL subsystems
	SDL_Quit();
}


/**
* The main entry point into our C++ programs.
* 
* Accepts one input: the file path of the mesh to be shown
*
* @return program status
*/
int main( int argc, char* args[] ){

    std::cout << std::endl << "Enter up to 3 equations in terms of variables x and y. (note: z axis is UP) " << std::endl;
    std::cout << "Example: ./project \"x^2 + y^2\" \"1/(x*y)\" ..." << std::endl;
    std::cout << "will graph equations:" << std::endl;
    std::cout << "z = x^2 + y^2" << std::endl;
    std::cout << "z = 1/x*y" << std::endl << std::endl;

    std::cout << "Press N to toggle the normals, H to toggle x-y grid highlights, and use the arrow keys to turn the camera" << std::endl;
    std::cout << std::endl;

    if (argc < 2) {
        std::cout << std::endl << "INPUT ERROR: Please specify an expression to load in terms of variables x and y." << std::endl;
        return 0;
    }
    
    if (argc >= 2) {
        gEquations.push_back(args[1]);
    }
    if (argc >= 3) {
        gEquations.push_back(args[2]);
    }
    if (argc >= 4) {
        gEquations.push_back(args[3]);
    }
    
	// 1. Setup the graphics program
	InitializeProgram();
	
	// 2. Setup our geometry
	VertexSpecification();
	
	// 3. Create our graphics pipeline
	// 	- At a minimum, this means the vertex and fragment shader
	CreateGraphicsPipeline();
	
	// 4. Call the main application loop
	MainLoop();	

	// 5. Call the cleanup function when our program terminates
	CleanUp();

	return 0;
}
