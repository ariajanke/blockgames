#include "Settings.hpp"

#include <fstream>

Settings::Settings() {
    try {
        std::ifstream fin;
        fin.open(k_settings_filename);
        // settings are not meant to be transferred between machines
        fin.read(reinterpret_cast<char *>(this), sizeof(decltype(this)));
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
        fout.write(reinterpret_cast<const char *>(&settings), sizeof(Settings));
    }  catch (...) {
        // give up basically
    }
}
