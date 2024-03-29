#include <scene.h>


DynamicEntity& PlayableEntity::getEntity() {
	return scene->getDynamicEntity(dynamic_ent_id);
}


Entity& PlayableEntity::getPointerEntity() {
	return scene->getStaticEntity(pointer_ent_id);
}


Entity& PlayableEntity::getBeamGunEntity() {
	return scene->getStaticEntity(beam_gun_ent_id);
}


const glm::vec3 PlayableEntity::eyePosition() const {
	DynamicEntity& player_ent = scene->getDynamicEntity(dynamic_ent_id);
	return player_ent.position + eye_offset;
}


void PlayableEntity::shootBall(const float dt_sec) {
	if (cooldown_remaining > 0.0f) {
		cooldown_remaining -= dt_sec;
		return;
	}

	cooldown_remaining = kWeaponCooldownSec / 2;

	DynamicEntity& player_ent = getEntity();
	DynamicEntity* projectile = scene->addDynamicEntity(
			projectile_model_id,
			0,
			player_ent.position,
			player_ent.rotation,
			0.2f,
			0.0f);
	projectile->initCollision(0.12f);
	projectile->springiness = 1.0f;
	projectile->velocity = viewDirection() * 10.0f;
}


void PlayableEntity::applyForceOnBox(bool is_active) {
	// reset the pointer
	Entity& pointer_ent = getPointerEntity();
	pointer_ent.position = eyePosition();

	if (!is_active) {
		return;
	}

	// someday we'll make this box spin
	Entity& spinny_box = scene->getStaticEntity(0);

	Ray ray{pointer_ent.position, viewDirection()};

	glm::vec3 point;
	float t_min;

	if (rayAABB(ray, spinny_box.collision.shape.box, t_min, point)) {
		pointer_ent.position = point;
		pointer_ent.scale = 0.05f;
	}
}


void PlayableEntity::shootBeam(const float dt_sec) {
	if (cooldown_remaining > 0.0f) {
		cooldown_remaining -= dt_sec;
		return;
	}

	cooldown_remaining = kWeaponCooldownSec / 1.0f;

	DynamicEntity& player_ent = getEntity();
	DynamicEntity* beam = scene->addDynamicEntity(
			beam_model_id,
			0,
			player_ent.position, // eyePosition(),
			AxisAngle::fromDirection(viewDirection()),
			0.2f,
			0.0f);
	beam->initCollision(0.12f);
	beam->springiness = 1.0f;
	beam->velocity = viewDirection() * 20.0f;

	beam->setPostAction([](DynamicEntity* self, const float dt_sec) {
		if (self->didCollide()) {
			glm::vec3 new_direction = util::safeNormalize(self->velocity);
			self->rotation = AxisAngle::fromDirection(new_direction);
		}
	});
}


void PlayableEntity::moveFromInputs(
		const float dt_sec,
		const Input::ButtonStates button_states,
		const Input::MouseState mouse_state) {
	// apply mouse movement to rotation
	// I could probably change this to axis angle as well, but it seems tricky
	view_rotation_euler.x += mouse_state.yOffset; // rotation about x axis
	view_rotation_euler.y += mouse_state.xOffset; // rotation about y axis

	// prevent breaking spine
	float max_x_angle = glm::half_pi<float>();
	view_rotation_euler.x = std::clamp(view_rotation_euler.x, -max_x_angle, max_x_angle);

	// the sign is flipped when dealing with world space
	float rotationAboutX = -view_rotation_euler.x;
	float rotationAboutY = -view_rotation_euler.y;

	glm::vec3 new_rotation_euler = glm::vec3(-view_rotation_euler.x, -view_rotation_euler.y, 0.0f);

	// apply key states to velocity
	glm::vec3 desired_direction{0.0f};

	if (button_states.forward) {
		desired_direction.z -= 1.0f;
	}
	if (button_states.reverse) {
		desired_direction.z += 1.0f;
	}
	if (button_states.left) {
		desired_direction.x -= 1.0f;
	}
	if (button_states.right) {
		desired_direction.x += 1.0f;
	}

	bool movement_input_detected = (desired_direction != glm::vec3{0.0f});
	DynamicEntity& ent = getEntity();
	bool is_already_moving = abs(ent.velocity.x) > 0.0f || abs(ent.velocity.z) > 0.0f;

	float max_speed = kMaxWalkSpeed;

	if (button_states.sprint) {
		max_speed = kMaxWalkSpeed * kSprintFactor;
	}

	float scalar_accel = max_speed * kAccelerationFactor;
	float delta_speed = scalar_accel * dt_sec;

	glm::vec3 current_velocity_xz{ent.velocity.x, 0.0f, ent.velocity.z};
	float current_speed_xz = 0.0f;

	if (is_already_moving) {
		current_speed_xz = glm::length(current_velocity_xz);
	}

	// the meat
	if (!movement_input_detected && current_speed_xz <= delta_speed) {
		// no input, moving slowly enough to just stop
		ent.velocity.x = 0.0f;
		ent.velocity.z = 0.0f;
	} else {
		glm::vec3 accel_direction{0.0f};
		glm::vec3 current_direction = glm::normalize(current_velocity_xz);

		if (!movement_input_detected) {
			// no input, but moving faster than delta_speed -> accel in direction opposite current_direction
			accel_direction = current_direction * -1.0f;

			if (!ent.isOnGround()) {
				scalar_accel = 0;
			}
		} else {
			// we'll use desired_direction, let's prepare it
			glm::mat4 y_rotation =
					glm::rotate(glm::mat4(1.0f), new_rotation_euler.y, glm::vec3(0.0f, 1.0f, 0.0f));
			desired_direction = glm::vec3(y_rotation * glm::vec4(desired_direction, 0.0f));
			desired_direction = glm::normalize(desired_direction);

			if (!is_already_moving) {
				// input, not moving -> just accel in desired_direction
				accel_direction = desired_direction;
			} else {
				// input, moving -> accel in direction that is the difference of desired and current
				if (abs(desired_direction.x - current_direction.x) > 0.1) {
					accel_direction = desired_direction - current_direction;
				} else {
					// they're already basically aligned, just go in the desired direction
					accel_direction = desired_direction;
				}
			}
		}

		glm::vec3 acceleration = accel_direction * scalar_accel;
		glm::vec3 new_velocity_xz = current_velocity_xz + (acceleration * dt_sec);
		float new_speed_xz = glm::length(new_velocity_xz);

		if (new_speed_xz > max_speed) {
			new_velocity_xz = glm::normalize(new_velocity_xz) * max_speed;
		}

		ent.velocity.x = new_velocity_xz.x;
		ent.velocity.z = new_velocity_xz.z;
	}

	// jump stuff
	if (button_states.jump && ent.isOnGround()) {
		constexpr float jump_speed = 0.8f;
		ent.velocity.y += 8.0f;
		// util::log("jumping!");
	}

	// player model should only rotate about the y axis
	glm::vec3 player_rotation_euler = glm::vec3(0.0f, new_rotation_euler.y, 0.0f);
	ent.rotation = AxisAngle::fromEulerAngles(player_rotation_euler);

	// action stuff
	if (button_states.action) {
		// shootBall(dt_sec);
		shootBeam(dt_sec);
	}

	applyForceOnBox(button_states.action);

	// move weapon
	Entity& weapon = getBeamGunEntity();
	weapon.position = ent.position;

	// actually uses x axis rotation as well as y
	weapon.rotation = AxisAngle::fromEulerAngles(new_rotation_euler);
}
