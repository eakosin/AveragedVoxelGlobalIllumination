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

bool framebufferToBMP = true;


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

//class coordinateTransformation
//{
//	private:
//		GLuint width2;
//		GLuint height2;
//		GLuint width3;
//		GLuint height3;
//		GLuint depth3;
//	public:
//		GLuint convert2(GLuint x, GLuint y)
//		{
//			GLuint retval = ((y * width2) + 
//					x);
//			return retval;
//		}
//		void set2(GLuint widthIn, GLuint heightIn)
//		{
//			width2 = widthIn;
//			height2 = heightIn;
//		}
//		GLuint convert3(GLuint x, GLuint y, GLuint z)
//		{
//			GLuint retval = ((z * height3 * width3) + 
//					(y * width3) +
//					x);
//			return retval;
//		}
//		void set3(GLuint widthIn, GLuint heightIn, GLuint depthIn)
//		{
//			width3 = widthIn;
//			height3 = heightIn;
//			depth3 = depthIn;
//		}
//};

namespace coordinateTransformation
{
	GLuint width2;
	GLuint height2;
	GLuint width3;
	GLuint height3;
	GLuint depth3;
	GLuint convert2(GLuint x, GLuint y)
	{
		GLuint retval = ((y * width2) + 
				x);
		return retval;
	}
	void set2(GLuint widthIn, GLuint heightIn)
	{
		width2 = widthIn;
		height2 = heightIn;
	}
	GLuint convert3(GLuint x, GLuint y, GLuint z)
	{
		GLuint retval = ((z * height3 * width3) + 
				(y * width3) +
				x);
		return retval;
	}
	void set3(GLuint widthIn, GLuint heightIn, GLuint depthIn)
	{
		width3 = widthIn;
		height3 = heightIn;
		depth3 = depthIn;
	}
};

void framebufferTo3DImageSpace(glm::u8vec3 * voxelTexture, glm::u8vec3 * lightTexture, GLfloat * depthTexture, GLuint voxelResolution)
{
	using namespace coordinateTransformation;
	for(GLuint index = 0; index < voxelResolution * voxelResolution * voxelResolution; index++)
	{
		voxelTexture[index] = glm::u8vec3(0.0f, 0.0f, 0.0f);
	}
	//coordinateTransformation ct;
	set2(voxelResolution, voxelResolution);
	set3(voxelResolution, voxelResolution, voxelResolution);
	GLuint z;
	GLfloat depth;
	for(GLuint y = 0; y < voxelResolution; y++)
	{
		for(GLuint x = 0; x < voxelResolution; x++)
		{
			depth = depthTexture[convert2(x,y)];
			z = (GLuint) glm::clamp(((depth) * voxelResolution), 0.0f, (GLfloat) (voxelResolution - 1));
			voxelTexture[convert3(x,(voxelResolution - 1 - z),y)] = lightTexture[convert2(x,y)];
		}
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
	const GLuint fullRowSize = voxelResolution * voxelPrecision;
	const GLuint scaledRowSize = voxelResolution * voxelPrecision * voxelPrecision;
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
				scaledCoverageIndex = (((coverageIndex / voxelResolution) * scaledRowSize) + ((coverageIndex % voxelResolution) * voxelPrecision));
				relativeIndex =  ( (scaledCoverageIndex + (rawIndexX)) + (rawIndexY * fullRowSize) );
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
		layer[index] = coverageTexture[index];
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

void processMeshes(const aiScene * scene, mesh * meshes)
{
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

		//Compute raw vertices.
		meshes[meshIndex].numberRawVertices = meshes[meshIndex].numberIndices;
		meshes[meshIndex].rawVertices = (glm::vec3*) ::operator new(sizeof(glm::vec3) * meshes[meshIndex].numberIndices);
		for(GLuint index = 0; index < meshes[meshIndex].numberIndices; index++)
		{
			meshes[meshIndex].rawVertices[index] = glm::vec3(meshes[meshIndex].vertexVector[meshes[meshIndex].indices[index]]);
		}

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
}

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

	GLint voxelOcclusionTextureUniform, lightVoxelTextureUniform;
	voxelOcclusionTextureUniform = glGetUniformLocation(mainShaderProgram, "voxelOcclusionTexture");
	lightVoxelTextureUniform = glGetUniformLocation(mainShaderProgram, "lightVoxelTexture");

	GLint modelUniform, voxelResolutionUniform, giResolutionUniform;
	modelUniform = glGetUniformLocation(mainShaderProgram, "model");
	voxelResolutionUniform = glGetUniformLocation(mainShaderProgram, "voxelResolution");
	giResolutionUniform = glGetUniformLocation(mainShaderProgram, "giResolution");

	GLint useAmbientOcclusionUniform, useAtmosphericOcclusionUniform, useTextureUniform, useGIUniform;
	useAmbientOcclusionUniform = glGetUniformLocation(mainShaderProgram, "useAmbientOcclusion");
	useAtmosphericOcclusionUniform = glGetUniformLocation(mainShaderProgram, "useAtmosphericOcclusion");
	useGIUniform = glGetUniformLocation(mainShaderProgram, "useGI");
	useTextureUniform = glGetUniformLocation(mainShaderProgram, "useTexture");

	GLint shiftXUniform, shiftYUniform, shiftZUniform;
	shiftXUniform = glGetUniformLocation(mainShaderProgram, "shiftX");
	shiftYUniform = glGetUniformLocation(mainShaderProgram, "shiftY");
	shiftZUniform = glGetUniformLocation(mainShaderProgram, "shiftZ");

	GLint diffuseTextureUniform;
	diffuseTextureUniform = glGetUniformLocation(mainShaderProgram, "diffuseTexture");
	GLint normalTextureUniform;
	normalTextureUniform = glGetUniformLocation(mainShaderProgram, "normalTexture");
	GLint specularTextureUniform;
	specularTextureUniform = glGetUniformLocation(mainShaderProgram, "specularTexture");


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


	// Log file visual seperator.
	logFile << "\nCoverage Shader Compilation\n-----------\n\n";

	// Prepare shaders.
	shader coverageVertex, coverageFragment;
	coverageVertex.prepare("shaders\\coverageVertex.glsl");
	coverageFragment.prepare("shaders\\coverageFragment.glsl");

	// Compile shaders.
	coverageVertex.object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(coverageVertex.object, 1, &coverageVertex.cstr, NULL);
	glCompileShader(coverageVertex.object);
	coverageFragment.object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(coverageFragment.object, 1, &coverageFragment.cstr, NULL);
	glCompileShader(coverageFragment.object);

	// Print shaders to logfile.
	//logFile << "\n\nVertex Shader:\n" << mainVertex.cstr << "\n\nFragment Shader:\n" << mainFragment.cstr << "\n\n";

	// Log shader compile errors.
	GLchar coverageVertexLog[1000];
	GLchar coverageFragmentLog[1000];
	glGetShaderInfoLog(coverageVertex.object, 1000, &length, coverageVertexLog);
	glGetShaderInfoLog(coverageFragment.object, 1000, &length, coverageFragmentLog);
	logFile << "\nVertex Shader Log:\n" << coverageVertexLog << "\n\nFragment Shader Log:\n" << coverageFragmentLog << "\n\n";

	// Attach shaders to program.
	GLuint coverageShaderProgram = glCreateProgram();
	glAttachShader(coverageShaderProgram, coverageVertex.object);
	glAttachShader(coverageShaderProgram, coverageFragment.object);

	// Link shader program.
	glLinkProgram(coverageShaderProgram);

	GLint framebufferTextureUniform;
	framebufferTextureUniform = glGetUniformLocation(coverageShaderProgram, "framebufferTexture");

	// Map mvp uniform from shaders.
	GLint coverageLayerResolutionUniform, coverageLayerPrecisionUniform;
	coverageLayerResolutionUniform = glGetUniformLocation(coverageShaderProgram, "coverageLayerResolution");
	coverageLayerPrecisionUniform = glGetUniformLocation(coverageShaderProgram, "coverageLayerPrecision");


	// Log file visual seperator.
	logFile << "\nLight Shader Compilation\n-----------\n\n";

	// Prepare shaders.
	shader lightVertex, lightFragment;
	lightVertex.prepare("shaders\\lightVertex.glsl");
	lightFragment.prepare("shaders\\lightFragment.glsl");

	// Compile shaders.
	lightVertex.object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(lightVertex.object, 1, &lightVertex.cstr, NULL);
	glCompileShader(lightVertex.object);
	lightFragment.object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(lightFragment.object, 1, &lightFragment.cstr, NULL);
	glCompileShader(lightFragment.object);

	// Print shaders to logfile.
	//logFile << "\n\nVertex Shader:\n" << mainVertex.cstr << "\n\nFragment Shader:\n" << mainFragment.cstr << "\n\n";

	// Log shader compile errors.
	GLchar lightVertexLog[1000];
	GLchar lightFragmentLog[1000];
	glGetShaderInfoLog(lightVertex.object, 1000, &length, lightVertexLog);
	glGetShaderInfoLog(lightFragment.object, 1000, &length, lightFragmentLog);
	logFile << "\nVertex Shader Log:\n" << lightVertexLog << "\n\nFragment Shader Log:\n" << lightFragmentLog << "\n\n";

	// Attach shaders to program.
	GLuint lightShaderProgram = glCreateProgram();
	glAttachShader(lightShaderProgram, lightVertex.object);
	glAttachShader(lightShaderProgram, lightFragment.object);

	// Link shader program.
	glLinkProgram(lightShaderProgram);

	// Map mvp uniform from shaders.
	GLint lightMVPUniform;
	lightMVPUniform = glGetUniformLocation(lightShaderProgram, "mvp");

	GLint lightDiffuseTextureUniform;
	lightDiffuseTextureUniform = glGetUniformLocation(lightShaderProgram, "diffuseTexture");



	// Log file visual seperator.
	logFile << "\nVoxelize Light Shader Compilation\n-----------\n\n";

	// Prepare shaders.
	shader voxelizeLightVertex, voxelizeLightFragment;
	voxelizeLightVertex.prepare("shaders\\voxelizeLightVertex.glsl");
	voxelizeLightFragment.prepare("shaders\\voxelizeLightFragment.glsl");

	// Compile shaders.
	voxelizeLightVertex.object = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(voxelizeLightVertex.object, 1, &voxelizeLightVertex.cstr, NULL);
	glCompileShader(voxelizeLightVertex.object);
	voxelizeLightFragment.object = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(voxelizeLightFragment.object, 1, &voxelizeLightFragment.cstr, NULL);
	glCompileShader(voxelizeLightFragment.object);

	// Print shaders to logfile.
	//logFile << "\n\nVertex Shader:\n" << mainVertex.cstr << "\n\nFragment Shader:\n" << mainFragment.cstr << "\n\n";

	// Log shader compile errors.
	GLchar voxelizeLightVertexLog[1000];
	GLchar voxelizeLightFragmentLog[1000];
	glGetShaderInfoLog(voxelizeLightVertex.object, 1000, &length, voxelizeLightVertexLog);
	glGetShaderInfoLog(voxelizeLightFragment.object, 1000, &length, voxelizeLightFragmentLog);
	logFile << "\nVertex Shader Log:\n" << voxelizeLightVertexLog << "\n\nFragment Shader Log:\n" << voxelizeLightFragmentLog << "\n\n";

	// Attach shaders to program.
	GLuint voxelizeLightShaderProgram = glCreateProgram();
	glAttachShader(voxelizeLightShaderProgram, voxelizeLightVertex.object);
	glAttachShader(voxelizeLightShaderProgram, voxelizeLightFragment.object);

	// Link shader program.
	glLinkProgram(voxelizeLightShaderProgram);

	// Map mvp uniform from shaders.
	GLint voxelizeLightMVPUniform;
	voxelizeLightMVPUniform = glGetUniformLocation(voxelizeLightShaderProgram, "mvp");

	GLint layerUniform;
	layerUniform = glGetUniformLocation(voxelizeLightShaderProgram, "layer");

	GLint lightFramebufferTextureUniform, lightNormalFramebufferTextureUniform, lightDepthFramebufferTextureUniform;
	lightFramebufferTextureUniform = glGetUniformLocation(voxelizeLightShaderProgram, "lightFramebufferTexture");
	lightNormalFramebufferTextureUniform = glGetUniformLocation(voxelizeLightShaderProgram, "lightNormalFramebufferTexture");
	lightDepthFramebufferTextureUniform = glGetUniformLocation(voxelizeLightShaderProgram, "lightDepthFramebufferTexture");


	// Log error from shader compiling and linking.
	logFile << glGetErrorReadable().c_str();


	// Log file visual seperator.
	logFile << "\nImport Model\n-----------\n\n";

	// Load scene.
	const aiScene* scene;
	Assimp::Importer importer;
	scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

	// Array of meshes in scene object.
	mesh* meshes = (mesh*) ::operator new(sizeof(mesh) * scene->mNumMeshes);
	GLuint numberMeshes = scene->mNumMeshes;

	processMeshes(scene, meshes);


	// Log file visual seperator.
	logFile << "\nVoxel Occlusion Preperation\n-----------\n\n";

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
	logFile << "\nVoxel Occlusion Generation\n-----------\n\n";


	GLuint voxelResolution = 256;
	GLuint voxelPrecision = 4;
	GLuint voxelSubPrecision = 16;
	GLfloat voxelStep = 1.0f / voxelResolution;
	GLfloat overlap = voxelStep / 0.75f;
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

	glm::vec3 screenSpaceVertices[6] = {
		glm::vec3( 1.0f,  1.0f,  0.0f),
		glm::vec3(-1.0f,  1.0f,  0.0f),
		glm::vec3(-1.0f, -1.0f,  0.0f),
		glm::vec3( 1.0f,  1.0f,  0.0f),
		glm::vec3(-1.0f, -1.0f,  0.0f),
		glm::vec3( 1.0f, -1.0f,  0.0f)};

	GLuint screenSpaceVBO, screenSpaceVAO;
	glGenBuffers(1, &screenSpaceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, screenSpaceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenSpaceVertices), screenSpaceVertices, GL_STATIC_DRAW);
	glGenVertexArrays(1, &screenSpaceVAO);
	glBindVertexArray(screenSpaceVAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

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
	
	voxelViewX = glm::lookAt(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	voxelViewY = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	voxelViewZ = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 voxelModel = voxelModelTranslate * voxelModelScale;

	glm::mat4 voxelMVP = voxelProjection * voxelViewY * voxelModel;

	glClearError(15);

	GLuint multisampleFramebuffer, coverageFramebuffer, linearResamplingFramebuffer;
	
	GLuint multisampleFramebufferTexture, coverageFramebufferTexture, linearResamplingFramebufferTexture;

	glGenFramebuffers(1, &multisampleFramebuffer);  //BREAKS NSIGHT
	glBindFramebuffer(GL_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, layerResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, layerResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleFramebuffer, GL_FRAMEBUFFER_DEFAULT_SAMPLES, voxelSubPrecision);  //BREAKS NSIGHT
	glGenTextures(1, &multisampleFramebufferTexture);  //BREAKS NSIGHT
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleFramebufferTexture);  //BREAKS NSIGHT
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, voxelSubPrecision, GL_RED, layerResolution, layerResolution, 0);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleFramebufferTexture, 0);  //BREAKS NSIGHT

	if(useLinearResampling)
	{
		glGenFramebuffers(1, &coverageFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, coverageFramebuffer);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, layerResolution);
		glFramebufferParameteri(coverageFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, layerResolution);
		glGenTextures(1, &coverageFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, layerResolution, layerResolution, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, coverageFramebufferTexture, 0);

		glGenFramebuffers(1, &linearResamplingFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, linearResamplingFramebuffer);
		glFramebufferParameteri(linearResamplingFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, voxelResolution);
		glFramebufferParameteri(linearResamplingFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, voxelResolution);
		glGenTextures(1, &linearResamplingFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, linearResamplingFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, voxelResolution, voxelResolution, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearResamplingFramebufferTexture, 0);
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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, layerResolution, layerResolution, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, coverageFramebufferTexture, 0);

		glGenFramebuffers(1, &linearResamplingFramebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, linearResamplingFramebuffer);
		glFramebufferParameteri(linearResamplingFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, voxelResolution);
		glFramebufferParameteri(linearResamplingFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, voxelResolution);
		glGenTextures(1, &linearResamplingFramebufferTexture);
		glBindTexture(GL_TEXTURE_2D, linearResamplingFramebufferTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, voxelResolution, voxelResolution, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, linearResamplingFramebufferTexture, 0);
		//logFile << glGetErrorReadable().c_str();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
			//startTime = glfwGetTime();
			voxelStep = (GLfloat) layerIndex / (GLfloat) voxelResolution;
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewX * voxelModel;

			glViewport(0, 0, layerResolution, layerResolution);

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
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, processedLayer.x[layerIndex]);
				//copyCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution);
			}
			else
			{
				startTime = glfwGetTime();
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glViewport(0, 0, voxelResolution, voxelResolution);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(coverageShaderProgram);
				glUniform1i(framebufferTextureUniform, 0);
				glUniform1i(coverageLayerResolutionUniform, voxelResolution);
				glUniform1i(coverageLayerPrecisionUniform, voxelPrecision);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
				glBindVertexArray(screenSpaceVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, coverageTexture);
				//calculateCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution, voxelPrecision);
				copyCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution);
				totalTime += glfwGetTime() - startTime;
			}
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			//totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\x").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				//saveBMP(string("layers\\px").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.x[layerIndex], voxelResolution, 1);
			}
		}
	}
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
			//startTime = glfwGetTime();
			voxelStep = (GLfloat) layerIndex / (GLfloat) voxelResolution;
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewY * voxelModel;

			glViewport(0, 0, layerResolution, layerResolution);

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
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, processedLayer.y[layerIndex]);
				//copyCoverage(coverageTexture, processedLayer.y[layerIndex], voxelResolution);
			}
			else
			{
				startTime = glfwGetTime();
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glViewport(0, 0, voxelResolution, voxelResolution);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(coverageShaderProgram);
				glUniform1i(framebufferTextureUniform, 0);
				glUniform1i(coverageLayerResolutionUniform, voxelResolution);
				glUniform1i(coverageLayerPrecisionUniform, voxelPrecision);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
				glBindVertexArray(screenSpaceVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, coverageTexture);
				//calculateCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution, voxelPrecision);
				copyCoverage(coverageTexture, processedLayer.y[layerIndex], voxelResolution);
				totalTime += glfwGetTime() - startTime;
			}
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			//totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\y").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				//saveBMP(string("layers\\py").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.y[layerIndex], voxelResolution, 1);
			}
		}
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	
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
			//startTime = glfwGetTime();
			voxelStep = (GLfloat) layerIndex / (GLfloat) voxelResolution;
			voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelStep - overlap, (1.0f / (GLfloat) voxelResolution) + voxelStep + overlap);

			voxelMVP = voxelProjection * voxelViewZ * voxelModel;

			glViewport(0, 0, layerResolution, layerResolution);

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
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, voxelResolution, voxelResolution, GL_COLOR_BUFFER_BIT, GL_LINEAR);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, processedLayer.z[layerIndex]);
				//copyCoverage(coverageTexture, processedLayer.z[layerIndex], voxelResolution);
			}
			else
			{
				startTime = glfwGetTime();
				glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleFramebuffer);  //BREAKS NSIGHT
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glBlitFramebuffer(0, 0, layerResolution, layerResolution, 0, 0, layerResolution, layerResolution, GL_COLOR_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
				glBindFramebuffer(GL_READ_FRAMEBUFFER, coverageFramebuffer);  //BREAKS NSIGHT
				glViewport(0, 0, voxelResolution, voxelResolution);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glUseProgram(coverageShaderProgram);
				glUniform1i(framebufferTextureUniform, 0);
				glUniform1i(coverageLayerResolutionUniform, voxelResolution);
				glUniform1i(coverageLayerPrecisionUniform, voxelPrecision);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coverageFramebufferTexture);
				glBindVertexArray(screenSpaceVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
				glBindFramebuffer(GL_READ_FRAMEBUFFER, linearResamplingFramebuffer);
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, linearResamplingFramebuffer);
				glReadPixels(0, 0, voxelResolution, voxelResolution, GL_RED, GL_UNSIGNED_BYTE, coverageTexture);
				//calculateCoverage(coverageTexture, processedLayer.x[layerIndex], voxelResolution, voxelPrecision);
				copyCoverage(coverageTexture, processedLayer.z[layerIndex], voxelResolution);
				totalTime += glfwGetTime() - startTime;
			}
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			//totalTime += glfwGetTime() - startTime;
			if(framebufferToBMP)
			{
				//saveBMP(string("layers\\z").append(to_string(layerIndex).append(string(".bmp"))).c_str(), coverageTexture, layerResolution, 3);
				//saveBMP(string("layers\\pz").append(to_string(layerIndex).append(string(".bmp"))).c_str(), processedLayer.z[layerIndex], voxelResolution, 1);
			}
		}
	}

	delete(coverageTexture);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glDeleteTextures(1, &multisampleFramebufferTexture);
	glDeleteTextures(1, &coverageFramebufferTexture);

	glDeleteFramebuffers(1, &multisampleFramebuffer);
	glDeleteFramebuffers(1, &coverageFramebuffer);

	if(useLinearResampling)
	{
		glDeleteTextures(1, &linearResamplingFramebufferTexture);
		glDeleteFramebuffers(1, &linearResamplingFramebuffer);
	}


	//startTime = glfwGetTime();

	//Build the 3D texture
	glm::u8vec3 * sceneVoxelOcclusionTexture = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * voxelResolution * voxelResolution * voxelResolution);

	//Copy X values to red channel
	for(GLuint layer = 0; layer < voxelResolution; layer++)
	{
		for(GLuint y = 0; y < voxelResolution; y++)
		{
			for(GLuint x = 0; x < voxelResolution; x++)
			{
				sceneVoxelOcclusionTexture[((voxelResolution - 1 - x) * voxelResolution * voxelResolution) + 
										((y) * voxelResolution) + 
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
										(x)].g = 
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
										((y) * voxelResolution) + 
										(x)].b = 
										processedLayer.z[layer][(y * voxelResolution) + x];
			}
		}
	}


	//totalTime += glfwGetTime() - startTime;

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

	glClearError(15);

	//MVP for Sun
	glm::vec3 & lightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 & lightUp = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 & lightRight = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 & lightForward = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 lightView;
	glm::mat4 lightViewX = glm::lookAt(glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightViewY = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
	glm::mat4 lightViewZ = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, -2.0f, 2.0f);
	glm::mat4 lightModel = glm::mat4(voxelModel);
	glm::mat4 lightMVP;
	glm::mat4 copyMVP;
	GLfloat lightHorizontalAngle = 0.0f;
	//GLfloat lightHorizontalAngle = -4.71451330f;
	GLfloat lightVerticalAngle = 1.57079633f;

	GLuint lightRenderResolution = 128;

	//glm::u8vec3 * lightArray = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * lightRenderResolution * lightRenderResolution);
	//glm::u8vec3 * lightNormalsArray = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * lightRenderResolution * lightRenderResolution);
	//GLfloat * lightDepthArray = (GLfloat *) ::operator new(sizeof(GLfloat) * lightRenderResolution * lightRenderResolution);

	//glm::u8vec3 * lightVoxelArray = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * lightRenderResolution * lightRenderResolution * lightRenderResolution);
	//glm::u8vec3 * lightVoxelNormalsArray = (glm::u8vec3 *) ::operator new(sizeof(glm::u8vec3) * lightRenderResolution * lightRenderResolution * lightRenderResolution);

	GLuint lightFramebuffer, lightFramebufferTexture, lightFramebufferDepthTexture, lightFramebufferNormalsTexture;
	GLuint multisampleLightFramebuffer, multisampleLightFramebufferTexture, multisampleLightFrambufferNormalsTexture, multisampleLightDepthTexture;

	GLuint framebufferAttachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_DEPTH_ATTACHMENT };

	glGenFramebuffers(1, &multisampleLightFramebuffer);  //BREAKS NSIGHT
	glBindFramebuffer(GL_FRAMEBUFFER, multisampleLightFramebuffer);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleLightFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, lightRenderResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleLightFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, lightRenderResolution);  //BREAKS NSIGHT
	glFramebufferParameteri(multisampleLightFramebuffer, GL_FRAMEBUFFER_DEFAULT_SAMPLES, 16);  //BREAKS NSIGHT

	glGenTextures(1, &multisampleLightFramebufferTexture);  //BREAKS NSIGHT
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleLightFramebufferTexture);  //BREAKS NSIGHT
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, lightRenderResolution, lightRenderResolution, 0);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, multisampleLightFramebufferTexture, 0);  //BREAKS NSIGHT

	glGenTextures(1, &multisampleLightFrambufferNormalsTexture);  //BREAKS NSIGHT
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleLightFrambufferNormalsTexture);  //BREAKS NSIGHT
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, lightRenderResolution, lightRenderResolution, 0);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, multisampleLightFrambufferNormalsTexture, 0);  //BREAKS NSIGHT

	glGenTextures(1, &multisampleLightDepthTexture);  //BREAKS NSIGHT
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleLightDepthTexture);  //BREAKS NSIGHT
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_DEPTH_COMPONENT, lightRenderResolution, lightRenderResolution, 0);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  //BREAKS NSIGHT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, multisampleLightDepthTexture, 0);  //BREAKS NSIGHT

	glDrawBuffers(2, framebufferAttachments);


	glGenFramebuffers(1, &lightFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, lightFramebuffer);
	glFramebufferParameteri(lightFramebuffer, GL_FRAMEBUFFER_DEFAULT_WIDTH, lightRenderResolution);
	glFramebufferParameteri(lightFramebuffer, GL_FRAMEBUFFER_DEFAULT_HEIGHT, lightRenderResolution);

	glGenTextures(1, &lightFramebufferTexture);
	glBindTexture(GL_TEXTURE_2D, lightFramebufferTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lightRenderResolution, lightRenderResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightFramebufferTexture, 0);

	glGenTextures(1, &lightFramebufferNormalsTexture);
	glBindTexture(GL_TEXTURE_2D, lightFramebufferNormalsTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lightRenderResolution, lightRenderResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightFramebufferNormalsTexture, 0);

	glGenTextures(1, &lightFramebufferDepthTexture);
	glBindTexture(GL_TEXTURE_2D, lightFramebufferDepthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, lightRenderResolution, lightRenderResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lightFramebufferDepthTexture, 0);

	glDrawBuffers(2, framebufferAttachments);


	GLuint voxelTextureFramebufferBindPoint;
	glGenFramebuffers(1, &voxelTextureFramebufferBindPoint);
	glBindFramebuffer(GL_FRAMEBUFFER, voxelTextureFramebufferBindPoint);
	glFramebufferParameteri(voxelTextureFramebufferBindPoint, GL_FRAMEBUFFER_DEFAULT_WIDTH, lightRenderResolution);
	glFramebufferParameteri(voxelTextureFramebufferBindPoint, GL_FRAMEBUFFER_DEFAULT_HEIGHT, lightRenderResolution);


	GLuint lightVoxelTexture;
	glGenTextures(1, &lightVoxelTexture);
	glBindTexture(GL_TEXTURE_3D, lightVoxelTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, lightRenderResolution, lightRenderResolution, lightRenderResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);

	GLuint lightNormalTexture;
	glGenTextures(1, &lightNormalTexture);
	glBindTexture(GL_TEXTURE_3D, lightNormalTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, lightRenderResolution, lightRenderResolution, lightRenderResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_3D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Initialize timers for frame time independent motion.
	GLfloat lastFrameTime = (GLfloat) glfwGetTime();
	GLfloat currentFrameTime = 0.0f;
	GLfloat deltaFrameTime = 0.0f;

	// Boolean flag for first frame after window click condition.
	GLboolean mouseReset = true;

	//GLfloat voxelViewPosition = 0.0f;

	bool inputLock = true;

	bool useTexture = true;
	bool useAmbientOcclusion = true;
	bool useAtmosphericOcclusion = false;
	bool useGI = true;

	int shiftX = 0;
	int shiftY = 0;
	int shiftZ = 0;

	bool first = true;

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
			// Calculate the change of angle from the distance moved with the mouse.
			horizontalAngle += mouseSpeed * deltaFrameTime * (GLfloat) (mouse.x - mouseLast.x);
			verticalAngle += mouseSpeed * deltaFrameTime * (GLfloat) (mouse.y - mouseLast.y);
		}
		else if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &mouse.x, &mouse.y);
			// Initialize mouse position of mouseLast to current position if
			// first frame after click.
			if(mouseReset)
			{
				glfwGetCursorPos(window, &mouseLast.x, &mouseLast.y);
				mouseReset = false;
			}
			// Calculate the change of angle from the distance moved with the mouse.
			lightHorizontalAngle += (mouseSpeed / 4.0f) * deltaFrameTime * (GLfloat) (mouse.x - mouseLast.x);
			lightVerticalAngle += (mouseSpeed / 4.0f) * deltaFrameTime * (GLfloat) (mouse.y - mouseLast.y);
		}
		else
		{
			mouseReset = true;
		}



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

		if(glfwGetKey(window, GLFW_KEY_R))
		{
			shiftX = 0;
			shiftY = 0;
			shiftZ = 0;
		}
		if(glfwGetKey(window, GLFW_KEY_T) && inputLock)
		{
			useTexture = !useTexture;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_1) && inputLock)
		{
			useAmbientOcclusion = !useAmbientOcclusion;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_2) && inputLock)
		{
			useAtmosphericOcclusion = !useAtmosphericOcclusion;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_3) && inputLock)
		{
			useGI = !useGI;
			inputLock = false;
		}
		//if(glfwGetKey(window, GLFW_KEY_E) && inputLock)
		//{
		//	voxelViewPosition += 1.0f / voxelResolution;
		//	inputLock = false;
		//}
		if(glfwGetKey(window, GLFW_KEY_UP) && inputLock)
		{
			shiftZ += 1;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_DOWN) && inputLock)
		{
			shiftZ -= 1;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_RIGHT) && inputLock)
		{
			shiftX += 1;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_LEFT) && inputLock)
		{
			shiftX-= 1;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_PAGE_UP) && inputLock)
		{
			shiftY += 1;
			inputLock = false;
		}
		if(glfwGetKey(window, GLFW_KEY_PAGE_DOWN) && inputLock)
		{
			shiftY -= 1;
			inputLock = false;
		}
		if(!glfwGetKey(window, GLFW_KEY_T) && 
			!glfwGetKey(window, GLFW_KEY_1) && 
			!glfwGetKey(window, GLFW_KEY_2) && 
			!glfwGetKey(window, GLFW_KEY_3) && 
			!glfwGetKey(window, GLFW_KEY_UP) && 
			!glfwGetKey(window, GLFW_KEY_DOWN) && 
			!glfwGetKey(window, GLFW_KEY_RIGHT) && 
			!glfwGetKey(window, GLFW_KEY_LEFT) && 
			!glfwGetKey(window, GLFW_KEY_PAGE_UP) && 
			!glfwGetKey(window, GLFW_KEY_PAGE_DOWN) && !inputLock)
		{
			inputLock = true;
		}
		

		//Voxel Test Perspective
		//voxelView = glm::lookAt(glm::vec3(0.0f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		//voxelProjection = glm::ortho(-0.5f, 0.5f, -0.5f, 0.5f, 0.0f + voxelViewPosition, (1.0f / (GLfloat) voxelResolution) + voxelViewPosition);
		//glm::mat4 & voxelMVP = voxelProjection * voxelView * voxelModel;


		// Calculate view matrix from position and directional vectors.
		view = glm::lookAt(position, position + forward, up);



		// Compute the mvp matrix.
		mvp = projection * view * model;


		// Calculate the right direction vector from the angle change.
		lightRight.x = sin(lightHorizontalAngle - (3.14f / 2.0f));
		lightRight.z = cos(lightHorizontalAngle - (3.14f / 2.0f));

		// Calculate the forward direction vector from the angle change.
		lightForward.x = cos(lightVerticalAngle) * sin(lightHorizontalAngle);
		lightForward.y = sin(lightVerticalAngle);
		lightForward.z = cos(lightVerticalAngle) * cos(lightHorizontalAngle);

		// Calculate the up direction vector by cross product of right and forward.
		lightUp = glm::cross(lightRight, lightForward);

		lightView = glm::lookAt(lightPosition + lightForward, lightPosition, lightUp);

		lightMVP = lightProjection * lightView * lightModel;

		if(useGI)
		{
			glEnable(GL_DEPTH_TEST);
			glClearError(25);
			//glDisable(GL_MULTISAMPLE);
			glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
			glViewport(0, 0, lightRenderResolution, lightRenderResolution);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampleLightFramebuffer);
			glDrawBuffers(3, framebufferAttachments);

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Enable shader_program in the state machine.
			glUseProgram(lightShaderProgram);

			// Assign matrix uniform from shader to uniformvs.
			glUniformMatrix4fv(lightMVPUniform, 1, GL_FALSE, glm::value_ptr(lightMVP));
			glUniform1i(lightDiffuseTextureUniform, 0);

			// Iterate through the meshes.
			for(GLuint index = 0; index < scene->mNumMeshes; index++)
			{
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, meshes[index].textures[0].textureID);
				// Set vao as active VAO in the state machine.
				glBindVertexArray(meshes[index].vao);

				// Draw the current VAO using the bound IBO.
				glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
			}
			glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampleLightFramebuffer);  //BREAKS NSIGHT
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lightFramebuffer);  //BREAKS NSIGHT
			glReadBuffer(GL_COLOR_ATTACHMENT0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glBlitFramebuffer(0, 0, lightRenderResolution, lightRenderResolution, 0, 0, lightRenderResolution, lightRenderResolution, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
			glReadBuffer(GL_COLOR_ATTACHMENT1);
			glDrawBuffer(GL_COLOR_ATTACHMENT1);
			glBlitFramebuffer(0, 0, lightRenderResolution, lightRenderResolution, 0, 0, lightRenderResolution, lightRenderResolution, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);  //BREAKS NSIGHT
			//glBindFramebuffer(GL_READ_FRAMEBUFFER, lightFramebuffer);  //BREAKS NSIGHT
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);  //BREAKS NSIGHT
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  //BREAKS NSIGHT
			//glReadPixels(0, 0, lightRenderResolution, lightRenderResolution, GL_RGB, GL_UNSIGNED_BYTE, lightArray);
			//glReadPixels(0, 0, lightRenderResolution, lightRenderResolution, GL_DEPTH_COMPONENT, GL_FLOAT, lightDepthArray);

			//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//glUniform1i(renderNormalsUniform, (int) true);

			//// Iterate through the meshes.
			//for(GLuint index = 0; index < scene->mNumMeshes; index++)
			//{
			//	glActiveTexture(GL_TEXTURE0);
			//	glBindTexture(GL_TEXTURE_2D, meshes[index].textures[0].textureID);
			//	// Set vao as active VAO in the state machine.
			//	glBindVertexArray(meshes[index].vao);

			//	// Draw the current VAO using the bound IBO.
			//	glDrawElements(GL_TRIANGLES, meshes[index].numberIndices, GL_UNSIGNED_INT, 0);
			//}
			////glBindTexture(GL_TEXTURE_2D, lightFramebufferTexture);
			//glReadPixels(0, 0, lightRenderResolution, lightRenderResolution, GL_RGB, GL_UNSIGNED_BYTE, lightNormalsArray);

			//if(first)
			//{
			//	saveBMP("light.bmp", (unsigned char *) lightNormalsArray, lightRenderResolution, 3);
			//	first = false;
			//}

			//framebufferTo3DImageSpace(lightVoxelArray, lightArray, lightDepthArray, lightRenderResolution);

			glDisable(GL_DEPTH_TEST);

			copyMVP = lightProjection * lightView * glm::mat4(1.0f);

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, voxelTextureFramebufferBindPoint);
			glBindFramebuffer(GL_READ_FRAMEBUFFER, lightFramebuffer);
			glUseProgram(voxelizeLightShaderProgram);

			glUniformMatrix4fv(voxelizeLightMVPUniform, 1, GL_FALSE, glm::value_ptr(copyMVP));
			glUniform1i(lightFramebufferTextureUniform, 0);
			glUniform1i(lightNormalFramebufferTextureUniform, 1);
			glUniform1i(lightDepthFramebufferTextureUniform, 2);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, lightFramebufferTexture);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, lightFramebufferNormalsTexture);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, lightFramebufferDepthTexture);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			//glActiveTexture(GL_TEXTURE11);
			//glBindTexture(GL_TEXTURE_3D, 0);


			for(GLuint layer = 0; layer < lightRenderResolution; layer++)
			{
				glUniform1i(layerUniform, layer);
				glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, lightVoxelTexture, 0, layer);
				glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, lightNormalTexture, 0, layer);
				glDrawBuffers(2, framebufferAttachments);
				glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glBindVertexArray(screenSpaceVAO);
				glDrawArrays(GL_TRIANGLES, 0, 6);
			}

			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

			glBindTexture(GL_TEXTURE_3D, lightVoxelTexture);
			glGenerateMipmap(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, lightNormalTexture);
			glGenerateMipmap(GL_TEXTURE_3D);
			glBindTexture(GL_TEXTURE_3D, 0);

			//unsigned char * arrayCastVoxel = (unsigned char *) sceneVoxelOcclusionTexture;

			//for(GLuint layer = 0; layer < voxelResolution; layer++)
			//{
			//	saveBMP(string("layers\\final").append(to_string(layer).append(string(".bmp"))).c_str(), &arrayCastVoxel[layer * voxelResolution * voxelResolution * 3], voxelResolution, 3);
			//}

			//glBindTexture(GL_TEXTURE_3D, lightVoxelTexture);
			//glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB, lightRenderResolution, lightRenderResolution, lightRenderResolution, 0, GL_RGB, GL_UNSIGNED_BYTE, lightVoxelArray);
			//glGenerateMipmap(GL_TEXTURE_3D);
			//glBindTexture(GL_TEXTURE_3D, 0);
			//logFile << glGetErrorReadable().c_str();
		}

		glEnable(GL_DEPTH_TEST);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		glEnable(GL_MULTISAMPLE);
		glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		glViewport(0, 0, width, height);

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
		glUniform1i(specularTextureUniform, 3);
		glUniform1i(voxelOcclusionTextureUniform, 10);
		glUniform1i(lightVoxelTextureUniform, 11);

		glUniform1i(useAmbientOcclusionUniform, (int) useAmbientOcclusion);
		glUniform1i(useAtmosphericOcclusionUniform, (int) useAtmosphericOcclusion);
		glUniform1i(useGIUniform, (int) useGI);
		glUniform1i(useTextureUniform, (int) useTexture);

		glUniform1ui(voxelResolutionUniform, voxelResolution);
		glUniform1ui(giResolutionUniform, lightRenderResolution);
		glUniform1i(shiftXUniform, shiftX);
		glUniform1i(shiftYUniform, shiftY);
		glUniform1i(shiftZUniform, shiftZ);

		glActiveTexture(GL_TEXTURE10);
		glBindTexture(GL_TEXTURE_3D, voxelOcclusionTexture);
		glActiveTexture(GL_TEXTURE11);
		glBindTexture(GL_TEXTURE_3D, lightVoxelTexture);
		glActiveTexture(GL_TEXTURE12);
		glBindTexture(GL_TEXTURE_3D, lightNormalTexture);

		//logFile << glGetErrorReadable().c_str();

		//startTime = glfwGetTime();

		// Iterate through the meshes.
		for(GLuint index = 0; index < scene->mNumMeshes; index++)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, meshes[index].textures[0].textureID);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, meshes[index].textures[1].textureID);
			//glActiveTexture(GL_TEXTURE3);
			//glBindTexture(GL_TEXTURE_2D, meshes[index].textures[3].textureID);
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