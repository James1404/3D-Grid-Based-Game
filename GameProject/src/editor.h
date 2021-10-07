#pragma once
#ifdef _DEBUG
#include "scene.h"

namespace editor {
	extern level editor_level;

	void init();
	void update(double dt);
	void draw();
	void clean();
}

#endif // _DEBUG