// Averaged Voxel GLobal Illumination
// Copyright (C) 2014  Evan Arthur Kosin
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#ifndef DATASTRUCTURES
#define DATASTRUCTURES


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
//textureID - OpenGL id of the texture
//width - width of texture
//height - height of texture
//depth - bit depth of texture
//imageData - array of image data
//modelRelativePath - relative path of the scene
//relativePath - relative path of textures
//cstrPath - C string of relativePath
//mapping - mapping type
//uvindex - unknown
//blend - blend method
//operation - blend operation
//mapmode - mapping method
//prepare() - open, load, upload if not duplicate
//            the texture to the GPU
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
		void prepare(GLuint type, texturesList &globalList)
		{
			GLuint ID = globalList.find(modelRelativePath);
			if(ID == 0)
			{
				relativePath = aiString("crytek-sponza\\");
				relativePath.Append(modelRelativePath.C_Str());
				cstrPath = relativePath.C_Str();
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


//Layers:
//numberLayers - number of layers
//x - x axis layers
//y - y axis layers
//z - z axis layers
//prepare() - create and allocate memory for layers of
//            layerResolution * layerResolution * depth
//            size for voxelResolution number of layers
class layers
{
	public:
		GLuint numberLayers;
		unsigned char** x;
		unsigned char** y;
		unsigned char** z;
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

#endif