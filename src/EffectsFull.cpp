#include "EffectsFull.hpp"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <random>

#include <cassert>

using Rng = std::default_random_engine;
#if 0
namespace {

[[deprecated]] void render_blocks
    (const ConstSubGrid<int> &, const sf::Texture &, sf::RenderTarget &,
     bool do_block_merging);

} // end of <anonymous> namespace
#endif
void FallEffectsFull::restart() {
    m_fall_effects.clear();
    start();
}

void FallEffectsFull::setup
    (int board_width, int board_height, const sf::Texture & texture)
{
    auto emp = k_empty_block;
    m_blocks_copy.set_size(board_width, board_height, std::move(emp));
    m_texture = &texture;

    Rng rng { std::random_device()() };
    m_rates_for_col.resize(board_width, 1.);
    for (auto & rate : m_rates_for_col) {
        rate = std::uniform_real_distribution<double>(0.75, 1.2)(rng)*double(board_height);
    }
}

void FallEffectsFull::update(double et) {
    for (auto & effect : m_fall_effects) {
        effect.fall_position += et*effect.rate;
        if (effect_is_finished(effect) && m_blocks_copy.has_position(effect.to)) {
            m_blocks_copy(effect.to) = effect.color;
        }
    }
    m_fall_effects.erase(std::remove_if(m_fall_effects.begin(), m_fall_effects.end(),
        effect_is_finished), m_fall_effects.end());
}

bool FallEffectsFull::has_effects() const {
    return !m_fall_effects.empty();
}

void FallEffectsFull::do_fall_in
    (Grid<int> & original_board, const Grid<int> & board_of_fallins)
{
    if (original_board.width () != board_of_fallins.width () ||
        original_board.height() != board_of_fallins.height())
    {
        throw std::invalid_argument("FallEffectsFull::do_fall_in: board size mismatch");
    }
    if (original_board.height() == 0 || original_board.width() == 0)
        { return; }

    const auto end_position = board_of_fallins.end_position();
    m_blocks_copy.set_size(original_board.width(), original_board.height());
    start();
    for (VectorI r; r != end_position; r = board_of_fallins.next(r)) {
        if (original_board(r) != k_empty_block) {
            post_stationary_block(r, original_board(r));
        }
    }
    for (int x = 0; x != original_board.width(); ++x) {
        // find the lowest empty space
        // (assumes all blocks have fallen)
        int lowest_empty = original_board.height() - 1;
        for (int y = 0; y != original_board.height(); ++y) {
            if (original_board(x, y) == k_empty_block) continue;
            lowest_empty = y - 1;
            break;
        }
        // no space available for fallins
        if (lowest_empty == -1) continue;
        // start from the bottom of the fallins grid
        int fallins_y = board_of_fallins.height() - 1;
        // iterate empty blocks
        for (int y = lowest_empty; y != -1; --y) {
            // find first fallin block
            for ( ;fallins_y != -1; --fallins_y) {
                if (board_of_fallins(x, fallins_y) != k_empty_block) { break; }
            }
            // breaks once we're out of fallins
            if (fallins_y == -1) { break; }
            assert(board_of_fallins(x, fallins_y) != k_empty_block);
            assert(original_board(x, y) == k_empty_block);

            int fallins_block = board_of_fallins(x, fallins_y);
            original_board(x, y) = fallins_block;
            post_block_fall(VectorI(x, y - lowest_empty - 1), // -1 - (lowest_empty - y)),
                            VectorI(x, y), fallins_block);
            --fallins_y;
        }
    }
    finish();
}

/* private */ void FallEffectsFull::start() {
    for (int & c : m_blocks_copy) c = k_empty_block;
}

/* private */ void FallEffectsFull::post_stationary_block
    (VectorI at, int color)
{
    m_blocks_copy(m_transf_v(at)) = color;
}

/* private */ void FallEffectsFull::post_block_fall
    (VectorI from, VectorI to, int color)
{
    if (from == to) {
        throw std::invalid_argument("FallEffectsFull::post_block_fall: from and to cannot be the same location");
    }
    FallEffect effect;
    effect.to    = m_transf_v(to  );
    effect.from  = m_transf_v(from);
    effect.color = color;
    effect.rate  = m_rates_for_col[from.x];
    m_fall_effects.push_back(effect);
}

/* private */ void FallEffectsFull::finish() {}

/* private */ void FallEffectsFull::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!has_effects()) return;
    using VectorF = sf::Vector2<float>;
    sf::Sprite brush;
    brush.setTexture(*m_texture);

    for (const auto & effect : m_fall_effects) {
        auto rd = normalize(VectorF(effect.to - effect.from));
        rd *= float(effect.fall_position*k_block_size);
        brush.setTextureRect(texture_rect_for(effect.color));
        brush.setColor(base_color_for_block(effect.color));
        brush.setPosition(rd + VectorF(effect.from*k_block_size));
        target.draw(brush, states);
    }
    brush.setPosition(0.f, 0.f);
    (m_render_merged ? render_merged_blocks : render_blocks)(m_blocks_copy, brush, target);
}

// ----------------------------------------------------------------------------

void PopEffectsPartial::update(double et) {
    static const auto is_flash_effect_done = [](const FlashEffect & effect)
        { return effect.remaining <= 0.; };

    for (auto & effect : m_flash_effects) {
        effect.remaining -= et;
        if (is_flash_effect_done(effect)) {
            spawn_piece_effects(effect);
        }
    }

    m_flash_effects.erase(
        std::remove_if(m_flash_effects.begin(), m_flash_effects.end(), is_flash_effect_done),
        m_flash_effects.end());

    static const VectorD k_gravity(0, PieceEffect::k_gravity);
    for (auto & effect : m_piece_effects) {
        effect.velocity += k_gravity*et;
        effect.location += effect.velocity*et;
        effect.remaining -= et;
    }

    static const auto is_piece_effect_done = [](const PieceEffect & effect)
        { return effect.remaining <= 0.; };
    m_piece_effects.erase(
        std::remove_if(m_piece_effects.begin(), m_piece_effects.end(), is_piece_effect_done),
        m_piece_effects.end());
}

bool PopEffectsPartial::has_effects() const {
    return !m_flash_effects.empty() || !m_piece_effects.empty();
}

/* protected */ void PopEffectsPartial::start() {

}

/* protected */ void PopEffectsPartial::finish() {

}

/* protected */ void PopEffectsPartial::post_pop_effect(VectorI at, int block_id) {
    // we can insert further pop behaviors here
    FlashEffect effect;
    effect.at = at;
    effect.block_id = block_id;
    m_flash_effects.push_back(effect);
    if (!m_blocks_copy.has_position(at)) {
        throw std::runtime_error("PopEffectsFull::post_pop_effect: pop effects "
                                 "does not have its own copy of the board, this "
                                 "can be fixed by calling the do_pop member function");
    }
    m_blocks_copy(at) = decay_block(m_blocks_copy(at));
}

/* private */ void PopEffectsPartial::draw
    (sf::RenderTarget & target, sf::RenderStates states) const
{
    if (!has_effects()) return;
    sf::Sprite brush;
    brush.setTexture(*m_texture);
    render_merged_blocks(m_blocks_copy, brush, target);
    for (const auto & effect : m_flash_effects) {
        draw_flash_effect(target, states, effect);
    }
    for (const auto & effect : m_piece_effects) {
        draw_piece_effect(target, states, effect);
    }
}

/* private */ void PopEffectsPartial::draw_flash_effect
    (sf::RenderTarget & target, sf::RenderStates states,
     const FlashEffect & effect) const
{
    sf::Sprite brush;
    brush.setTexture(*m_texture);
    brush.setTextureRect(texture_rect_for(effect.block_id));
    brush.setColor(brighten_by(base_color_for_block(effect.block_id), effect));
#   if 0
    bind_block_to_sprite(spt, effect.block_id, TileEdges().set());
#   endif
    brush.setPosition(float(effect.at.x*k_block_size), float(effect.at.y*k_block_size));
#   if 0
    sf::Color c = block_is_color(effect.block_id) ? to_sfcolor(effect.block_id) : sf::Color::White;
    spt.setColor(brighten_by(c, effect));
#   endif
    target.draw(brush, states);
}

/* private */ void PopEffectsPartial::draw_piece_effect
    (sf::RenderTarget & target, sf::RenderStates states,
     const PieceEffect & effect) const
{
    static const auto get_decayed_color = [](const PieceEffect & effect) {
        static constexpr const auto k_init_rem = PieceEffect::k_init_remaining;
        int darken_amount = int(std::round(
            ((k_init_rem - effect.remaining) / k_init_rem)*200
        ));
        assert(darken_amount + 55 <= 255);
        sf::Color rv = effect.color;
        rv.r = std::max(0, rv.r - darken_amount);
        rv.g = std::max(0, rv.g - darken_amount);
        rv.b = std::max(0, rv.b - darken_amount);
        rv.a = int(std::round((effect.remaining / k_init_rem)*200)) + 55;
        return rv;
    };
    assert(effect.block_id != k_empty_block);
    auto trect = texture_rect_for(effect.block_id);
    trect.width  /= 2;
    trect.height /= 2;
    trect.top  += effect.texture_offset.y*(k_block_size / 2);
    trect.left += effect.texture_offset.x*(k_block_size / 2);
    auto color = get_decayed_color(effect);

    sf::Sprite spt;
    spt.setTexture(*m_texture);
    spt.setTextureRect(trect);
    spt.setPosition(sf::Vector2f(effect.location));
    spt.setColor(color);
    target.draw(spt, states);
}

/* private */ void PopEffectsPartial::spawn_piece_effects
    (const FlashEffect & flash_effect)
{
    using std::make_pair;
    static constexpr const auto k_pi = get_pi<double>();

    using RealDistri = std::uniform_real_distribution<double>;
    RealDistri top_interval   (0, k_pi / 4.);
    RealDistri bottom_interval(0, k_pi / 5.);
    auto & rng = m_rng;

    auto piece_list = {
        make_pair(VectorI(0, 0), -(1./8. + 0.5)*k_pi - top_interval   (rng)),
        make_pair(VectorI(0, 1), -(1./6. + 0.5)*k_pi - bottom_interval(rng)),
        make_pair(VectorI(1, 0),  (1./8. - 0.5)*k_pi + top_interval   (rng)),
        make_pair(VectorI(1, 1),  (1./6. - 0.5)*k_pi + bottom_interval(rng))
    };

    static constexpr const auto k_init_speed = PieceEffect::k_init_speed;
    static const auto ucircle = [](double t)
        { return VectorD(std::cos(t), std::sin(t)); };
    sf::Color init_color = sf::Color::White;
    if (is_block_color(flash_effect.block_id)) {
        init_color = brighten_color(base_color_for_block(flash_effect.block_id), 1.);
    }

    for (auto [offset, angle] : piece_list) {
        PieceEffect effect;
        effect.color = init_color;
        effect.texture_offset = offset;
        effect.velocity = ucircle(angle)*k_init_speed;
        effect.location = VectorD(flash_effect.at)*double(k_block_size);
        effect.location += VectorD(offset)*double(k_block_size / 2);
        effect.block_id = flash_effect.block_id;
        m_piece_effects.push_back(effect);
    }
}

// ----------------------------------------------------------------------------
#if 0
void render_merged_blocks
    (const ConstSubGrid<int> & blocks, const sf::Texture & texture,
     sf::RenderTarget & target)
{ render_blocks(blocks, texture, target, true); }

void render_blocks
    (const ConstSubGrid<int> & blocks, const sf::Texture & texture,
     sf::RenderTarget & target)
{ render_blocks(blocks, texture, target, false); }
#endif
// not obvious on how to keep this
#if 0
void bind_block_to_sprite(sf::Sprite & brush, int block_id, TileEdges edges) {
    if (block_is_color(block_id)) {
        brush.setColor(to_sfcolor(block_id));
        brush.setTextureRect(texture_rect_for(edges));
    } else {
        brush.setColor(sf::Color::White);
        int x_offset = (block_id - k_glass_block)*k_block_size;
        brush.setTextureRect(sf::IntRect(
            x_offset, 4*k_block_size, k_block_size, k_block_size
        ));
    }
}
#endif
namespace {
#if 0
TileEdges get_edges_for(const ConstSubGrid<int> &, VectorI);
#endif
#if 0
void render_blocks
    (const ConstSubGrid<int> & blocks, const sf::Texture & texture,
     sf::RenderTarget & target, bool do_block_merging)
{
    sf::Sprite brush;
    brush.setTexture(texture);
    for (VectorI r; r != blocks.end_position(); r = blocks.next(r)) {
        if (blocks(r) == k_empty_block) continue;
        brush.setPosition(float(r.x*k_block_size), float(r.y*k_block_size));
        auto edges = do_block_merging ? TileDrawer::get_edges_for(blocks, r) : TileEdges().flip();
        bind_block_to_sprite(brush, blocks(r), edges);
        target.draw(brush);
    }
}
#endif
// ----------------------------------------------------------------------------


} // end of <anonymous> namespace
