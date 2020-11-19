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

constexpr const char * const k_settings_filename = "bgsettings.bin";

PolyominoEnabledSet enable_tetromino_only();

// program wide settings for various games
struct Settings {
    Settings();
    Settings(const Settings &) = delete;
    Settings(Settings &&) = delete;
    ~Settings();

    Settings & operator = (const Settings &) = delete;
    Settings & operator = (Settings &&) = delete;

    struct Board {
        Board() {}
        Board(int width_, int height_, int colors_):
            width(width_), height(height_), colors(colors_)
        {}
        int width  = k_min_board_size;
        int height = k_min_board_size;
        int colors = k_min_colors;
    };
    struct Puyo : public Board {
        Puyo();
        int pop_requirement = 4;
        double fall_speed = 1.5;

        int scenario_number = k_free_play_scenario;
    };
    struct Tetris : public Board {
        Tetris();
        PolyominoEnabledSet enabled_polyominos = enable_tetromino_only();
        double fall_speed = 1.5;
    };
    struct SameGame : public Board {
        SameGame();
        bool gameover_on_singles = false;
    };

    Puyo puyo;
    Tetris tetris;
    SameGame samegame;
};

void save_settings(const Settings &) noexcept;
