#pragma once
#ifdef _DEBUG
#include "scene.h"

namespace editor {
	static scene editor_scene;

	void init();
	void update(double dt);
	void draw();
	void clean();
}

#endif // _DEBUG