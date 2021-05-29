/****************************************************************************

    Copyright 2020 Aria Janke

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*****************************************************************************/

#pragma once

#include "Defs.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace sf {
    class RenderTarget;
    class Sprite;
}

namespace asgl {
    class SfmlFlatEngine;
    struct ImageResource;
}

sf::Image to_image(const Grid<sf::Color> &);

constexpr const int k_icon_size = 64;

// functions getting resources

const uint8_t * get_icon_image();

struct BuiltinBlockGraphics {
    static const sf::Texture & as_texture();
    static std::shared_ptr<asgl::ImageResource> as_asgl_image(asgl::SfmlFlatEngine &);
    static std::shared_ptr<asgl::ImageResource> as_asgl_image();
};

[[deprecated]] const sf::Texture & load_builtin_block_texture();

// ----------------------------------------------------------------------------

// helpers to render tiles

sf::IntRect texture_rect_for(BlockId, TileEdges);

sf::IntRect texture_rect_for(BlockId);

sf::IntRect texture_rect_for_background();

constexpr const int k_wood_board_count = 3;
sf::IntRect texture_rect_for_wood_board(int);

sf::IntRect texture_rect_for_score();

sf::IntRect texture_rect_for_next();

sf::IntRect texture_rect_for_char(char);

sf::IntRect texture_rect_for_control(PlayControlId, bool is_pressed);

sf::Color base_color_for_block(BlockId);

sf::Color brighten_color(sf::Color, double);

void render_blocks
    (const ConstBlockSubGrid &, const sf::Sprite &, sf::RenderTarget &);

void render_merged_blocks
    (const ConstBlockSubGrid &, const sf::Sprite &, sf::RenderTarget &);

void render_blocks
    (const ConstBlockSubGrid &, const sf::Sprite &, sf::RenderTarget &,
     sf::RenderStates);

void render_merged_blocks
    (const ConstBlockSubGrid &, const sf::Sprite &, sf::RenderTarget &,
     sf::RenderStates);
