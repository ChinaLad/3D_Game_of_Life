#ifndef OPENGLARTICLE_VAO_H
#define OPENGLARTICLE_VAO_H

#include "include/glad/gl.h"
#include "vbo.h"

class VAO {
public:
    GLuint id_;
    VAO() {glGenVertexArrays(1, &id_);}

    static void linkAttrib(VBO& vbo, const GLuint layout, const GLuint numComponents, const GLenum type, const GLsizeiptr stride, const void* offset) {
        vbo.bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);
        vbo.unbind();
    }
    void bind() const {glBindVertexArray(id_);}
    void unbind() {glBindVertexArray(0);}
    void deleteVAO() const {glDeleteVertexArrays(1, &id_);}
};


#endif //OPENGLARTICLE_VAO_H