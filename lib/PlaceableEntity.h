#pragma once

#include <string>

struct PlaceableEntity {

    std::string name;

    // these SHOULD be modeled as glm::mat4 types
    float model_transform[16];
    float view_transform[16];

};
