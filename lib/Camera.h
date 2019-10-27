#pragma once

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif // GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera {
private:
    glm::vec3 _loc;  // location
    glm::vec3 _look; // looking at this point
    glm::vec3 _up;

public:
    // default constructor
    Camera(void);

    // set the cameras location
    void setLocation(glm::vec3 _loc);

    // set the point that the camera is looking at
    void setLookingAt(glm::vec3 _look);

    // set the up direction for this camera
    void setUp(glm::vec3 _up);

    // get the Veiw matrix used in Model,View,Projection math
    glm::mat4 getViewTf(void);
};

Camera::Camera(void) {
    this->_up = glm::vec3(0, 1, 0);
}

void Camera::setLocation(glm::vec3 _loc) {
    this->_loc = _loc;
}

void Camera::setLookingAt(glm::vec3 _look) {
    this->_look = _look;
}

void Camera::setUp(glm::vec3 _up) {
    this->_up = _up;
}

glm::mat4 Camera::getViewTf(void) {
    return glm::lookAt(_loc, _look, _up);
}
