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

#pragma once

#include <entt/core/utility.hpp>
#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include "antara/gaming/ecs/system.manager.hpp"
#include "antara/gaming/event/quit.game.hpp"

namespace antara::gaming::world
{
    class app
    {
    public:
        app() noexcept;

        //! Public callbacks
        void receive_quit_game(const event::quit_game &evt) noexcept;
        int run() noexcept;
        void process_one_frame();
    private:
        bool is_running_{false};
        int game_return_value_{0};
    protected:
        entt::registry entity_registry_;
        entt::dispatcher dispatcher_;
        ecs::system_manager system_manager_{entity_registry_, dispatcher_};
    };
}