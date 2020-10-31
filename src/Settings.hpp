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
        int pop_requirement = 2;
        double fall_speed = 0.5;
    };
    struct Tetris : public Board {
        Tetris();
        PolyominoEnabledSet enabled_polyominos = enable_tetromino_only();
        double fall_speed = 0.5;
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
