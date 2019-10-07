/******************************************************************************
 * Copyright © 2013-2019 The Komodo Platform Developers.                      *
 *                                                                            *
 * See the AUTHORS, DEVELOPER-AGREEMENT and LICENSE files at                  *
 * the top-level directory of this distribution for the individual copyright  *
 * holder information and the developer policies on copyright and licensing.  *
 *                                                                            *
 * Unless otherwise agreed in a custom licensing agreement, no part of the    *
 * Komodo Platform software, including this file may be copied, modified,     *
 * propagated or distributed except according to the terms contained in the   *
 * LICENSE file                                                               *
 *                                                                            *
 * Removal or modification of this copyright notice is prohibited.            *
 *                                                                            *
 ******************************************************************************/

#include <cmath>
#include <SFML/Graphics/RenderTexture.hpp>
#include <entt/entity/helper.hpp>
#include <antara/gaming/ecs/component.layer.hpp>
#include <antara/gaming/ecs/component.position.hpp>
#include <antara/gaming/sfml/component.drawable.hpp>
#include "tic.tac.toe.factory.hpp"
#include "tic.tac.toe.constants.hpp"
#include "tic.tac.toe.components.hpp"

namespace tictactoe ::example
{
    using namespace antara::gaming;

    entt::entity tic_tac_toe_factory::create_grid_entity(entt::registry &entity_registry) noexcept
    {
        auto window_info = entity_registry.ctx<sf::RenderTexture>().getSize();
        auto grid_entity = entity_registry.create();
        auto &grid_cmp = entity_registry.assign<sfml::vertex_array>(grid_entity, sf::VertexArray(sf::Lines, 16));
        sf::VertexArray &lines = grid_cmp.drawable;

        auto constants = entity_registry.ctx<tic_tac_toe_constants>();
        for (std::size_t counter = 0, i = 0; i <= constants.nb_cells; ++i, counter += 4) {
            lines[counter].position = sf::Vector2f(i * constants.cell_width, 0);
            lines[counter + 1].position = sf::Vector2f(i * constants.cell_width, window_info.y);
            lines[counter + 2].position = sf::Vector2f(0, i * constants.cell_height);
            lines[counter + 3].position = sf::Vector2f(window_info.x, i * constants.cell_height);
        }

        entity_registry.assign<entt::tag<"grid"_hs>>(grid_entity);
        entity_registry.assign<entt::tag<"game_scene"_hs>>(grid_entity);
        entity_registry.assign<ecs::component::layer<0>>(grid_entity);
        return grid_entity;
    }

    entt::entity tic_tac_toe_factory::create_board(entt::registry &entity_registry) noexcept
    {
        auto board_entity = entity_registry.create();
        auto constants = entity_registry.ctx<tic_tac_toe_constants>();
        entity_registry.assign<board_component>(board_entity, constants.nb_cells);
        entity_registry.assign<entt::tag<"game_scene"_hs>>(board_entity);
        return board_entity;
    }

    entt::entity
    tic_tac_toe_factory::create_x(entt::registry &entity_registry, std::size_t row, std::size_t column) noexcept
    {
        auto constants = entity_registry.ctx<tic_tac_toe_constants>();
        const float half_box_side = static_cast<float>(std::fmin(constants.cell_width, constants.cell_height) * 0.25f);
        const float center_x = static_cast<float>(constants.cell_width * 0.5 + column * constants.cell_width);
        const float center_y = static_cast<float>(constants.cell_height * 0.5 + row * constants.cell_height);

        auto x_entity = entity_registry.create();
        entity_registry.assign<cell_position>(x_entity, row, column);
        auto &cross_cmp = entity_registry.assign<sfml::vertex_array>(x_entity,
                                                                     sf::VertexArray(sf::Lines, 4));

        sf::VertexArray &lines = cross_cmp.drawable;

        lines[0].color = sf::Color::Red;
        lines[0].position = sf::Vector2f(center_x - half_box_side, center_y - half_box_side);
        lines[1].color = sf::Color::Red;
        lines[1].position = sf::Vector2f(center_x + half_box_side, center_y + half_box_side);

        lines[2].color = sf::Color::Red;
        lines[2].position = sf::Vector2f(center_x + half_box_side, center_y - half_box_side);
        lines[3].color = sf::Color::Red;
        lines[3].position = sf::Vector2f(center_x - half_box_side, center_y + half_box_side);

        entity_registry.assign<entt::tag<"game_scene"_hs>>(x_entity);
        entity_registry.assign<entt::tag<"player_x"_hs>>(x_entity);
        entity_registry.assign<ecs::component::layer<1>>(x_entity);
        return x_entity;
    }

    entt::entity
    tic_tac_toe_factory::create_o(entt::registry &entity_registry, std::size_t row, std::size_t column) noexcept
    {
        auto constants = entity_registry.ctx<tic_tac_toe_constants>();
        const auto half_box_side = static_cast<float>(std::fmin(constants.cell_width, constants.cell_height) * 0.25f);
        const auto center_x = static_cast<float>(constants.cell_width * 0.5 + column * constants.cell_width);
        const auto center_y = static_cast<float>(constants.cell_height * 0.5 + row * constants.cell_height);

        auto o_entity = entity_registry.create();
        auto &circle_cmp = entity_registry.assign<sfml::circle>(o_entity,
                                                                sf::CircleShape(half_box_side));
        sf::CircleShape &circle = circle_cmp.drawable;
        entity_registry.assign<ecs::component::position>(o_entity,
                                                         center_x - circle.getGlobalBounds().width / 2,
                                                         center_y -
                                                         circle.getGlobalBounds().height / 2);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineThickness(1.f);
        circle.setOutlineColor(sf::Color::Blue);

        entity_registry.assign<entt::tag<"game_scene"_hs>>(o_entity);
        entity_registry.assign<ecs::component::layer<1>>(o_entity);
        return o_entity;
    }

    void tic_tac_toe_factory::reset_x(entt::registry &entity_registry) noexcept
    {
        auto constants = entity_registry.ctx<tic_tac_toe_constants>();
        entity_registry.view<antara::gaming::sfml::vertex_array,
                entt::tag<"player_x"_hs>, cell_position>().less([constants](auto &vertex_array,
                                                                            cell_position &pos) {
            const float half_box_side = static_cast<float>(std::fmin(constants.cell_width, constants.cell_height) *
                                                           0.25f);
            const float center_x = static_cast<float>(constants.cell_width * 0.5 + pos.column * constants.cell_width);
            const float center_y = static_cast<float>(constants.cell_height * 0.5 + pos.row * constants.cell_height);

            sf::VertexArray &lines = vertex_array.drawable;
            lines[0].position = sf::Vector2f(center_x - half_box_side, center_y - half_box_side);
            lines[1].position = sf::Vector2f(center_x + half_box_side, center_y + half_box_side);
            lines[2].position = sf::Vector2f(center_x + half_box_side, center_y - half_box_side);
            lines[3].position = sf::Vector2f(center_x - half_box_side, center_y + half_box_side);
        });
    }

    void tic_tac_toe_factory::reset_grid(entt::registry &entity_registry) noexcept
    {
        auto window_info = entity_registry.ctx<sf::RenderTexture>().getSize();
        entity_registry.view<antara::gaming::sfml::vertex_array, entt::tag<"grid"_hs>>().less(
                [&entity_registry, &window_info](antara::gaming::sfml::vertex_array &array_cmp) {
                    auto constants = entity_registry.ctx<tic_tac_toe_constants>();
                    sf::VertexArray &lines = array_cmp.drawable;
                    for (std::size_t counter = 0, i = 0; i <= constants.nb_cells; ++i, counter += 4) {
                        lines[counter].position = sf::Vector2f(i * constants.cell_width, 0);
                        lines[counter + 1].position = sf::Vector2f(i * constants.cell_width, window_info.y);
                        lines[counter + 2].position = sf::Vector2f(0, i * constants.cell_height);
                        lines[counter + 3].position = sf::Vector2f(window_info.x, i * constants.cell_height);
                    }
                });
    }
}