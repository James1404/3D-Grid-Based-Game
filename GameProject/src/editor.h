#pragma once
#ifdef _DEBUG
#include "scene.h"

namespace editor {
	void init();
	void update(double dt);
	void draw();
	void clean();
	void clear_selected();
}

#endif // _DEBUG