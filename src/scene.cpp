#include <scene.h>

void PlayableEntity::moveFromInputs(
    const float dt_sec,
    const Input::ButtonStates button_states,
    const Input::MouseState mouse_state,
    Scene* scene) {
  // apply mouse movement to rotation
  view_rotation.x += mouse_state.yOffset; // rotation about x axis
  view_rotation.y += mouse_state.xOffset; // rotation about y axis

  // prevent breaking spine
  float max_x_angle = glm::half_pi<float>();
  view_rotation.x = std::clamp(view_rotation.x, -max_x_angle, max_x_angle);

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
  bool is_already_moving = abs(velocity.x) > 0.0f || abs(velocity.z) > 0.0f;

  float max_speed = kMaxWalkSpeed;

  if (button_states.sprint) {
    max_speed = kMaxWalkSpeed * kSprintFactor;
  }

  float scalar_accel = max_speed * kAccelerationFactor;
  float delta_speed = scalar_accel * dt_sec;

  glm::vec3 current_velocity_xz{velocity.x, 0.0f, velocity.z};
  float current_speed_xz = 0.0f;

  if (is_already_moving) {
    current_speed_xz = glm::length(current_velocity_xz);
  }

  // the meat
  if (!movement_input_detected && current_speed_xz <= delta_speed) {
    // no input, moving slowly enough to just stop
    velocity.x = 0.0f;
    velocity.z = 0.0f;
  } else {
    glm::vec3 accel_direction{0.0f};
    glm::vec3 current_direction = glm::normalize(current_velocity_xz);

    if (!movement_input_detected) {
      // no input, but moving faster than delta_speed -> accel in direction opposite current_direction
      accel_direction = glm::normalize(current_direction) * -1.0f;

      if (!is_on_ground) {
        scalar_accel = 0;
      }
    } else {
      // we'll use desired_direction, let's prepare it
      glm::mat4 y_rotation =
          glm::rotate(glm::mat4(1.0f), -view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
      desired_direction = glm::vec3(y_rotation * glm::vec4(desired_direction, 1.0f));
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

    velocity.x = new_velocity_xz.x;
    velocity.z = new_velocity_xz.z;
  }

  // jump stuff
  if (button_states.jump && is_on_ground) {
    constexpr float jump_speed = 0.8f;
    velocity.y += 8.0f;
    is_on_ground = false;
    util::log("jumping!");
  }

  // player model should only rotate about the y axis
  rotation.y = -view_rotation.y;

  // shooting stuff
  if (cooldown_remaining > 0.0f) {
		cooldown_remaining -= dt_sec;
	} else if (button_states.action) {
		cooldown_remaining = kWeaponCooldownSec / 2;

    glm::vec3 projectile_pos = position;
    projectile_pos.y += 1.0;
    DynamicEntity* projectile = scene->addDynamicEntity(
        projectile_model_id,
        0,
        projectile_pos,
        rotation,
        0.2f,
        0.0f);
    projectile->initCollision(0.12f);
    projectile->springiness = 1.0f;

    glm::mat4 direction_rotation =
        glm::rotate(glm::mat4(1.0f), -view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    direction_rotation = glm::rotate(direction_rotation, -view_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 direction = glm::vec3(direction_rotation * glm::vec4(0.0, 0.0, -1.0f, 0.0f));

    projectile->velocity = direction * 10.0f;
  }
}
