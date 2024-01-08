#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <GL/glew.h>   // The GL Header File
#include <GL/gl.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

GLuint gProgram[4];
GLint gIntensityLoc;
float gIntensity = 1000;
int gWidth = 1280, gHeight = 720;

struct Vertex
{
    Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Texture
{
    Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
    GLfloat u, v;
};

struct Normal
{
    Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
    GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
    GLuint vIndex[3], tIndex[3], nIndex[3];
};

struct ModelData {
    std::vector<Vertex> vertices;
    std::vector<Texture> textures;
    std::vector<Normal> normals;
    std::vector<Face> faces;
};

ModelData bunnyModel;
ModelData cubeModel;
ModelData quadModel;

GLuint gTextVBO;

struct VBOData {
    GLuint vertexAttribBuffer;
    GLuint indexBuffer;
    GLint inVertexLoc;
    GLint inNormalLoc;
    int vertexDataSizeInBytes;
    int normalDataSizeInBytes;
};

VBOData bunnyVBO;
VBOData cubeVBO;
VBOData quadVBO;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;


/*-----------------------------player state-------------------------------------*/
bool isAlive = true;
float xDisplacement = 0;
float zDisplacement = 0;
int score = 0;
float speed = 0.025f;
bool isQubeYellow[3];



void shuffleCubeColors() {
    isQubeYellow[0] = false;
    isQubeYellow[1] = false;
    isQubeYellow[2] = false;

    int index = rand() % 3;
    cout << "random index is: " << index << endl;
    isQubeYellow[index] = true;
}

bool ParseObj(const string& fileName, ModelData& model)
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            stringstream str(curLine);
            GLfloat c1, c2, c3;
            GLuint index[9];
            string tmp;

            if (curLine.length() >= 2)
            {
                if (curLine[0] == '#') // comment
                {
                    continue;
                }
                else if (curLine[0] == 'v')
                {
                    if (curLine[1] == 't') // texture
                    {
                        str >> tmp; // consume "vt"
                        str >> c1 >> c2;
                        model.textures.push_back(Texture(c1, c2));
                    }
                    else if (curLine[1] == 'n') // normal
                    {
                        str >> tmp; // consume "vn"
                        str >> c1 >> c2 >> c3;
                        model.normals.push_back(Normal(c1, c2, c3));
                    }
                    else // vertex
                    {
                        str >> tmp; // consume "v"
                        str >> c1 >> c2 >> c3;
                        model.vertices.push_back(Vertex(c1, c2, c3));
                    }
                }
                else if (curLine[0] == 'f') // face
                {
                    str >> tmp; // consume "f"
					char c;
					int vIndex[3],  nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0]; 
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1]; 
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2]; 

					assert(vIndex[0] == nIndex[0] &&
						   vIndex[1] == nIndex[1] &&
						   vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

                    model.faces.push_back(Face(vIndex, tIndex, nIndex));
                }
                else
                {
                    cout << "Ignoring unidentified line in obj file: " << curLine << endl;
                }
            }

            //data += curLine;
            if (!myfile.eof())
            {
                //data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x, 
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x, 
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x, 
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}
	*/

	assert(model.vertices.size() == model.normals.size());

    return true;
}

bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

void createVS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

    glAttachShader(program, vs);
}

void createFS(GLuint& program, const string& filename)
{
    string shaderSource;

    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

    glAttachShader(program, fs);
}

void initShaders()
{
    gProgram[0] = glCreateProgram(); //bunny
    gProgram[1] = glCreateProgram(); //quad
    gProgram[2] = glCreateProgram(); //cubes
    gProgram[3] = glCreateProgram(); //text

    createVS(gProgram[0], "vert0.glsl"); //bunny
    createFS(gProgram[0], "frag0.glsl");

    createVS(gProgram[1], "vert1.glsl"); //quad
    createFS(gProgram[1], "frag1.glsl");

    createVS(gProgram[2], "vert2.glsl"); //cubes
    createFS(gProgram[2], "frag2.glsl");

    createVS(gProgram[3], "vert_text.glsl");
    createFS(gProgram[3], "frag_text.glsl");

    glBindAttribLocation(gProgram[0], 0, "inVertex");
    glBindAttribLocation(gProgram[0], 1, "inNormal");
    glBindAttribLocation(gProgram[1], 0, "inVertex");
    glBindAttribLocation(gProgram[1], 1, "inNormal");
    glBindAttribLocation(gProgram[2], 0, "inVertex");
    glBindAttribLocation(gProgram[2], 1, "inNormal");

    glBindAttribLocation(gProgram[3], 2, "vertex");

    glLinkProgram(gProgram[0]);
    glLinkProgram(gProgram[1]);
    glLinkProgram(gProgram[2]);
    glLinkProgram(gProgram[3]);
    glUseProgram(gProgram[0]);

    gIntensityLoc = glGetUniformLocation(gProgram[0], "intensity");
    cout << "gIntensityLoc = " << gIntensityLoc << endl;
    glUniform1f(gIntensityLoc, gIntensity);
}

void initVBO(ModelData& model, VBOData& currentVBO)
{
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &currentVBO.vertexAttribBuffer);
    glGenBuffers(1, &currentVBO.indexBuffer);

    assert(currentVBO.vertexAttribBuffer > 0 && currentVBO.indexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, currentVBO.vertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentVBO.indexBuffer);

    currentVBO.vertexDataSizeInBytes = model.vertices.size() * 3 * sizeof(GLfloat);
    currentVBO.normalDataSizeInBytes = model.normals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = model.faces.size() * 3 * sizeof(GLuint);
    GLfloat* vertexData = new GLfloat [model.vertices.size() * 3];
    GLfloat* normalData = new GLfloat [model.normals.size() * 3];
    GLuint* indexData = new GLuint [model.faces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < model.vertices.size(); ++i)
    {
        vertexData[3*i] = model.vertices[i].x;
        vertexData[3*i+1] = model.vertices[i].y;
        vertexData[3*i+2] = model.vertices[i].z;

        minX = std::min(minX, model.vertices[i].x);
        maxX = std::max(maxX, model.vertices[i].x);
        minY = std::min(minY, model.vertices[i].y);
        maxY = std::max(maxY, model.vertices[i].y);
        minZ = std::min(minZ, model.vertices[i].z);
        maxZ = std::max(maxZ, model.vertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < model.normals.size(); ++i)
    {
        normalData[3*i] = model.normals[i].x;
        normalData[3*i+1] = model.normals[i].y;
        normalData[3*i+2] = model.normals[i].z;
    }

    for (int i = 0; i < model.faces.size(); ++i)
    {
        indexData[3*i] = model.faces[i].vIndex[0];
        indexData[3*i+1] = model.faces[i].vIndex[1];
        indexData[3*i+2] = model.faces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, currentVBO.vertexDataSizeInBytes + currentVBO.normalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, currentVBO.vertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, currentVBO.vertexDataSizeInBytes, currentVBO.normalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    // done copying; can free now
    delete[] vertexData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(currentVBO.vertexDataSizeInBytes));

}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    //glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[3]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[3], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "C:/Users/BOSS/Desktop/20231/CENG477/ceng477-hw3/Bunny Run/times.ttf", 0, &face)) // in linux if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSerif-Italic.ttf", 0, &face))
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void init() 
{
	//ParseObj("armadillo.obj");
	ParseObj("bunny.obj", bunnyModel);
	ParseObj("cube.obj", cubeModel);
	ParseObj("quad.obj", quadModel);

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initFonts(gWidth, gHeight);
    initVBO(bunnyModel, bunnyVBO);
    initVBO(cubeModel, cubeVBO);
    initVBO(quadModel, quadVBO);

    shuffleCubeColors();
}

void drawModel(ModelData& model, VBOData& currentVBO)
{
	glBindBuffer(GL_ARRAY_BUFFER, currentVBO.vertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, currentVBO.indexBuffer);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(currentVBO.vertexDataSizeInBytes));

	glDrawElements(GL_TRIANGLES, model.faces.size() * 3, GL_UNSIGNED_INT, 0);
}

void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[3]);
    glUniform3f(glGetUniformLocation(gProgram[3], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	static float angle = 0;
    glm::vec3 textColor = glm::vec3(255, 255, 0);
    


    /*------------------------------------------bunny transformation and draw-------------------------------------------------*/

    glUseProgram(gProgram[0]);
	//glLoadIdentity();
	//glTranslatef(-2, 0, -10);
	//glRotatef(angle, 0, 1, 0);

    glm::mat4 T = glm::translate(glm::mat4(1.f), glm::vec3(0.f + xDisplacement, -1.5f, -1.2f));

    glm::mat4 S = glm::scale(glm::mat4(1.f), glm::vec3(0.15f, 0.25f, 0.1f));

    glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(-90.f), glm::vec3(0, 1, 0));
    glm::mat4 modelMat = T * S * R;
    glm::mat4 modelMatInv = glm::transpose(glm::inverse(modelMat));
    glm::mat4 perspMat = glm::perspective(glm::radians(120.0f), 1.f, 1.0f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[0], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    drawModel(bunnyModel, bunnyVBO);

    /*------------------------------------------quad transformation and draw-------------------------------------------------*/

    glUseProgram(gProgram[1]);
	//glLoadIdentity();
	//glTranslatef(2, 0, -10);
	//glRotatef(-angle, 0, 1, 0);

    T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -2.f, -10.f));

    S = glm::scale(glm::mat4(1.f), glm::vec3(4.f, 1.f, 100000.f));

    R = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0));
    modelMat = T * S * R;
    modelMatInv = glm::transpose(glm::inverse(modelMat));

    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[1], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    glUniform1f(glGetUniformLocation(gProgram[1], "offset"), zDisplacement *8);
        
    drawModel(quadModel, quadVBO);

    /*------------------------------------------cubes transformation and draw-------------------------------------------------*/
    
    glUseProgram(gProgram[2]);

    // place cubes on x axis and move them on z using displacement, if displacement is above some amount, make it reset to 0
    
    T = glm::translate(glm::mat4(1.f), glm::vec3(-0.25f, -0.1f, -10.f + zDisplacement));

    S = glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.2f, 0.05f));

    modelMat = T * S;
    modelMatInv = glm::transpose(glm::inverse(modelMat));

    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    if (isQubeYellow[0]) {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 0);
    }

    drawModel(cubeModel, cubeVBO);

    T = glm::translate(glm::mat4(1.f), glm::vec3(0.f, -0.1f, -10.f + zDisplacement));

    modelMat = T * S;
    modelMatInv = glm::transpose(glm::inverse(modelMat));

    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    if (isQubeYellow[1]) {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 0);
    }

    drawModel(cubeModel, cubeVBO);

    T = glm::translate(glm::mat4(1.f), glm::vec3(0.25f, -0.1f, -10.f + zDisplacement));

    modelMat = T * S;
    modelMatInv = glm::transpose(glm::inverse(modelMat));

    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMat"), 1, GL_FALSE, glm::value_ptr(modelMat));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "modelingMatInvTr"), 1, GL_FALSE, glm::value_ptr(modelMatInv));
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "perspectiveMat"), 1, GL_FALSE, glm::value_ptr(perspMat));

    if (isQubeYellow[2]) {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 1);
    }
    else {
        glUniform1i(glGetUniformLocation(gProgram[2], "isYellow"), 0);
    }

    drawModel(cubeModel, cubeVBO);
    

    /*------------------------------------------text transformation and draw-------------------------------------------------*/

    assert(glGetError() == GL_NO_ERROR);
    if (!isAlive) {
        textColor = glm::vec3(255, 0, 0);
    }

    renderText("Score: " + std::to_string(score), 0, 686, 1, textColor);

    assert(glGetError() == GL_NO_ERROR);

	angle += 0.5;
    if (isAlive) {
        zDisplacement += speed;
        score += speed;

        if(zDisplacement > 10){ //if qubes are behind the camera, reset their position and shuffle their colors
            zDisplacement = 0;
            shuffleCubeColors();
        }
    }
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    else if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        cout << "R pressed" << endl;
        xDisplacement = 0;
        zDisplacement = 0;
        score = 0;
        speed = 0.05f;
        //glUseProgram(gProgram[1]);
    }
    else if (key == GLFW_KEY_A && action == GLFW_PRESS)
    {
        cout << "A pressed" << endl;
        //glUseProgram(gProgram[0]);
        xDisplacement -= 0.75f;
        if (xDisplacement < -1.5f) {
            xDisplacement = -1.5f;
        }
    }
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        cout << "D pressed" << endl;
        xDisplacement += 0.75f;
        if (xDisplacement > 1.5f) {
            xDisplacement = 1.5f;
        }
    }
    else if (key == GLFW_KEY_K && action == GLFW_PRESS)
    {
        cout << "K pressed" << endl;
        isAlive = !isAlive;
    }
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(gWidth, gHeight, "Simple Example", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy_s(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat_s(rendererInfo, " - ");
    strcat_s(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);

    reshape(window, gWidth, gHeight); // need to call this once ourselves
    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

