#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include "include/glad/gl.h"

class VBO {
    public:
        GLuint id_;
        VBO(const GLfloat* vertices, const GLsizeiptr size) {
            glGenBuffers(1, &id_);
            glBindBuffer(GL_ARRAY_BUFFER, id_);
            glBufferData(
                GL_ARRAY_BUFFER,
                size,
                vertices,
                GL_STATIC_DRAW);
        }

        void bind() const {glBindBuffer(GL_ARRAY_BUFFER, id_);}
        void unbind() {glBindBuffer(GL_ARRAY_BUFFER, 0);}
        void deleteVBO() const {glDeleteBuffers(1, &id_);}
};

#endif