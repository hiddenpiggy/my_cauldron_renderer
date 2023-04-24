#ifndef SCENE_HPP
#define SCENE_HPP
#include "gltfModel.hpp"

namespace hiddenpiggy {
    struct Scene {
        glTFModel scene;
        glTFModel skyBox;
    };
}
#endif