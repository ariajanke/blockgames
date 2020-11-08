#include "Defs.hpp"
#include "Graphics.hpp"

#include <common/SubGrid.hpp>

#include <SFML/Graphics/RenderTarget.hpp>

#include <stdexcept>
#include <memory>
#include <tuple>

#include <cassert>
#include <cstring>
#include <cmath>

namespace {

using ColorGrid = Grid<sf::Color>;
using ColorGroupInfo = std::tuple<sf::Color, VectorI>;

constexpr const int k_color_group_size = 4;

const ColorGrid & builtin_blocks();
sf::Color get_group_color_value(int color);
uint8_t treat_as_grey(sf::Color);
sf::Color mask_color(uint8_t grey, sf::Color fullbright);
sf::Color blend_with_alpha(sf::Color, sf::Color);
ColorGroupInfo get_color_group_info(int color);
sf::IntRect to_tile_rect(VectorI loc);

void render_blocks
    (const ConstSubGrid<int> &, const sf::Sprite &, sf::RenderTarget &,
     bool do_block_merging);

} // end of <anonymous> namespace

sf::Image to_image(const Grid<sf::Color> & grid) {
    sf::Image img;
    img.create(grid.width(), grid.height());
    for (VectorI r; r != grid.end_position(); r = grid.next(r)) {
        img.setPixel(r.x, r.y, grid(r));
    }
    return img;
}

const uint8_t * get_icon_image() {
    static Grid<sf::Color> grid;
    if (!grid.is_empty()) {
        return reinterpret_cast<const uint8_t *>(&*grid.begin());
    }

    auto background = []() {
        auto rect = texture_rect_for_background();
        return make_const_sub_grid(builtin_blocks(), VectorI( rect.left, rect.top ), rect.width, rect.height);
    }();
    Grid<sf::Color> small_grid;
    small_grid.set_size(k_block_size*2, k_block_size*2);
    for (VectorI r; r != background.end_position(); r = background.next(r)) {
        small_grid(r) = background(r);
        small_grid(r + VectorI(0, k_block_size)) = background(r);
        small_grid(r + VectorI(k_block_size, 0)) = background(r);
        small_grid(r + VectorI(k_block_size, k_block_size)) = background(r);
    }

    const auto & blocks = builtin_blocks();
    using std::make_tuple;

    auto make_unmerged_block = [](int group) {
        auto r = std::get<1>(get_color_group_info(group));
        r += VectorI(3, 3)*k_block_size;
        return make_const_sub_grid(builtin_blocks(), r, k_block_size, k_block_size);
    };

    auto glass_block = make_const_sub_grid(blocks, VectorI(1, 2*4)*k_block_size, k_block_size, k_block_size);
    auto list = {
        make_tuple(sf::Color::White, glass_block, VectorI(k_block_size, k_block_size)),
        make_tuple(get_group_color_value(1), make_unmerged_block(1), VectorI(k_block_size, 0)),
        make_tuple(get_group_color_value(2), make_unmerged_block(2), VectorI(0, k_block_size)),
        make_tuple(get_group_color_value(3), make_unmerged_block(3), VectorI(0, 0))
    };
    for (const auto & [fullbright, srcgrid, offset] : list) {
        for (VectorI r; r != srcgrid.end_position(); r = srcgrid.next(r)) {
            auto & dest = small_grid(r + offset);
            auto src_color = mask_color(treat_as_grey(srcgrid(r)), fullbright);
            src_color.a = srcgrid(r).a;
            dest = blend_with_alpha(dest, src_color);
        }
    }

    static_assert(k_icon_size % (2*k_block_size) == 0, "");
    static_assert(k_icon_size > (2*k_block_size), "");
    static constexpr const int k_scale = k_icon_size / (2*k_block_size);
    grid.set_size(k_icon_size, k_icon_size);
    for (VectorI r; r != grid.end_position(); r = grid.next(r)) {
        grid(r) = small_grid(r.x / k_scale, r.y / k_scale);
    }
    return reinterpret_cast<const uint8_t *>(&*grid.begin());
}

const sf::Texture & load_builtin_block_texture() {
    static std::unique_ptr<sf::Texture> rv;
    if (rv) { return *rv; }

    rv = std::make_unique<sf::Texture>();
    rv->loadFromImage(to_image(builtin_blocks()));
    to_image(builtin_blocks()).saveToFile("/media/ramdisk/out.png");
    return *rv;
}

// <-------------------------- block drawer helpers -------------------------->

sf::IntRect texture_rect_for(int n, TileEdges edges) {
    if (!is_block_color(n)) {
        throw std::invalid_argument("texture_rect_for: block type cannot have edges.");
    }
    auto val = edges.to_ulong();
    VectorI u(val % 4, val / 4);
    return to_tile_rect(u*k_block_size + std::get<1>(get_color_group_info(n)));
}

sf::IntRect texture_rect_for(int n) {
    if (!is_block_color(n)) {
        int x_offset = (n - k_glass_block)*k_block_size;
        return sf::IntRect(x_offset, 4*2*k_block_size, k_block_size, k_block_size);
    }
    return texture_rect_for(n, TileEdges().set());
}

sf::IntRect texture_rect_for_background() {
    return sf::IntRect(0, (k_color_group_size*2 + 1)*k_block_size, k_block_size, k_block_size);
}

sf::Color base_color_for_block(int n) {
    return is_block_color(n) ? get_group_color_value(n) : sf::Color::White;
}

sf::Color brighten_color(sf::Color c, double brightenfactor) {
    if (brightenfactor < 0. || brightenfactor > 1.) {
        throw std::invalid_argument("brighten_color: brightness must be in [0 1]");
    }
    int brighten_amount = int(std::round(brightenfactor*150.));
    c.r = std::min(255, brighten_amount + c.r);
    c.g = std::min(255, brighten_amount + c.g);
    c.b = std::min(255, brighten_amount + c.b);
    return c;
}

void render_blocks
    (const ConstSubGrid<int> & blocks, const sf::Sprite & brush,
     sf::RenderTarget & target)
{
    render_blocks(blocks, brush, target, false);
}

void render_merged_blocks
    (const ConstSubGrid<int> & blocks, const sf::Sprite & brush,
     sf::RenderTarget & target)
{
    render_blocks(blocks, brush, target, true);
}

namespace {

void add_edge_masks(SubGrid<sf::Color>);

const char * get_color_mask(int);

int to_16x16_index(VectorI r);

void add_specials(SubGrid<sf::Color>);

void add_builtin_background(SubGrid<sf::Color>);

TileEdges get_edges_for(const ConstSubGrid<int> &, VectorI);

const ColorGrid & builtin_blocks() {
    static std::unique_ptr<ColorGrid> rv;
    if (rv) return *rv;

    rv = std::make_unique<ColorGrid>();
    Grid<sf::Color> & grid = *rv;
    grid.set_size( k_color_group_size*3     *k_block_size,
                  (k_color_group_size*2 + 2)*k_block_size, sf::Color::White);
    for (int color = k_min_colors; color != k_max_colors + 1; ++color) {
        auto [color_val, offset] = get_color_group_info(color);
        (void)color_val; // color added at draw time
        auto subg = make_sub_grid(grid, offset, k_block_size*k_color_group_size, k_block_size*k_color_group_size);
        add_edge_masks(subg);
        for (int x = 0; x != k_color_group_size; ++x) {
        for (int y = 0; y != k_color_group_size; ++y) {
            auto block = make_sub_grid(subg, VectorI(x, y)*k_block_size, k_block_size, k_block_size);
            auto mask_src = get_color_mask(color);
            for (VectorI r; r != block.end_position(); r = block.next(r)) {
                switch (mask_src[to_16x16_index(r)]) {
                case ' ': continue;
                case 'X':
                    block(r).r /= 3;
                    block(r).g /= 3;
                    block(r).b /= 3;
                    //std::swap(block(r).r, block(r).g);
                    //std::swap(block(r).g, block(r).b);
                    //std::swap(block(r).b, block(r).r);
                    break;
                default: break;
                }
            }
        }}
    }

    add_specials(make_sub_grid(grid, VectorI(0, k_color_group_size*2*k_block_size)));
    add_builtin_background(make_sub_grid(grid,
        VectorI(0, (k_color_group_size*2 + 1)*k_block_size),
        k_block_size, k_block_size));
    return *rv;
}

sf::Color get_group_color_value(int color) {
    return std::get<0>(get_color_group_info(color));
}

uint8_t treat_as_grey(sf::Color color) {
    assert(color.r == color.g && color.g == color.b);
    return color.r;
}

sf::Color mask_color(uint8_t grey, sf::Color fullbright) {
    return sf::Color( (grey*fullbright.r) / 255, (grey*fullbright.g) / 255, (grey*fullbright.b) / 255 );
}

sf::Color blend_with_alpha(sf::Color a, sf::Color b) {
    a.r = (a.r*(255 - b.a)) / 255;
    a.g = (a.g*(255 - b.a)) / 255;
    a.b = (a.b*(255 - b.a)) / 255;

    b.r = (b.r*b.a) / 255;
    b.g = (b.g*b.a) / 255;
    b.b = (b.b*b.a) / 255;

    return sf::Color(a.r + b.r, a.g + b.g, a.b + b.b, 255);
}

ColorGroupInfo get_color_group_info(int color) {
    using sf::Color;
    using std::make_tuple;
    static const auto mk_v = [](int x, int y) {
        assert(y < 2);
        return VectorI(x, y)*k_block_size*k_color_group_size;
    };
    switch (color) {
    case k_empty_block:
        throw std::invalid_argument("color_block_nfo: Empty block has no color");
    case 1: return make_tuple(Color(230,  70,  70), mk_v(0, 0));
    case 2: return make_tuple(Color( 70, 230,  70), mk_v(1, 0));
    case 3: return make_tuple(Color( 70,  70, 230), mk_v(2, 0));
    case 4: return make_tuple(Color(230, 230,  70), mk_v(0, 1));
    case 5: return make_tuple(Color(230,  70, 230), mk_v(1, 1));
    }
    throw std::invalid_argument("color_block_nfo: invalid color");
}

sf::IntRect to_tile_rect(VectorI loc) {
    return sf::IntRect(loc, VectorI(1, 1)*k_block_size);
}

void render_blocks
    (const ConstSubGrid<int> & blocks, const sf::Sprite & brush_,
     sf::RenderTarget & target, bool do_block_merging)
{
    sf::Sprite brush = brush_;

    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (blocks(r) == k_empty_block) continue;
        brush.setPosition(sf::Vector2f(r*k_block_size) + brush_.getPosition());
        brush.setColor(base_color_for_block(blocks(r)));

        brush.setTextureRect([&blocks, r, do_block_merging]() {
            if (!is_block_color(blocks(r))) return texture_rect_for(blocks(r));
            auto edges = do_block_merging ? get_edges_for(blocks, r) : TileEdges().flip();
            return texture_rect_for(blocks(r), edges);
        }());

        target.draw(brush);
    }
}

// ----------------------------------------------------------------------------

const char * get_edge_mask(const TileEdges &);

const char * get_color_mask_impl(int);

const char * verify_16x16(const char *);

const char * get_special_block(int x);

#if 0
const char * get_edge_mask(const TileEdges & edges) {
    return verify_16x16(get_edge_mask_impl(edges));
}
#endif
void add_edge_masks(SubGrid<sf::Color> grid) {
    auto itr = k_full_edge_list.begin();
    for (int y = 0; y != 4; ++y) {
    for (int x = 0; x != 4; ++x) {
        assert(itr != k_full_edge_list.end());
        auto mask = get_edge_mask(*itr++);
        SubGrid<sf::Color> subg = grid.make_sub_grid
            (VectorI(x*k_block_size, y*k_block_size), k_block_size, k_block_size);
        for (VectorI r; r != subg.end_position(); r = subg.next(r)) {
            auto c = subg(r);
            switch (mask[to_16x16_index(r)]) {
            case ' ': break;
            case 'X':
                //c.a /= 6;
                c = sf::Color(0, 0, 0, 0);
                break;
            case 'x':
                c.a = (c.a*2) / 3;
                c.r /= 3;
                c.g /= 3;
                c.b /= 3;
                break;
            case '-':
                //c.a = (c.a*3) / 4;
                c.r /= 2;
                c.g /= 2;
                c.b /= 2;
                break;
            }
            subg(r) = c;
        }
    }}
}

const char * get_color_mask(int n)
    { return verify_16x16(get_color_mask_impl(n)); }

int to_16x16_index(VectorI r)
    { return r.x + r.y*k_block_size; }

void add_specials(SubGrid<sf::Color> grid) {
    static const auto k_specials_list = { 6 , 7 };//, 8, 9 };

    for (auto itr = k_specials_list.begin(); itr != k_specials_list.end(); ++itr) {
        assert(itr != k_specials_list.end());
        SubGrid<sf::Color> subg = grid.make_sub_grid
            (VectorI((itr - k_specials_list.begin())*k_block_size, 0),
             k_block_size, k_block_size);
        for (VectorI r; r != subg.end_position(); r = subg.next(r)) {
            auto & c = subg(r);
            switch (get_special_block(*itr)[to_16x16_index(r)]) {
            case ' ': c.a =   0; break;
            case 'x': c.a = 128; break;
            case 'X': c.a = 255; break;
            default: throw;
            }
        }
    }
    add_builtin_background(grid.make_sub_grid(
        VectorI(k_specials_list.size()*k_block_size, 0), k_block_size, k_block_size
    ));
}

void add_builtin_background(SubGrid<sf::Color> subgrid) {
    assert(subgrid.width() == k_block_size && subgrid.height() == k_block_size);

    static constexpr const char * const k_bricks =
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """       X    ---X"// 1
        """    -- X       X"// 2
        """--     X --    X"// 3
        """XXXXXXXXXXXXXXXX"// 4
        """  -X     --X    "// 5
        """-  X ---   X  --"// 6
        """   X       X    "// 7
        """XXXXXXXXXXXXXXXX"// 8
        """    ---X    ---X"// 9
        """ --    X --    X"// A
        """       X       X"// B
        """XXXXXXXXXXXXXXXX"// C
        """-- X       X   -"// D
        """   X   --- X--  "// E
        """   X--     X    "// F
        ;

    for (VectorI r; r != subgrid.end_position(); r = subgrid.next(r)) {
        subgrid(r) = [](char c) {
            switch (c) {
            case 'X': return sf::Color( 73,  73,  73);
            case '-': return sf::Color(160, 160, 160);
            case ' ': return sf::Color(126, 126, 126);
            default: throw ;
            }
        }(k_bricks[to_16x16_index(r)]);
    }
}

TileEdges get_edges_for(const ConstSubGrid<int> & blocks, VectorI r) {
    if (!is_block_color(blocks(r))) { return TileEdges().set(); }
    using std::make_pair;
    auto neighbors = {
        make_pair(VectorI( 0, -1) + r, k_top_edge   ),
        make_pair(VectorI( 0,  1) + r, k_bottom_edge),
        make_pair(VectorI(-1,  0) + r, k_left_edge  ),
        make_pair(VectorI( 1,  0) + r, k_right_edge ),
    };
    TileEdges edges;
    edges.set();
    for (auto [npt, side] : neighbors) {
        if (!blocks.has_position(npt)) continue;
        edges.set(side, blocks(npt) != blocks(r));
    }
    return edges;
}

// ----------------------------------------------------------------------------

const char * get_edge_mask_impl(const TileEdges &);

const char * get_special_block_impl(int);

const char * get_edge_mask(const TileEdges & edges) {
    return verify_16x16(get_edge_mask_impl(edges));
}

const char * get_color_mask_impl(int n) {
    switch (n) {
    case 1: return
        // 0123456789ABCDEF
        """                "// 0
        """                "// 1
        """                "// 2
        """       XX       "// 3
        """      XXXX      "// 4
        """   XXXX  XXXX   "// 5
        """    XX    XX    "// 6
        """     XX  XX     "// 7
        """      XX X      "// 8
        """     XXXXXX     "// 9
        """    XXXXXXXX    "// A
        """    XX    XX    "// B
        """    X      X    "// C
        """                "// D
        """                "// E
        """                "// F
        ;
    case 2: return
        // 0123456789ABCDEF
        """                "// 0
        """                "// 1
        """                "// 2
        """     XXXXXX     "// 3
        """    XX    XX    "// 4
        """   XX      XX   "// 5
        """   XX      XX   "// 6
        """   XX      XX   "// 7
        """   XX      XX   "// 8
        """   XX      XX   "// 9
        """   XX      XX   "// A
        """    XX    XX    "// B
        """     XXXXXX     "// C
        """                "// D
        """                "// E
        """                "// F
        ;
    case 3: return
        // 0123456789ABCDEF
        """                "// 0
        """                "// 1
        """                "// 2
        """     XX  XX     "// 3
        """    XXXXXXXX    "// 4
        """   XX  XX  XX   "// 5
        """   XX      XX   "// 6
        """   XX      XX   "// 7
        """   XX      XX   "// 8
        """    XX    XX    "// 9
        """     XX  XX     "// A
        """      XXXX      "// B
        """       XX       "// C
        """                "// D
        """                "// E
        """                "// F
        ;
    case 4: return
        // 0123456789ABCDEF
        """                "// 0
        """                "// 1
        """                "// 2
        """     XXXXXX     "// 3
        """    XXX  XXX    "// 4
        """   XX  XX  XX   "// 5
        """   XX XXXX XX   "// 6
        """   X XX  XX X   "// 7
        """   X XX  XX X   "// 8
        """   XX XXXX XX   "// 9
        """   XX  XX  XX   "// A
        """    XXX  XXX    "// B
        """     XXXXXX     "// C
        """                "// D
        """                "// E
        """                "// F
        ;
    case 5: return
        // 0123456789ABCDEF
        """                "// 0
        """                "// 1
        """                "// 2
        """       X        "// 3
        """       XX       "// 4
        """       XX       "// 5
        """      XXXX      "// 6
        """   XXXX  XXXX   "// 7
        """    XXX  XXX    "// 8
        """      XXXX      "// 9
        """       XX       "// A
        """       XX       "// B
        """        X       "// C
        """                "// D
        """                "// E
        """                "// F
        ;
    default: break;
    }
    throw std::invalid_argument("get_color_mask_impl: cannot get mask for unknown color index.");
}

const char * verify_16x16(const char * str) {
    if (strlen(str) == k_block_size*k_block_size) return str;
    throw std::invalid_argument("mask must be 16x16");
}

const char * get_special_block(int x) {
    return verify_16x16(get_special_block_impl(x));
}

// ----------------------------------------------------------------------------

const char * get_edge_mask_impl(const TileEdges & edges) {
    switch (edges.to_ulong()) {
    // 0
    case GetEdgeValue<>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """x-            -x"// 1
        """-              -"// 2
        """                "// 3
        """                "// 4
        """                "// 5
        """                "// 6
        """                "// 7
        """                "// 8
        """                "// 9
        """                "// A
        """                "// B
        """                "// C
        """-              -"// D
        """x-            -x"// E
        """Xx-          -xX"// F
        ;
    // 1
    case GetEdgeValue<k_left_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """Xx-           -x"// 1
        """Xx-            -"// 2
        """Xx-             "// 3
        """Xx-             "// 4
        """Xx-             "// 5
        """Xx-             "// 6
        """Xx-             "// 7
        """Xx-             "// 8
        """Xx-             "// 9
        """Xx-             "// A
        """Xx-             "// B
        """Xx-             "// C
        """Xx-            -"// D
        """Xx-           -x"// E
        """Xx-          -xX"// F
        ;
    // 2
    case GetEdgeValue<k_right_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """x-           -xX"// 1
        """-            -xX"// 2
        """             -xX"// 3
        """             -xX"// 4
        """             -xX"// 5
        """             -xX"// 6
        """             -xX"// 7
        """             -xX"// 8
        """             -xX"// 9
        """             -xX"// A
        """             -xX"// B
        """             -xX"// C
        """-            -xX"// D
        """x-           -xX"// E
        """Xx-          -xX"// F
        ;
    // 3
    case GetEdgeValue<k_left_edge, k_right_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """Xx-          -xX"// 1
        """Xx-          -xX"// 2
        """Xx-          -xX"// 3
        """Xx-          -xX"// 4
        """Xx-          -xX"// 5
        """Xx-          -xX"// 6
        """Xx-          -xX"// 7
        """Xx-          -xX"// 8
        """Xx-          -xX"// 9
        """Xx-          -xX"// A
        """Xx-          -xX"// B
        """Xx-          -xX"// C
        """Xx-          -xX"// D
        """Xx-          -xX"// E
        """Xx-          -xX"// F
        ;
    // 4
    case GetEdgeValue<k_top_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """xxxxxxxxxxxxxxxx"// 1
        """----------------"// 2
        """                "// 3
        """                "// 4
        """                "// 5
        """                "// 6
        """                "// 7
        """                "// 8
        """                "// 9
        """                "// A
        """                "// B
        """                "// C
        """-              -"// D
        """x-            -x"// E
        """Xx-          -xX"// F
        ;
    // 5
    case GetEdgeValue<k_left_edge, k_top_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """XXxxxxxxxxxxxxxx"// 1
        """Xxx-------------"// 2
        """Xx-             "// 3
        """Xx-             "// 4
        """Xx-             "// 5
        """Xx-             "// 6
        """Xx-             "// 7
        """Xx-             "// 8
        """Xx-             "// 9
        """Xx-             "// A
        """Xx-             "// B
        """Xx-             "// C
        """Xx-            -"// D
        """Xx-           -x"// E
        """Xx-          -xX"// F
        ;
    // 6
    case GetEdgeValue<k_right_edge, k_top_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """xxxxxxxxxxxxxxXX"// 1
        """-------------xxX"// 2
        """             -xX"// 3
        """             -xX"// 4
        """             -xX"// 5
        """             -xX"// 6
        """             -xX"// 7
        """             -xX"// 8
        """             -xX"// 9
        """             -xX"// A
        """             -xX"// B
        """             -xX"// C
        """-            -xX"// D
        """x-           -xX"// E
        """Xx-          -xX"// F
        ;
    // 7
    case GetEdgeValue<k_left_edge, k_right_edge, k_top_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """XXxxxxxxxxxxxxXX"// 1
        """Xxx----------xxX"// 2
        """Xx-          -xX"// 3
        """Xx-          -xX"// 4
        """Xx-          -xX"// 5
        """Xx-          -xX"// 6
        """Xx-          -xX"// 7
        """Xx-          -xX"// 8
        """Xx-          -xX"// 9
        """Xx-          -xX"// A
        """Xx-          -xX"// B
        """Xx-          -xX"// C
        """Xx-          -xX"// D
        """Xx-          -xX"// E
        """Xx-          -xX"// F
        ;
    // 8
    case GetEdgeValue<k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """x-            -x"// 1
        """-              -"// 2
        """                "// 3
        """                "// 4
        """                "// 5
        """                "// 6
        """                "// 7
        """                "// 8
        """                "// 9
        """                "// A
        """                "// B
        """                "// C
        """----------------"// D
        """xxxxxxxxxxxxxxxx"// E
        """XXXXXXXXXXXXXXXX"// F
        ;

    // 9
    case GetEdgeValue<k_left_edge, k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """Xx-           -x"// 1
        """Xx-            -"// 2
        """Xx-             "// 3
        """Xx-             "// 4
        """Xx-             "// 5
        """Xx-             "// 6
        """Xx-             "// 7
        """Xx-             "// 8
        """Xx-             "// 9
        """Xx-             "// A
        """Xx-             "// B
        """Xx-             "// C
        """Xxx-------------"// D
        """XXxxxxxxxxxxxxxx"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 10
    case GetEdgeValue<k_right_edge, k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """x-           -xX"// 1
        """-            -xX"// 2
        """             -xX"// 3
        """             -xX"// 4
        """             -xX"// 5
        """             -xX"// 6
        """             -xX"// 7
        """             -xX"// 8
        """             -xX"// 9
        """             -xX"// A
        """             -xX"// B
        """             -xX"// C
        """-------------xxX"// D
        """xxxxxxxxxxxxxxXX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 11
    case GetEdgeValue<k_left_edge, k_right_edge, k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """Xx-          -xX"// 0
        """Xx-          -xX"// 1
        """Xx-          -xX"// 2
        """Xx-          -xX"// 3
        """Xx-          -xX"// 4
        """Xx-          -xX"// 5
        """Xx-          -xX"// 6
        """Xx-          -xX"// 7
        """Xx-          -xX"// 8
        """Xx-          -xX"// 9
        """Xx-          -xX"// A
        """Xx-          -xX"// B
        """Xx-          -xX"// C
        """Xxx----------xxX"// D
        """XXxxxxxxxxxxxxXX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 12
    case GetEdgeValue<k_bottom_edge, k_top_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """xxxxxxxxxxxxxxxx"// 1
        """----------------"// 2
        """                "// 3
        """                "// 4
        """                "// 5
        """                "// 6
        """                "// 7
        """                "// 8
        """                "// 9
        """                "// A
        """                "// B
        """                "// C
        """----------------"// D
        """xxxxxxxxxxxxxxxx"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 13
    case GetEdgeValue<k_left_edge, k_top_edge, k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """XXxxxxxxxxxxxxxx"// 1
        """Xxx-------------"// 2
        """Xx-             "// 3
        """Xx-             "// 4
        """Xx-             "// 5
        """Xx-             "// 6
        """Xx-             "// 7
        """Xx-             "// 8
        """Xx-             "// 9
        """Xx-             "// A
        """Xx-             "// B
        """Xx-             "// C
        """Xxx-------------"// D
        """XXxxxxxxxxxxxxxx"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 14
    case GetEdgeValue<k_right_edge, k_top_edge, k_bottom_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """xxxxxxxxxxxxxxXX"// 1
        """-------------xxX"// 2
        """             -xX"// 3
        """             -xX"// 4
        """             -xX"// 5
        """             -xX"// 6
        """             -xX"// 7
        """             -xX"// 8
        """             -xX"// 9
        """             -xX"// A
        """             -xX"// B
        """             -xX"// C
        """-------------xxX"// D
        """xxxxxxxxxxxxxxXX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    // 15
    case GetEdgeValue<k_right_edge, k_top_edge, k_bottom_edge, k_left_edge>::k_value: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """XXxxxxxxxxxxxxXX"// 1
        """Xxx----------xxX"// 2
        """Xx-          -xX"// 3
        """Xx-          -xX"// 4
        """Xx-          -xX"// 5
        """Xx-          -xX"// 6
        """Xx-          -xX"// 7
        """Xx-          -xX"// 8
        """Xx-          -xX"// 9
        """Xx-          -xX"// A
        """Xx-          -xX"// B
        """Xx-          -xX"// C
        """Xxx----------xxX"// D
        """XXxxxxxxxxxxxxXX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    default: break;
    }
    throw std::invalid_argument("");
}

const char * get_special_block_impl(int x) {
    switch (x) {
    case 6: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """Xxx          xxX"// 1
        """Xx   X        xX"// 2
        """X   Xx    X    X"// 3
        """X  Xx    Xx   XX"// 4
        """X Xx    Xx   XxX"// 5
        """X x    Xx    x X"// 6
        """X      x       X"// 7
        """X         X    X"// 8
        """X   X    Xx    X"// 9
        """X  Xx   Xx     X"// A
        """X Xx   Xx   X  X"// B
        """XXx   Xx   Xx  X"// C
        """Xx   Xx   Xx  xX"// D
        """Xxx       x  xxX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    case 7: return
        // 0123456789ABCDEF
        """XXXXXXXXXXXXXXXX"// 0
        """X  xX  xX  xX xX"// 1
        """X  xX  xX  xX xX"// 2
        """XxxxXxxxXxxxXxxX"// 3
        """XXXXXXXXXXXXXXXX"// 4
        """X  xX  xX  xX xX"// 5
        """X  xX  xX  xX xX"// 6
        """XxxxXxxxXxxxXxxX"// 7
        """XXXXXXXXXXXXXXXX"// 8
        """X  xX  xX  xX xX"// 9
        """X  xX  xX  xX xX"// A
        """XxxxXxxxXxxxXxxX"// B
        """XXXXXXXXXXXXXXXX"// C
        """X  xX  xX  xX xX"// D
        """XxxxXxxxXxxxXxxX"// E
        """XXXXXXXXXXXXXXXX"// F
        ;
    default: break;
    }
    throw std::invalid_argument("get_special_block_impl: argument must be a special block type");
}

} // end of <anonymous> namespace
