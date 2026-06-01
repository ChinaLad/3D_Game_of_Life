#ifndef CAMERA_H
#define CAMERA_H

#pragma once
#include <GLFW/glfw3.h>

#include "include/glad/gl.h"
#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>

#include "shader.h"


class Camera {
public:
    // camera attributes
    glm::vec3 position_;
    glm::vec3 orientation_ = glm::vec3(-1.0f, -1.0f, -1.0f);
    glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);

    // screen width & height
    int width_;
    int height_;

    // movement speed & sensitivity of the camera
    float speed_ = 0.8f;
    float sensitivity_ = 1.0f;

    bool firstClick_ = true;

    Camera(const int width, const int height, const glm::vec3 position)
        : position_(position), width_(width), height_(height) {
    }

    void matrix(
        const float FOVdeg,
        const float nearPlane,
        const float farPlane,
        const Shader &shader) const {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        view = glm::lookAt(position_, position_ + orientation_, up_);
        projection = glm::perspective(glm::radians(FOVdeg), (float)width_ / (float)height_, nearPlane, farPlane);

        // inverse matrices for ray marching (2D pixels -> 3D rays)
        glm::mat4 inverseView = glm::inverse(view);
        glm::mat4 inverseProj = glm::inverse(projection);

        glUniformMatrix4fv(glGetUniformLocation(shader.id_, "inverseView"), 1, GL_FALSE, glm::value_ptr(inverseView));
        glUniformMatrix4fv(glGetUniformLocation(shader.id_, "inverseProj"), 1, GL_FALSE, glm::value_ptr(inverseProj));

        // camera position for ray origin
        glUniform3f(glGetUniformLocation(shader.id_, "camPos"), position_.x, position_.y, position_.z);
    }

    void inputs(GLFWwindow *window) {
        // keyboard input
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            // move camera forwards on W press
            position_ += speed_ * orientation_;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            // move camera backwards on S press
            position_ -= speed_ * orientation_;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            // move camera left on A press
            position_ -= speed_ * glm::normalize(glm::cross(orientation_, up_));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            // move camera right on D press
            position_ += speed_ * glm::normalize(glm::cross(orientation_, up_));
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            // move camera up on SPACE press
            position_ += speed_ * up_;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            // move camera down on Left CTRL press
            position_ -= speed_ * up_;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            // close window on ESC press
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // mouse input
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (firstClick_) {
                glfwSetCursorPos(window, (width_ / 2), (height_ / 2));
                firstClick_ = false;
            }

            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            float rotX = sensitivity_ * (float) (mouseY - (height_ / 2)) / height_;
            float rotY = sensitivity_ * (float) (mouseX - (width_ / 2)) / width_;

            glm::vec3 newOrientation = glm::rotate(
                orientation_, glm::radians(-rotX),
                glm::normalize(glm::cross(orientation_, up_)));
            if (abs(glm::angle(newOrientation, up_) - glm::radians(90.0f)) <= glm::radians(85.0f)) {
                orientation_ = newOrientation;
            }

            // rotates left & right
            orientation_ = glm::rotate(orientation_, glm::radians(-rotY), up_);

            // keep mouse pointer in the middle
            glfwSetCursorPos(window, (width_ / 2), (height_ / 2));
        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            firstClick_ = true;
        }
    }
};
#endif