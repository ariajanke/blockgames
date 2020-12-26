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

#include "Settings.hpp"
#include "PuyoScenario.hpp"

#include <fstream>

#include <cassert>

namespace {

using Board = Settings::Board;
#if 0
// free play scenarios only!
const auto k_builtin_puyo_scenario_count = []() {
    int count = 0;
    for (auto & uptr : Scenario::make_all_scenarios()) {
        if (uptr->is_sequential()) break;
        ++count;
    }
    return count;
}();
#endif
} // end of <anonymous> namespace

static void     save_i8 (std::ostream &, int8_t);
static void     save_f32(std::ostream &, float );
static void     save_u32(std::ostream &, uint32_t);
static int8_t   load_i8 (std::istream &);
static float    load_f32(std::istream &);
static uint32_t load_u32(std::istream &);

static_assert(Polyomino::k_total_polyomino_count <= 32, "");

static void  save_board(std::ostream &, const Board &);
static Board load_board(std::istream &);

Settings::Settings() {
    assert(Scenario::k_freeplay_scenario_count <= std::numeric_limits<int8_t>::max());
#   if 0
    m_puyo_scenarios.resize(Scenario::k_freeplay_scenario_count);
    std::vector<Puyo> puyo_scenarios;
#   endif
    std::map<const char *, Puyo> scen_map;
    // populate the map first before reading!
    for (const auto & scen_ptr : Scenario::get_all_scenarios()) {
        scen_map[scen_ptr->name()] = scen_ptr->default_settings();
    }
    try {
        std::ifstream fin;
        fin.open(k_settings_filename);
        fin.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        // settings are not meant to be transferred between machines
#       if 0
        auto scen_count = load_i8(fin);
        if (scen_count != Scenario::k_freeplay_scenario_count) {
            // do not load (invalidate) if scenario numbers do not match
            // (shouldn't I tell the user at some point?)
            return;
        }
#       endif
        for (const auto & scen_ptr : Scenario::get_all_scenarios()) {
            auto itr = scen_map.find(scen_ptr->name());
            assert(itr != scen_map.end());
            auto & scen = itr->second;
            if (scen.colors != k_unused_i) {
                scen.colors = load_i8(fin);
            }
            if (scen.width != k_unused_i) {
                scen.width = load_i8(fin);
            }
            if (scen.height != k_unused_i) {
                scen.height = load_i8(fin);
            }
            if (scen.pop_requirement != k_unused_i) {
                scen.pop_requirement = load_i8(fin);
            }
            if (std::equal_to<double>()(k_unused_d, scen.fall_speed)) {
                scen.fall_speed = load_f32(fin);
            }
        }
#       if 0
        puyo_scenarios.resize(scen_count);
        for (auto & board : puyo_scenarios) {
            static_cast<Board &>(board) = load_board(fin);
            board.fall_speed      = load_f32(fin);
            board.pop_requirement = load_i8(fin);
        }
#       endif
        default_puyo_freeplay_scenario = load_i8(fin);

        Tetris tetris;
        static_cast<Board &>(tetris) = load_board(fin);
        tetris.fall_speed = load_f32(fin);
        tetris.enabled_polyominos = PolyominoEnabledSet(load_u32(fin));

        SameGame samegame;
        static_cast<Board &>(samegame) = load_board(fin);
        samegame.gameover_on_singles = load_i8(fin) != 0;

    }  catch (...) {
        //
    }
    scen_map.swap(m_puyo_settings);
#   if 0
    m_puyo_scenarios.swap(puyo_scenarios);
#   endif
}

Settings::~Settings()
    { save_settings(*this); }

Settings::Puyo::Puyo(): Board(6, 12, 5) {}

Settings::Tetris::Tetris(): Board(10, 20, 3) {}

Settings::SameGame::SameGame(): Board(12, 12, 4) {}

PolyominoEnabledSet enable_tetromino_only() {
    PolyominoEnabledSet rv;
    std::size_t i = Polyomino::k_domino_count + Polyomino::k_tromino_count;
    std::size_t end = i + Polyomino::k_tetromino_count;
    for (; i != end; ++i) {
        rv.set(i);
    }
    return rv;
}

const Settings::Puyo & Settings::get_puyo_settings(const char * name_ptr) const {
    auto itr = m_puyo_settings.find(name_ptr);
    if (itr == m_puyo_settings.end()) {
        throw std::invalid_argument("Settings::get_puyo_settings: cannot find name pointer (was matching string contents provided?)");
    }
    return itr->second;
}

Settings::WritablePuyo Settings::get_puyo_settings(const char * name_ptr) {
    auto itr = m_puyo_settings.find(name_ptr);
    if (itr == m_puyo_settings.end()) {
        throw std::invalid_argument("Settings::get_puyo_settings: cannot find name pointer (was matching string contents provided?)");
    }
    return WritablePuyo(itr->second);
}

#if 0
Settings::Puyo & Settings::get_puyo_scenario(int idx)
    { return m_puyo_scenarios.at(std::size_t(idx)); }

const Settings::Puyo & Settings::get_puyo_scenario(int idx) const
    { return m_puyo_scenarios.at(std::size_t(idx)); }

int Settings::puyo_scenario_count() const
    { return int(m_puyo_scenarios.size()); }
#endif
void save_settings(const Settings & settings) noexcept {
    static constexpr const auto k_unused_i = Settings::k_unused_i;
    static constexpr const auto k_unused_d = Settings::k_unused_d;
    try {
        std::ofstream fout;
        fout.open(k_settings_filename);
        // settings are not meant to be transferred between machines
#       if 0
        assert(settings.puyo_scenario_count() <= std::numeric_limits<int8_t>::max());
        save_i8(fout, settings.puyo_scenario_count());
        for (auto & board : make_puyo_settings_view(settings)) {
            save_board(fout, board);
            save_f32(fout, float(board.fall_speed));
            save_i8 (fout, int8_t(board.pop_requirement));
        }
#       endif
        for (const auto & [name_ptr, scen] : settings.m_puyo_settings) {
            (void)name_ptr;
            if (scen.colors != k_unused_i) {
                save_i8(fout, scen.colors);
            }
            if (scen.width != k_unused_i) {
                save_i8(fout, scen.width);
            }
            if (scen.height != k_unused_i) {
                save_i8(fout, scen.height);
            }
            if (scen.pop_requirement != k_unused_i) {
                save_i8(fout, scen.pop_requirement);
            }
            if (std::equal_to<double>()(k_unused_d, scen.fall_speed)) {
                save_i8(fout, scen.fall_speed);
            }
        }
        save_i8(fout, settings.default_puyo_freeplay_scenario);

        save_board(fout, settings.tetris);
        save_f32(fout, settings.tetris.fall_speed);
        save_u32(fout, uint32_t(settings.tetris.enabled_polyominos.to_ulong()));

        save_board(fout, settings.samegame);
        save_i8(fout, settings.samegame.gameover_on_singles ? 1 : 0);
    }  catch (...) {
        // give up basically
    }
}

static void save_i8(std::ostream & out, int8_t i) {
    out.write(reinterpret_cast<const char *>(&i), sizeof(int8_t));
}

static void save_f32(std::ostream & out, float fp) {
    out.write(reinterpret_cast<const char *>(&fp), sizeof(float));
}

static void save_u32(std::ostream & out, uint32_t u) {
    out.write(reinterpret_cast<const char *>(&u), sizeof(uint32_t));
}

static int8_t load_i8(std::istream & in) {
    int8_t out;
    in.read(reinterpret_cast<char *>(&out), sizeof(int8_t));
    return out;
}

static float load_f32(std::istream & in) {
    float out;
    in.read(reinterpret_cast<char *>(&out), sizeof(float));
    return out;
}

static uint32_t load_u32(std::istream & in) {
    uint32_t out;
    in.read(reinterpret_cast<char *>(&out), sizeof(uint32_t));
    return out;
}

static void save_board(std::ostream & out, const Board & board) {
    save_i8(out, board.colors);
    save_i8(out, board.height);
    save_i8(out, board.width );
}

static Board load_board(std::istream & in) {
    Board rv;
    rv.colors = load_i8(in);
    rv.height = load_i8(in);
    rv.width  = load_i8(in);
    return rv;
}
