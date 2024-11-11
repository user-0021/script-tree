#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>

// vertexShader
const GLchar* _wtVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;

void main() {
    gl_Position = vec4(aPos,0.0, 1.0);
}
)";

// flagmentShader
const GLchar* _wtFragmentShaderSource = R"(
#version 330 core
uniform vec4 userSetColor;
out vec4 FragColor;

void main() {
    FragColor = userSetColor;
}
)";

// textVertexShader
const GLchar* _wtTextVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}  
)";

// textFlagmentShader
const GLchar* _wtTextFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

int _wtGenerateShader(int* programID,const GLchar* const vertexShaderSource,const GLchar* const fragmentShaderSource);

int _wtInitShader(){
	// init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr,"Failed to initialize GLAD\n");
		return 0;
    }	
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

	return 1;
}

int _wtGenerateShader(int* programID,const GLchar* const vertexShaderSource,const GLchar* const fragmentShaderSource){
	
	//vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1,&vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	//check compile
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		fprintf(stderr,"Faild compile vertex shader:%s\n",infoLog);
		return 0;
	}

	//fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1,&fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	//check compile
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		fprintf(stderr,"Faild compile fragmend shader:%s\n",infoLog);
		return 0;
	}

	//link shader program
	*programID = glCreateProgram();
	glAttachShader(*programID, vertexShader);
	glAttachShader(*programID, fragmentShader);
	glLinkProgram(*programID);

	//check link 
	glGetProgramiv(*programID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(*programID, 512, NULL, infoLog);
		fprintf(stderr,"Faild compile fragmend shader:%s\n",infoLog);
		return 0;
	}
	
	//delate shader
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	return 1;
}
