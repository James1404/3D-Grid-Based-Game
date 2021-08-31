#pragma once
#include "Scene.h"

void ClearScene(Scene& t_scene);
void Serialize(Scene& t_scene, const std::string& filePath);
void Deserialize(Scene& t_scene, const std::string& filePath);