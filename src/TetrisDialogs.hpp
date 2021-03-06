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
#include "Polyomino.hpp"
#include "Dialog.hpp"

#include <ksg/Frame.hpp>
#include <ksg/Button.hpp>
#include <ksg/TextButton.hpp>
#include <ksg/TextArea.hpp>
#include <ksg/OptionsSlider.hpp>

using EnabledPolyominoBits = std::bitset<Polyomino::k_total_polyomino_count>;

class PolyominoDialogPage : public ksg::Frame {
public:
    using PolyominoItr = std::vector<Polyomino>::const_iterator;
    virtual PolyominoItr set
        (PolyominoItr cont_beg, PolyominoItr beg, PolyominoItr end,
         EnabledPolyominoBits &) = 0;
    virtual void update_selections() = 0;
};

// ----------------------------------------------------------------------------

class PolyominoButton final : public ksg::Button {
public:
    void set_polyomino(const Polyomino & poly);

    // scale depends on the dims of the polyomino
    void set_on () { m_on = true ; }
    void set_off() { m_on = false; }

private:
    static constexpr const int k_use_small_thershold = 3;
    static constexpr const int k_big_scale   = 2;
    static constexpr const int k_small_scale = 1;
    static constexpr const int k_max_size_in_blocks = 6;
    static constexpr const int k_pixels_for_blocks = k_block_size*k_max_size_in_blocks;

    static_assert(k_big_scale  *k_block_size*k_use_small_thershold <= k_pixels_for_blocks, "");
    static_assert(k_small_scale*k_block_size*k_max_size_in_blocks  <= k_pixels_for_blocks, "");

    void draw(sf::RenderTarget & target, sf::RenderStates states) const override;

    void issue_auto_resize() override;

    bool m_on = false;
    VectorF m_polyomino_offset;
    Polyomino m_polyomino;
    int m_scale = 2;
};

// ----------------------------------------------------------------------------

class PolyominoSetSelectPage final : public PolyominoDialogPage {
public:
    using BoardOptions = Settings::Board;

    explicit PolyominoSetSelectPage(BoardOptions &);

    PolyominoItr set
        (PolyominoItr cont_beg, PolyominoItr, PolyominoItr,
         EnabledPolyominoBits & enabledbits) override;

    void update_selections() override;
private:
    using PolyominoSet = Polyomino::PolyominoSet;

    static constexpr const auto k_sets = {
        PolyominoSet::domino, PolyominoSet::tromino, PolyominoSet::tetromino,
        PolyominoSet::pentomino
    };

    void setup();

    // gets [begin end)
    std::pair<std::size_t, std::size_t> range_for_set(PolyominoSet) const;

    enum AmtSet { k_some, k_all, k_none };
    AmtSet amount_set_for(PolyominoSet set) const;

    const UString & display_string_for_set(PolyominoSet set) const;

    EnabledPolyominoBits * m_enabled_polyominos = nullptr;

    std::vector<ksg::TextArea  > m_set_notices;
    std::vector<ksg::TextButton> m_set_endis_buttons;

    BoardConfigDialog m_board_config;
    ksg::TextArea m_poly_set_nfo;

    ksg::TextArea m_all_off_notice;
};

// ----------------------------------------------------------------------------

class PolyominoIndividualSelectPage final : public PolyominoDialogPage {
public:
    PolyominoItr set
        (PolyominoItr cont_beg, PolyominoItr beg, PolyominoItr end,
         EnabledPolyominoBits & enabledbits) override;

    void update_selections() override;

private:
    static constexpr const int k_max_polyominos_per_page = 6;
    static constexpr const int k_max_polyominos_per_row  = 3;
    static constexpr const auto * k_off_string = U"Off";
    static constexpr const auto * k_on_string = U"On";

    void check_invarients() const;

    EnabledPolyominoBits * m_enabled_polyominos = nullptr;
    std::size_t m_start_index;

    ksg::TextArea m_activate_polyonmino_notice;

    std::vector<PolyominoButton> m_buttons;
    std::vector<ksg::TextArea  > m_poly_enabled_ta;
};

// ----------------------------------------------------------------------------

class PolyominoSelectDialog final : public Dialog {
public:
    using PagePtr = std::unique_ptr<PolyominoDialogPage>;

private:
    void setup_() override;

    void flip_to_page(PolyominoDialogPage & page);

    ksg::TextButton m_back_to_menu;
    ksg::OptionsSlider m_page_slider;

    std::vector<PagePtr> m_pages;
#   if 0
    EnabledPolyominoBits m_enabled_polyominos;
#   endif
};
