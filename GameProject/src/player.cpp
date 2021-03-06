#include "player.h"

#include "input.h"
#include "camera.h"
#include "world.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

void player_entity::init()
{
	grid_pos = glm::ivec3(0);
	visual_transform = transform_t();

	vel = glm::vec3(0, 0, 0);
	interp_speed = walk_speed;

	renderer_t::get().add_instance("data/models/player.gltf", "data/models/diffuse.jpg", "data/shaders/model_loading.glsl", false, &visual_transform, 0);
}

void player_entity::move_grid_pos(glm::ivec3 _dir)
{
	if (is_moving(visual_transform, grid_pos))
	{
		return;
	}

	_dir.y = 0;
	glm::ivec3 new_pos = grid_pos + _dir;
	if (is_grounded(grid_pos))
	{
		if (!world_t::get().check_collision(new_pos))
		{
			if(!world_t::get().check_collision(new_pos + glm::ivec3(0,1,0)))
			{
				if (is_grounded(new_pos))
				{
					set_grid_pos(grid_pos, previous_grid_pos, new_pos);
				}
				else
				{
					if (!world_t::get().check_collision(new_pos + glm::ivec3(0, -1, 0)))
					{
						if (is_grounded(new_pos + glm::ivec3(0, -1, 0)))
							set_grid_pos(grid_pos, previous_grid_pos, new_pos + glm::ivec3(0, -1, 0));
					}
				}
			}
		}
		else
		{
			// TODO: make no climb work
			if (!world_t::get().check_collision(grid_pos + glm::ivec3(0, 1, 0)))
			{
				if (!world_t::get().check_collision(grid_pos + glm::ivec3(0, 2, 0)))
				{
					if (!world_t::get().check_collision(new_pos + glm::ivec3(0, 1, 0)))
					{
						set_grid_pos(grid_pos, previous_grid_pos, new_pos + glm::ivec3(0, 1, 0));
					}
				}
			}
		}
	}
}

void player_entity::update(double dt)
{
	camera_manager_t::get().set_camera("Player");

	if (auto player_cam = camera_manager_t::get().get_camera("Player").lock())
	{
		if (is_first_person)
		{
			//model.is_paused = true;
			visual_transform.scale = glm::vec3(0);

			glm::vec3 target_pos = ((glm::vec3)grid_pos + glm::vec3(0, 1, 0));
			player_cam->position = target_pos;
			vel = { 0,0,0 };

			if (!(input_t::get().button_pressed("MoveUp") && input_t::get().button_pressed("MoveDown")))
			{
				if (input_t::get().button_pressed("MoveUp"))
				{
					vel.x = 1;
				}
				else if (input_t::get().button_pressed("MoveDown"))
				{
					vel.x = -1;
				}
			}

			if (!(input_t::get().button_pressed("MoveLeft") && input_t::get().button_pressed("MoveRight")))
			{
				if (input_t::get().button_pressed("MoveLeft"))
				{
					vel.y = -1;
				}
				else if (input_t::get().button_pressed("MoveRight"))
				{
					vel.y = 1;
				}
			}

			vel = sqrt_magnitude(vel);
			vel *= 0.1f;
			vel *= dt;

			fp_look_rotation += vel;

			if (fp_look_rotation.x > 50.0f)
				fp_look_rotation.x = 50.0f;
			if (fp_look_rotation.x < -60.0f)
				fp_look_rotation.x = -60.0f;

			if (fp_look_rotation.y > 360.0f)
				fp_look_rotation.y = 0.0f;
			if (fp_look_rotation.y < 0.0f)
				fp_look_rotation.y = 360.0f;

			player_cam->rotation = fp_look_rotation;
		}
		else
		{
			//model.is_paused = false;
			visual_transform.scale = glm::vec3(1);

			bool is_wall_collision = false;
			for (int i = 0; i < 6; i++)
			{
				//add_primitive_wireframe_cube(grid_pos + glm::ivec3(0, i, i), glm::vec3(1), colour::cyan);
				if (world_t::get().check_collision(grid_pos + glm::ivec3(0, i, i)))
				{
					is_wall_collision = true;
				}
			}

			if (is_wall_collision)
			{
				camera_view_type = camera_view_wall_infront;
			}
			else
			{
				camera_view_type = camera_view_default;
			}

			switch(camera_view_type)
			{
			case camera_view_default:
				{
					glm::vec3 offset = glm::vec3(0, 5, 6);
					glm::vec3 target_pos = ((glm::vec3)grid_pos + offset);
					player_cam->position = lerp(player_cam->position, target_pos, camera_speed * dt);
					player_cam->rotation = glm::vec3(-30, -90, 0);
				} break;
			case camera_view_wall_infront:
				{
					glm::vec3 offset = glm::vec3(0, 6, 0);
					glm::vec3 target_pos = ((glm::vec3)grid_pos + offset);
					player_cam->position = lerp(player_cam->position, target_pos, camera_speed * dt);
					player_cam->rotation = glm::vec3(-89, -90, 0);
				} break;
			}
		}
	}

	if (!is_moving(visual_transform, grid_pos))
	{
		if (input_t::get().key_pressed(SDL_SCANCODE_SPACE))
		{
			if (!is_first_person)
			{
				is_first_person = true;
				fp_look_rotation = fp_look_direction;
			}
		}
		else
		{
			is_first_person = false;
		}

		if (!is_first_person)
		{
			vel = { 0,0,0 };

			if (!(input_t::get().button_pressed("MoveUp") && input_t::get().button_pressed("MoveDown")))
			{
				if (input_t::get().button_pressed("MoveUp"))
				{
					vel = { 0,0,-1 };
					fp_look_direction = { 0, 270, 0 };
				}
				else if (input_t::get().button_pressed("MoveDown"))
				{
					vel = { 0,0,1 };
					fp_look_direction = { 0, 90, 0 };
				}
			}

			if (!(input_t::get().button_pressed("MoveLeft") && input_t::get().button_pressed("MoveRight")))
			{
				if (input_t::get().button_pressed("MoveLeft"))
				{
					vel = { -1,0,0 };
					fp_look_direction = { 0, 180, 0 };
				}
				else if (input_t::get().button_pressed("MoveRight"))
				{
					vel = { 1,0,0 };
					fp_look_direction = { 0, 0, 0 };
				}
			}

			interp_speed = walk_speed;
			if (input_t::get().button_pressed("Run"))
			{
				interp_speed = run_speed;
			}

			move_grid_pos(vel);
		}
	}

	interp_visuals(visual_transform, grid_pos, dt, interp_speed);
}

bool player_entity::is_grounded(glm::ivec3 _pos)
{
	return world_t::get().check_collision(_pos + glm::ivec3(0, -1, 0));
}
