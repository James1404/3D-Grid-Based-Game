#pragma once
#include "Scene.h"

class SceneSerialization
{
public:
	SceneSerialization(Scene& t_scene);

	void ClearScene();

	void Serialize(const std::string& filePath);
	void Deserialize(const std::string& filePath);
private:
	Scene* scene;
};