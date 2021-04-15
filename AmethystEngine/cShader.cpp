#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <memory>

#ifdef _DEBUG
#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__ , __LINE__)
#else
#define DBG_NEW
#endif

#include "cShaderManager.hpp"
#include "GLCommon.hpp"

#include <iostream>

cShaderManager::cShader::cShader() {
	this->ID = 0;
	this->shaderType = cShader::UNKNOWN;
	return;
}

cShaderManager::cShader::~cShader() { }

std::string cShaderManager::cShader::getShaderTypeString() {
	switch (this->shaderType) {
	case cShader::VERTEX_SHADER:
		return "VERTEX_SHADER";
		break;
	case cShader::FRAGMENT_SHADER:
		return "FRAGMENT_SHADER";
		break;
	case cShader::UNKNOWN:
	default:
		return "UNKNOWN_SHADER_TYPE";
		break;
	}
	return "UNKNOWN_SHADER_TYPE";
}


// Look up the uniform inside the shader, then save it, if it finds it
bool cShaderManager::cShaderProgram::LoadUniformLocation(std::string variableName) {
	GLint uniLocation = glGetUniformLocation(this->ID, variableName.c_str());
	// Did it find it (not -1)
	if (uniLocation == -1)
		return false;
	// Save it
	this->mapUniformName_to_UniformLocation[variableName.c_str()] = uniLocation;
	return true;
}

// Look up the uniform location and save it.
int cShaderManager::cShaderProgram::getUniformID_From_Name(std::string name) {
	std::map< std::string /*name of uniform variable*/, int /* uniform location ID */ >::iterator itUniform = this->mapUniformName_to_UniformLocation.find(name);

	if (itUniform == this->mapUniformName_to_UniformLocation.end())
		return -1;		// OpenGL uniform not found value
	return itUniform->second;		// second if the "int" value
}

void cShaderManager::cShaderProgram::LoadActiveUniforms(void)
{
	// Find out the max size of the buffer we need. 
	int maxNameSize = 0;
	glGetProgramiv(this->ID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxNameSize);
	std::cout << "Max uniform name size is : " << maxNameSize << std::endl;

	// Create a buffer of this max size
	char* uniformName = new char[maxNameSize];
	// Clear it
	memset(uniformName, 0, maxNameSize);

	// How many uniforms? 
	int numberOfActiveUniforms = 0;
	glGetProgramiv(this->ID, GL_ACTIVE_UNIFORMS, &numberOfActiveUniforms);
	std::cout << "There are " << numberOfActiveUniforms << " active uniforms" << std::endl;

	for (int index = 0; index != numberOfActiveUniforms; index++)
	{
		const int BUFFERSIZE = 1000;
		char myBuffer[BUFFERSIZE] = { 0 };

		GLsizei numCharWritten = 0;
		GLint sizeOfVariable = 0;
		GLenum uniformType = 0;
		memset(uniformName, 0, maxNameSize);

		glGetActiveUniform(this->ID,
			index,				// Uniform index (0 to ...)
			BUFFERSIZE,			// Max number of chars
			&numCharWritten,	// How many it REALLY wrote
			&sizeOfVariable,	// Size in bytes?
			&uniformType,
			uniformName);

		std::cout << "Uniform # " << index << std::endl;
		std::cout << "\t" << uniformName << std::endl;
		std::cout << "\t" << getNameStringFromType(uniformType) << std::endl;
		std::cout << "\t" << sizeOfVariable << std::endl;
	}

	return;
}

std::string cShaderManager::cShaderProgram::getNameStringFromType(int glVariableType)
{
	switch (glVariableType)
	{
	case GL_FLOAT: return "float"; break;
	case GL_FLOAT_VEC2: return "vec2"; break;
	case GL_FLOAT_VEC3: return "vec3"; break;
	case GL_FLOAT_VEC4: return "vec4"; break;
	case GL_DOUBLE: return "double"; break;
	case GL_DOUBLE_VEC2: return "dvec2"; break;
	case GL_DOUBLE_VEC3: return "dvec3"; break;
	case GL_DOUBLE_VEC4: return "dvec4"; break;
	case GL_INT: return "int"; break;
	case GL_INT_VEC2: return "ivec2"; break;
	case GL_INT_VEC3: return "ivec3"; break;
	case GL_INT_VEC4: return "ivec4"; break;
	case GL_UNSIGNED_INT: return "unsigned int"; break;
	case GL_UNSIGNED_INT_VEC2: return "uvec2"; break;
	case GL_UNSIGNED_INT_VEC3: return "uvec3"; break;
	case GL_UNSIGNED_INT_VEC4: return "uvec4"; break;
	case GL_BOOL: return "bool"; break;
	case GL_BOOL_VEC2: return "bvec2"; break;
	case GL_BOOL_VEC3: return "bvec3"; break;
	case GL_BOOL_VEC4: return "bvec4"; break;
	case GL_FLOAT_MAT2: return "mat2"; break;
	case GL_FLOAT_MAT3: return "mat3"; break;
	case GL_FLOAT_MAT4: return "mat4"; break;
	case GL_FLOAT_MAT2x3: return "mat2x3"; break;
	case GL_FLOAT_MAT2x4: return "mat2x4"; break;
	case GL_FLOAT_MAT3x2: return "mat3x2"; break;
	case GL_FLOAT_MAT3x4: return "mat3x4"; break;
	case GL_FLOAT_MAT4x2: return "mat4x2"; break;
	case GL_FLOAT_MAT4x3: return "mat4x3"; break;
	case GL_DOUBLE_MAT2: return "dmat2"; break;
	case GL_DOUBLE_MAT3: return "dmat3"; break;
	case GL_DOUBLE_MAT4: return "dmat4"; break;
	case GL_DOUBLE_MAT2x3: return "dmat2x3"; break;
	case GL_DOUBLE_MAT2x4: return "dmat2x4"; break;
	case GL_DOUBLE_MAT3x2: return "dmat3x2"; break;
	case GL_DOUBLE_MAT3x4: return "dmat3x4"; break;
	case GL_DOUBLE_MAT4x2: return "dmat4x2"; break;
	case GL_DOUBLE_MAT4x3: return "dmat4x3"; break;
	case GL_SAMPLER_1D: return "sampler1D"; break;
	case GL_SAMPLER_2D: return "sampler2D"; break;
	case GL_SAMPLER_3D: return "sampler3D"; break;
	case GL_SAMPLER_CUBE: return "samplerCube"; break;
	case GL_SAMPLER_1D_SHADOW: return "sampler1DShadow"; break;
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow"; break;
	case GL_SAMPLER_1D_ARRAY: return "sampler1DArray"; break;
	case GL_SAMPLER_2D_ARRAY: return "sampler2DArray"; break;
	case GL_SAMPLER_1D_ARRAY_SHADOW: return "sampler1DArrayShadow"; break;
	case GL_SAMPLER_2D_ARRAY_SHADOW: return "sampler2DArrayShadow"; break;
	case GL_SAMPLER_2D_MULTISAMPLE: return "sampler2DMS"; break;
	case GL_SAMPLER_2D_MULTISAMPLE_ARRAY: return "sampler2DMSArray"; break;
	case GL_SAMPLER_CUBE_SHADOW: return "samplerCubeShadow"; break;
	case GL_SAMPLER_BUFFER: return "samplerBuffer"; break;
	case GL_SAMPLER_2D_RECT: return "sampler2DRect"; break;
	case GL_SAMPLER_2D_RECT_SHADOW: return "sampler2DRectShadow"; break;
	case GL_INT_SAMPLER_1D: return "isampler1D"; break;
	case GL_INT_SAMPLER_2D: return "isampler2D"; break;
	case GL_INT_SAMPLER_3D: return "isampler3D"; break;
	case GL_INT_SAMPLER_CUBE: return "isamplerCube"; break;
	case GL_INT_SAMPLER_1D_ARRAY: return "isampler1DArray"; break;
	case GL_INT_SAMPLER_2D_ARRAY: return "isampler2DArray"; break;
	case GL_INT_SAMPLER_2D_MULTISAMPLE: return "isampler2DMS"; break;
	case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "isampler2DMSArray"; break;
	case GL_INT_SAMPLER_BUFFER: return "isamplerBuffer"; break;
	case GL_INT_SAMPLER_2D_RECT: return "isampler2DRect"; break;
	case GL_UNSIGNED_INT_SAMPLER_1D: return "usampler1D"; break;
	case GL_UNSIGNED_INT_SAMPLER_2D: return "usampler2D"; break;
	case GL_UNSIGNED_INT_SAMPLER_3D: return "usampler3D"; break;
	case GL_UNSIGNED_INT_SAMPLER_CUBE: return "usamplerCube"; break;
	case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY: return "usampler2DArray"; break;
	case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY: return "usampler2DArray"; break;
	case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE: return "usampler2DMS"; break;
	case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return "usampler2DMSArray"; break;
	case GL_UNSIGNED_INT_SAMPLER_BUFFER: return "usamplerBuffer"; break;
	case GL_UNSIGNED_INT_SAMPLER_2D_RECT: return "usampler2DRect"; break;
	case GL_IMAGE_1D: return "image1D"; break;
	case GL_IMAGE_2D: return "image2D"; break;
	case GL_IMAGE_3D: return "image3D"; break;
	case GL_IMAGE_2D_RECT: return "image2DRect"; break;
	case GL_IMAGE_CUBE: return "imageCube"; break;
	case GL_IMAGE_BUFFER: return "imageBuffer"; break;
	case GL_IMAGE_1D_ARRAY: return "image1DArray"; break;
	case GL_IMAGE_2D_ARRAY: return "image2DArray"; break;
	case GL_IMAGE_2D_MULTISAMPLE: return "image2DMS"; break;
	case GL_IMAGE_2D_MULTISAMPLE_ARRAY: return "image2DMSArray"; break;
	case GL_INT_IMAGE_1D: return "iimage1D"; break;
	case GL_INT_IMAGE_2D: return "iimage2D"; break;
	case GL_INT_IMAGE_3D: return "iimage3D"; break;
	case GL_INT_IMAGE_2D_RECT: return "iimage2DRect"; break;
	case GL_INT_IMAGE_CUBE: return "iimageCube"; break;
	case GL_INT_IMAGE_BUFFER: return "iimageBuffer"; break;
	case GL_INT_IMAGE_1D_ARRAY: return "iimage1DArray"; break;
	case GL_INT_IMAGE_2D_ARRAY: return "iimage2DArray"; break;
	case GL_INT_IMAGE_2D_MULTISAMPLE: return "iimage2DMS"; break;
	case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "iimage2DMSArray"; break;
	case GL_UNSIGNED_INT_IMAGE_1D: return "uimage1D"; break;
	case GL_UNSIGNED_INT_IMAGE_2D: return "uimage2D"; break;
	case GL_UNSIGNED_INT_IMAGE_3D: return "uimage3D"; break;
	case GL_UNSIGNED_INT_IMAGE_2D_RECT: return "uimage2DRect"; break;
	case GL_UNSIGNED_INT_IMAGE_CUBE: return "uimageCube"; break;
	case GL_UNSIGNED_INT_IMAGE_BUFFER: return "uimageBuffer"; break;
	case GL_UNSIGNED_INT_IMAGE_1D_ARRAY: return "uimage1DArray"; break;
	case GL_UNSIGNED_INT_IMAGE_2D_ARRAY: return "uimage2DArray"; break;
	case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE: return "uimage2DMS"; break;
	case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY: return "uimage2DMSArray"; break;
	case GL_UNSIGNED_INT_ATOMIC_COUNTER: return "atomic_uint"; break;

	default:
		return "Unknown";
	}

	return "Unknown";
}



int cShaderManager::cShaderProgram::getUniformLocID(std::string uniformname)
{
	std::map<std::string /*uniform name*/,
		GLint /*uniform location*/>::iterator itUniform
		= map_UniformNameToLocID.find(uniformname);

	if (itUniform != map_UniformNameToLocID.end())
	{
		return itUniform->second;
	}

	// Didn't find it.
	return -1;
}
