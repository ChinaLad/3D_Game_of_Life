#ifndef OPENGLARTICLE_EBO_H
#define OPENGLARTICLE_EBO_H

#include "include/glad/gl.h"
#include <glm/glm.hpp>
#include <vector>

class EBO {
public:
    GLuint id_;
    EBO(const GLuint* indices, const GLsizeiptr size) {
        glGenBuffers(1, &id_);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            size,
            indices,
            GL_STATIC_DRAW);
    }

    void bind() const {glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);}
    void unbind() {glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);}
    void deleteEBO() const {glDeleteBuffers(1, &id_);}
};


#endif //OPENGLARTICLE_EBO_H