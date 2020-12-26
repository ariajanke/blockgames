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

#include "PuyoState.hpp"

class Scenario {
public:
    virtual ~Scenario();
    static const std::pair<BlockId, BlockId> k_random_pair;

    using Response = PuyoStateN::Response;
    using ContinueFall = PuyoStateN::ContinueFall;
    using PuyoSettings = Settings::Puyo;

    PuyoSettings setup(PuyoSettings settings) {
        verify_unuseds_match(settings);
        return setup_(settings);
    }

    virtual Response on_turn_change() = 0;

    virtual const char * name() const = 0;
    virtual const char * description() const = 0;
    virtual bool is_sequential() const = 0;
    virtual ScenarioPtr clone() const = 0;
    virtual const PuyoSettings & default_settings() const = 0;

    double fall_speed() const;

    void assign_board(BlockSubGrid);

    template<typename T>
    static ScenarioPtr make_scenario() {
        ScenarioPtr rv;
        rv.reset(new T());
        return rv;
    }

    static ScenarioPtr make_pop_forever();
    static ScenarioPtr make_glass_waves();

    static const std::vector<ConstScenarioPtr> & get_all_scenarios();

    static const int k_freeplay_scenario_count;

protected:
    virtual PuyoSettings setup_(PuyoSettings) = 0;

    BlockSubGrid & board();

    void set_fall_speed(double);

private:
    void verify_unuseds_match(const PuyoSettings &) const;

    /** layout guaranteed to be:
     *  is_sequential calls begin with false value then true
     *  afterward each sequential stage is in order
     */
    static std::vector<ScenarioPtr> make_all_scenarios();

    double m_fall_speed = 1.;
    BlockSubGrid m_board_ref;
};

