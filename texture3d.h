#ifndef OPENGLARTICLE_TEXTURE_H
#define OPENGLARTICLE_TEXTURE_H
#include "include/glad/gl.h"
#include "stb_image.h"
#include "shader.h"


class Texture3D {
public:
    GLuint id_;

    Texture3D(int gridWidth, int gridHeight, int gridDepth) {
        glGenTextures(1, &id_);
        glBindTexture(GL_TEXTURE_3D, id_);

        glTexImage3D(
            GL_TEXTURE_3D, // target texture
            0, // level of detail (lower = better quality)
            GL_R8UI, // texture color components (1 bit state + 5 bits for # neighbors)
            gridWidth, // texture width
            gridHeight, // texture height
            gridDepth, // texture depth
            0, // border
            GL_RED_INTEGER, // pixel data format
            GL_UNSIGNED_BYTE, // datatype of pixel data
            nullptr // empty texture
        );

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_3D, 0);
    }

    static void texUnit(const Shader& shader, const char* uniform, const GLuint unit) {
        const GLuint texUni = glGetUniformLocation(shader.id_, uniform);
        shader.activateShader();
        glUniform1i(texUni, unit);
    }
    void bind(GLuint unit, GLuint texture, GLenum access) const {
        glBindImageTexture(
            unit, //
            texture,
            0, // level of the texture that is to be bound
            GL_TRUE, // is the texture layered
            0,
            access, // type of access to be performed on the image
            GL_R8UI // format of image elements
        );
    }
    void unbind() const {glBindTexture(GL_TEXTURE_3D, 0);}
    void deleteTexture() const {glDeleteTextures(1, &id_);}
};

#endif //OPENGLARTICLE_TEXTURE_H