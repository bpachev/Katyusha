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

#ifndef ANALYZE_H_INCLUDED
#define ANALYZE_H_INCLUDED

#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

#include "evaluate.h"
#include "movegen.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "timeman.h"
#include "uci.h"
#include "bitcount.h"
#include "material.h"
#include "pawns.h"

using namespace std;

namespace Analyze {
  //The features are as follows
  // Side to move:
  // Castling WK, WQ, BK, BQ

  enum feature {
     SIDE_TO_MOVE,
     WCASTLE_OO,
     WCASTLE_OOO,
     BCASTLE_OO,
     BCASTLE_OOO,
     NUM_WQ,
     NUM_WR,
     NUM_WB,
     NUM_WN,
     NUM_WP,
     NUM_BQ,
     NUM_BR,
     NUM_BB,
     NUM_BN,
     NUM_BP,
     WK_RANK, WK_FILE, BK_RANK, BK_FILE,
     WQ1_EXISTS, WQ1_RANK, WQ1_FILE, WQ1_MIN_DEFENDER, WQ1_MIN_ATTACKER, WQ1_SQUARES,
     WR1_EXISTS, WR1_RANK, WR1_FILE, WR1_MIN_DEFENDER, WR1_MIN_ATTACKER, WR1_SQUARES,
     WR2_EXISTS, WR2_RANK, WR2_FILE, WR2_MIN_DEFENDER, WR2_MIN_ATTACKER, WR2_SQUARES,
     WB1_EXISTS, WB1_RANK, WB1_FILE, WB1_MIN_DEFENDER, WB1_MIN_ATTACKER, WB1_SQUARES,
     WB2_EXISTS, WB2_RANK, WB2_FILE, WB2_MIN_DEFENDER, WB2_MIN_ATTACKER, WB2_SQUARES,
     WK1_EXISTS, WK1_RANK, WK1_FILE, WK1_MIN_DEFENDER, WK1_MIN_ATTACKER,
     WK2_EXISTS, WK2_RANK, WK2_FILE, WK2_MIN_DEFENDER, WK2_MIN_ATTACKER,
     WP1_EXISTS, WP1_RANK, WP1_FILE, WP1_MIN_DEFENDER, WP1_MIN_ATTACKER, WP2_EXISTS, WP2_RANK, WP2_FILE, WP2_MIN_DEFENDER, WP2_MIN_ATTACKER, WP3_EXISTS, WP3_RANK, WP3_FILE, WP3_MIN_DEFENDER, WP3_MIN_ATTACKER, WP4_EXISTS, WP4_RANK, WP4_FILE, WP4_MIN_DEFENDER, WP4_MIN_ATTACKER,
     WP5_EXISTS, WP5_RANK, WP5_FILE, WP5_MIN_DEFENDER, WP5_MIN_ATTACKER, WP6_EXISTS, WP6_RANK, WP6_FILE, WP6_MIN_DEFENDER, WP6_MIN_ATTACKER, WP7_EXISTS, WP7_RANK, WP7_FILE, WP7_MIN_DEFENDER, WP7_MIN_ATTACKER, WP8_EXISTS, WP8_RANK, WP8_FILE, WP8_MIN_DEFENDER, WP8_MIN_ATTACKER,
     BQ1_EXISTS, BQ1_RANK, BQ1_FILE, BQ1_MIN_DEFENDER, BQ1_MIN_ATTACKER, BQ1_SQUARES, BR1_EXISTS, BR1_RANK, BR1_FILE, BR1_MIN_DEFENDER, BR1_MIN_ATTACKER, BR1_SQUARES, BR2_EXISTS, BR2_RANK, BR2_FILE, BR2_MIN_DEFENDER, BR2_MIN_ATTACKER, BR2_SQUARES,
     BB1_EXISTS, BB1_RANK, BB1_FILE, BB1_MIN_DEFENDER, BB1_MIN_ATTACKER, BB1_SQUARES, BB2_EXISTS, BB2_RANK, BB2_FILE, BB2_MIN_DEFENDER, BB2_MIN_ATTACKER, BB2_SQUARES,
     BK1_EXISTS, BK1_RANK, BK1_FILE, BK1_MIN_DEFENDER, BK1_MIN_ATTACKER, BK2_EXISTS, BK2_RANK, BK2_FILE, BK2_MIN_DEFENDER, BK2_MIN_ATTACKER,
     BP1_EXISTS, BP1_RANK, BP1_FILE, BP1_MIN_DEFENDER, BP1_MIN_ATTACKER, BP2_EXISTS, BP2_RANK, BP2_FILE, BP2_MIN_DEFENDER, BP2_MIN_ATTACKER,
     BP3_EXISTS, BP3_RANK, BP3_FILE, BP3_MIN_DEFENDER, BP3_MIN_ATTACKER, BP4_EXISTS, BP4_RANK, BP4_FILE, BP4_MIN_DEFENDER, BP4_MIN_ATTACKER, BP5_EXISTS, BP5_RANK, BP5_FILE, BP5_MIN_DEFENDER, BP5_MIN_ATTACKER,
     BP6_EXISTS, BP6_RANK, BP6_FILE, BP6_MIN_DEFENDER, BP6_MIN_ATTACKER,
     BP7_EXISTS, BP7_RANK, BP7_FILE, BP7_MIN_DEFENDER, BP7_MIN_ATTACKER,
     BP8_EXISTS, BP8_RANK, BP8_FILE, BP8_MIN_DEFENDER, BP8_MIN_ATTACKER,
     ATTACK_SQUARE_OFF,
     DEFEND_SQUARE_OFF = ATTACK_SQUARE_OFF+64,
     WHITE_PAWN_FILE = DEFEND_SQUARE_OFF+64,
     BLACK_PAWN_FILE = WHITE_PAWN_FILE+8,
     NB_FEATURES = BLACK_PAWN_FILE+8
  };


void evaluate_game_list(string infile, string ofile);
void evaluate_game_list(std::istringstream& is);
void evaluate_pos_list(string infile, string ofile);
void evaluate_pos_list(std::istringstream& is);

void print_pos_features(Position& pos);
void feature_game_list(string infile, string ofile);
void feature_game_list(std::istringstream& is);
void feature_pos_list(string infile, string ofile);
void feature_pos_list(std::istringstream& is);
void print_pos_rep(Position& pos);
void process_game_list(string infile, string ofile);
void process_pos_list(string infile, string ofile);
void gen_training_set(string infile, string ofile, int npositions);
void random_moves(Position& pos, int moves, int punishment_moves);
void Katyusha_pos_rep(const Position& pos, int * features);


}

#endif // #ifndef ANALYZE_H_INCLUDED
