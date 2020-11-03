#include "Settings.hpp"

#include <fstream>

#include <cassert>

namespace {

using Board = Settings::Board;

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
    try {
        std::ifstream fin;
        fin.open(k_settings_filename);
        // settings are not meant to be transferred between machines
        Puyo puyo;
        static_cast<Board &>(puyo) = load_board(fin);
        puyo.fall_speed      = load_f32(fin);
        puyo.pop_requirement = load_i8(fin);

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

void save_settings(const Settings & settings) noexcept {
    try {
        std::ofstream fout;
        fout.open(k_settings_filename);
        // settings are not meant to be transferred between machines
        save_board(fout, settings.puyo);
        save_f32(fout, float(settings.puyo.fall_speed));
        save_i8(fout, int8_t(settings.puyo.pop_requirement));

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
