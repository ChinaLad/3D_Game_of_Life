#ifndef OPENGLARTICLE_SHADER_H
#define OPENGLARTICLE_SHADER_H

#include "include/glad/gl.h"
#include <string>
#include <fstream>
#include <iostream>
#include <cerrno>

inline std::string getFileContents(const char* filename) {
    if (std::ifstream in(filename, std::ios::binary); in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

class Shader {
public:
    GLuint id_;
    Shader() {}

    void activateShader() const {glUseProgram(id_);}
    void deleteShader() const {glDeleteShader(id_);}
protected:
    static void compileErrors(const unsigned int shader, const char* type) {
        GLint hasCompiled; // store compilation status
        char infoLog[1024]; // store error message inside

        if (std::string(type) != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "SHADER_COMPILATION_ERROR for:" << type << "\n" << infoLog << std::endl;
            }
        }
        else {
            glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
            if (hasCompiled == GL_FALSE) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "SHADER_LINKING_ERROR for:" << type << "\n" << infoLog << std::endl;
            }
        }
    }
};

class RenderShader : public Shader {
public:
    RenderShader(const char* vertexPath, const char* fragmentPath) {
        // get the shader source codes as strings
        const std::string vertexCode = getFileContents(vertexPath);
        const std::string fragmentCode = getFileContents(fragmentPath);

        // turn shaders to C strings
        const char* vertexSource = vertexCode.c_str();
        const char* fragmentSource = fragmentCode.c_str();

        // create & compile vertex shader
        const unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexSource, NULL);
        glCompileShader(vertexShader);
        compileErrors(vertexShader, "VERTEX"); // check for compilation errors

        // create & compile fragment shader
        const unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
        glCompileShader(fragmentShader);
        compileErrors(fragmentShader, "FRAGMENT"); // check for compilation errors

        // create the final shader program
        id_ = glCreateProgram();
        glAttachShader(id_, vertexShader);
        glAttachShader(id_, fragmentShader);
        glLinkProgram(id_);

        compileErrors(id_, "PROGRAM"); // check for linker errors

        // delete shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }
};

class ComputeShader : public Shader {
public:
    ComputeShader(const char* computePath) {
        // get the shader source code as strings
        const std::string computeCode = getFileContents(computePath);

        // turn the shader to a C string
        const char* computeSource = computeCode.c_str();

        // create & compile compute shader
        const unsigned int computeShader = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(computeShader, 1, &computeSource, NULL);
        glCompileShader(computeShader);
        compileErrors(computeShader, "COMPUTE"); // check for compilation errors

        // create the final shader program
        id_ = glCreateProgram();
        glAttachShader(id_, computeShader);
        glLinkProgram(id_);

        compileErrors(id_, "PROGRAM"); // check for linker errors

        // delete shader objects
        glDeleteShader(computeShader);
    }
};


#endif //OPENGLARTICLE_SHADER_H