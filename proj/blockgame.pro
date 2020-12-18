QT      -= core gui
CONFIG  -= c++11

QMAKE_CXXFLAGS += -std=c++17 -pedantic -Wall
QMAKE_LFLAGS   += -std=c++17
LIBS           += -lsfml-graphics -lsfml-window -lsfml-system -lksg -lcommon \
                  -lX11 \ # -ldiscord_game_sdk \
                  -L/usr/lib/x86_64-linux-gnu -L$$PWD/../lib/cul -L$$PWD/../lib/ksg
                  -L$$PWD/../../ext/discord-sdk/lib/x86_64

linux {
    QMAKE_CXXFLAGS += -DMACRO_PLATFORM_LINUX -DMACRO_TEST_DRIVER_ENTRY_FUNCTION=enter_tests_driver
    contains(QT_ARCH, i386) {
        LIBS += -L../../bin/linux/g++-x86
    } else:contains(QT_ARCH, x86_64) {
        LIBS += -L../../bin/linux/g++-x86_64 \
                -L/usr/lib/x86_64-linux-gnu
    }
}

release {
    TARGET  = blockgame
}

debug {
    TARGET   = blockgame
    QMAKE_CXXFLAGS += -DMACRO_DEBUG
}

SOURCES += \
    ../src/Graphics.cpp \
    ../src/main.cpp \
    ../src/BlockAlgorithm.cpp \
    ../src/EffectsFull.cpp \
    ../src/Defs.cpp \
    ../src/FallingPiece.cpp \
    ../src/Polyomino.cpp \
    ../src/AppState.cpp \
    ../src/DialogState.cpp \
    ../src/TetrisDialogs.cpp \
    ../src/PuyoDialogs.cpp \
    ../src/Dialog.cpp \
    ../src/BoardStates.cpp \
    ../src/PuyoScenario.cpp \
    ../src/Settings.cpp \
    ../src/PuyoState.cpp \
    ../src/ColumnsClone.cpp \
    ../src/PlayControl.cpp \
    ../src/WakefullnessUpdater.cpp \
    ../src/PuyoAiScript.cpp \
    ../unit-tests/test-driver.cpp \
    \ ##############################################################
    #../../ext/discord-sdk/cpp/core.cpp \
    #../../ext/discord-sdk/cpp/storage_manager.cpp \
    #../../ext/discord-sdk/cpp/application_manager.cpp \
    #../../ext/discord-sdk/cpp/achievement_manager.cpp \
    #../../ext/discord-sdk/cpp/image_manager.cpp \
    #../../ext/discord-sdk/cpp/overlay_manager.cpp \
    #../../ext/discord-sdk/cpp/activity_manager.cpp \
    #../../ext/discord-sdk/cpp/store_manager.cpp \
    #../../ext/discord-sdk/cpp/voice_manager.cpp \
    #../../ext/discord-sdk/cpp/relationship_manager.cpp \
    #../../ext/discord-sdk/cpp/network_manager.cpp \
    #../../ext/discord-sdk/cpp/user_manager.cpp \
    #../../ext/discord-sdk/cpp/lobby_manager.cpp \
    #../../ext/discord-sdk/cpp/types.cpp

HEADERS += \
    ../src/Graphics.hpp \
    ../src/Defs.hpp \
    ../src/BlockAlgorithm.hpp \
    ../src/EffectsFull.hpp \
    ../src/FallingPiece.hpp \
    ../src/Polyomino.hpp \
    ../src/AppState.hpp \
    ../src/DialogState.hpp \
    ../src/TetrisDialogs.hpp \
    ../src/PuyoDialogs.hpp \
    ../src/Dialog.hpp \
    ../src/BoardStates.hpp \
    ../src/PuyoScenario.hpp \
    ../src/Settings.hpp \
    ../src/PuyoState.hpp \
    ../src/ColumnsClone.hpp \
    ../src/WakefullnessUpdater.hpp \
    ../src/PlayControl.hpp \
    ../src/PuyoAiScript.hpp

INCLUDEPATH += \
    ../lib/cul/inc \
    ../lib/ksg/inc \
    ../../ext/discord-sdk/cpp
