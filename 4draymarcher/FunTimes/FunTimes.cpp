// FunTimes.cpp : Defines the entry point for the application.
//

#include "FunTimes.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

#include "utils.h"
#include "ECSSystem.h"

using namespace std;

float aspect_ratio = 600.0 / 800.0;

void error_callback(int errorCode, const char* description);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// Simple glfw initialization with an error callback



int initializeGlfw() {
	
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		cerr << "glfw init failed" << endl;
		return 1;
	}

	return 0;

}

// initializing the opengl environment with good old glad
int initializeOpengl() {
	if (!gladLoadGL()) {
		cerr << "opengl initialization failed" << endl;
		glfwTerminate();
		return 1;
	}
	return 0;
}

const char* vertexSource = R"glsl(
    #version 150 core

	uniform mat4 rayMatrix;
	
    in vec3 position;

	out vec4 Ray;

    void main()
    {
		int index = int(position.z);
		
		Ray = rayMatrix[index].xyzw;
        gl_Position = vec4(position.xy,0, 1.0);
    }
)glsl";

const char* fragmentSource = R"glsl(
#version 150 core

out vec4 outColor;

uniform float time;

uniform vec4 light;

uniform bool pointLight;

in vec4 Ray;

float sdBox( vec4 p, vec4 b )
{
  
  vec4 d = abs(p) - b;
  return length(max(d,0.0))
         + min(max(d.x,max(d.y,max(d.z,d.w))),0.0); // remove this line for an only partially signed sdf 
}

float opUnion( float d1, float d2 ) { return min(d1,d2); }
float opSubtraction( float d1, float d2 ) { return max(-d1,d2); }

float getDistance(vec4 position){
	vec4 oldpos = position;
	position = vec4(2.5,1.5,5,0)-position;

	float ptime = 3.14/8;
	mat4 XW = mat4(
					cos(ptime),0,0,sin(ptime),
					0,1,0,0,
					0,0,1,0,
					-sin(ptime),0,0,cos(ptime)
					);
	mat4 YW = mat4(
					
					1,0,0,0,
					0,cos(ptime),0,sin(ptime),
					0,0,1,0,
					0,-sin(ptime),0,cos(ptime)
					);
	mat4 ZW = mat4(
					
					1,0,0,0,
					0,1,0,0,
					0,0,cos(ptime),sin(ptime),
					0,0,-sin(ptime),cos(ptime)
					);
	mat4 XY = mat4(
					cos(ptime),sin(ptime),0,0,
					-sin(ptime),cos(ptime),0,0,
					0,0,1,0,
					0,0,0,1
					);
	position = YW*position;
	position.y/=2;
	return opUnion(length(position-vec4(0,0,0,sin(time)*0))-1.1+sin(position.x*10)*.1+cos(position.z*10)*.1,4+oldpos.y);//opSubtraction(sdBox(position,vec4(1,1,1,1)));//-1+sin(position.y*10)*.1+cos(position.z*10)*.1;
}

vec4 calculateGradient(vec4 position){
	vec4 smallDist = vec4(0.001,0,0,0);
	
	vec4 grad = vec4(
		getDistance(position + smallDist.xyyy) - getDistance(position - smallDist.xyyy),
		getDistance(position + smallDist.yxyy) - getDistance(position - smallDist.yxyy),
		getDistance(position + smallDist.yyxy) - getDistance(position - smallDist.yyxy),
		getDistance(position + smallDist.yyyx) - getDistance(position - smallDist.yyyx)
	);

	return normalize(grad);
}

vec4 rayMarch(vec4 ray,vec4 origin){
	vec4 starting = origin;
	for(int i = 0;i<512;i++){
		float distance = getDistance(starting);
		
		if(distance<0.00001){
			vec4 normal = calculateGradient(starting);
			if(false)
			{
				
				return vec4(vec3(pow(i/512.0,1/4.0),1,0),1)*(.5*dot(-normalize(light),normal)+.5);
			}
			else{

				vec4 lightDirection = normalize(light-starting);
				vec4 start = lightDirection*0.001 + starting;
				origin = start;
				bool gotToLight = false;
				for(int j = 0;j<512;j++){
					float lightDistance = length(light-start)-0.1;
					float inDistance = opUnion(getDistance(start),lightDistance);
					if(inDistance<0.00001){
						if(lightDistance<0.00001) gotToLight = true;
						break;
					}
					start = start + lightDirection*inDistance;
					if(length(start-origin)>100){
						break;
					}
				}

				float multiplier = gotToLight?1:.2;
				return vec4(vec3(pow(1,1/4.0),1,0),1) * multiplier*(.5*dot(normalize(lightDirection),normal)+.5);;
			}
		}
		starting = starting + ray*distance;
		if(length(starting-origin)>100) break;
	}
	return vec4(0,0,0,0);
}

void main()
{
	outColor = rayMarch(Ray,vec4(0));
}
)glsl";

void compileShaderOrFAIL(GLuint shader) {
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) return;
	char buffer[512];
	glGetShaderInfoLog(shader, 512, NULL, buffer);
	cerr << buffer;
	properExit(status);
}
glm::mat4 getCameraFrustumMatrix(float fov, float aspectRatio) {
	glm::mat4 matrix(1);
	glm::vec3 forward(0, 0, -1);
	glm::vec3 right(0, 1, 0);
	glm::vec3 up(1, 0, 0);

	fov *= Deg2Rad;

	auto halffov(fov * .5);

	auto fovTan = tanf(halffov);


	auto toRight = fovTan * aspectRatio * right;
	auto toTop = fovTan * up;

	auto topLeft = glm::normalize(glm::vec4(-forward - toRight + toTop,0));
	auto topRight = glm::normalize(glm::vec4(-forward + toRight + toTop, 0));
	auto bottomRight = glm::normalize(glm::vec4(-forward + toRight - toTop, 0));
	auto bottomLeft = glm::normalize(glm::vec4(-forward - toRight - toTop, 0));

	matrix[0] = glm::vec4(topLeft);
	matrix[1] = glm::vec4(topRight);
	matrix[2] = glm::vec4(bottomRight);
	matrix[3] = glm::vec4(bottomLeft);

	return matrix;
}
int main()
{

	//cout << glm::to_string(getCameraFrustumMatrix(90, 1.6));
	// Initializing GLFW
	conditionalExit(initializeGlfw());
	
	// create our window and show it, as well as making it our opengl context
	GLFWwindow* window = glfwCreateWindow(800, 600, "Title", NULL, NULL);

	if (!window) {
		cout << "glfw window failed" << endl;
		properExit(1);
	}
	glfwShowWindow(window);
	glfwMakeContextCurrent(window);

	// initialize opengl
	conditionalExit(initializeOpengl());

	// create the opengl viewport
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	float vertices[] = {
		1,1,1, // top right
		-1,1,2, // top left
		-1,-1,3, // bottom left
		1,-1,0 // bottom right
	};

	unsigned int elements[] = {
		1,3,0,
		1,2,3
	};

	unsigned int vbo;
	glGenBuffers(1, &vbo);

	unsigned int ebo;
	glGenBuffers(1, &ebo);

	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	


	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	auto fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	compileShaderOrFAIL(fragment);

	auto vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	compileShaderOrFAIL(vertex);

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);

	glBindFragDataLocation(program, 0, "outColor");

	glLinkProgram(program);

	glUseProgram(program);


	auto posAttribute = glGetAttribLocation(program, "position");

	glVertexAttribPointer(posAttribute, 3, GL_FLOAT, GL_FALSE, 0,0);
	glEnableVertexAttribArray(posAttribute);

	glUniformMatrix4fv(glGetUniformLocation(program, "rayMatrix"), 1, GL_FALSE, glm::value_ptr(getCameraFrustumMatrix(90, aspect_ratio)));

	//glUniform1i(glGetUniformLocation(program, "pointLight"), GL_TRUE);

	
	while (!glfwWindowShouldClose(window)) {

		// gotta do this every frame
		glClear(GL_COLOR_BUFFER_BIT);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glm::mat4 turn(1);
		glUniform4fv(glGetUniformLocation(program, "light"), 1, glm::value_ptr(glm::vec4(10, 10, 0, sin(glfwGetTime()) * 5)));
		auto val = glfwGetTime();
		//turn[2] = glm::vec4(0, 0, sinf(val), -cosf(val));
		//turn[3] = glm::vec4( 0, 0, cosf(val), sinf(val));

		auto frustrumMat(getCameraFrustumMatrix(90, aspect_ratio));
		frustrumMat[0] = turn * frustrumMat[0];
		frustrumMat[1] = turn * frustrumMat[1];
		frustrumMat[2] = turn * frustrumMat[2];
		frustrumMat[3] = turn * frustrumMat[3];
		glUniformMatrix4fv(glGetUniformLocation(program, "rayMatrix"), 1, GL_FALSE, glm::value_ptr(frustrumMat));

		glUniform1f(glGetUniformLocation(program, "time"), glfwGetTime());
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	
	glDeleteProgram(program);
	glDeleteShader(fragment);
	glDeleteShader(vertex);



	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);

	// clean up stuff
	glfwDestroyWindow(window);

	properExit(0);
	return 0;
}

void error_callback(int code, const char* desc) {
	cout << "Error: " << desc << endl;
}

// resize the viewport along with the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	aspect_ratio = (((float)height/ (float)width)) ;
}

