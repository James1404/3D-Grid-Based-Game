#pragma once
#include "scene.h"

void clear_scene(scene& t_scene);
void save_scene(scene& t_scene, const std::string& filePath);
void load_scene(scene& t_scene, const std::string& filePath);