#include "player.h"

#include "input.h"
#include "camera.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <memory>

player_entity::player_entity()
	: model("data/models/player.gltf", "data/models/diffuse.jpg", &visual_pos, &visual_rotation, &visual_scale),
	vel(0, 0, 0), interp_speed(walk_speed)
{
	name = "player";
}

player_entity::~player_entity()
{

}

void player_entity::update(double dt)
{
#ifdef _DEBUG
	model.index = index;
#endif

	set_camera("Player");

	if (auto player_cam = get_camera("Player").lock())
	{
		if (is_first_person)
		{
			model.is_paused = true;

			glm::vec3 target_pos = ((glm::vec3)grid_pos + glm::vec3(0, .4f, 0));
			player_cam->position = lerp(player_cam->position, target_pos, camera_speed * dt);
			vel = { 0,0,0 };

			if (!(input_button_pressed("MoveUp") && input_button_pressed("MoveDown")))
			{
				if (input_button_pressed("MoveUp"))
				{
					vel.x = 1;
				}
				else if (input_button_pressed("MoveDown"))
				{
					vel.x = -1;
				}
			}

			if (!(input_button_pressed("MoveLeft") && input_button_pressed("MoveRight")))
			{
				if (input_button_pressed("MoveLeft"))
				{
					vel.y = -1;
				}
				else if (input_button_pressed("MoveRight"))
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
			// TODO: dont allow rotation until camera has reached first-person position
			player_cam->rotation = fp_look_rotation;
			//player_cam->rotation = lerp(player_cam->rotation, fp_look_rotation, camera_speed * dt);
		}
		else
		{
			model.is_paused = false;

			glm::vec3 offset = glm::vec3(0, 6, 5);

			glm::vec3 target_pos = ((glm::vec3)grid_pos + offset);
			player_cam->position = lerp(player_cam->position, target_pos, camera_speed * dt);
			player_cam->rotation = lerp(player_cam->rotation, glm::vec3(-50, -90, 0), camera_speed * dt);
		}

		/* TOP DOWN CAMERA
		glm::vec3 offset = glm::vec3(0, 4, 0);
		glm::vec3 target_pos = ((glm::vec3)grid_pos + offset);
		player_cam->position = lerp(player_cam->position, target_pos, camera_speed * dt);
		player_cam->rotation = glm::vec3(-89, -90, 0);
		*/
	}

	if (!is_moving())
	{
		if (input_key_pressed(SDL_SCANCODE_SPACE))
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

			if (!(input_button_pressed("MoveUp") && input_button_pressed("MoveDown")))
			{
				if (input_button_pressed("MoveUp"))
				{
					vel = { 0,0,-1 };
					fp_look_direction = { 0, 270, 0 };
				}
				else if (input_button_pressed("MoveDown"))
				{
					vel = { 0,0,1 };
					fp_look_direction = { 0, 90, 0 };
				}
			}

			if (!(input_button_pressed("MoveLeft") && input_button_pressed("MoveRight")))
			{
				if (input_button_pressed("MoveLeft"))
				{
					vel = { -1,0,0 };
					fp_look_direction = { 0, 180, 0 };
				}
				else if (input_button_pressed("MoveRight"))
				{
					vel = { 1,0,0 };
					fp_look_direction = { 0, 0, 0 };
				}
			}

			interp_speed = walk_speed;
			if (input_button_pressed("Run"))
			{
				interp_speed = run_speed;
			}

			move_grid_pos(vel);
		}
	}

	interp_visuals(dt, interp_speed);
}
