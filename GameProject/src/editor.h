#pragma once
#ifdef _DEBUG
#include "scene.h"

namespace editor {
	void init();
	void update(double dt);
	void draw();
	void clean();
}

#endif // _DEBUG