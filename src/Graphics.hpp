#pragma once

#include "Defs.hpp"

#include "SubGrid.hpp"

namespace sf {
    class RenderTarget;
}

sf::Image to_image(const Grid<sf::Color> &);

constexpr const int k_icon_size = 64;

// functions getting resources

const uint8_t * get_icon_image();

const sf::Texture & load_builtin_block_texture();

// ----------------------------------------------------------------------------

// helpers to render tiles

sf::IntRect texture_rect_for(int, TileEdges);

sf::IntRect texture_rect_for(int);

sf::IntRect texture_rect_for_background();

sf::Color base_color_for_block(int);

sf::Color brighten_color(sf::Color, double);

void render_blocks
    (const ConstSubGrid<int> &, const sf::Sprite &, sf::RenderTarget &);

void render_merged_blocks
    (const ConstSubGrid<int> &, const sf::Sprite &, sf::RenderTarget &);
