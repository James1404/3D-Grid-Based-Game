#pragma once
#include "camera.h"
#include "entity.h"
#include "input.h"

void init_editor(entity_manager_t& _manager);
void shutdown_editor();

void handle_editor_events();

void update_editor(double dt);
void draw_editor();

// framebuffer
void init_editor_framebuffer();
void shutdown_editor_framebuffer();
void bind_editor_framebuffer();
void unbind_editor_framebuffer();
void draw_editor_framebuffer();
