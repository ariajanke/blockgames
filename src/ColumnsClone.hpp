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

#include "BoardStates.hpp"
#include "Dialog.hpp"
#include "PlayControl.hpp"

class ColumnsPiece final : public FallingPieceBase {
public:
    static constexpr const int k_piece_size = 3;
    using BlocksArray = std::array<std::pair<VectorI, BlockId>, k_piece_size>;
    ColumnsPiece() {}
    ColumnsPiece(BlockId bottom, BlockId mid, BlockId top);

    void rotate_down();
    void rotate_up();

    bool descend(const BlockGrid &);
    void move_left(const BlockGrid &) override;
    void move_right(const BlockGrid &) override;

    void place(BlockGrid &) const;
    // places piece at the top
    void set_column_position(int);

    // it is quite possible for a block to fall outside of the grid
    BlocksArray as_blocks() const;

    VectorI bottom() const;

private:
    void rotate_left(const BlockGrid &) override { rotate_up(); }
    void rotate_right(const BlockGrid &) override { rotate_down(); }

    enum { k_top, k_middle, k_bottom };
    static constexpr const auto k_positions = { k_top, k_middle, k_bottom };

    static VectorI offset_for(decltype(k_top));

    void rotate(int direction);
    bool move(const BlockGrid &, VectorI offset);
    void check_invarients() const;

    std::array<BlockId, k_piece_size> m_blocks = {};
    VectorI m_bottom = VectorI(0, -1);
};

class ColumnsState final : public PauseableWithFallingPieceState {
    void setup_board(const Settings &) override;

    int width_in_blocks () const override;

    int height_in_blocks() const override;

    void update(double et) override;
#   if 0
    void process_event(const sf::Event & event) override;

    void handle_event(PlayControlEvent) override;
#   endif
    void draw(sf::RenderTarget &, sf::RenderStates) const override;

    int scale() const override { return 3; }

    FallingPieceBase & piece_base() override { return m_falling_piece; }

    const BlockGrid & blocks() const override { return m_blocks; }

    void check_invarients() const;
#   if 0
    static constexpr const double k_fast_fall = 5.;
    static constexpr const double k_slow_fall = 1.;
#   endif
    ColumnsPiece m_falling_piece;
    double m_fall_offset     = 0.; // normalized to [0 1]
    double m_fall_rate       = 1.5;
#   if 0
    double m_fall_multiplier = k_slow_fall;
    bool m_paused = false;
#   endif
    FallEffectsFull m_fall_ef;

    BlockGrid m_blocks;
    Rng m_rng = Rng{ std::random_device()() };
};

class ColumnsSettingsDialog final : public Dialog {
    void setup_() override;

    ksg::TextArea m_unimplemented;
    ksg::TextButton m_back_to_main;
};
