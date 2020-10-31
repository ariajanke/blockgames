#pragma once

#include "Defs.hpp"

class Polyomino {
public:
    struct Block {
        Block() {}
        Block(VectorI r, int c): offset(r), color(c) {}
        VectorI offset;
        int color = k_empty_block;
    };
    enum class Domino { l, count };
    enum class Tromino { i, l, count };
    enum class Tetromino { i, j, l, o, s, t, z, count };
    enum class Pentomino { f, i, l, n, p, t, u, v, w, x, y, z, count };

    enum class PolyominoSet { domino, tromino, tetromino, pentomino, count };

    static constexpr const int k_domino_count = static_cast<int>(Domino::count);
    static constexpr const int k_tromino_count = static_cast<int>(Tromino::count);
    static constexpr const int k_tetromino_count = static_cast<int>(Tetromino::count);
    static constexpr const int k_pentomino_count = static_cast<int>(Pentomino::count);
    static constexpr const int k_polyomino_set_count = static_cast<int>(PolyominoSet::count);
    static constexpr const int k_total_polyomino_count =
        k_domino_count + k_tromino_count + k_tetromino_count + k_pentomino_count;

    static const std::vector<Polyomino> & default_domino();
    static const std::vector<Polyomino> & default_trominos();
    static const std::vector<Polyomino> & default_tetrominos();
    static const std::vector<Polyomino> & default_pentominos();
    static const std::vector<Polyomino> & all_polyminos();

    Polyomino() {}
    explicit Polyomino(std::vector<Block> &&);

    void rotate_left(const Grid<int> &);
    void rotate_right(const Grid<int> &);
    void move_up(const Grid<int> &);
    bool move_down(const Grid<int> &);
    void move_right(const Grid<int> &);
    void move_left(const Grid<int> &);
    void set_location(int x, int y);
    void place(Grid<int> &) const;
    VectorI location() const { return m_location; }
    void set_colors(int);
    void enable_rotation() { m_rotation_enabled = true; }
    void disable_rotation() { m_rotation_enabled = false; }

    int block_count() const;
    int block_color(int) const;
    VectorI block_location(int) const;
private:
    using RotateFunc = VectorI(*)(VectorI);
    bool move(const Grid<int> &, VectorI & location, VectorI offset) const;
    bool rotate(const Grid<int> &, std::vector<Block> &, RotateFunc) const;

    static VectorI rotate_plus_halfpi (VectorI r) { return VectorI(-r.y, r.x); }
    static VectorI rotate_minus_halfpi(VectorI r) { return VectorI(r.y, -r.x); }

    std::vector<Block> m_blocks;
    VectorI m_location;
    bool m_rotation_enabled = true;
};
#if 0
std::size_t to_index(Polyomino::Domino);
std::size_t to_index(Polyomino::Tromino);
std::size_t to_index(Polyomino::Tetromino);
std::size_t to_index(Polyomino::Pentomino);
std::size_t get_count(Polyomino::PolyominoSet);
#endif

using PolyominoEnabledSet = std::bitset<Polyomino::k_total_polyomino_count>;
const char * to_string(Polyomino::PolyominoSet);