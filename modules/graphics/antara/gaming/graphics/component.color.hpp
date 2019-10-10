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

#include <utility>
#include <cstdint>

namespace antara::gaming::graphics
{
    struct color
    {
        constexpr color() noexcept : r(0), g(0), b(0), a(255)
        {

        }

        constexpr color(std::uint8_t r_, std::uint8_t g_, std::uint8_t b_, std::uint8_t a_ = 255) noexcept : r(r_),
                                                                                                             g(g_),
                                                                                                             b(b_),
                                                                                                             a(a_)
        {

        }

        constexpr bool operator==(const color &rhs) const noexcept
        {
            return r == rhs.r &&
                   g == rhs.g &&
                   b == rhs.b &&
                   a == rhs.a;
        }

        constexpr bool operator!=(const color &rhs) const noexcept
        {
            return !(rhs == *this);
        }

        std::uint8_t r{0};
        std::uint8_t g{0};
        std::uint8_t b{0};
        std::uint8_t a{255};
    };

    inline constexpr color black{0, 0, 0};
    inline constexpr color white{255, 255, 255};
    inline constexpr color red{255, 0, 0};
    inline constexpr color green{0, 255, 0};
    inline constexpr color blue{0, 0, 255};
    inline constexpr color yellow{255, 255, 0};
    inline constexpr color magenta{255, 0, 255};
    inline constexpr color cyan{0, 255, 255};
    inline constexpr color transparent{0, 0, 0, 0};


    struct outline_color : color
    {
        template <typename ... TArgs>
        constexpr outline_color(TArgs&& ...args) noexcept : graphics::color(std::forward<TArgs>(args)...)
        {

        }
    };

    struct fill_color : color
    {
        template <typename ... TArgs>
        constexpr fill_color(TArgs&& ...args) noexcept : graphics::color(std::forward<TArgs>(args)...)
        {

        }
    };
}