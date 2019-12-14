/*
CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/

#include <iostream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"

#include "WindowManager.h"
#include "Shape.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;
using namespace glm;
shared_ptr<Shape> sphere, rectangle, asteroid;


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

struct Planet {
    vec3* position;
    float radius;
};

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, h, n, r, f, perspective, jump;
    Planet planetToVisit;
    bool reachedRotation = 0;
    float forwardMoveSpeed, backwardMoveSpeed, upMoveSpeed, downMoveSpeed;
    float leftRotateSpeed, rightRotateSpeed, upRotateSpeed, downRotateSpeed;
	camera()
	{
        reset();
	}
    
    void reset()
    {
        w = a = s = d = h = n = r = f = perspective = jump = 0;
        rot = glm::vec3(0, -1, 0);
        pos = glm::vec3(-65, 0, -5);
        forwardMoveSpeed = -2;
        backwardMoveSpeed = 2;
        upMoveSpeed = 2;
        downMoveSpeed = -2;
        leftRotateSpeed = 0;
        rightRotateSpeed = 0;
        upRotateSpeed = 0;
        downRotateSpeed = 0;
    }
    
	glm::mat4 process(double ftime)
	{
		float flatSpeed = 0;
        float heightSpeed = 0;
        
		if (w == 1)
		{
            if(forwardMoveSpeed < 200)
            {
                forwardMoveSpeed += 2.5;
            }

			flatSpeed = (forwardMoveSpeed)*ftime;
		}
		else if (s == 1)
		{
            flatSpeed = (backwardMoveSpeed)*ftime;
            if(backwardMoveSpeed > -200)
            {
                backwardMoveSpeed -= 2.5;
            }
            
		}
        else if (h == 1)
        {
            heightSpeed = upMoveSpeed*ftime;
            if(upMoveSpeed > - 200)
            {
                upMoveSpeed -= 2.5;
            }
        }
        else if (n == 1)
        {
            heightSpeed = downMoveSpeed*ftime;
            if(downMoveSpeed < 200)
            {
                downMoveSpeed += 2.5;
            }
        }
		float yangle=0;
		if (a == 1)
        {
            yangle = leftRotateSpeed*ftime;
            if(leftRotateSpeed > -5)
            {
                leftRotateSpeed -= 0.1;
            }
            
        }
			
		else if(d==1)
        {
            yangle = rightRotateSpeed*ftime;
            if(rightRotateSpeed < 5)
            {
                rightRotateSpeed += 0.1;
            }
        }
			
        float xangle = 0;
        if (r == 1)
        {
            xangle = upRotateSpeed*ftime;
            if(upRotateSpeed > -5)
            {
                upRotateSpeed -= 0.1;
            }
        }
        else if (f == 1)
        {
            xangle = downRotateSpeed*ftime;
            if(downRotateSpeed < 5)
            {
                downRotateSpeed += 0.1;
            }
            
        }
            
//        if (perspective == 1)
//        {
////            mat4 lookatMatrix =
////            vec3 lookatVector = vec3(lookatMatrix[3][0], lookatMatrix[3][1], lookatMatrix[3][2]);
////
////            float t = distance(pos, lookatVector);
////            pos = lookatVector*(1 - t) + pos*t;
//
////            pos = normalize(lookatVector - pos)*5.0f + pos;
//            vec3* planetPos = planetToVisit.position;
//            vec3 target = -*planetPos;
//            float dist = planetToVisit.radius + planetToVisit.radius/3;
//            pos = vec3(target[0] + dist, target[1], target[2] + dist);
////            pos = vec3(target[0] + 5, target[1], target[2] + 5);
//            target = *planetPos;
//            return lookAt(vec3(pos[0] + 5, pos[1], pos[2] + 5), target, vec3(0, 1, 0));
//        }
        if (jump == 1)
        {
            vec3* planetPos = planetToVisit.position;
            vec3 target = -*planetPos;
            float dist = 4*planetToVisit.radius;
            pos = vec3(target[0] + dist, target[1] , target[2] - dist);
//            return lookAt(pos, vec3(-target[0], target[1], -target[2]), vec3(0, 1, 0));
//            target = *planetToVisit;
        }
		rot.y += yangle;
        rot.x += xangle;
		glm::mat4 RY = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
        glm::mat4 RX = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, heightSpeed, flatSpeed,1);
		dir = dir * RX * RY;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return RX * RY *T;
	}
    
    mat4 shiftMatrix(mat4 from, mat4 to)
    {
        mat4 res = from;
        for(int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                if(res[i][j] < to[i][j])
                {
                    res[i][j] += 1;
                }
                else if(res[i][j] > to[i][j])
                {
                    res[i][j] -= 1;
                }
            }
        }
        return res;
    }
    
    
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;
    
    
    glm::vec3 sunPos, mercuryPos, venusPos, earthPos, marsPos, jupiterPos, saturnPos,
    uranusPos, neptunePos, plutoPos, cometPos;
        
    string currentPlanet = "sun";

	// Our shader program
    std::shared_ptr<Program> sunProg, earthProg, venusProg, mercuryProg, marsProg, jupiterProg, saturnProg, ringProg, uranusProg, uranusRingProg, neptuneProg, plutoProg, universeProg, cometProg, cometTailProg;
    
    std::shared_ptr<Program> lunaProg, europaProg, callistoProg, ganymedeProg, ioProg, iapetusProg, rheaProg, titanProg;


	// Contains vertex information for OpenGL
	GLuint SaturnRingArrayID, UranusRingArrayID, CometTailArrayID;

	// Data necessary to give our box to OpenGL
	GLuint SaturnVertexBufferID, UranusVertexBufferID, VertexColorIDBox, SaturnVertexNormDBox, UranusVertexNormDBox, SaturnVertexTexBox, UranusVertexTexBox, SaturnRingIndexBufferIDBox, UranusRingIndexBufferIDBox;
    
    GLuint TailVertexBufferID, TailColorIDBox, TailVertexNormDBox, TailVertexTexBox, TailIndexBufferIDBox;

	//texture data
	GLuint EarthTexture, EarthSpecularTexture, EarthCloudsTexture;
    GLuint VenusTexture, VenusAtmosphereTexture;
    GLuint MercuryTexture;
    GLuint MarsTexture;
    GLuint SunTexture, SunTexture2;
    GLuint JupiterTexture;
    GLuint SaturnTexture, SaturnRingTexture;
    GLuint UranusTexture, UranusRingTexture;
    GLuint NeptuneTexture;
    GLuint PlutoTexture;
    GLuint UniverseTexture;
    
    GLuint LunaTexture;
    GLuint EuropaTexture, IoTexture, CallistoTexture, GanymedeTexture;
    GLuint IapetusTexture, TitanTexture, RheaTexture;
    
    GLuint CometTexture, CometTailTexture;
    
    Planet getNextPlanetToVisit() {
        if(currentPlanet == "sun")
        {
            currentPlanet = "mer";
            return Planet {&mercuryPos, 0.4};
        }
        else if(currentPlanet == "mer")
        {
            currentPlanet = "ven";
            return Planet {&venusPos, 0.475};
        }
        else if(currentPlanet == "ven")
        {
            currentPlanet = "ear";
            return Planet {&earthPos, 0.5};
        }
        else if(currentPlanet == "ear")
        {
            currentPlanet = "mar";
            return Planet {&marsPos, 0.275};
        }
        else if(currentPlanet == "mar")
        {
            currentPlanet = "jup";
            return Planet {&jupiterPos, 4.5};
        }
        else if(currentPlanet == "jup")
        {
            currentPlanet = "sat";
            return Planet {&saturnPos, 4.25};
        }
        else if(currentPlanet == "sat")
        {
            currentPlanet = "ura";
            return Planet {&uranusPos, 1.5};
        }
        else if(currentPlanet == "ura")
        {
            currentPlanet = "nep";
            return Planet {&neptunePos, 1.45};
        }
        else if(currentPlanet == "nep")
        {
            currentPlanet = "plu";
            return Planet {&plutoPos, 0.15};
        }
        else if(currentPlanet == "plu")
        {
            currentPlanet = "com";
            return Planet {&cometPos, 8};
        }
        currentPlanet = "sun";
        return Planet{&sunPos, 54.5};
    }
    
    void setPreviousPlanetToVisit() {
        if(currentPlanet == "sun")
        {
            currentPlanet = "plu";
        }
        else if(currentPlanet == "com")
        {
            currentPlanet = "nep";
        }
        else if(currentPlanet == "mer")
        {
            currentPlanet = "com";
        }
        else if(currentPlanet == "ven")
        {
            currentPlanet = "sun";
        }
        else if(currentPlanet == "ear")
        {
            currentPlanet = "mer";
        }
        else if(currentPlanet == "mar")
        {
            currentPlanet = "ven";
        }
        else if(currentPlanet == "jup")
        {
            currentPlanet = "ear";
        }
        else if(currentPlanet == "sat")
        {
            currentPlanet = "mar";
        }
        else if(currentPlanet == "ura")
        {
            currentPlanet = "jup";
        }
        else if(currentPlanet == "nep")
        {
            currentPlanet = "sat";
        }
        else
        {
            currentPlanet = "ura";
        }
    }

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
            mycam.forwardMoveSpeed = -2;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
            mycam.backwardMoveSpeed = 2;
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
            mycam.leftRotateSpeed = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
            mycam.rightRotateSpeed = 0;
		}
        if (key == GLFW_KEY_H && action == GLFW_PRESS)
        {
            mycam.h = 1;
        }
        if (key == GLFW_KEY_H && action == GLFW_RELEASE)
        {
            mycam.h = 0;
            mycam.upMoveSpeed = 2;
        }
        if (key == GLFW_KEY_N && action == GLFW_PRESS)
        {
            mycam.n = 1;
        }
        if (key == GLFW_KEY_N && action == GLFW_RELEASE)
        {
            mycam.n = 0;
            mycam.downMoveSpeed = -2;
        }
        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            mycam.r = 1;
        }
        if (key == GLFW_KEY_R && action == GLFW_RELEASE)
        {
            mycam.r = 0;
            mycam.upRotateSpeed = 0;
        }
        if (key == GLFW_KEY_F && action == GLFW_PRESS)
        {
            mycam.f = 1;
        }
        if (key == GLFW_KEY_F && action == GLFW_RELEASE)
        {
            mycam.f = 0;
            mycam.downRotateSpeed = 0;
        }
        if (key == GLFW_KEY_J && action == GLFW_PRESS)
        {
            mycam.jump = 1;
        
            mycam.planetToVisit = getNextPlanetToVisit();
//            mycam.planetToVisit = &saturnPos;

            cout << currentPlanet << endl;
        }
        if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
        {
            mycam.jump = 0;
            mycam.perspective = 0;
        }
        if (key == GLFW_KEY_P && action == GLFW_RELEASE)
        {
            setPreviousPlanetToVisit();
            mycam.jump = 1;
                    
            mycam.planetToVisit = getNextPlanetToVisit();
//            mycam.planetToVisit = &saturnPos;

            cout << currentPlanet << endl;
        }
        if (key == GLFW_KEY_BACKSPACE && action == GLFW_RELEASE)
        {
            mycam.reset();
        }
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
//		if (action == GLFW_PRESS)
//		{
//			glfwGetCursorPos(window, &posX, &posY);
//			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
//
//			//change this to be the points converted to WORLD
//			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
//			newPt[0] = 0;
//			newPt[1] = 0;
//
//			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
//			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
//			//update the vertex array with the updated points
//			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
//			glBindBuffer(GL_ARRAY_BUFFER, 0);
//		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		string resourceDirectory = "../../resources" ;
		// Initialize mesh.
		sphere = make_shared<Shape>();
        rectangle = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();
        asteroid = make_shared<Shape>();
        asteroid->loadMesh(resourceDirectory + "/asteroid.obj");
        asteroid->resize();
        asteroid->init();
//        rectangle->loadMesh(resourceDirectory + "/rectangle.obj");
//        rectangle->resize();
//        rectangle->init();
        
        // Set up the saturn rings
        //generate the VAO
        glGenVertexArrays(1, &SaturnRingArrayID);
        glBindVertexArray(SaturnRingArrayID);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &SaturnVertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, SaturnVertexBufferID);

        GLfloat cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,//LD
            1.0, -1.0,  1.0,//RD
            1.0,  1.0,  1.0,//RU
            -1.0,  1.0,  1.0,//LU
        };
        //make it a bit smaller
        for (int i = 0; i < 12; i++)
            cube_vertices[i] *= 0.5;
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_DYNAMIC_DRAW);

        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        GLfloat cube_norm[] = {
            // front colors
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,

        };
        glGenBuffers(1, &SaturnVertexNormDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, SaturnVertexNormDBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_norm), cube_norm, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        glm::vec2 cube_tex[] = {
            // front colors
            glm::vec2(0.0, 1.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.0),
            glm::vec2(0.0, 0.0),

        };
        glGenBuffers(1, &SaturnVertexTexBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, SaturnVertexTexBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cube_tex), cube_tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &SaturnRingIndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SaturnRingIndexBufferIDBox);
        GLushort cube_elements[] = {

            // front
            0, 1, 2,
            2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
        
        // Set up the Comet tail
        //generate the VAO
        glGenVertexArrays(1, &CometTailArrayID);
        glBindVertexArray(CometTailArrayID);

        //generate vertex buffer to hand off to OGL
        glGenBuffers(1, &TailVertexBufferID);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, TailVertexBufferID);

        GLfloat tail_cube_vertices[] = {
            // front
            -1.0, -1.0,  1.0,//LD
            1.0, -1.0,  1.0,//RD
            1.0,  1.0,  1.0,//RU
            -1.0,  1.0,  1.0,//LU
        };
        //make it a bit smaller
        for (int i = 0; i < 12; i++)
            tail_cube_vertices[i] *= 0.5;
        //actually memcopy the data - only do this once
        glBufferData(GL_ARRAY_BUFFER, sizeof(tail_cube_vertices), tail_cube_vertices, GL_DYNAMIC_DRAW);

        //we need to set up the vertex array
        glEnableVertexAttribArray(0);
        //key function to get up how many elements to pull out at a time (3)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        GLfloat tail_cube_norm[] = {
            // front colors
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,
            0.0, 0.0, 1.0,

        };
        glGenBuffers(1, &TailVertexNormDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, TailVertexNormDBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tail_cube_norm), tail_cube_norm, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        //color
        glm::vec2 tail_cube_tex[] = {
            // front colors
            glm::vec2(0.0, 1.0),
            glm::vec2(1.0, 1.0),
            glm::vec2(1.0, 0.0),
            glm::vec2(0.0, 0.0),

        };
        glGenBuffers(1, &TailVertexTexBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ARRAY_BUFFER, TailVertexTexBox);
        glBufferData(GL_ARRAY_BUFFER, sizeof(tail_cube_tex), tail_cube_tex, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glGenBuffers(1, &TailIndexBufferIDBox);
        //set the current state to focus on our vertex buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TailIndexBufferIDBox);
        GLushort tail_cube_elements[] = {

            // front
            0, 1, 2,
            2, 3, 0,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tail_cube_elements), tail_cube_elements, GL_STATIC_DRAW);
        
        glBindVertexArray(0);
        

        
		int width, height, channels;
		char filepath[1000];

        // Sun map
        string str = resourceDirectory + "/2k_sun.jpg";
        strcpy(filepath, str.c_str());
        unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &SunTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SunTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Sun second texture map
        str = resourceDirectory + "/sunmap.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &SunTexture2);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, SunTexture2);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Setting up Sun textures:
        GLuint SunTexLoc = glGetUniformLocation(sunProg->pid, "sunTex");//tex, tex2... sampler in the fragment shader
        GLuint SunTexLoc2 = glGetUniformLocation(sunProg->pid, "sunTex2");

        // Then bind the uniform samplers to texture units:
        glUseProgram(sunProg->pid);
        glUniform1i(SunTexLoc, 0);
        glUniform1i(SunTexLoc2, 1);

        
		// earth map
		str = resourceDirectory + "/2k_earth_daymap.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &EarthTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, EarthTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		
        //earth specular map
        str = resourceDirectory + "/2k_earth_specular_map.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &EarthSpecularTexture);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, EarthSpecularTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        //earth clouds
        str = resourceDirectory + "/2k_earth_clouds.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &EarthCloudsTexture);
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, EarthCloudsTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Setting up earth textures:
        GLuint EarthTex1Loc = glGetUniformLocation(earthProg->pid, "tex");//tex, tex2... sampler in the fragment shader
        GLuint Tex2Location = glGetUniformLocation(earthProg->pid, "tex2");
        GLuint Tex3Location = glGetUniformLocation(earthProg->pid, "tex3");
        // Then bind the uniform samplers to texture units:
        glUseProgram(earthProg->pid);
        glUniform1i(EarthTex1Loc, 2);
        glUniform1i(Tex2Location, 3);
        glUniform1i(Tex3Location, 4);
        
        // mercury map
        str = resourceDirectory + "/2k_mercury.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &MercuryTexture);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, MercuryTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // Setting up mercury textures
        GLuint MercuryTexLoc = glGetUniformLocation(mercuryProg->pid, "mercuryTex");
        glUseProgram(mercuryProg->pid);
       glUniform1i(MercuryTexLoc, 5);
        
        //venus map
       str = resourceDirectory + "/2k_venus_surface.jpg";
       strcpy(filepath, str.c_str());
       data = stbi_load(filepath, &width, &height, &channels, 4);
       glGenTextures(1, &VenusTexture);
       glActiveTexture(GL_TEXTURE6);
       glBindTexture(GL_TEXTURE_2D, VenusTexture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       glGenerateMipmap(GL_TEXTURE_2D);
        
        // venus cloud map
        str = resourceDirectory + "/2k_venus_atmosphere.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &VenusAtmosphereTexture);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, VenusAtmosphereTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        
        // Setting up Venus textures:
        GLuint VenusTex1Loc = glGetUniformLocation(venusProg->pid, "venusTex");//tex, tex2... sampler in the fragment shader
        GLuint VenusTex2Loc = glGetUniformLocation(venusProg->pid, "venusAtmos");
        // Then bind the uniform samplers to texture units:
        glUseProgram(venusProg->pid);
        glUniform1i(VenusTex1Loc, 6);
        glUniform1i(VenusTex2Loc, 7);
		
        // Mars texture
        str = resourceDirectory + "/2k_mars.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &MarsTexture);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, MarsTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        
        // Setting up Mars textures:
        GLuint MarsTex = glGetUniformLocation(marsProg->pid, "marsTex");//tex, tex2... sampler in the fragment shader
        // Then bind the uniform samplers to texture units:
        glUseProgram(marsProg->pid);
        glUniform1i(MarsTex, 8);
        
        // Jupiter texture
        str = resourceDirectory + "/2k_jupiter.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &JupiterTexture);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, JupiterTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        
        
        // Setting up Jupiter textures:
        GLuint JupTex = glGetUniformLocation(jupiterProg->pid, "jupTex");//tex, tex2... sampler in the fragment shader
        // Then bind the uniform samplers to texture units:
        glUseProgram(jupiterProg->pid);
        glUniform1i(JupTex, 9);
        
        // Saturn texture
        str = resourceDirectory + "/2k_saturn.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &SaturnTexture);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, SaturnTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
       // Setting up Saturn Ring textures:
        GLuint SatTex = glGetUniformLocation(saturnProg->pid, "satTex");//tex,
        glUseProgram(saturnProg->pid);
         glUniform1i(SatTex, 10);
        
        // Saturn rings texture
       str = resourceDirectory + "/saturn_rings.png";
       strcpy(filepath, str.c_str());
       data = stbi_load(filepath, &width, &height, &channels, 4);
               glGenTextures(1, &SaturnRingTexture);
       glActiveTexture(GL_TEXTURE11);
       glBindTexture(GL_TEXTURE_2D, SaturnRingTexture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       glGenerateMipmap(GL_TEXTURE_2D);
       
        GLuint SatRingTex = glGetUniformLocation(ringProg->pid, "satRingTex");//tex, tex2... sampler in the fragment shader
        glUseProgram(ringProg->pid);
        glUniform1i(SatRingTex, 11);
        
        // Uranus texture
      str = resourceDirectory + "/2k_uranus.jpg";
      strcpy(filepath, str.c_str());
      data = stbi_load(filepath, &width, &height, &channels, 4);
      glGenTextures(1, &UranusTexture);
      glActiveTexture(GL_TEXTURE12);
      glBindTexture(GL_TEXTURE_2D, UranusTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      
       GLuint UrTex = glGetUniformLocation(uranusProg->pid, "urTex");//tex, tex2... sampler in the fragment shader
       glUseProgram(uranusProg->pid);
       glUniform1i(UrTex, 12);
        
//     // Uranus rings texture
//    str = resourceDirectory + "/saturn_rings.png";
//    strcpy(filepath, str.c_str());
//    data = stbi_load(filepath, &width, &height, &channels, 4);
//            glGenTextures(1, &UranusRingTexture);
//    glActiveTexture(GL_TEXTURE13);
//    glBindTexture(GL_TEXTURE_2D, UranusRingTexture);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
//    glGenerateMipmap(GL_TEXTURE_2D);
//
//     GLuint UraRingTex = glGetUniformLocation(uranusRingProg->pid, "uraRingTex");//tex, tex2... sampler in the fragment shader
//     glUseProgram(uranusRingProg->pid);
//     glUniform1i(UraRingTex, 13);

    // Neptune texture
      str = resourceDirectory + "/2k_neptune.jpg";
      strcpy(filepath, str.c_str());
      data = stbi_load(filepath, &width, &height, &channels, 4);
      glGenTextures(1, &NeptuneTexture);
      glActiveTexture(GL_TEXTURE14);
      glBindTexture(GL_TEXTURE_2D, NeptuneTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      
       GLuint NepTex = glGetUniformLocation(neptuneProg->pid, "nepTex");//tex, tex2... sampler in the fragment shader
       glUseProgram(neptuneProg->pid);
       glUniform1i(NepTex, 14);
        
    // Pluto texture
    str = resourceDirectory + "/2k_pluto.jpg";
    strcpy(filepath, str.c_str());
    data = stbi_load(filepath, &width, &height, &channels, 4);
    glGenTextures(1, &PlutoTexture);
    glActiveTexture(GL_TEXTURE15);
    glBindTexture(GL_TEXTURE_2D, PlutoTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
     GLuint PluTex = glGetUniformLocation(plutoProg->pid, "pluTex");//tex, tex2... sampler in the fragment shader
     glUseProgram(plutoProg->pid);
     glUniform1i(PluTex, 15);
        
    str = resourceDirectory + "/8k_stars_milky_way.jpg";
    strcpy(filepath, str.c_str());
    data = stbi_load(filepath, &width, &height, &channels, 4);
    glGenTextures(1, &UniverseTexture);
    glActiveTexture(GL_TEXTURE16);
    glBindTexture(GL_TEXTURE_2D, UniverseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint UniverseTex = glGetUniformLocation(universeProg->pid, "uniTex");//tex, tex2... sampler in the fragment shader
    glUseProgram(universeProg->pid);
    glUniform1i(UniverseTex, 16);
        
    str = resourceDirectory + "/2k_moon.jpg";
    strcpy(filepath, str.c_str());
    data = stbi_load(filepath, &width, &height, &channels, 4);
    glGenTextures(1, &LunaTexture);
    glActiveTexture(GL_TEXTURE17);
    glBindTexture(GL_TEXTURE_2D, LunaTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    GLuint LunTex = glGetUniformLocation(lunaProg->pid, "lunTex");//tex, tex2... sampler in the fragment shader
    glUseProgram(lunaProg->pid);
    glUniform1i(LunTex, 17);
        

        str = resourceDirectory + "/1k_europa.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &EuropaTexture);
        glActiveTexture(GL_TEXTURE18);
        glBindTexture(GL_TEXTURE_2D, EuropaTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint EurTex = glGetUniformLocation(europaProg->pid, "eurTex");//tex, tex2... sampler in the fragment shader
        glUseProgram(europaProg->pid);
        glUniform1i(EurTex, 18);
        
        str = resourceDirectory + "/2k_ganymede.jpg";
       strcpy(filepath, str.c_str());
       data = stbi_load(filepath, &width, &height, &channels, 4);
       glGenTextures(1, &GanymedeTexture);
       glActiveTexture(GL_TEXTURE19);
       glBindTexture(GL_TEXTURE_2D, GanymedeTexture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       glGenerateMipmap(GL_TEXTURE_2D);
       GLuint GanTex = glGetUniformLocation(ganymedeProg->pid, "ganTex");//tex, tex2... sampler in the fragment shader
       glUseProgram(ganymedeProg->pid);
       glUniform1i(GanTex, 19);
        
        str = resourceDirectory + "/2k_io.png";
       strcpy(filepath, str.c_str());
       data = stbi_load(filepath, &width, &height, &channels, 4);
       glGenTextures(1, &IoTexture);
       glActiveTexture(GL_TEXTURE20);
       glBindTexture(GL_TEXTURE_2D, IoTexture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       glGenerateMipmap(GL_TEXTURE_2D);
       GLuint IoTex = glGetUniformLocation(ioProg->pid, "ioTex");//tex, tex2... sampler in the fragment shader
       glUseProgram(ioProg->pid);
       glUniform1i(IoTex, 20);
        
        str = resourceDirectory + "/2k_callisto.jpg";
       strcpy(filepath, str.c_str());
       data = stbi_load(filepath, &width, &height, &channels, 4);
       glGenTextures(1, &CallistoTexture);
       glActiveTexture(GL_TEXTURE21);
       glBindTexture(GL_TEXTURE_2D, CallistoTexture);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
       glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
       glGenerateMipmap(GL_TEXTURE_2D);
       GLuint CalTex = glGetUniformLocation(callistoProg->pid, "calTex");//tex, tex2... sampler in the fragment shader
       glUseProgram(callistoProg->pid);
       glUniform1i(CalTex, 21);
        
        str = resourceDirectory + "/2k_rhea.jpg";
      strcpy(filepath, str.c_str());
      data = stbi_load(filepath, &width, &height, &channels, 4);
      glGenTextures(1, &RheaTexture);
      glActiveTexture(GL_TEXTURE22);
      glBindTexture(GL_TEXTURE_2D, RheaTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      GLuint RheTex = glGetUniformLocation(rheaProg->pid, "rheTex");//tex, tex2... sampler in the fragment shader
      glUseProgram(rheaProg->pid);
      glUniform1i(RheTex, 22);
        
        str = resourceDirectory + "/4k_iapetus.jpg";
      strcpy(filepath, str.c_str());
      data = stbi_load(filepath, &width, &height, &channels, 4);
      glGenTextures(1, &IapetusTexture);
      glActiveTexture(GL_TEXTURE23);
      glBindTexture(GL_TEXTURE_2D, IapetusTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      GLuint IapTex = glGetUniformLocation(iapetusProg->pid, "iapTex");//tex, tex2... sampler in the fragment shader
      glUseProgram(iapetusProg->pid);
      glUniform1i(IapTex, 23);
        
        str = resourceDirectory + "/2k_titan.png";
      strcpy(filepath, str.c_str());
      data = stbi_load(filepath, &width, &height, &channels, 4);
      glGenTextures(1, &TitanTexture);
      glActiveTexture(GL_TEXTURE24);
      glBindTexture(GL_TEXTURE_2D, TitanTexture);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      glGenerateMipmap(GL_TEXTURE_2D);
      GLuint TitTex = glGetUniformLocation(titanProg->pid, "titTex");//tex, tex2... sampler in the fragment shader
      glUseProgram(titanProg->pid);
      glUniform1i(TitTex, 24);
        
         str = resourceDirectory + "/2k_ganymede.jpg";
        strcpy(filepath, str.c_str());
        data = stbi_load(filepath, &width, &height, &channels, 4);
        glGenTextures(1, &CometTexture);
        glActiveTexture(GL_TEXTURE25);
        glBindTexture(GL_TEXTURE_2D, CometTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        GLuint ComTex = glGetUniformLocation(cometProg->pid, "comTex");//tex, tex2... sampler in the fragment shader
        glUseProgram(cometProg->pid);
        glUniform1i(ComTex, 25);
        
        str = resourceDirectory + "/comet_tail.png";
         strcpy(filepath, str.c_str());
         data = stbi_load(filepath, &width, &height, &channels, 4);
         glGenTextures(1, &CometTailTexture);
         glActiveTexture(GL_TEXTURE26);
         glBindTexture(GL_TEXTURE_2D, CometTailTexture);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
         glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
         glGenerateMipmap(GL_TEXTURE_2D);
         GLuint ComTailTex = glGetUniformLocation(cometTailProg->pid, "tailTex");//tex, tex2... sampler in the fragment shader
         glUseProgram(cometTailProg->pid);
         glUniform1i(ComTailTex, 26);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
//        glEnable(GL_BLEND);
//        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//        glEnable(GL_CULL_FACE);
//        glFrontFace(GL_CCW);

        universeProg = std::make_shared<Program>();
        universeProg->setVerbose(true);
        universeProg->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
        if (!universeProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        glEnable(GL_BLEND);

        universeProg->addUniform("P");
        universeProg->addUniform("V");
        universeProg->addUniform("M");
        universeProg->addUniform("campos");
        universeProg->addAttribute("vertPos");
        universeProg->addAttribute("vertNor");
        universeProg->addAttribute("vertTex");
        
		// Initialize the GLSL program.
		earthProg = std::make_shared<Program>();
		earthProg->setVerbose(true);
		earthProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/earth_fragment_shader.glsl");
		if (!earthProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		earthProg->addUniform("P");
		earthProg->addUniform("V");
		earthProg->addUniform("M");
		earthProg->addUniform("campos");
        earthProg->addUniform("lightPos");
		earthProg->addAttribute("vertPos");
		earthProg->addAttribute("vertNor");
		earthProg->addAttribute("vertTex");
    
        
        venusProg = std::make_shared<Program>();
        venusProg->setVerbose(true);
        venusProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/venus_fragment_shader.glsl");
        if (!venusProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        venusProg->addUniform("P");
        venusProg->addUniform("V");
        venusProg->addUniform("M");
        venusProg->addUniform("campos");
        venusProg->addUniform("lightPos");
        venusProg->addAttribute("vertPos");
        venusProg->addAttribute("vertNor");
        venusProg->addAttribute("vertTex");
        
        mercuryProg = std::make_shared<Program>();
        mercuryProg->setVerbose(true);
        mercuryProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/mercury_fragment_shader.glsl");
        if (!mercuryProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        mercuryProg->addUniform("P");
        mercuryProg->addUniform("V");
        mercuryProg->addUniform("M");
        mercuryProg->addUniform("campos");
        mercuryProg->addUniform("lightPos");
        mercuryProg->addAttribute("vertPos");
        mercuryProg->addAttribute("vertNor");
        mercuryProg->addAttribute("vertTex");
        
        marsProg = std::make_shared<Program>();
        marsProg->setVerbose(true);
        marsProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/mars_fragment_shader.glsl");
        if (!marsProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        marsProg->addUniform("P");
        marsProg->addUniform("V");
        marsProg->addUniform("M");
        marsProg->addUniform("campos");
        marsProg->addUniform("lightPos");
        marsProg->addAttribute("vertPos");
        marsProg->addAttribute("vertNor");
        marsProg->addAttribute("vertTex");
        
        jupiterProg = std::make_shared<Program>();
        jupiterProg->setVerbose(true);
        jupiterProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/jupiter_fragment_shader.glsl");
        if (!jupiterProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        jupiterProg->addUniform("P");
        jupiterProg->addUniform("V");
        jupiterProg->addUniform("M");
        jupiterProg->addUniform("campos");
        jupiterProg->addUniform("lightPos");
        jupiterProg->addAttribute("vertPos");
        jupiterProg->addAttribute("vertNor");
        jupiterProg->addAttribute("vertTex");
        
        saturnProg = std::make_shared<Program>();
        saturnProg->setVerbose(true);
        saturnProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/saturn_fragment_shader.glsl");
        if (!saturnProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        saturnProg->addUniform("P");
        saturnProg->addUniform("V");
        saturnProg->addUniform("M");
        saturnProg->addUniform("campos");
        saturnProg->addUniform("lightPos");
        saturnProg->addAttribute("vertPos");
        saturnProg->addAttribute("vertNor");
        saturnProg->addAttribute("vertTex");
        
        ringProg = std::make_shared<Program>();
        ringProg->setVerbose(true);
        ringProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/saturn_ring_fragment_shader.glsl");
        if (!ringProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        ringProg->addUniform("P");
        ringProg->addUniform("V");
        ringProg->addUniform("M");
        ringProg->addUniform("campos");
        ringProg->addUniform("lightPos");
        ringProg->addAttribute("vertPos");
        ringProg->addAttribute("vertNor");
        ringProg->addAttribute("vertTex");
        ringProg->addUniform("planet");
        
        uranusProg = std::make_shared<Program>();
        uranusProg->setVerbose(true);
        uranusProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/uranus_fragment_shader.glsl");
        if (!uranusProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        uranusProg->addUniform("P");
        uranusProg->addUniform("V");
        uranusProg->addUniform("M");
        uranusProg->addUniform("campos");
        uranusProg->addUniform("lightPos");
        uranusProg->addAttribute("vertPos");
        uranusProg->addAttribute("vertNor");
        uranusProg->addAttribute("vertTex");
        
//        uranusRingProg = std::make_shared<Program>();
//        uranusRingProg->setVerbose(true);
//        uranusRingProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/uranus_ring_fragment_shader.glsl");
//        if (!uranusRingProg->init())
//        {
//            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
//            exit(1);
//        }
//        uranusRingProg->addUniform("P");
//        uranusRingProg->addUniform("V");
//        uranusRingProg->addUniform("M");
//        uranusRingProg->addUniform("campos");
//        uranusRingProg->addUniform("lightPos");
//        uranusRingProg->addAttribute("vertPos");
//        uranusRingProg->addAttribute("vertNor");
//        uranusRingProg->addAttribute("vertTex");
        
        neptuneProg = std::make_shared<Program>();
        neptuneProg->setVerbose(true);
        neptuneProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/neptune_fragment_shader.glsl");
        if (!neptuneProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        neptuneProg->addUniform("P");
        neptuneProg->addUniform("V");
        neptuneProg->addUniform("M");
        neptuneProg->addUniform("campos");
        neptuneProg->addUniform("lightPos");
        neptuneProg->addAttribute("vertPos");
        neptuneProg->addAttribute("vertNor");
        neptuneProg->addAttribute("vertTex");
        
        plutoProg = std::make_shared<Program>();
        plutoProg->setVerbose(true);
        plutoProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/pluto_fragment_shader.glsl");
        if (!plutoProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        plutoProg->addUniform("P");
        plutoProg->addUniform("V");
        plutoProg->addUniform("M");
        plutoProg->addUniform("campos");
        plutoProg->addUniform("lightPos");
        plutoProg->addAttribute("vertPos");
        plutoProg->addAttribute("vertNor");
        plutoProg->addAttribute("vertTex");
        
        sunProg = std::make_shared<Program>();
        sunProg->setVerbose(true);
        sunProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/sun_fragment_shader.glsl");
        if (!sunProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        sunProg->addUniform("P");
        sunProg->addUniform("V");
        sunProg->addUniform("M");
        sunProg->addUniform("campos");
        sunProg->addUniform("sunTimeReference");
        sunProg->addAttribute("vertPos");
        sunProg->addAttribute("vertNor");
        sunProg->addAttribute("vertTex");
        
        lunaProg = std::make_shared<Program>();
        lunaProg->setVerbose(true);
        lunaProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/luna_fragment_shader.glsl");
        if (!lunaProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        lunaProg->addUniform("P");
        lunaProg->addUniform("V");
        lunaProg->addUniform("M");
        lunaProg->addUniform("campos");
        lunaProg->addUniform("lightPos");
        lunaProg->addAttribute("vertPos");
        lunaProg->addAttribute("vertNor");
        lunaProg->addAttribute("vertTex");
        
        europaProg = std::make_shared<Program>();
        europaProg->setVerbose(true);
        europaProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/europa_fragment_shader.glsl");
        if (!europaProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        europaProg->addUniform("P");
        europaProg->addUniform("V");
        europaProg->addUniform("M");
        europaProg->addUniform("campos");
        europaProg->addUniform("lightPos");
        europaProg->addAttribute("vertPos");
        europaProg->addAttribute("vertNor");
        europaProg->addAttribute("vertTex");
        
        ioProg = std::make_shared<Program>();
        ioProg->setVerbose(true);
        ioProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/io_fragment_shader.glsl");
        if (!ioProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        ioProg->addUniform("P");
        ioProg->addUniform("V");
        ioProg->addUniform("M");
        ioProg->addUniform("campos");
        ioProg->addUniform("lightPos");
        ioProg->addAttribute("vertPos");
        ioProg->addAttribute("vertNor");
        ioProg->addAttribute("vertTex");
        
        ganymedeProg = std::make_shared<Program>();
        ganymedeProg->setVerbose(true);
        ganymedeProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/ganymede_fragment_shader.glsl");
        if (!ganymedeProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        ganymedeProg->addUniform("P");
        ganymedeProg->addUniform("V");
        ganymedeProg->addUniform("M");
        ganymedeProg->addUniform("campos");
        ganymedeProg->addUniform("lightPos");
        ganymedeProg->addAttribute("vertPos");
        ganymedeProg->addAttribute("vertNor");
        ganymedeProg->addAttribute("vertTex");
        
        callistoProg = std::make_shared<Program>();
        callistoProg->setVerbose(true);
        callistoProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/callisto_fragment_shader.glsl");
        if (!callistoProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        callistoProg->addUniform("P");
        callistoProg->addUniform("V");
        callistoProg->addUniform("M");
        callistoProg->addUniform("campos");
        callistoProg->addUniform("lightPos");
        callistoProg->addAttribute("vertPos");
        callistoProg->addAttribute("vertNor");
        callistoProg->addAttribute("vertTex");
        
        titanProg = std::make_shared<Program>();
        titanProg->setVerbose(true);
        titanProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/titan_fragment_shader.glsl");
        if (!titanProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        titanProg->addUniform("P");
        titanProg->addUniform("V");
        titanProg->addUniform("M");
        titanProg->addUniform("campos");
        titanProg->addUniform("lightPos");
        titanProg->addAttribute("vertPos");
        titanProg->addAttribute("vertNor");
        titanProg->addAttribute("vertTex");
        
        iapetusProg = std::make_shared<Program>();
        iapetusProg->setVerbose(true);
        iapetusProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/iapetus_fragment_shader.glsl");
        if (!iapetusProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        iapetusProg->addUniform("P");
        iapetusProg->addUniform("V");
        iapetusProg->addUniform("M");
        iapetusProg->addUniform("campos");
        iapetusProg->addUniform("lightPos");
        iapetusProg->addAttribute("vertPos");
        iapetusProg->addAttribute("vertNor");
        iapetusProg->addAttribute("vertTex");
        
        rheaProg = std::make_shared<Program>();
        rheaProg->setVerbose(true);
        rheaProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/rhea_fragment_shader.glsl");
        if (!rheaProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        rheaProg->addUniform("P");
        rheaProg->addUniform("V");
        rheaProg->addUniform("M");
        rheaProg->addUniform("campos");
        rheaProg->addUniform("lightPos");
        rheaProg->addAttribute("vertPos");
        rheaProg->addAttribute("vertNor");
        rheaProg->addAttribute("vertTex");
        
        
         cometProg = std::make_shared<Program>();
         cometProg->setVerbose(true);
         cometProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/halley_fragment_shader.glsl");
         if (!cometProg->init())
         {
             std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
             exit(1);
         }
         cometProg->addUniform("P");
         cometProg->addUniform("V");
         cometProg->addUniform("M");
         cometProg->addUniform("campos");
         cometProg->addUniform("lightPos");
         cometProg->addAttribute("vertPos");
         cometProg->addAttribute("vertNor");
         cometProg->addAttribute("vertTex");
        
        cometTailProg = std::make_shared<Program>();
        cometTailProg->setVerbose(true);
        cometTailProg->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/halley_tail_fragment_shader.glsl");
        if (!cometTailProg->init())
        {
            std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
            exit(1);
        }
        cometTailProg->addUniform("P");
        cometTailProg->addUniform("V");
        cometTailProg->addUniform("M");
        cometTailProg->addUniform("campos");
        cometTailProg->addUniform("lightPos");
        cometTailProg->addAttribute("vertPos");
        cometTailProg->addAttribute("vertNor");
        cometTailProg->addAttribute("vertTex");
	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
    
		double frametime = get_last_elapsed_time();

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V, M, P; //View, Model and Perspective matrix
		V = mycam.process(frametime);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones

		//animation with the model matrix:
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float angle = -3.1415926/2.0;
        
        universeProg->bind();
        float sangle = 3.1415926 / 2.;
        glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 camp = -mycam.pos;
        glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
        glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

        M = TransSky * RotateXSky * SSky;
                
        //send the matrices to the shaders
        glUniformMatrix4fv(universeProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(universeProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(universeProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(universeProg->getUniform("campos"), 1, &mycam.pos[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, UniverseTexture);
        glDisable(GL_DEPTH_TEST);
        sphere->draw(universeProg, false);
        glEnable(GL_DEPTH_TEST);
    
        universeProg->unbind();
        
		glm::vec3 EarthScale = glm::vec3(0.5f, 0.5f, 0.5f);
        
        glm::vec3 sunPosition = glm::vec3(0, 0, 0);
        sunPos = sunPosition;
        
        sunProg->bind();
        glm::mat4 TransSun = glm::translate(glm::mat4(1.0f), sunPosition);
        glm::mat4 RSun = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
        glm::mat4 RotateSun = glm::rotate(glm::mat4(1.0f), w/20, glm::vec3(0, 1, 0));
//        glm::mat4 ScaleSun = glm::scale(glm::mat4(1.0f), glm::vec3(20, 20, 20));
        glm::mat4 ScaleSun = glm::scale(glm::mat4(1), 109.0f*EarthScale);
        glm::mat4 MSun = TransSun * RotateSun * RSun;
        M = MSun * ScaleSun;
        glUniformMatrix4fv(sunProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(sunProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(sunProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(sunProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform1f(sunProg->getUniform("sunTimeReference"), glfwGetTime());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, SunTexture);
        sphere->draw(sunProg,false);
        sunProg->unbind();
        
        
        mercuryProg->bind();
        glm::mat4 RotateMercury = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0, 1, 0));
        glm::mat4 MercOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(80, 0, 0));
        static float mercuryOrbit = 0;
        mercuryOrbit += 0.02;
        glm::mat4 MercuryOrbit = glm::rotate(glm::mat4(1.0f), mercuryOrbit, glm::vec3(0, 1, 0));
        glm::mat4 MercuryScale = glm::scale(glm::mat4(1.0f), 0.45f*EarthScale);
        glm::mat4 MMercury = MercuryOrbit *  MercOrbTrans * RotateMercury;
        mercuryPos = vec3(MMercury[3][0], MMercury[3][1], MMercury[3][2]);
        M = MMercury * MercuryScale;
        glUniformMatrix4fv(mercuryProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(mercuryProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(mercuryProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(mercuryProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(mercuryProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(mercuryProg,false);
        mercuryProg->unbind();
        
        venusProg->bind();
        glm::mat4 RotateVenus = glm::rotate(glm::mat4(1.0f), -w, glm::vec3(0, 1, 0));
        glm::mat4 VenusOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(105, 0, 0));
        static float venusOrbit = 0;
        venusOrbit += 0.008;
        glm::mat4 VenusOrbit = glm::rotate(glm::mat4(1.0f), venusOrbit, glm::vec3(0, 1, 0));
        glm::mat4 MVenus = VenusOrbit * VenusOrbTrans * RotateVenus;
        glm::mat4 ScaleVenus = glm::scale(glm::mat4(1), 0.95f*EarthScale);
        venusPos = vec3(MVenus[3][0], MVenus[3][1], MVenus[3][2]);
        M = MVenus * ScaleVenus;
        glUniformMatrix4fv(venusProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(venusProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(venusProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(venusProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(venusProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(venusProg,false);
        venusProg->unbind();

		// Draw the box using GLSL.
		earthProg->bind();		
		//send the matrices to the shaders

//        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 EarthOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 130));
        glm::mat4 REarth = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
        static float earthRotation = 0;
        earthRotation += 0.018;
        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        static float earthOrbit = 0;
        earthOrbit += 0.008;
//        earthOrbit += 0.01;
        glm::mat4 EarthOrbit = glm::rotate(glm::mat4(1.0f), earthOrbit, glm::vec3(0, 1, 0));
        glm::mat4 MEarth = EarthOrbit * EarthOrbTrans * RotateEarth * REarth;
        earthPos = vec3(MEarth[3][0], MEarth[3][1], MEarth[3][2]);
        glm::mat4 ScaleEarth = glm::scale(glm::mat4(1), EarthScale);
        M = MEarth * ScaleEarth;
		glUniformMatrix4fv(earthProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(earthProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(earthProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(earthProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(earthProg->getUniform("lightPos"), 1, &sunPosition[0]);
		sphere->draw(earthProg,false);
		earthProg->unbind();
        
        // Draw the box using GLSL.
        lunaProg->bind();
        //send the matrices to the shaders

//        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 LunaOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(5, 0, 0));
        glm::mat4 RLuna = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
//        static float lunaRotation = 0;
//        earthRotation += 0.1;
//        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleLuna = glm::scale(mat4(1), 0.25f*EarthScale);
        static float lunaOrbit = 0;
        lunaOrbit -= 0.1;
        glm::mat4 LunaOrbit = glm::rotate(glm::mat4(1.0f), lunaOrbit, glm::vec3(1   , 0, 0));
        glm::mat4 MLuna = MEarth * LunaOrbTrans;
        M =MLuna * ScaleLuna;
        glUniformMatrix4fv(lunaProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(lunaProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(lunaProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(lunaProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(lunaProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(lunaProg,false);
        lunaProg->unbind();
        
        marsProg->bind();
       
        glm::mat4 MarsOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-155, 0, 0));
        glm::mat4 RMars = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
        static float marsOrbit = 0;
        marsOrbit += 0.002;
        glm::mat4 MarsOrbit = glm::rotate(glm::mat4(1.0f), marsOrbit, glm::vec3(0, 1, 0));
        glm::mat4 RotateMars = glm::rotate(glm::mat4(1.0f), earthOrbit, glm::vec3(0, 1, 0));
        glm::mat4 MMars = MarsOrbit * MarsOrbTrans * RotateMars * RMars;
        marsPos = vec3(MMars[3][0], MMars[3][1], MMars[3][2]);
        glm::mat4 ScaleMars = glm::scale(glm::mat4(1), 0.55f*EarthScale);
        M = MMars * ScaleMars;
        glUniformMatrix4fv(marsProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(marsProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(marsProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(marsProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(marsProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(marsProg,false);
        marsProg->unbind();
        
        jupiterProg->bind();
        glm::mat4 RJupiter = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 JupiterOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -480));
        static float jupiterOrbit = 0;
        jupiterOrbit += 0.0009;
        glm::mat4 JupiterOrbit = glm::rotate(glm::mat4(1.0f), jupiterOrbit, glm::vec3(0, 1, 0));
        glm::mat4 RotateJupiter = glm::rotate(glm::mat4(1.0f), earthRotation*1.3f, glm::vec3(0, 1, 0));
        glm::mat4 MJupiter = JupiterOrbit * JupiterOrbTrans * RotateJupiter * RJupiter;
        jupiterPos = vec3(MJupiter[3][0], MJupiter[3][1], MJupiter[3][2]);
        glm::mat4 ScaleJupiter = glm::scale(glm::mat4(1), 9.0f*EarthScale);
        M = MJupiter * ScaleJupiter;
        glUniformMatrix4fv(jupiterProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(jupiterProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(jupiterProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(jupiterProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(jupiterProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(jupiterProg,false);
        jupiterProg->unbind();
        
        ioProg->bind();
        //send the matrices to the shaders

    //        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 IoOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(8, 0, 0));
        glm::mat4 RIo = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
    //        static float lunaRotation = 0;
    //        earthRotation += 0.1;
    //        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleIo = glm::scale(mat4(1), 0.25f*EarthScale);
        static float ioOrbit = 0;
        ioOrbit += 0.016;
        glm::mat4 IoOrbit = glm::rotate(glm::mat4(1.0f), ioOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MIo = MJupiter * IoOrbit * IoOrbTrans  ;
        M =MIo * ScaleIo;
        glUniformMatrix4fv(ioProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(ioProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(ioProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(ioProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(ioProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(ioProg,false);
        ioProg->unbind();
        
        europaProg->bind();
        //send the matrices to the shaders

//        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 EuropaOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0,11, 0));
        glm::mat4 REuropa = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
//        static float lunaRotation = 0;
//        earthRotation += 0.1;
//        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleEuropa = glm::scale(mat4(1), 0.25f*EarthScale);
        static float europaOrbit = 0;
        europaOrbit -= 0.0016;
        glm::mat4 EuropaOrbit = glm::rotate(glm::mat4(1.0f), europaOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MEuropa = MJupiter * EuropaOrbit * EuropaOrbTrans  ;
        M =MEuropa * ScaleEuropa;
        glUniformMatrix4fv(europaProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(europaProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(europaProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(europaProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(europaProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(europaProg,false);
        europaProg->unbind();
        
        ganymedeProg->bind();
        //send the matrices to the shaders

//        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 GanymedeOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(14, 14, 0));
//        glm::mat4 REuropa = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
//        static float lunaRotation = 0;
//        earthRotation += 0.1;
//        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleGanymede = glm::scale(mat4(1), 0.5f*EarthScale);
        static float ganymedeOrbit = 0;
        ganymedeOrbit += 0.008;
        glm::mat4 GanymedeOrbit = glm::rotate(glm::mat4(1.0f), ganymedeOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MGanymede = MJupiter * GanymedeOrbit * GanymedeOrbTrans  ;
        M =MGanymede * ScaleGanymede;
        glUniformMatrix4fv(ganymedeProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(ganymedeProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(ganymedeProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(ganymedeProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(ganymedeProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(ganymedeProg,false);
        ganymedeProg->unbind();
        
        callistoProg->bind();
        //send the matrices to the shaders

//        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 CallistoOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-16, 0, 0));
//        glm::mat4 REuropa = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
//        static float lunaRotation = 0;
//        earthRotation += 0.1;
//        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleCallisto = glm::scale(mat4(1), 0.41f*EarthScale);
        static float callistoOrbit = 0;
        callistoOrbit += 0.003;
        glm::mat4 CallistoOrbit = glm::rotate(glm::mat4(1.0f), callistoOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MCallisto = MJupiter * CallistoOrbit * CallistoOrbTrans  ;
        M =MCallisto * ScaleCallisto;
        glUniformMatrix4fv(callistoProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(callistoProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(callistoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(callistoProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(callistoProg->getUniform("lightPos"), 1, &sunPosition[0]);
//        sphere->draw(callistoProg,false);
        callistoProg->unbind();

        saturnProg->bind();
        glm::mat4 RSaturn = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 SaturnOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(600, 0,0));
//        mat4 SaturnOrbTrans = translate(mat4(1), vec3(80, 0, 0));
        static float saturnOrbit = 0;
        saturnOrbit += 0.0008;
        mat4 SaturnAxis = rotate(mat4(1), 2.0f, vec3(1, 0, 0));
        glm::mat4 SaturnOrbit = glm::rotate(glm::mat4(1.0f), saturnOrbit, glm::vec3(0, 1, 0));
        glm::mat4 RotateSaturn = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 MSaturn = SaturnOrbit * SaturnOrbTrans * RotateSaturn * RSaturn;
        saturnPos = vec3(MSaturn[3][0], MSaturn[3][1], MSaturn[3][2]);
        glm::mat4 ScaleSaturn = glm::scale(glm::mat4(1), 8.5f*EarthScale);
        M = MSaturn * ScaleSaturn;
        glUniformMatrix4fv(saturnProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(saturnProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(saturnProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(saturnProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(saturnProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(saturnProg,false);
        saturnProg->unbind();
//        glm::mat4 MSaturnRings = glm::translate(glm::mat4(1.0f), glm::vec3(60, 0, 0));
        
        ringProg->bind();
        glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(SaturnRingArrayID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SaturnRingIndexBufferIDBox);
        
        mat4 ScaleRings = scale(mat4(1), vec3(20, 20, 20));
        mat4 RotateRings = rotate(mat4(1), -1.2f, vec3(1, 0, 0));
        mat4 TranslateRings = translate(mat4(1), vec3(saturnPos.x, saturnPos.y - 9.5, saturnPos.z - 3.5));
        M = TranslateRings * RotateRings * ScaleRings;
        glUniformMatrix4fv(ringProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(ringProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(ringProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(ringProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(ringProg->getUniform("lightPos"), 1, &sunPosition[0]);
        glad_glUniform1f(ringProg->getUniform("planet"), 0);
        
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, SaturnRingTexture);
//        glDisable(GL_DEPTH_TEST);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        ringProg->unbind();
        
        rheaProg->bind();
        //send the matrices to the shaders

    //        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 RheaOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(8, 0, 0));
//        glm::mat4 RIo = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
    //        static float lunaRotation = 0;
    //        earthRotation += 0.1;
    //        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleRhea = glm::scale(mat4(1), 0.125f*EarthScale);
        static float rheaOrbit = 0;
        rheaOrbit += 0.016;
        mat4 RotateRheaOrbitAxis = rotate(mat4(1), 0.9f, vec3(1, 0, 0));
        glm::mat4 RheaOrbit = glm::rotate(glm::mat4(1.0f), rheaOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MRhea = MSaturn * RheaOrbit * RheaOrbTrans * RotateRheaOrbitAxis;
        M =MRhea * ScaleRhea;
        glUniformMatrix4fv(rheaProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(rheaProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(rheaProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(rheaProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(rheaProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(rheaProg,false);
        rheaProg->unbind();
        
        titanProg->bind();
        //send the matrices to the shaders

    //        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 TitanOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(16, 0, 0));
//        glm::mat4 RIo = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
    //        static float lunaRotation = 0;
    //        earthRotation += 0.1;
    //        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleTitan = glm::scale(mat4(1), 0.35f*EarthScale);
        static float titanOrbit = 0;
        titanOrbit += 0.0016;
        glm::mat4 TitanOrbit = glm::rotate(glm::mat4(1.0f), titanOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MTitan = MSaturn * TitanOrbit * TitanOrbTrans;
        M =MTitan * ScaleTitan;
        glUniformMatrix4fv(titanProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(titanProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(titanProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(titanProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(titanProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(titanProg,false);
        titanProg->unbind();
        
        iapetusProg->bind();
        //send the matrices to the shaders

    //        M =  TransEarth * RotateZ * RotateY * RotateX  * S;
        glm::mat4 IapetusOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-19, 0, 0));
//        glm::mat4 RIo = glm::rotate(glm::mat4(1.0f), -1.3f, glm::vec3(1, 0, 0));
    //        static float lunaRotation = 0;
    //        earthRotation += 0.1;
    //        glm::mat4 RotateEarth = glm::rotate(glm::mat4(1.0f), earthRotation, glm::vec3(0, 1, 0));
        glm::mat4 ScaleIapetus = glm::scale(mat4(1), 0.12f*EarthScale);
        static float iapetusOrbit = 0;
        iapetusOrbit += 0.0008;
        glm::mat4 IapetusOrbit = glm::rotate(glm::mat4(1.0f), iapetusOrbit, glm::vec3(0, 0, 1));
        glm::mat4 MIapetus = MSaturn * IapetusOrbit * IapetusOrbTrans;
        M =MIapetus * ScaleIapetus;
        glUniformMatrix4fv(iapetusProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(iapetusProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(iapetusProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(iapetusProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(iapetusProg->getUniform("lightPos"), 1, &sunPosition[0]);
        sphere->draw(iapetusProg,false);
        iapetusProg->unbind();
                
       
        uranusProg->bind();
//        glm::mat4 RSaturn = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 UranusOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 750));
//        mat4 UranusOrbTrans = translate(mat4(1), vec3(70, 0, 0));
        // Using this to create elliptic orbit around sun
        glm::mat4 UranusEllipseOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(20, 0, 0));
        static float uranusOrbit = 0;
        uranusOrbit += 0.0003;
        glm::mat4 UranusOrbit = glm::rotate(glm::mat4(1.0f), uranusOrbit, glm::vec3(0, 1, 0));
        glm::mat4 Rotateuranus = glm::rotate(glm::mat4(1.0f), earthRotation*2, glm::vec3(0, 0, 1));
        glm::mat4 MUranus = UranusOrbit * UranusOrbTrans * UranusEllipseOrbTrans * Rotateuranus;
        uranusPos = vec3(MUranus[3][0], MUranus[3][1], MUranus[3][2]);
        glm::mat4 ScaleUranus = glm::scale(glm::mat4(1), 3.0f*EarthScale);
        M = MUranus * ScaleUranus;
        glUniformMatrix4fv(uranusProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(uranusProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(uranusProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(uranusProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(uranusProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(uranusProg,false);
        uranusProg->unbind();
    
        ringProg->bind();
        glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(SaturnRingArrayID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SaturnRingIndexBufferIDBox);
        
        ScaleRings = scale(mat4(1), vec3(13, 13, 13));
        RotateRings = rotate(mat4(1), 0.0f, vec3(1, 0, 0));
        TranslateRings = translate(mat4(1), vec3(uranusPos.x, uranusPos.y , uranusPos.z - 6.5));
        M = TranslateRings * RotateRings * ScaleRings;
        glUniformMatrix4fv(ringProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(ringProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(ringProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(ringProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(ringProg->getUniform("lightPos"), 1, &sunPosition[0]);
        glad_glUniform1f(ringProg->getUniform("planet"), 1);
        
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, SaturnRingTexture);
//        glDisable(GL_DEPTH_TEST);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        ringProg->unbind();
        
         neptuneProg->bind();
        //        glm::mat4 RSaturn = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 NeptuneOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(900, 0, 0));
        // Using this to create elliptic orbit around sun
        glm::mat4 NeptuneEllipseOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(50, 0, 0));
        static float neptuneOrbit = 0;
        neptuneOrbit += 0.0001;
        glm::mat4 NeptuneOrbit = glm::rotate(glm::mat4(1.0f), neptuneOrbit, glm::vec3(0, 1, 0));
        glm::mat4 RotateNeptune = glm::rotate(glm::mat4(1.0f), earthRotation/2, glm::vec3(0, 1, 0));
        glm::mat4 RNeptune = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 MNeptune = NeptuneOrbit * NeptuneOrbTrans * NeptuneEllipseOrbTrans * RotateNeptune * RNeptune;
        neptunePos = vec3(MNeptune[3][0], MNeptune[3][1], MNeptune[3][2]);
        glm::mat4 NeptuneScale = glm::scale(glm::mat4(1), 2.9f*EarthScale);
        M = MNeptune * NeptuneScale;
        glUniformMatrix4fv(neptuneProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(neptuneProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(neptuneProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(neptuneProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(neptuneProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(neptuneProg,false);
        neptuneProg->unbind();
        
        plutoProg->bind();
        //        glm::mat4 RSaturn = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 PlutoOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(1050, 0, 0));
        // Using this to create elliptic orbit around sun
        glm::mat4 PlutoEllipseOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(-50, 0, 0));
        static float plutoOrbit = 0;
        plutoOrbit += 0.0001;
        glm::mat4 PlutoOrbit = glm::rotate(glm::mat4(1.0f), plutoOrbit, glm::vec3(0.2f, 1, 0));
        glm::mat4 RotatePluto = glm::rotate(glm::mat4(1.0f), earthRotation/6, glm::vec3(0, 1, 0));
        glm::mat4 RPluto = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 MPluto = PlutoOrbit * PlutoOrbTrans * PlutoEllipseOrbTrans * RotatePluto * RPluto;
        plutoPos = vec3(MPluto[3][0], MPluto[3][1], MPluto[3][2]);
        glm::mat4 PlutoScale = glm::scale(glm::mat4(1), 0.3f*EarthScale);
        M = MPluto * PlutoScale;
        glUniformMatrix4fv(plutoProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(plutoProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(plutoProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(plutoProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(plutoProg->getUniform("lightPos"), 1, &sunPosition[0]);

        sphere->draw(plutoProg,false);
        plutoProg->unbind();
        
        cometProg->bind();
        //        glm::mat4 RSaturn = glm::rotate(glm::mat4(1.0f), -1.56f, glm::vec3(1, 0, 0));
        glm::mat4 CometOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(300, 300, 0));
        // Using this to create elliptic orbit around sun
        static float cometRotate = 0;
        cometRotate += 0.01;
        mat4 RotateComet = rotate(mat4(1), cometRotate, vec3(1, 0, 0));
        glm::mat4 CometEllipseOrbTrans = glm::translate(glm::mat4(1.0f), glm::vec3(500, 0, 0));
        static float cometOrbit = 0;
        cometOrbit += 0.003;
        glm::mat4 CometOrbit = glm::rotate(glm::mat4(1.0f), cometOrbit, glm::vec3(0.2f, 1, 0));
//        glm::mat4 MComet = CometOrbit * CometOrbTrans * CometEllipseOrbTrans;
        glm::mat4 MComet = CometOrbit * CometOrbTrans * CometEllipseOrbTrans * RotateComet;
        glm::mat4 CometScale = glm::scale(glm::mat4(1), 0.8f*EarthScale);
        cometPos = vec3(MComet[3][0], MComet[3][1], MComet[3][2]);
        M = MComet * CometScale;
        glUniformMatrix4fv(cometProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(cometProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(cometProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(cometProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(cometProg->getUniform("lightPos"), 1, &sunPosition[0]);

        asteroid->draw(cometProg,false);
        cometProg->unbind();
        
        cometTailProg->bind();
        glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindVertexArray(CometTailArrayID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TailIndexBufferIDBox);
        mat4 Vi = glm::transpose(V);
        Vi[0][3] = 0;
        Vi[1][3] = 0;
        Vi[2][3] = 0;
        mat4 ScaleTail = scale(mat4(1), vec3(20, 70, 20));
        mat4 RotateTail = rotate(mat4(1), -1.8f, vec3(0, 0, 1));
        mat4 TranslateTail = translate(mat4(1), vec3(cometPos.x + 24, cometPos.y , cometPos.z - 10));
        mat4 RotTailTrans = translate(mat4(1), vec3(0, 0, -70));
        mat4 MTail = TranslateTail * RotateTail;
        M = MTail * ScaleTail;
        glUniformMatrix4fv(cometTailProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
        glUniformMatrix4fv(cometTailProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
        glUniformMatrix4fv(cometTailProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
        glUniform3fv(cometTailProg->getUniform("campos"), 1, &mycam.pos[0]);
        glUniform3fv(cometTailProg->getUniform("lightPos"), 1, &sunPosition[0]);
        
        glActiveTexture(GL_TEXTURE26);
        glBindTexture(GL_TEXTURE_2D, CometTailTexture);
//        glDisable(GL_DEPTH_TEST);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);
        glDisable(GL_BLEND);

        cometTailProg->unbind();
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
