#pragma once
#ifndef SHADERPROGRAM_H_
#define SHADERPROGRAM_H_

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <memory>
#include <exception>

#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "../Core/Resource.h"

namespace NobleResources
{
	/**
	*Stores a shader source code file.
	*/
	struct Shader : public NobleCore::Resource
	{
		std::shared_ptr<std::string> shaderCode;

		void OnLoad()
		{
			std::ifstream shaderFile;
			shaderFile.open(resourcePath);

			if (!shaderFile.is_open())
			{
				std::cout << "Couldn't find shader file at " << resourcePath << std::endl;
				throw std::exception();
			}
			std::stringstream shaderStream;
			shaderStream << shaderFile.rdbuf();
			shaderFile.close();

			shaderCode = std::make_shared<std::string>(shaderStream.str());
		}
	};

	/**
	*Stores a shader location GLint. It is more optimal to store these rather than to find them every frame.
	*/
	struct ShaderLocation
	{
		std::string locationName;
		GLint locationID;

		static std::shared_ptr<ShaderLocation> CreateLocation(GLuint _shaderProgram, std::string _location)
		{
			std::shared_ptr<ShaderLocation> rtn = std::make_shared<ShaderLocation>();
			rtn->locationName = _location;
			rtn->locationID = glGetUniformLocation(_shaderProgram, _location.c_str());
			if (rtn->locationID == -1)
			{
				//std::cout << "Couldn't find location " << location << " in shader program " << shaderProgram << std::endl;
				//return a null ptr here and dont add to the locations list.
			}

			return rtn;
		}
	};
	/**
	*Made up of several shader resources, these are used in rendering graphics within the engine. 
	*/
	struct ShaderProgram
	{
		GLuint programID;

		std::weak_ptr<ShaderProgram> self;
		std::vector<std::shared_ptr<ShaderLocation>> shaderLocations;

	public:
		ShaderProgram()
		{
			programID = glCreateProgram();
		}
		/**
		*Gets a location from the list, or if it does not contain it creates and adds it to list.
		*/
		GLint GetLocation(std::string _location)
		{
			for (size_t i = 0; i < shaderLocations.size(); i++)
			{
				if (shaderLocations.at(i)->locationName == _location)
				{
					return shaderLocations.at(i)->locationID;
				}
			}

			std::shared_ptr<ShaderLocation> rtn = ShaderLocation::CreateLocation(programID, _location);
			shaderLocations.push_back(rtn);
			return rtn->locationID;
		}
		/**
		*Binds a shader resource to the shader program.
		*/
		void BindShader(std::shared_ptr<Shader> _shader, GLenum _shaderType)
		{
			const GLchar* shaderSource = _shader->shaderCode->c_str();

			GLuint shaderID = glCreateShader(_shaderType);
			glShaderSource(shaderID, 1, &shaderSource, NULL);
			glCompileShader(shaderID);
			GLint success = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				std::cout << "Failed to compile shader with path " << _shader->resourcePath << "!" << std::endl;

				GLint maxLength = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

				for (int i = 0; i < errorLog.size(); i++)
				{
					std::cout << errorLog[i];
				}

				glDeleteShader(shaderID);
				throw std::exception();
			}

			glAttachShader(programID, shaderID);
		}
		/**
		*Binds a shader source code string to the shader program.
		*/
		void BindShader(const GLchar* _shaderSourceString, GLenum _shaderType)
		{
			GLuint shaderID = glCreateShader(_shaderType);
			glShaderSource(shaderID, 1, &_shaderSourceString, NULL);
			glCompileShader(shaderID);
			GLint success = 0;
			glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				std::cout << "Failed to compile shader with path passed by string!" << std::endl;

				GLint maxLength = 0;
				glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(shaderID, maxLength, &maxLength, &errorLog[0]);

				for (int i = 0; i < errorLog.size(); i++)
				{
					std::cout << errorLog[i];
				}

				glDeleteShader(shaderID);
				throw std::exception();
			}

			glAttachShader(programID, shaderID);
		}
		/**
		*This function binds an integer to a uniform int in the shader.
		*/
		void BindInt(std::string _location, int _value)
		{
			GLint intLocation = GetLocation(_location);
			glUniform1i(intLocation, _value);
		}
		/**
		*This function binds an float to a uniform int in the shader.
		*/
		void BindFloat(std::string _location, float _value)
		{
			GLint floatLocation = GetLocation(_location);
			glUniform1f(floatLocation, _value);
		}
		/**
		*This function binds an matrix4 to a uniform matrix4 in the shader.
		*/
		void BindMat4(std::string _location, glm::mat4 _matrix)
		{
			GLint matrixLocation = GetLocation(_location);
			glUniformMatrix4fv(matrixLocation, 1, GL_FALSE, glm::value_ptr(_matrix));
		}
		/**
		*This function binds an vec3 to a uniform vec3 in the shader.
		*/
		void BindVector3(std::string _location, glm::vec3 _vector)
		{
			GLint vectorLocation = GetLocation(_location);
			glUniform3f(vectorLocation, _vector.x, _vector.y, _vector.z);
		}
		/**
		*Links the program for use in the engine. Also responsible for setting up the uniform variables in the shader code. If this function isn't called then the shader program cannot be used.
		*/
		void LinkShaderProgram(std::shared_ptr<ShaderProgram> _selfPtr)
		{
			self = _selfPtr;

			GLint success = 0;
			glLinkProgram(programID);
			glGetProgramiv(programID, GL_LINK_STATUS, &success);

			if (success == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> errorLog(maxLength);
				glGetShaderInfoLog(programID, maxLength, &maxLength, &errorLog[0]);

				for (int i = 0; i < errorLog.size(); i++)
				{
					std::cout << errorLog[i];
				}

				glDeleteProgram(programID);
				throw std::exception();
			}
		}
		/**
		*Sets the currently used shader program to this.
		*/
		void UseProgram()
		{
			glUseProgram(programID);
		}
	};
}
#endif