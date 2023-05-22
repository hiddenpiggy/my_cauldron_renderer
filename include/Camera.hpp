#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/ext/quaternion_geometric.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "UniformBuffers.hpp"


namespace hiddenpiggy {
    class Camera {
    public:
        Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldup) : position(position), worldUp(worldup) {
            forward = glm::normalize(target-position);
            right = glm::normalize(glm::cross(forward, worldUp));
            up = glm::normalize(glm::cross(right, forward));
        }


        glm::mat4 getViewMatrix() {
            return glm::lookAt(position, position+forward, worldUp);
        }

        glm::mat4 getProjectionMatrix() {
            return glm::perspective(glm::radians(fov), aspectRatio, znear, zfar);
        }

        void setPerspectiveParameters(float fov, float znear, float zfar, float aspectRatio) {
            this->fov = fov;
            this->znear = znear;
            this->zfar = zfar;
            this->aspectRatio = aspectRatio;
        }

        void setViewParameters(glm::vec3 position, glm::vec3 target, glm::vec3 worldup) {
            this->position = position;
            this->worldUp = worldUp;
            this->forward = glm::normalize(target-position);
            this->right = glm::normalize(glm::cross(forward, worldup));
            this->up = glm::normalize(glm::cross(right, forward));
        }


        void setSensitivity(float sensitivity) {
            this->m_sensitivity = sensitivity;
        }


        void RotationAroundUpdate(glm::vec3 delta, float sensitivity, const glm::vec3& target) {
            float distance = glm::length(target - position);
            this->position += sensitivity * (delta.x * right + delta.y * up);

            glm::vec3 newOutDir = glm::normalize(position - target);
            this->position = target + newOutDir * distance;

            this->forward = glm::normalize(target-position);
            this->right = glm::normalize(glm::cross(forward, this->worldUp));
            this->up = glm::normalize(glm::cross(right, forward));
        }


    private:
        float fov;  //this is degree
        float znear, zfar;
        float aspectRatio;

        glm::vec3 position = glm::vec3();
        glm::vec3 forward = glm::vec3();
        glm::vec3 right = glm::vec3();
        glm::vec3 up = glm::vec3();
        glm::vec3 worldUp = glm::vec3();

        float m_sensitivity = 0.1f;

        struct {
            glm::mat4 perspective;
            glm::mat4 view;
        } matrices;

    };
}
#endif