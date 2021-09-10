#pragma once
#include <string>
#include <glm.hpp>

void UpdateInput();

bool ButtonDown(std::string button);
bool ButtonPressed(std::string button);
bool ButtonReleased(std::string button);

const glm::ivec2* GetMousePos();

void SaveInput();
void LoadInput();