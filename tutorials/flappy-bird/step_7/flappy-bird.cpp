#include <random>
#include <antara/gaming/collisions/basic.collision.system.hpp>
#include <antara/gaming/graphics/component.layer.hpp>
#include <antara/gaming/graphics/component.canvas.hpp>
#include <antara/gaming/math/vector.hpp>
#include <antara/gaming/scenes/scene.manager.hpp>
#include <antara/gaming/sfml/graphic.system.hpp>
#include <antara/gaming/sfml/input.system.hpp>
#include <antara/gaming/sfml/resources.manager.hpp>
#include <antara/gaming/world/world.app.hpp>
#include <antara/gaming/graphics/component.sprite.hpp>

//! For convenience
using namespace antara::gaming;

namespace {
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    float random_float(float lower, float higher) {
        std::uniform_real_distribution<float> dist(lower, higher);
        return dist(gen);
    }
}

struct flappy_bird_constants {
    // Controls
    const input::mouse_button jump_button{input::mouse_button::left};
    // Player
    const float gravity{2000.f};
    const float jump_force{500.f};

    // Pipes
    const float gap_height{200.f};
    const float column_min{0.2f};
    const float column_max{0.8f};
    const float column_thickness{100.f};
    const float column_distance{400.f};
    const std::size_t column_count{6};
    const float pipe_cap_extra_width{10.f};
    const float pipe_cap_height{50.f};
    const graphics::color pipe_color{92, 181, 61};
    const graphics::outline_color pipe_outline_color{2.0f, graphics::color{76, 47, 61}};
    const float scroll_speed{200.f};
    const std::string player_image_name{"player.png"};

    // Background
    const float ground_thickness{100.0f};
    const float grass_thickness{20.0f};
    const graphics::color background_color{82, 189, 199};
    const graphics::color ground_color{220, 209, 143};
    const graphics::color grass_color{132, 227, 90};
    const graphics::outline_color grass_outline_color{2.0f, graphics::color{76, 47, 61}};
};

// A Flappy Bird column which has two pipes
struct pipe {
    entt::entity body{entt::null};
    entt::entity cap{entt::null};

    void destroy(entt::registry &registry) {
        registry.destroy(body);
        registry.destroy(cap);
    }
};

struct column {
    //! Entities representing the Flappy Bird pipes
    pipe top_pipe{entt::null};
    pipe bottom_pipe{entt::null};

    void destroy(entt::registry &registry, entt::entity entity) {
        top_pipe.destroy(registry);
        bottom_pipe.destroy(registry);
        registry.destroy(entity);
    }
};

//! Contains all the function that will be used for logic  and factory
namespace {
    // Factory for pipes, requires to know if it's a top one, position x of the column, and the gap starting position Y
    pipe create_pipe(entt::registry &registry, bool is_top, float pos_x, float gap_start_pos_y) {
        // Retrieve constants
        const auto canvas_height = registry.ctx<graphics::canvas_2d>().canvas.size.y();
        const auto constants = registry.ctx<flappy_bird_constants>();

        // PIPE BODY
        // Top pipe is at Y: 0 and bottom pipe is at canvas_height, bottom of the canvas
        transform::position_2d body_pos{pos_x, is_top ? 0.f : canvas_height};

        // Size X is the column thickness,
        // Size Y is the important part.
        // If it's a top pipe, gap_start_pos_y should be bottom of the rectangle
        //  So half size should be gap_start_pos_y since center of the rectangle is at 0.
        // If it's the bottom pipe, top of the rectangle will be at gap_start_pos_y + gap_height
        //  So half size should be canvas_height - (gap_start_pos_y + gap_height)
        // Since these are half-sizes, and the position is at the screen border, we multiply these sizes by two
        math::vec2f body_size{constants.column_thickness,
                              is_top ?
                              gap_start_pos_y * 2.0f :
                              (canvas_height - (gap_start_pos_y + constants.gap_height)) * 2.0f};

        auto body = geometry::blueprint_rectangle(registry, body_size, constants.pipe_color, body_pos, constants.pipe_outline_color);

        // PIPE CAP
        // Let's prepare the pipe cap
        // Size of the cap is defined in constants
        math::vec2f cap_size{constants.column_thickness + constants.pipe_cap_extra_width, constants.pipe_cap_height};

        // Position, X is same as the body. Bottom of the cap is aligned with bottom of the body,
        // or start of the gap, we will use start of the gap here, minus half of the cap height
        transform::position_2d cap_pos{body_pos.x(),
                                       is_top ?
                                       gap_start_pos_y - constants.pipe_cap_height * 0.5f :
                                       gap_start_pos_y + constants.gap_height + constants.pipe_cap_height * 0.5f
        };

        // Construct the cap
        auto cap = geometry::blueprint_rectangle(registry, cap_size, constants.pipe_color, cap_pos, constants.pipe_outline_color);

        // Set layers, cap should be in front of body
        registry.assign<graphics::layer<4>>(cap);
        registry.assign<graphics::layer<3>>(body);
        registry.assign<entt::tag<"game_scene"_hs>>(cap);
        registry.assign<entt::tag<"game_scene"_hs>>(body);
        registry.assign<entt::tag<"dynamic"_hs>>(cap);
        registry.assign<entt::tag<"dynamic"_hs>>(body);

        // Construct a pipe with body and cap and return it
        return {body, cap};
    }

    // Returns a random gap start position Y
    float get_random_gap_start_pos(entt::registry &registry) {
        //! Retrieve constants
        const auto canvas_height = registry.ctx<graphics::canvas_2d>().canvas.size.y();
        const auto constants = registry.ctx<flappy_bird_constants>();

        float top_limit = canvas_height * constants.column_min;
        float bottom_limit = canvas_height * constants.column_max - constants.gap_height;

        return random_float(top_limit, bottom_limit);
    }

    // Factory to create single column
    void create_column(entt::registry &registry, float pos_x) noexcept {
        // Create a fresh entity for a new column
        auto entity_column = registry.create();

        // Get a random gap start position Y, between pipes
        float gap_start_pos_y = get_random_gap_start_pos(registry);

        // Create pipes, is_top variable is false for bottom one
        auto top_pipe = create_pipe(registry, true, pos_x, gap_start_pos_y);
        auto bottom_pipe = create_pipe(registry, false, pos_x, gap_start_pos_y);

        // Make a column from these two pipes and mark it as "column"
        registry.assign<column>(entity_column, top_pipe, bottom_pipe);
        registry.assign<entt::tag<"column"_hs>>(entity_column);
        registry.assign<entt::tag<"game_scene"_hs>>(entity_column);
        registry.assign<entt::tag<"dynamic"_hs>>(entity_column);
    }

    //! Factory for creating a Flappy Bird columns
    void create_columns(entt::registry &registry) noexcept {
        //! Retrieve constants
        const auto canvas_width = registry.ctx<graphics::canvas_2d>().canvas.size.x();
        const auto constants = registry.ctx<flappy_bird_constants>();

        // Spawn columns out of the screen, out of the canvas
        const float column_pos_offset = canvas_width + constants.column_thickness * 2.0f;

        // Create the columns
        for(std::size_t i = 0; i < constants.column_count; ++i) {
            // Horizontal position (X) increases for every column, keeping the distance
            float pos_x = column_pos_offset + i * constants.column_distance;

            create_column(registry, pos_x);
        }
    }

    //! Factory for creating a Flappy Bird background
    void create_background(entt::registry &registry) noexcept {
        //! Retrieve constants
        const auto[canvas_width, canvas_height] = registry.ctx<graphics::canvas_2d>().canvas.size;
        const auto constants = registry.ctx<flappy_bird_constants>();

        // Create Sky
        {
            // Sky is whole canvas so position is middle of it
            transform::position_2d pos{canvas_width * 0.5f, canvas_height * 0.5f};

            // And the size is full canvas
            math::vec2f size{canvas_width, canvas_height};

            auto sky = geometry::blueprint_rectangle(registry, size, constants.background_color, pos);
            registry.assign<graphics::layer<1>>(sky);
            registry.assign<entt::tag<"game_scene"_hs>>(sky);
        }

        // Create Grass
        {
            // Ground expands to whole canvas width so position is middle of it,
            // But position Y is at top of the ground, so it's canvas height minus ground thickness
            transform::position_2d pos{canvas_width * 0.5f, canvas_height - constants.ground_thickness};

            // Size X is full canvas but the height is defined in constants
            // We also make it a bit longer by adding the thickness of the outline to hide the outline at sides
            math::vec2f size{canvas_width + constants.grass_outline_color.thickness * 2.0f, constants.grass_thickness};

            auto grass = geometry::blueprint_rectangle(registry, size, constants.grass_color, pos, constants.grass_outline_color);
            registry.assign<graphics::layer<3>>(grass);
            registry.assign<entt::tag<"game_scene"_hs>>(grass);
        }

        // Create Ground
        {
            // Ground expands to whole canvas width so position is middle of it,
            // But position Y is at bottom of the screen so it's full canvas_height minus half of the ground thickness
            transform::position_2d pos{canvas_width * 0.5f, canvas_height - constants.ground_thickness * 0.5f};

            // Size X is full canvas but the height is defined in constants
            math::vec2f size{canvas_width, constants.ground_thickness};

            auto ground = geometry::blueprint_rectangle(registry, size, constants.ground_color, pos);
            registry.assign<graphics::layer<3>>(ground);
            registry.assign<entt::tag<"game_scene"_hs>>(ground);
        }
    }

    //! Factory for creating the player
    entt::entity create_player(entt::registry &registry) {
        //! Retrieve constants
        const auto [canvas_width, canvas_height] = registry.ctx<graphics::canvas_2d>().canvas.size;
        const auto constants = registry.ctx<flappy_bird_constants>();

        auto entity = graphics::blueprint_sprite(registry,
                                                        graphics::sprite{constants.player_image_name.c_str()},
                                                        transform::position_2d{canvas_width * 0.2f, canvas_height * 0.2f});
        registry.assign<antara::gaming::graphics::layer<5>>(entity);
        registry.assign<entt::tag<"player"_hs>>(entity);
        registry.assign<entt::tag<"game_scene"_hs>>(entity);
        registry.assign<entt::tag<"dynamic"_hs>>(entity);

        return entity;
    }
}

class column_logic final : public ecs::logic_update_system<column_logic> {
public:
    explicit column_logic(entt::registry &registry) noexcept : system(registry) { }

    void update() noexcept final {
        auto& registry = entity_registry_;

        //! Retrieve constants
        const auto constants = registry.ctx<flappy_bird_constants>();

        // Loop all columns
        for(auto entity : registry.view<column>()) {
            auto& col = registry.get<column>(entity);

            // Move pipes, and check if they are out of the screen
            bool col_out_of_screen = move_pipe(registry, col.top_pipe) || move_pipe(registry, col.bottom_pipe);

            // If column is out of the screen
            if(col_out_of_screen) {
                // Remove this column
                col.destroy(registry, entity);

                // Create a new column at far end
                create_column(registry, furthest_pipe_position(registry) + constants.column_distance);
            }
        }
    }

private:
    // Find the most far pipe's position X
    float furthest_pipe_position(entt::registry &registry) {
        float furthest = 0.f;

        auto view = registry.view<column>();

        for(auto entity : view) {
            auto& col = registry.get<column>(entity);
            float x = entity_registry_.get<transform::position_2d>(col.top_pipe.body).x();
            if(x > furthest) furthest = x;
        }

        return furthest;
    }

    // Move the pipe and return if it's out of the screen
    bool move_pipe(entt::registry &registry, pipe& pipe) {
        // Retrieve constants
        const auto constants = registry.ctx<flappy_bird_constants>();

        // Get current position of the pipe
        auto pos = registry.get<transform::position_2d>(pipe.body);

        // Shift pos X to left by scroll_speed but multiplying with dt because we do this so many times a second,
        // Delta time makes sure that it's applying over time, so in one second it will move scroll_speed pixels
        auto new_pos_x = pos.x() - constants.scroll_speed * timer::time_step::get_fixed_delta_time();

        // Set the new position value
        registry.assign_or_replace<transform::position_2d>(pipe.body, new_pos_x, pos.y());

        // Set cap position too
        auto cap_pos = registry.get<transform::position_2d>(pipe.cap);
        registry.assign_or_replace<transform::position_2d>(pipe.cap, new_pos_x, cap_pos.y());

        // Return the info about if this pipe is out of the screen
        return new_pos_x < -constants.column_thickness * 2.0f;
    }
};

//! Give a name to our system
REFL_AUTO(type(column_logic));

class player_logic final : public ecs::logic_update_system<player_logic> {
public:
    player_logic(entt::registry &registry, entt::entity player_) noexcept : system(registry), player(player_) { }

    void update() noexcept final {
        auto& registry = entity_registry_;

        //! Retrieve constants
        const auto constants = registry.ctx<flappy_bird_constants>();

        // Get current position of the pipe
        auto pos = registry.get<transform::position_2d>(player);

        // Add gravity to movement speed, multiply with delta time to apply it over time
        movement_speed.set_y(movement_speed.y() + constants.gravity * timer::time_step::get_fixed_delta_time());

        // Check if jump key is tapped
        bool jump_key_pressed = input::is_mouse_button_pressed(constants.jump_button);
        bool jump_key_tapped = jump_key_pressed && !jump_key_pressed_last_tick;
        jump_key_pressed_last_tick = jump_key_pressed;

        // If jump is tapped, add jump force to the movement speed
        if (jump_key_tapped) movement_speed.set_y(-constants.jump_force);

        // Add movement speed to position, but apply over time with delta time
        pos += movement_speed * timer::time_step::get_fixed_delta_time();

        // Set the new position value
        registry.assign_or_replace<transform::position_2d>(player, pos);
    }

private:
    entt::entity player;
    math::vec2f movement_speed{0.f, 0.f};
    bool jump_key_pressed_last_tick = false;
};

//! Give a name to our system
REFL_AUTO(type(player_logic));

class collision_logic final : public ecs::logic_update_system<collision_logic> {
public:
    collision_logic(entt::registry &registry, entt::entity player_, bool& player_died_) noexcept : system(registry), player(player_), player_died(player_died_) { }

    void update() noexcept final {
        auto& registry = entity_registry_;

        // Do not check anything if player is already dead
        if(player_died) return;

        // Loop all columns to check collisions with the pipes
        for(auto entity : registry.view<graphics::layer<3>>()) {
            // Check collision between player and a collidable object
            if(collisions::basic_collision_system::query_rect(registry, player, entity)) {
                // Mark player died as true
                player_died = true;
            }
        }
    }

private:
    entt::entity player;
    bool& player_died;
};

//! Give a name to our system
REFL_AUTO(type(collision_logic));

class game_scene final : public scenes::base_scene {
public:
    game_scene(entt::registry &registry, ecs::system_manager& system_manager_) noexcept : base_scene(registry), system_manager(system_manager_) {
        //! Set the constants that will be used in the program
        registry.set<flappy_bird_constants>();

        //! Create the columns
        create_background(registry);
        init_dynamic_objects(registry);
    }

    //! Scene name
    std::string scene_name() noexcept final {
        return "game_scene";
    }

private:
    //! Update the game every tick
    void update() noexcept final {
        // Retrieve constants
        const auto constants = entity_registry_.ctx<flappy_bird_constants>();

        // Check if jump key is tapped
        bool jump_key_pressed = input::is_mouse_button_pressed(constants.jump_button);
        bool jump_key_tapped = jump_key_pressed && !jump_key_pressed_last_tick;
        jump_key_pressed_last_tick = jump_key_pressed;

        // If game is not started yet and jump key is tapped
        if(!started_playing && jump_key_tapped) {
            // Game starts, player started playing
            started_playing = true;
            resume_physics();
        }

        // If player died, game over, and pause physics
        if(player_died) {
            player_died = false;
            game_over = true;
            pause_physics();
        }

        // If game is over, and jump key is pressed, reset game
        if(game_over && jump_key_pressed) reset_game();
    }

    void init_dynamic_objects(entt::registry &registry) {
        create_columns(registry);

        auto player = create_player(registry);

        //! Create systems
        system_manager.create_system<column_logic>();
        system_manager.create_system<player_logic>(player);

        // Disable physics and everything at start
        pause_physics();

        // Collision system
        system_manager.create_system<collision_logic>(player, player_died);

        // Reset state values
        started_playing = false;
        player_died = false;
        game_over = false;
    }

    void pause_physics() {
        system_manager.disable_systems<column_logic, player_logic>();
    }

    void resume_physics() {
        system_manager.enable_systems<column_logic, player_logic>();
    }

    void destroy_all() {
        //! Retrieve the collection of entities from the game scene
        auto view = entity_registry_.view<entt::tag<"dynamic"_hs>>();

        //! Iterate the collection and destroy each entities
        entity_registry_.destroy(view.begin(), view.end());

        //! Delete systems
        system_manager.mark_systems<player_logic, collision_logic>();
    }

    void reset_game() {
        destroy_all();
        this->need_reset = true;
    }

    void post_update() noexcept final {
        if(need_reset) {
            //! Reinitialize all these
            init_dynamic_objects(entity_registry_);
            need_reset = false;
        }
    }

    ecs::system_manager& system_manager;

    // States
    bool started_playing{false};
    bool player_died{false};
    bool game_over{false};
    bool jump_key_pressed_last_tick{false};
    bool need_reset{false};
};

//! Game world
struct flappy_bird_world : world::app {
    //! Game entry point
    flappy_bird_world() noexcept {
        //! Load our graphical system
        auto &graphic_system = system_manager_.create_system<sfml::graphic_system>();

        //! Load our resources system
        entity_registry_.set<antara::gaming::sfml::resources_system>(entity_registry_);

        //! Load our input system with the window from the graphical system
        system_manager_.create_system<sfml::input_system>(graphic_system.get_window());

        //! Load the scenes manager
        auto &scene_manager = system_manager_.create_system<scenes::manager>();

        //! Change the current_scene to "game_scene" by pushing it.
        scene_manager.change_scene(std::make_unique<game_scene>(entity_registry_, system_manager_), true);
    }
};

int main() {
    //! Declare our world
    flappy_bird_world game;

    //! Run the game
    return game.run();
}