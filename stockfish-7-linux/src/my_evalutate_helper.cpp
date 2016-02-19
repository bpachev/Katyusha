/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2016 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <sstream>
#include <string>

#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "timeman.h"
#include "uci.h"


double centipawn_evaluate(Position& pos)
{
  Value v = evaluate<true>(pos);
  v = pos.side_to_move() == WHITE ? v : -v; // White's point of view
  return to_cp(v); //convert to centipawns
}

double evaluate_fen(string fen)
{
  Position pos(fen, false, Threads.main());
  return centipawn_evaluate(pos);
}

string analyze_game(string moves)
{
   SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);

    istringstream is(moves);


    std::stringstream ss;
    ss << centipawn_evaluate(pos);

    // Parse move list (if any)
    while (is >> token && (m = UCI::to_move(pos, token)) != MOVE_NONE)
    {
        SetupStates->push(StateInfo());
        pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));        
        ss << "," << centipawn_evaluate(pos);
    }
    return ss.str();
}


