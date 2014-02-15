#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <cmath>
#include <new>
#include "glew/glew.h"
#define GLFW_DLL
#include "glfw/glfw3.h"
#include "assimp/Importer.hpp"      // C++ importer interface
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#define GLM_FORCE_RADIANS
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//#include "soil/SOIL.h"
#include "stb_image.c"
#include "textures.h"
#include "EasyBMP.h"

// Globals.
const GLfloat pi = 3.14159265358979323846f;

GLboolean resizeFlag = false;

GLint width = 1024;
GLint height = 1024;

GLfloat fov = 1.1693706f;

glm::mat4 & projection = glm::mat4();

aiTextureType textureType[4] = {aiTextureType_DIFFUSE, aiTextureType_HEIGHT, aiTextureType_SPECULAR, aiTextureType_OPACITY};

texturesList globalList;

GLdouble totalTime = 0.0;
GLdouble currentTime = 0.0;
GLdouble startTime = 0.0;
GLdouble endTime = 0.0;

bool framebufferToBMP = false;


// Callback to update framebuffer size and projection matrix from new window size.
void windowSizeCallback(GLFWwindow * window, int newWidth, int newHeight)
{
	width = newWidth;
	height = newHeight;
	glViewport(0, 0, width, height);
	projection = glm::perspectiveFov(fov, (GLfloat) width, (GLfloat) height, 0.1f, 10000.0f);
	resizeFlag = true;
}



// Return a readable description of GL error enums.
// Pulled from the official API.
std::string glGetErrorReadable()
{
	GLenum errorCode = glGetError();
	if(errorCode == GL_NO_ERROR)
	{
		return "No Error\n";
	}
	else if(errorCode == GL_INVALID_ENUM)
	{
		return "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.\n";
	}
	else if(errorCode == GL_INVALID_VALUE)
	{
		return "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.\n";
	}
	else if(errorCode == GL_INVALID_OPERATION)
	{
		return "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n";
	}
	else if(errorCode == GL_INVALID_FRAMEBUFFER_OPERATION)
	{
		return "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.\n";
	}
	else if(errorCode == GL_OUT_OF_MEMORY)
	{
		return "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n";
	}
	else if(errorCode == GL_STACK_UNDERFLOW)
	{
		return "An attempt has been made to perform an operation that would cause an internal stack to underflow.\n";
	}
	else if(errorCode == GL_STACK_OVERFLOW)
	{
		return "An attempt has been made to perform an operation that would cause an internal stack to overflow.\n";
	}
	else
	{
		return "Unknown error code\n";
	}
}



// Call glGetError severaltimes to clear the error log.
void glClearError(GLuint iterations)
{
	for(GLuint i = 0; i < iterations; i++)
	{
		glGetError();
	}
}



// Save unsigned char * to a .bmp file.
void saveBMP(const char * filename, unsigned char * data, unsigned int size, GLuint depth)
{
	glm::u8vec4 *dataVector = (glm::u8vec4*) ::operator new(sizeof(glm::u8vec4) * size * size);
	for(GLuint index = 0; index < size * size; index++)
	{
		dataVector[index].r = data[(depth * index)];
		if(depth > 1)
		{
			dataVector[index].g = data[(depth * index) + 1];
			dataVector[index].b = data[(depth * index) + 2];
			if(depth == 4) dataVector[index].a = data[(depth * index) + 3];
		}
	}
	BMP imageOut;
	imageOut.SetSize(size, size);
	imageOut.SetBitDepth(depth == 4 ? 32 : 24);
	GLuint index = 0;
	for(GLuint y = 0; y < size; y++)
	{
		for(GLuint x = 0; x < size; x++)
		{
			if(depth == 1)
			{
				imageOut(x, y)->Red = dataVector[index].r;
				imageOut(x, y)->Green = dataVector[index].r;
				imageOut(x, y)->Blue = dataVector[index].r;
			}
			else
			{
				imageOut(x, y)->Red = dataVector[index].r;
				imageOut(x, y)->Green = dataVector[index].g;
				imageOut(x, y)->Blue = dataVector[index].b;
				if(depth == 4) imageOut(x, y)->Alpha = dataVector[index].a;
			}
			index++;
		}
	}
	imageOut.WriteToFile(filename);
	delete(dataVector);
}

// Save unsigned char * to a .bmp file.
void saveBMP(char * filename, unsigned char * data, unsigned int width, unsigned int height, GLuint depth)
{
	glm::u8vec4 *dataVector = (glm::u8vec4*) ::operator new(sizeof(glm::u8vec4) * width * height);
	for(GLuint index = 0; index < width * height; index++)
	{
		dataVector[index].r = data[(depth * index)];
		if(depth > 1)
		{
			dataVector[index].g = data[(depth * index) + 1];
			dataVector[index].b = data[(depth * index) + 2];
			if(depth == 4) dataVector[index].a = data[(depth * index) + 3];
		}
	}
	BMP imageOut;
	imageOut.SetSize(width, height);
	imageOut.SetBitDepth(depth == 4 ? 32 : 24);
	GLuint index = 0;
	for(GLuint y = 0; y < height; y++)
	{
		for(GLuint x = 0; x < width; x++)
		{
			if(depth == 1)
			{
				imageOut(x, y)->Red = dataVector[index].r;
				imageOut(x, y)->Green = dataVector[index].g;
				imageOut(x, y)->Blue = dataVector[index].b;
			}
			else
			{
				imageOut(x, y)->Red = dataVector[index].r;
				imageOut(x, y)->Green = dataVector[index].g;
				imageOut(x, y)->Blue = dataVector[index].b;
				if(depth == 4) imageOut(x, y)->Alpha = dataVector[index].a;
			}
			index++;
		}
	}
	imageOut.WriteToFile(filename);
	delete(dataVector);
}



void calculateCoverage(unsigned char * coverageTexture, unsigned char * layer, GLuint voxelResolution, GLuint voxelPrecision)
{
	const GLuint fullRowSize = voxelResolution * voxelPrecision * 3;
	const GLuint scaledRowSize = voxelResolution * voxelPrecision * voxelPrecision * 3;
	GLuint relativeIndex, scaledCoverageIndex;
	GLuint * sumLayer = (GLuint *) ::operator new(sizeof(GLuint) * voxelResolution * voxelResolution);
	for(GLuint index = 0; index < voxelResolution * voxelResolution; index++)
	{
		sumLayer[index] = 0;
	}
	for(GLuint coverageIndex = 0; coverageIndex < voxelResolution * voxelResolution; coverageIndex++)
	{
		for(GLuint rawIndexY = 0; rawIndexY < voxelPrecision; rawIndexY++)
		{
			for(GLuint rawIndexX = 0; rawIndexX < voxelPrecision; rawIndexX++)
			{
				//GLuint value = (coverageIndex / voxelResolution);
				scaledCoverageIndex = (((coverageIndex / voxelResolution) * scaledRowSize) + ((coverageIndex % voxelResolution) * voxelPrecision * 3));
				relativeIndex =  ( (scaledCoverageIndex + (rawIndexX * 3)) + (rawIndexY * fullRowSize) );
				sumLayer[coverageIndex] += coverageTexture[relativeIndex];
			}
		}
	}
	for(GLuint index = 0; index < voxelResolution * voxelResolution; index++)
	{
		layer[index] = (unsigned char) ((GLfloat) sumLayer[index] / (GLfloat) (voxelPrecision * voxelPrecision));
	}
	delete(sumLayer);
}

void copyCoverage(unsigned char * coverageTexture, unsigned char * layer, GLuint voxelResolution)
{
	for(GLuint index = 0; index < voxelResolution * voxelResolution; index++)
	{
		layer[index] = coverageTexture[index * 3];
	}
}



//Shader:
//file - file containing shader code
//string - std::string contents of file
//cstr - character array of string
//object - GL id for shader object
//prepare() - open and save shader code
//	returns: size of shader code
class shader
{
	std::ifstream file;
	public:
		std::string string;
		const char* cstr;
		GLuint object;
		GLuint64 prepare(char* filename)
		{
			file.open(filename);
			getline(file, string, '\0');
			cstr = string.c_str();
			return (GLuint64) string.size();
		}
};

//Texture:
class texture
{
	public:
		GLuint textureID;
		GLint width, height, depth;
		unsigned char* imageData;
		aiString modelRelativePath;
		aiString relativePath;
		const char* cstrPath;
		aiTextureMapping mapping;
		GLuint uvindex;
		GLfloat blend;
		aiTextureOp operation;
		aiTextureMapMode mapmode;
		void prepare(GLuint type)
		{
			GLuint ID = globalList.find(modelRelativePath);
			if(ID == 0)
			{
				relativePath = aiString("crytek-sponza\\");
				relativePath.Append(modelRelativePath.C_Str());
				cstrPath = relativePath.C_Str();
				//GLuint flags = SOIL_FLAG_MIPMAPS | ((mapping == aiTextureMapMode_Wrap) ? SOIL_FLAG_TEXTURE_REPEATS : 0);
				imageData = stbi_load(cstrPath, &width, &height, &depth, 0);
				glGenTextures(1, &textureID);
				globalList.add(modelRelativePath, textureID);
				glActiveTexture(textureSlots[type]);
				glBindTexture(GL_TEXTURE_2D, textureID);
				if(depth == 4)
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
				}
				else
				{
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
				}
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16);
				glBindTexture(GL_TEXTURE_2D, 0);
			}
			else
			{
				textureID = ID;
			}
		}
};

//Mesh:
//vao - id for the Vertex Array Object
//ibo - id for the Index Buffer Object
//numberIndices - number of indices in indices
//indicies - pointer to an array of indices
//vbo - array of ids for the Vertex Buffer Objects
//numberVertices - number of vertices in vertexVector
//vertexVector - array of glm::vec3 containing indexed vertices
//normalVector - array of glm::vec3 containing indexed normals
//uv - array of glm::vec2 containing indexed uvs
//tangentVector - array of glm::vec3 containing indexed tangents
//biTangentVector - array of glm::vec3 containing indexed biTangents
//rawVertices - array of glm::vec3 containing unindexed vertices

struct mesh
{
	GLuint vao;

	GLuint ibo;
	GLuint numberIndices;
	GLuint* indices;

	GLuint vbo[5];
	GLuint numberVertices;
	glm::vec3* vertexVector;
	glm::vec3* normalVector;
	glm::vec2* uv;
	glm::vec3* tangentVector;
	glm::vec3* bitangentVector;

	GLuint numberRawVertices;
	glm::vec3* rawVertices;

	aiMaterial* material;
	texture textures[4];
};



class layers
{
	public:
		GLuint numberLayers;
		unsigned char** x;
		unsigned char** y;
		unsigned char** z;
		//glm::vec3 * & xVector;
		//glm::vec3 * & yVector;
		//glm::vec3 * & zVector;
		void prepare(GLuint voxelResolution, GLuint layerResolution, GLuint depth)
		{
			numberLayers = voxelResolution;
			x = (unsigned char**) ::operator new(sizeof(unsigned char*) * voxelResolution);
			for(GLuint layerIndex = 0; layerIndex < voxelResolution; layerIndex++)
			{
				x[layerIndex] = (unsigned char*) ::operator new(sizeof(unsigned char) * layerResolution * layerResolution * depth);
			}
			y = (unsigned char**) ::operator new(sizeof(unsigned char*) * voxelResolution);
			for(GLuint layerIndex = 0; layerIndex < voxelResolution; layerIndex++)
			{
				y[layerIndex] = (unsigned char*) ::operator new(sizeof(unsigned char) * layerResolution * layerResolution * depth);
			}
			z = (unsigned char**) ::operator new(sizeof(unsigned char*) * voxelResolution);
			for(GLuint layerIndex = 0; layerIndex < voxelResolution; layerIndex++)
			{
				z[layerIndex] = (unsigned char*) ::operator new(sizeof(unsigned char) * layerResolution * layerResolution * depth);
			}
		}
};



//TODO: If first argument exists, use as model file.
int WinMain(int argc, char** argv)
{
	using namespace std;
	// Default model courtesy of crytek and tweaks by McGuire.
	// graphics.cs.williams.edu/data/meshes.xml
	char* file = "crytek-sponza\\sponza.obj";



	// Log file.
	std::ofstream logFile;
	logFile.open("log.txt");



	logFile << "Averaged Voxel Global Illumination\n";

	// Log file visual seperator.
	logFile << "\nSetup\n-----------\n\n";

	// Initialize GLFW.
	if(!GL_TRUE == glfwInit())
	{
		logFile << "GLFW Failed to Initialize\n";
		return -1;
	}

	// glfw hints for window creation.
	int major = 4; 
	int minor = 3;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_SAMPLES, 16);
	glfwWindowHint(GLFW_AUX_BUFFERS, 0);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Monitor object for fullscreen use.
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	// Create the window
	GLFWwindow *window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);

	// Set the callback for window resizes.
	glfwSetWindowSizeCallback(window, windowSizeCallback);

	// Clear glfw errors from the glLog.
	glClearError(5);
	
	// Make the window the current context.
	glfwMakeContextCurrent(window);

	// Initialize GLEW.
	glewExperimental = GL_TRUE;
	GLint glewInitError = glewInit();
	if(glewInitError != GLEW_OK)
	{
		logFile << "GLEW Failed to Initialize: " << glewInitError << "\n";
		return -1;
	}
	
	// Clear glew errors from the glLog.
	glClearError(5);

	// Get version info.
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	logFile << "Renderer: " << renderer << "\n";
	logFile << "OpenGL version supported " << version << "\n";

	// Enable depth testing GL_LESS and backface culling GL_CCW.
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	//glDepthFunc(GL_LESS);
	//glEnable (GL_CULL_FACE);
	//glCullFace (GL_BACK);
	//glFrontFace (GL_CCW);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDisable(GL_MULTISAMPLE);
	//glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);


	// Load scene.
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	// Log file visual seperator.
	logFile << "\nShader Compilation\n-----------\n\n";

	// Prepare shaders.
	shader mainVertex, mainFragment;
	mainVertex.prepare("shaders\\mainVertex.glsl");
	mainFragment.prepare("shaders\\mainFragment.glsl");

	// Compile shaders.
	mainVertex.object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(mainVertex.object, 1, &mainVertex.cstr, NULL);
	glCompileShader(mainVertex.object);
	mainFragment.object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(mainFragment.object, 1, &mainFragment.cstr, NULL);
	glCompileShader(mainFragment.object);

	// Print shaders to logfile.
	//logFile << "\n\nVertex Shader:\n" << mainVertex.cstr << "\n\nFragment Shader:\n" << mainFragment.cstr << "\n\n";

	// Log shader compile errors.
	GLsizei length = 0;
	GLchar mainVertexLog[1000];
	GLchar mainFragmentLog[1000];
	glGetShaderInfoLog(mainVertex.object, 1000, &length, mainVertexLog);
	glGetShaderInfoLog(mainFragment.object, 1000, &length, mainFragmentLog);
	logFile << "\nVertex Shader Log:\n" << mainVertexLog << "\n\nFragment Shader Log:\n" << mainFragmentLog << "\n\n";

	// Attach shaders to program.
	GLuint mainShaderProgram = glCreateProgram();
	glAttachShader(mainShaderProgram, mainVertex.object);
	glAttachShader(mainShaderProgram, mainFragment.object);

	// Link shader program.
	glLinkProgram(mainShaderProgram);

	// Map mvp uniform from shaders.
	GLint mainMVPUniform;
	mainMVPUniform = glGetUniformLocation(mainShaderProgram, "mvp");

	GLint diffuseTextureUniform;
	diffuseTextureUniform = glGetUniformLocation(mainShaderProgram, "diffuseTexture");
	GLint normalTextureUniform;
	normalTextureUniform = glGetUniformLocation(mainShaderProgram, "normalTexture");
	GLint opacityTextureUniform;
	opacityTextureUniform = glGetUniformLocation(mainShaderProgram, "opacityTexture");

	// Log error from shader compiling and linking.
	logFile << glGetErrorReadable().c_str();

	// Log file visual seperator.
	logFile << "\nImport Model\n-----------\n\n";

	GLuint totalIndices = 0;

	// Array of meshes in scene object.
	mesh* meshes = (mesh*) ::operator new(sizeof(mesh) * scene->mNumMeshes);
	GLuint numberMeshes = scene->mNumMeshes;

	// Process all meshes.
	for(GLuint meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
	{
		// Grab the current mesh.
		const aiMesh* currentMesh = scene->mMeshes[meshIndex];

		// Count up faces that have only 3 indices.
		meshes[meshIndex].numberIndices = 0;
		for(GLuint faceIndex = 0; faceIndex < currentMesh->mNumFaces; faceIndex++)
		{
			const aiFace* currentFace = &currentMesh->mFaces[faceIndex];
			if(currentFace->mNumIndices == 3)
			{
				meshes[meshIndex].numberIndices += 3;
			}
		}

		// Allocate space for indices.
		meshes[meshIndex].indices = (GLuint*) ::operator new(sizeof(GLuint) * meshes[meshIndex].numberIndices);

		// Copy indices into array for buffering.
		for(GLuint faceIndex = 0; faceIndex < currentMesh->mNumFaces; faceIndex++)
		{
			const aiFace* currentFace = &currentMesh->mFaces[faceIndex];
			if(currentFace->mNumIndices == 3)
			{
				for(GLuint indicesIndex = 0; indicesIndex < 3; indicesIndex++)
				{
					meshes[meshIndex].indices[(faceIndex * 3) + indicesIndex] = currentFace->mIndices[indicesIndex];
				}
			}
		}

		// Save the number of vertices in the mesh.
		meshes[meshIndex].numberVertices = currentMesh->mNumVertices;

		//Allocate space for vertices, normals, uvs, tangents, and bitangents.
		meshes[meshIndex].vertexVector = (glm::vec3*) ::operator new(sizeof(glm::vec3) * currentMesh->mNumVertices);
		meshes[meshIndex].normalVector = (glm::vec3*) ::operator new(sizeof(glm::vec3) * currentMesh->mNumVertices);
		meshes[meshIndex].uv = (glm::vec2*) ::operator new(sizeof(glm::vec2) * currentMesh->mNumVertices);
		meshes[meshIndex].tangentVector = (glm::vec3*) ::operator new(sizeof(glm::vec3) * currentMesh->mNumVertices);
		meshes[meshIndex].bitangentVector = (glm::vec3*) ::operator new(sizeof(glm::vec3) * currentMesh->mNumVertices);

		for(GLuint vertexIndex = 0; vertexIndex < currentMesh->mNumVertices; vertexIndex++)
		{
			// Add vertex.
			const aiVector3D* currentVertex = &currentMesh->mVertices[vertexIndex];
			meshes[meshIndex].vertexVector[vertexIndex].x = (GLfloat) currentVertex->x;
			meshes[meshIndex].vertexVector[vertexIndex].y = (GLfloat) currentVertex->y;
			meshes[meshIndex].vertexVector[vertexIndex].z = (GLfloat) currentVertex->z;
			// Add normal.
			const aiVector3D* currentNormal = &currentMesh->mNormals[vertexIndex];
			meshes[meshIndex].normalVector[vertexIndex].x = (GLfloat) currentNormal->x;
			meshes[meshIndex].normalVector[vertexIndex].y = (GLfloat) currentNormal->y;
			meshes[meshIndex].normalVector[vertexIndex].z = (GLfloat) currentNormal->z;
			// Add uv.
			const aiVector3D* currentUV = &currentMesh->mTextureCoords[0][vertexIndex];
			meshes[meshIndex].uv[vertexIndex].s = (GLfloat) currentUV->x;
			meshes[meshIndex].uv[vertexIndex].t = (GLfloat) currentUV->y;
			// Add tangent.
			const aiVector3D* currentTangent = &currentMesh->mTangents[vertexIndex];
			meshes[meshIndex].tangentVector[vertexIndex].x = (GLfloat) currentTangent->x;
			meshes[meshIndex].tangentVector[vertexIndex].y = (GLfloat) currentTangent->y;
			meshes[meshIndex].tangentVector[vertexIndex].z = (GLfloat) currentTangent->z;
			// Add bitangent.
			const aiVector3D* currentBitangent = &currentMesh->mBitangents[vertexIndex];
			meshes[meshIndex].bitangentVector[vertexIndex].x = (GLfloat) currentBitangent->x;
			meshes[meshIndex].bitangentVector[vertexIndex].y = (GLfloat) currentBitangent->y;
			meshes[meshIndex].bitangentVector[vertexIndex].z = (GLfloat) currentBitangent->z;
		}

		startTime = glfwGetTime();

		//Compute raw vertices.
		meshes[meshIndex].numberRawVertices = meshes[meshIndex].numberIndices;
		meshes[meshIndex].rawVertices = (glm::vec3*) ::operator new(sizeof(glm::vec3) * meshes[meshIndex].numberIndices);
		for(GLuint index = 0; index < meshes[meshIndex].numberIndices; index++)
		{
			meshes[meshIndex].rawVertices[index] = glm::vec3(meshes[meshIndex].vertexVector[meshes[meshIndex].indices[index]]);
		}

		endTime = glfwGetTime();
		currentTime = endTime - startTime;
		totalTime += currentTime;
		totalIndices += meshes[meshIndex].numberIndices;

		//logFile << "Time(" << meshes[meshIndex].numberIndices << "): " << currentTime << "\n";

		aiReturn dummy;

		meshes[meshIndex].material = scene->mMaterials[currentMesh->mMaterialIndex];
		aiMaterial* currentMaterial = meshes[meshIndex].material;

		GLuint countMaterial;

		// Load textures
		for(GLuint type = 0; type < 4; type++)
		{
			countMaterial = currentMaterial->GetTextureCount(textureType[type]);
			if(countMaterial == 1)
			{
				dummy = currentMaterial->GetTexture(textureType[type],
													0,
													&meshes[meshIndex].textures[type].modelRelativePath,
													&meshes[meshIndex].textures[type].mapping,
													&meshes[meshIndex].textures[type].uvindex,
													&meshes[meshIndex].textures[type].blend,
													&meshes[meshIndex].textures[type].operation,
													&meshes[meshIndex].textures[type].mapmode);
				meshes[meshIndex].textures[type].prepare(type);
				//logFile << glGetErrorReadable().c_str();
			}
			else
			{
				meshes[meshIndex].textures[type].textureID = 0;
			}
		}


		// Generate Vertex Array Object.
		glGenVertexArrays(1, &meshes[meshIndex].vao);
		// Bind the meshes vao to store vbo and ibo attributes for easy processing.
		glBindVertexArray(meshes[meshIndex].vao);

		// Generate 5 Vertex Buffer Objects.
		glGenBuffers(5, meshes[meshIndex].vbo);

		//Buffer vertices to vbo[0].
		glBindBuffer(GL_ARRAY_BUFFER, meshes[meshIndex].vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, (3 * currentMesh->mNumVertices * sizeof(GLfloat)), meshes[meshIndex].vertexVector, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//Buffer normals to vbo[1].
		glBindBuffer(GL_ARRAY_BUFFER, meshes[meshIndex].vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, (3 * currentMesh->mNumVertices * sizeof(GLfloat)), meshes[meshIndex].normalVector, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//Buffer uvs to vbo[2].
		glBindBuffer(GL_ARRAY_BUFFER, meshes[meshIndex].vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, (2 * currentMesh->mNumVertices * sizeof(GLfloat)), meshes[meshIndex].uv, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
		//Buffer tangents to vbo[3].
		glBindBuffer(GL_ARRAY_BUFFER, meshes[meshIndex].vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, (3 * currentMesh->mNumVertices * sizeof(GLfloat)), meshes[meshIndex].tangentVector, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, 0);
		//Buffer bitangents to vbo[4].
		glBindBuffer(GL_ARRAY_BUFFER, meshes[meshIndex].vbo[4]);
		glBufferData(GL_ARRAY_BUFFER, (3 * currentMesh->mNumVertices * sizeof(GLfloat)), meshes[meshIndex].bitangentVector, GL_STATIC_DRAW);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, 0);

		// Enable the Vertex Attributes for the 5 vbos.
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glEnableVertexAttribArray(4);

		// Build the Index Buffer Object
		glGenBuffers(1, &meshes[meshIndex].ibo);

		// Buffer indices to ibo.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshes[meshIndex].ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (meshes[meshIndex].numberIndices * sizeof(GLuint)), meshes[meshIndex].indices, GL_STATIC_DRAW);

		// Unbind the VAO
		glBindVertexArray(0);
	}


	logFile << "Total Time(" << totalIndices << "): " << totalTime << "\n";

	logFile << "\nVoxel Occlusion Preperation\n-----------\n\n";


	//Generate the Occlusion 3D Texture

	glm::vec3 minimum, maximum;

	minimum = glm::vec3(meshes[0].rawVertices[0]);
	maximum = glm::vec3(meshes[0].rawVertices[0]);

	//Find the bounding box of the model
	for(GLuint meshIndex = 0; meshIndex < numberMeshes; meshIndex++)
	{
		for(GLuint vertexIndex = 0; vertexIndex < meshes[meshIndex].numberRawVertices; vertexIndex++)
		{
			const glm::vec3* currentVertex = &meshes[meshIndex].rawVertices[vertexIndex];
			minimum.x = minimum.x <= currentVertex->x ? minimum.x : currentVertex->x;
			minimum.y = minimum.y <= currentVertex->y ? minimum.y : currentVertex->y;
			minimum.z = minimum.z <= currentVertex->z ? minimum.z : currentVertex->z;
			maximum.x = maximum.x >= currentVertex->x ? maximum.x : currentVertex->x;
			maximum.y = maximum.y >= currentVertex->y ? maximum.y : currentVertex->y;
			maximum.z = maximum.z >= currentVertex->z ? maximum.z : currentVertex->z;
		}

	}

	logFile << "Minimum\nX: " << minimum.x << "\nY: " << minimum.y << "\nZ: " << minimum.z << "\n";
	logFile << "Maximum\nX: " << maximum.x << "\nY: " << maximum.y << "\nZ: " << maximum.z << "\n";

	// Log file visual seperator.
	logFile << "\nVoxel Shader Compilation\n-----------\n\n";

	// Prepare shaders.
	shader voxelVertex, voxelFragment;
	voxelVertex.prepare("shaders\\voxelVertex.glsl");
	voxelFragment.prepare("shaders\\voxelFragment.glsl");

	// Compile shaders.
	voxelVertex.object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(voxelVertex.object, 1, &voxelVertex.cstr, NULL);
	glCompileShader(voxelVertex.object);
	voxelFragment.object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(voxelFragment.object, 1, &voxelFragment.cstr, NULL);
	glCompileShader(voxelFragment.object);

	// Print shaders to logfile.
	//logFile << "\n\nVertex Shader:\n" << mainVertex.cstr << "\n\nFragment Shader:\n" << mainFragment.cstr << "\n\n";

	// Log shader compile errors.
	GLchar voxelVertexLog[1000];
	GLchar voxelFragmentLog[1000];
	glGetShaderInfoLog(voxelVertex.object, 1000, &length, voxelVertexLog);
	glGetShaderInfoLog(voxelFragment.object, 1000, &length, voxelFragmentLog);
	logFile << "\nVertex Shader Log:\n" << voxelVertexLog << "\n\nFragment Shader Log:\n" << voxelFragmentLog << "\n\n";

	// Attach shaders to program.
	GLuint voxelShaderProgram = glCreateProgram();
	glAttachShader(voxelShaderProgram, voxelVertex.object);
	glAttachShader(voxelShaderProgram, voxelFragment.object);

	// Link shader program.
	glLinkProgram(voxelShaderProgram);

	// Map mvp uniform from shaders.
	GLint voxelMVPUniform;
	voxelMVPUniform = glGetUniformLocation(voxelShaderProgram, "mvp");

	glUseProgram(voxelShaderProgram);

	// Log file visual seperator.
	logFile << "\nVoxel Occlusion Generation\n-----------\n\n";


	GLuint voxelResolution = 128;
	GLuint voxelPrecision = 4;
	GLuint voxelSubPrecision = 8;
	GLfloat voxelStep = 1.0f / voxelResolution;
	GLfloat overlap = voxelStep / 8.0f;
	bool useLinearResampling = false;

	GLuint layerResolution = voxelResolution * voxelPrecision;

	layers emptyLayer;
	emptyLayer.prepare(1, voxelResolution, 3);

	//Generate an empty layer.
	for(GLuint index = 0; index < voxelResolution * voxelResolution; index++)
	{
		emptyLayer.x[0][index] = 0x00;
		emptyLayer.y[0][index] = 0x00;
		emptyLayer.z[0][index] = 0x00;
	}

	glViewport(0, 0, layerResolution, layerResolution);

	//Allocate 2D image storage and 3D compositing storage.
	layers processedLayer;
	//rawLayer.prepare(voxelResolution, layerResolution, 3);
	processedLayer.prepare(voxelResolution, voxelResolution, 1);

	//Compute normalization scale.
	glm::vec3 size = maximum - minimum;
	GLfloat modelSize = size.x >= size.y ? size.x : size.y;
	modelSize = modelSize >= size.z ? modelSize : size.z;
	GLfloat modelScale = 1.0f / modelSize;

	logFile << "Model Size\nX: " << modelSize << "\n";
	logFile << "Model Scale\nX: " << modelScale << "\n";

	glm::vec3 center = glm::vec3();

	center = maximum + minimum;

	logFile << "Center\nX: " << center.x << "\nY: " << center.y << "\nZ: " << center.z << "\n";

	center = center / modelSize;
	center = center / 2.0f;

	logFile << "Scaled Center\nX: " << center.x << "\nY: " << center.y << "\nZ: " << center.z << "\n";

	glm::mat4 voxelModelScale = glm::scale(glm::mat4(1.0f), glm::vec3(modelScale));
	glm::mat4 voxelModelTranslate = glm::translate(glm::mat4(1.0f), -center);

	glm::mat4 voxelViewX, voxelViewY, voxelViewZ;
	//
	//glm::mat4 & voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, -2.0f, 2.0f);
	glm::mat4 voxelProjection;
	
	voxelViewX = glm::lookAt(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	voxelViewY = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	voxelViewZ = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));

	glm::mat4 voxelModel = voxelModelTranslate * voxelModelScale;

	glm::mat4 voxelMVP = voxelProjection * voxelViewY * voxelModel;

	glClearError(15);

	GLuint multisampleFramebuffer, coverageFramebuffer;
	
	GLuint multisampleFramebufferTexture, coverageFramebufferTexture;

	glGenFramebuffers(1, &multisampleFramebuffer);  //BREAKS NSIGHT
	glBindFramebuffer(GL_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, layerResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, layerResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_SAMPLES, voxelSubPrecision);  //BREAKS NSIGHT
	glGenTextures(1, &multisampleFramebufferTexture);  //BREAKS NSIGHT
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleFramebufferTexture);  //BREAKS NSIGHT
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, voxelSubPrecision, GL_RGB, layerResolution, layerResolution, 0);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleFramebufferTexture, 0);  //BREAKS NSIGHT

	if(useLinearResampling)
	{
		glGenFramebuffers(1, &coverageFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, voxelResolution);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, voxelResolution);
		glGenTextures(1, &coverageFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, voxelResolution, voxelResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, coverageFramebufferTexture, 0);
		//logFile << glGetErrorReadable().c_str();
	}
	else
	{
		glGenFramebuffers(1, &coverageFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, layerResolution);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, layerResolution);
		glGenTextures(1, &coverageFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, layerResolution, layerResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, coverageFramebufferTexture, 0);
		//logFile << glGetErrorReadable().c_str();
	}

	totalTime = 0;

	//GLfloat layerScalar, modelScalarMin, modelScalarMax;

	unsigned char * coverageTexture = (unsigned char *) ::operator new(sizeof(unsigned char) * layerResolution * layerResolution * 3);

	//Compute X layers.
	for(GLuint layerIndex = 0; layerIndex < processedLayer.numberLayers; layerIndex++)
	{
		//layerScalar = ((((GLfloat) layerIndex + 1.0f) / (GLfloat) rawLayer.numberLayers) - 0.5f);
		//modelScalarMin = ((minimum.y * modelScale) + -center.y);
		//modelScalarMax = ((maximum.y * modelScale) + -center.y);
		//logFile << "Layer\nLayer Scalar: " << layerScalar << "\nModel Scalar Min: " << modelScalarMin << "\nModel Scalar Max: " << modelScalarMax << "\n";
		if( ((( (GLfloat) layerIndex + 2.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) < ((minimum.x * modelScale) + -center.x) )
		{
			//delete(processedLayer.x[layerIndex]);
			processedLayer.x[layerIndex] = emptyLayer.x[0];
		}
		else if( ((( (GLfloat) layerIndex + 1.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) > ((maximum.x * modelScale) + -center.x) )
		{
			//delete(processedLayer.x[layerIndex]);
			processedLayer.x[layerIndex] = emptyLayer.x[0];
		}
		else
		{
			startTime = glfwGetTime();
			voxelStep = (((GLfloat) layerIndex + 1.0f) / (GLfloat) voxelResolution) - (1.0f / voxelResolution);
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewX * voxelModel;

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
			//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);

			// Clear the screen.
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Enable shader_program in the state machine.
			glUseProgram(voxelShaderProgram);

			// Assign matrix uniform from shader to uniformvs.
			glUniformMatrix4fv(voxelMVPUniform, 1, GL_FALSE, glm::value_ptr(voxelMVP));


			// Iterate through the meshes.
			for(GLuint index = 0; index < scene->mNumMeshes; index++)
			{
				// Set vao as active VAO in the state machine.
				glBindVertexArray(meshes[index].vao);

				// Draw the current VAO using the bound IBO.
				glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
			}
			if(useLinearResampling)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				copyCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution);
			}
			else
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, layerResolution, layerResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				calculateCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution, voxelPrecision);
			}
			totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\x").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				saveBMP(string("layers\\px").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.x[layerIndex], voxelResolution, 1);
			}
		}
	}

	//Compute Y layers.
	for(GLuint layerIndex = 0; layerIndex < processedLayer.numberLayers; layerIndex++)
	{
		//layerScalar = ((((GLfloat) layerIndex + 1.0f) / (GLfloat) rawLayer.numberLayers) - 0.5f);
		//modelScalarMin = ((minimum.y * modelScale) + -center.y);
		//modelScalarMax = ((maximum.y * modelScale) + -center.y);
		//logFile << "Layer\nLayer Scalar: " << layerScalar << "\nModel Scalar Min: " << modelScalarMin << "\nModel Scalar Max: " << modelScalarMax << "\n";
		if( ((( (GLfloat) layerIndex + 2.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) < ((minimum.y * modelScale) + -center.y) )
		{
			delete(processedLayer.y[layerIndex]);
			processedLayer.y[layerIndex] = emptyLayer.y[0];
		}
		else if( ((( (GLfloat) layerIndex + 1.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) > ((maximum.y * modelScale) + -center.y) )
		{
			delete(processedLayer.y[layerIndex]);
			processedLayer.y[layerIndex] = emptyLayer.y[0];
		}
		else
		{
			startTime = glfwGetTime();
			voxelStep = (((GLfloat) layerIndex + 1.0f) / (GLfloat) voxelResolution) - (1.0f / voxelResolution);
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewY * voxelModel;

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
			//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);

			// Clear the screen.
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Enable shader_program in the state machine.
			glUseProgram(voxelShaderProgram);

			// Assign matrix uniform from shader to uniformvs.
			glUniformMatrix4fv(voxelMVPUniform, 1, GL_FALSE, glm::value_ptr(voxelMVP));


			// Iterate through the meshes.
			for(GLuint index = 0; index < scene->mNumMeshes; index++)
			{
				// Set vao as active VAO in the state machine.
				glBindVertexArray(meshes[index].vao);

				// Draw the current VAO using the bound IBO.
				glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
			}
			if(useLinearResampling)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				copyCoverage(coverageTexture, processedLayer.y[layerIndex], voxelResolution);
			}
			else
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, layerResolution, layerResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				calculateCoverage(coverageTexture, processedLayer.y[layerIndex], voxelResolution, voxelPrecision);
			}
			totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\y").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				saveBMP(string("layers\\py").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.y[layerIndex], voxelResolution, 1);
			}
		}
	}

	//Compute Z layers.
	for(GLuint layerIndex = 0; layerIndex < processedLayer.numberLayers; layerIndex++)
	{
		//layerScalar = ((((GLfloat) layerIndex + 1.0f) / (GLfloat) rawLayer.numberLayers) - 0.5f);
		//modelScalarMin = ((minimum.y * modelScale) + -center.y);
		//modelScalarMax = ((maximum.y * modelScale) + -center.y);
		//logFile << "Layer\nLayer Scalar: " << layerScalar << "\nModel Scalar Min: " << modelScalarMin << "\nModel Scalar Max: " << modelScalarMax << "\n";
		if( ((( (GLfloat) layerIndex + 2.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) < ((minimum.z * modelScale) + -center.z) )
		{
			delete(processedLayer.z[layerIndex]);
			processedLayer.z[layerIndex] = emptyLayer.z[0];
		}
		else if( ((( (GLfloat) layerIndex + 1.0f) / (GLfloat) processedLayer.numberLayers) - 0.5f) > ((maximum.z * modelScale) + -center.z) )
		{
			delete(processedLayer.z[layerIndex]);
			processedLayer.z[layerIndex] = emptyLayer.z[0];
		}
		else
		{
			startTime = glfwGetTime();
			voxelStep = (((GLfloat) layerIndex + 1.0f) / (GLfloat) voxelResolution) - (1.0f / voxelResolution);
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewZ * voxelModel;

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
			//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);

			// Clear the screen.
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Enable shader_program in the state machine.
			glUseProgram(voxelShaderProgram);

			// Assign matrix uniform from shader to uniformvs.
			glUniformMatrix4fv(voxelMVPUniform, 1, GL_FALSE, glm::value_ptr(voxelMVP));


			// Iterate through the meshes.
			for(GLuint index = 0; index < scene->mNumMeshes; index++)
			{
				// Set vao as active VAO in the state machine.
				glBindVertexArray(meshes[index].vao);

				// Draw the current VAO using the bound IBO.
				glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
			}
			if(useLinearResampling)
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				copyCoverage(coverageTexture, processedLayer.z[layerIndex], voxelResolution);
			}
			else
			{
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				//glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
				glReadPixels(0, 0, layerResolution, layerResolution, GL_RGB, GL_UNSIGNED_BYTE, coverageTexture);
				calculateCoverage(coverageTexture, processedLayer.z[layerIndex], voxelResolution, voxelPrecision);
			}
			totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\z").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				saveBMP(string("layers\\pz").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.z[layerIndex], voxelResolution, 1);
			}
		}
	}

	delete(coverageTexture);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	startTime = glfwGetTime();

	//Build the 3D texture
	glm::u8vec3 * sceneVoxelOcclusionTexture = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * voxelResolution * voxelResolution * voxelResolution);

	//Copy X values to red channel
	for(GLuint layer = 0; layer < voxelResolution; layer++)
	{
		for(GLuint y = 0; y < voxelResolution; y++)
		{
			for(GLuint x = 0; x < voxelResolution; x++)
			{
				sceneVoxelOcclusionTexture[((x) * voxelResolution * voxelResolution) + 
										((voxelResolution - 1 - y) * voxelResolution) + 
										(voxelResolution - 1 - layer)].r = 
										processedLayer.x[layer][(y * voxelResolution) + x];
			}
		}
	}

	//Copy Y values to green channel
	for(GLuint layer = 0; layer < voxelResolution; layer++)
	{
		for(GLuint y = 0; y < voxelResolution; y++)
		{
			for(GLuint x = 0; x < voxelResolution; x++)
			{
				sceneVoxelOcclusionTexture[((voxelResolution - 1 - y) * voxelResolution * voxelResolution) + 
										((voxelResolution - 1 - layer) * voxelResolution) + 
										(voxelResolution - 1 - x)].g = 
										processedLayer.y[layer][(y * voxelResolution) + x];
			}
		}
	}

	//Copy Z values to blue channel
	for(GLuint layer = 0; layer < voxelResolution; layer++)
	{
		for(GLuint y = 0; y < voxelResolution; y++)
		{
			for(GLuint x = 0; x < voxelResolution; x++)
			{
				sceneVoxelOcclusionTexture[((voxelResolution - 1 - layer) * voxelResolution * voxelResolution) + 
										((voxelResolution - 1 - y) * voxelResolution) + 
										(voxelResolution - 1 - x)].b = 
										processedLayer.z[layer][(y * voxelResolution) + x];
			}
		}
	}

	totalTime += glfwGetTime() - startTime;

	logFile << "\nTotal Time: " << totalTime << "\n";

	//unsigned char * arrayCastVoxel = (unsigned char *) sceneVoxelOcclusionTexture;

	//for(GLuint layer = 0; layer < voxelResolution; layer++)
	//{
	//	saveBMP(string("layers\\final").append(to_string(layer).append(string(".bmp"))).c_str(), &arrayCastVoxel[layer * voxelResolution * voxelResolution * 3], voxelResolution, 3);
	//}

	GLuint voxelOcclusionTexture;
	glGenTextures(1, &voxelOcclusionTexture);
	glBindTexture(GL_TEXTURE_3D, voxelOcclusionTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, voxelResolution, voxelResolution, voxelResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, sceneVoxelOcclusionTexture);
	glGenerateMipmap(GL_TEXTURE_3D);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLint voxelOcclusionTextureUniform;
	voxelOcclusionTextureUniform = glGetUniformLocation(mainShaderProgram, "voxelOcclusionTexture");

	GLint modelUniform, centerUniform, modelScaleUniform, voxelResolutionUniform;
	modelUniform = glGetUniformLocation(mainShaderProgram, "model");
	centerUniform = glGetUniformLocation(mainShaderProgram, "center");
	modelScaleUniform = glGetUniformLocation(mainShaderProgram, "modelScale");
	voxelResolutionUniform = glGetUniformLocation(mainShaderProgram, "voxelResolution");


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glViewport(0, 0, width, height);

	// Data for camera control.
	glm::dvec2 & mouse = glm::dvec2(0.0f, 0.0f);
	glm::dvec2 & mouseLast = glm::dvec2(0.0f, 0.0f);
	glm::vec3 & position = glm::vec3(-894.206177f, 694.136108f, 0.837747276);
	glm::vec3 & up = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 & right = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 & forward = glm::vec3(0.0f, 0.0f, 0.0f);
	GLfloat horizontalAngle = -4.71451330f;
	GLfloat verticalAngle = 5.84226465f;

	// Speed of camera controls.
	GLfloat baseSpeed = 200.0f;
	GLfloat speed = baseSpeed;
	GLfloat mouseSpeed = 0.15f;



	// Model and view matrix construction and initial projection matrix generation.
	glm::mat4 & model = glm::mat4(1.0f);
	glm::mat4 & view = glm::mat4();
	projection = glm::perspectiveFov(fov, (GLfloat) width, (GLfloat) height, 0.1f, 10000.0f);
	//projection = glm::ortho(-1000.0f, 1000.0f, -1000.0f, 1000.0f, 0.1f, 10000.0f);

	// Create MVP matrix.
	glm::mat4 & mvp = glm::mat4();



	// Log file visual seperator.
	logFile << "\nRender\n-----------\n\n";

	// Initialize timers for frame time independent motion.
	GLfloat lastFrameTime = (GLfloat) glfwGetTime();
	GLfloat currentFrameTime = 0.0f;
	GLfloat deltaFrameTime = 0.0f;

	// Boolean flag for first frame after window click condition.
	GLboolean mouseReset = true;

	//GLfloat voxelViewPosition = 0.0f;

	bool inputLock = true;


	glClearError(15);

	// Main loop with exit on ESC or window close
	while(!glfwGetKey(window, GLFW_KEY_ESCAPE) && !glfwWindowShouldClose(window))
	{
		// Input processing code from opengl-tutorial.org
		lastFrameTime = currentFrameTime;
		currentFrameTime = (GLfloat) glfwGetTime();
		deltaFrameTime = currentFrameTime - lastFrameTime;

		// Save previous mouse position.
		mouseLast.x = mouse.x;
		mouseLast.y = mouse.y;

		// Grab current mouse position if Button 1 is pressed.
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &mouse.x, &mouse.y);
			// Initialize mouse position of mouseLast to current position if
			// first frame after click.
			if(mouseReset)
			{
				glfwGetCursorPos(window, &mouseLast.x, &mouseLast.y);
				mouseReset = false;
			}
		}
		else
		{
			mouseReset = true;
		}

		// Calculate the change of angle from the distance moved with the mouse.
		horizontalAngle += mouseSpeed * deltaFrameTime * (GLfloat) (mouse.x - mouseLast.x);
		verticalAngle += mouseSpeed * deltaFrameTime * (GLfloat) (mouse.y - mouseLast.y);

		// Always use mouse position for motion and bind mouse to the center of the screen.
		//glfwGetCursorPos(window, &mouse.x, &mouse.y);
		//glfwSetCursorPos(window, width / 2, height / 2);
		//horizontalAngle += mouseSpeed * deltaTime * (((GLfloat) width / 2) - mouse.x);
		//verticalAngle += mouseSpeed * deltaTime * (((GLfloat) height / 2) - mouse.y);

		// Calculate the right direction vector from the angle change.
		right.x = sin(horizontalAngle - (3.14f / 2.0f));
		right.z = cos(horizontalAngle - (3.14f / 2.0f));

		// Calculate the forward direction vector from the angle change.
		forward.x = cos(verticalAngle) * sin(horizontalAngle);
		forward.y = sin(verticalAngle);
		forward.z = cos(verticalAngle) * cos(horizontalAngle);

		// Calculate the up direction vector by cross product of right and forward.
		up = glm::cross(right, forward);

		// Change position using directional vectors.
		if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
		{
			speed = baseSpeed * 3.0f;
		}
		else
		{
			speed = baseSpeed;
		}
		if(glfwGetKey(window, GLFW_KEY_W))
		{
			position += forward * deltaFrameTime * speed;
		}
		if(glfwGetKey(window, GLFW_KEY_S))
		{
			position -= forward * deltaFrameTime * speed;
		}
		if(glfwGetKey(window, GLFW_KEY_A))
		{
			position -= right * deltaFrameTime * speed;
		}
		if(glfwGetKey(window, GLFW_KEY_D))
		{
			position += right * deltaFrameTime * speed;
		}
		if(glfwGetKey(window, GLFW_KEY_Q))
		{
			position -= up * deltaFrameTime * speed;
		}
		if(glfwGetKey(window, GLFW_KEY_E))
		{
			position += up * deltaFrameTime * speed;
		}



		//if(glfwGetKey(window, GLFW_KEY_Q) && inputLock)
		//{
		//	voxelViewPosition -= 1.0f / voxelResolution;
		//	inputLock = false;
		//}
		//if(glfwGetKey(window, GLFW_KEY_E) && inputLock)
		//{
		//	voxelViewPosition += 1.0f / voxelResolution;
		//	inputLock = false;
		//}
		//if((!glfwGetKey(window, GLFW_KEY_Q) && !glfwGetKey(window, GLFW_KEY_E)) && !inputLock)
		//{
		//	inputLock = true;
		//}


		//Voxel Test Perspective
		//voxelView = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelViewPosition, (1.0f / (GLfloat) voxelResolution) + voxelViewPosition);
		//glm::mat4 & voxelMVP = voxelProjection * voxelView * voxelModel;


		// Calculate view matrix from position and directional vectors.
		view = glm::lookAt(position, position + forward, up);

		// Compute the mvp matrix.
		mvp = projection * view * model;

		// Clear the screen.
		glClearColor(0.714f, 0.827f, 0.937f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Enable shader_program in the state machine.
		glUseProgram(mainShaderProgram);

		// Assign matrix uniform from shader to uniformvs.
		glUniformMatrix4fv(mainMVPUniform, 1, GL_FALSE, glm::value_ptr(mvp));

		glUniformMatrix4fv(modelUniform, 1, GL_FALSE, glm::value_ptr(voxelModel));

		glUniform1i(diffuseTextureUniform, 0);
		glUniform1i(normalTextureUniform, 1);
		glUniform1i(opacityTextureUniform, 3);
		glUniform1i(voxelOcclusionTextureUniform, 10);

		glUniform3fv(centerUniform, 1, glm::value_ptr(center));
		glUniform1f(modelScaleUniform, modelScale);
		glUniform1ui(voxelResolutionUniform, voxelResolution);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_3D, voxelOcclusionTexture);

		//logFile << glGetErrorReadable().c_str();

		//startTime = glfwGetTime();

		// Iterate through the meshes.
		for(GLuint index = 0; index < scene->mNumMeshes; index++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshes[index].textures[0].textureID);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshes[index].textures[1].textureID);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, meshes[index].textures[3].textureID);
			// Set vao as active VAO in the state machine.
			glBindVertexArray(meshes[index].vao);

			// Draw the current VAO using the bound IBO.
			glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
		}

		//endTime = glfwGetTime();
		//currentTime = endTime - startTime;

		//logFile << "Frame Time: " << currentTime << "\n";

		// Swap buffers and poll for events.
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Teardown.
	glfwDestroyWindow(window);
	glfwTerminate();

	// Close log.
	logFile.close();

	//system("PAUSE");
	return 0;
}