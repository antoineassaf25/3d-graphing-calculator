  // ==================================================================
  #version 330 core
  
  // Note: That the 'in' means this is coming from a previous stage of
  //       our pipeline.
  in vec4 v_vertexColors;
  in vec3 v_normals;
  in vec2 v_textureCoordinates;
  in float highlight;
  in vec3 coloring;

  uniform sampler2D u_DiffuseTexture;
  
  // The fragment shader should have exactly one output.
  // That output is the final color in which we rasterize this 
  // fragment. We can name it anything we want, but it should
  // be outputting a vec4.
  out vec4 color;

  void main()
  {
    
   vec3 diffuseColor = vec3(0.0f, 0.0f, 0.0f);
   diffuseColor = texture(u_DiffuseTexture, v_textureCoordinates).rgb;


   //vec4 VertexColors = vec4(v_vertexColors.r, v_vertexColors.g, v_vertexColors.b, 1.0f);

   // color is a vec4 representing color. Because we are in a fragment
   // shader, we are expecting in our pipeline a color output.
   // That is essentially the job of the fragment shader--
   // to output one final color.
   
   if (coloring.x < 0 && coloring.y < 0 && coloring.z < 0) {
       color = vec4(diffuseColor, 1.0f) * v_vertexColors * highlight;
   } else {
       color = vec4(coloring, 1.0f) * highlight;
   }
  }
  // ==================================================================

