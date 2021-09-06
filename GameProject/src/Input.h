#pragma once
#include <map>
#include <string>

void SaveInput();
void LoadInput();

void UpdatePrevInput();

bool ButtonDown(std::string button);
bool ButtonPressed(std::string button);
bool ButtonReleased(std::string button);