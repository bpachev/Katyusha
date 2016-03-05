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
#include "analyze.h"

using namespace std;

namespace {

const char* StartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Search::StateStackPtr SetupStates;

double to_cp(Value v) { return double(v) / PawnValueEg; }

double centipawn_evaluate(Position& pos)
{
  Value v = Eval::evaluate<true>(pos);
  v = pos.side_to_move() == WHITE ? v : -v; // White's point of view
  return to_cp(v); //convert to centipawns
}

double evaluate_fen(string fen)
{
  Position pos(fen, false, Threads.main());
  return centipawn_evaluate(pos);
}

string analyze_game(string& moves)
{
   SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);

    istringstream is(moves);

    std::stringstream ss;
    string token;
    Move m;
    Position pos(StartFEN, false, Threads.main());
    ss << centipawn_evaluate(pos);

    // Parse move list
    while (is >> token && (m = UCI::to_move(pos, token)) != MOVE_NONE)
    {
        SetupStates->push(StateInfo());
        pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));
        ss << "," << centipawn_evaluate(pos);
    }
    return ss.str();
}

}//namespace


void output_game(ostream& out, string& mov_str)
{
 out << analyze_game(mov_str) << "\n";
}

/*
 Reads in a file of games, and writes the static evaluation move-by-move to the given outputfile.
*/
void Analyze::game_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  fstream f;
  f.open(infile);
  string line;
  ofstream out;
  out.open(ofile);

  string mov_str;
  while (getline(f, line))
  {
    if (line.length())
    {
      mov_str += line;
      mov_str += " "; //so there will be a space between moves
    }
    else
    {
      output_game(out, mov_str);
      mov_str = "";
    }
  }

  if (mov_str.length())
  {
    output_game(out, mov_str);
  }

  f.close();
  out.close();
}

/*

*/
void Analyze::pos_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  fstream f;
  f.open(infile);
  string line;
  ofstream out;
  out.open(ofile);
  while(getline(f, line))
  {
    if (line.length()) {
      double e = evaluate_fen(line);
      out << line << '\n' <<  e << '\n';
    }
  }
  f.close();
  out.close();
}


//extract a feature representation from the position pos
//The features are as follows
// Side to move:
// Castling WK, WQ, BK, BQ

#define NFEATURES 400

enum feature {
   TEMPO_FEATURE,
   WCASTLE_OO,
   WCASTLE_OOO,
   BCASTLE_OO,
   BCASTLE_OOO,
   NUM_WQ,
   NUM_WR
   NUM_WB,
   NUM_WN,
   NUM_WP,
   NUM_BQ,
   NUM_BR
   NUM_BB,
   NUM_BN,
   NUM_BP,

   WQ_EXISTS,
   BQ_EXISTS,
   WR_EXISTS,
   BR_EXISTS,
   WB_EXISTS,
   BB_EXISTS,

};

//utility function to convert Stockfish's internal feature representation into a feature vector I can use to train Katusha.
// NOTE: this feature representation is based on that used by Giraffe, with some tweaks.
void Analyze::Katyusha_pos_rep(Position& pos)
{
  int i;
  int * features = (int*)malloc(sizeof(int) * NFEATURES);
  std::memeset(features, 0, sizeof(int)*NFEATURES);

  //tempo
  if (pos.side_to_move() == WHITE)
  {
    features[TEMPO_FEATURE] = 1;
  }
  else features[TEMPO_FEATURE] = 0;

  //castling rights
  features[WCASTLE_OO] = pos.can_castle(WHITE_OO);
  features[WCASTLE_OOO] = pos.can_castle(WHITE_OOO);
  features[BCASTLE_OO] = pos.can_castle(BLACK_OO);
  features[BCASTLE_OOO] = pos.can_castle(BLACK_OOO);

  //material configuration
  features[NUM_WQ] = <QUEEN>pos.count(WHITE);
  features[NUM_WR] = <ROOK>pos.count(WHITE);
  features[NUM_WB] = <BISHOP>pos.count(WHITE);
  features[NUM_WN] = <KNIGHT>pos.count(WHITE);
  features[NUM_WP] = <PAWN>pos.count(WHITE);
  features[NUM_BQ] = <QUEEN>pos.count(BLACK);
  features[NUM_BR] = <ROOK>pos.count(BLACK);
  features[NUM_BB] = <BISHOP>pos.count(BLACK);
  features[NUM_BN] = <KNIGHT>pos.count(BLACK);
  features[NUM_BP] = <PAWN>pos.count(BLACK);

  //piece-centric information
  //this consists of coordinates, existence, and mobility
  //NOTE: the below code is generated using a python script
  // I am trying to produce reasonably efficient code that is quick to develop and avoids bugs
  features[WQ1_EXISTS] = (<QUEEN>pos.count(WHITE) >= 1) ? 1 : 0;
  if (features[WQ1_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[1];
    features[WQ1_RANK] = piece_sq/8;
    features[WQ1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(QUEEN,WHITE);
    features[WQ1_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WR1_EXISTS] = (<ROOK>pos.count(WHITE) >= 1) ? 1 : 0;
  if (features[WR1_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[1];
    features[WR1_RANK] = piece_sq/8;
    features[WR1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(ROOK,WHITE);
    features[WR1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[WR2_EXISTS] = (<ROOK>pos.count(WHITE) >= 2) ? 1 : 0;
  if (features[WR2_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[2];
    features[WR2_RANK] = piece_sq/8;
    features[WR2_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(ROOK,WHITE);
    features[WR2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WB1_EXISTS] = (<BISHOP>pos.count(WHITE) >= 1) ? 1 : 0;
  if (features[WB1_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[1];
    features[WB1_RANK] = piece_sq/8;
    features[WB1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(BISHOP,WHITE);
    features[WB1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[WB2_EXISTS] = (<BISHOP>pos.count(WHITE) >= 2) ? 1 : 0;
  if (features[WB2_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[2];
    features[WB2_RANK] = piece_sq/8;
    features[WB2_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(BISHOP,WHITE);
    features[WB2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WK1_EXISTS] = (<KNIGHT>pos.count(WHITE) >= 1) ? 1 : 0;
  if (features[WK1_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[1];
    features[WK1_RANK] = piece_sq/8;
    features[WK1_FILE] = piece_sq%8;
  }

  features[WK2_EXISTS] = (<KNIGHT>pos.count(WHITE) >= 2) ? 1 : 0;
  if (features[WK2_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[2];
    features[WK2_RANK] = piece_sq/8;
    features[WK2_FILE] = piece_sq%8;
  }


  features[WP1_EXISTS] = (<PAWN>pos.count(WHITE) >= 1) ? 1 : 0;
  if (features[WP1_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[1];
    features[WP1_RANK] = piece_sq/8;
    features[WP1_FILE] = piece_sq%8;
  }

  features[WP2_EXISTS] = (<PAWN>pos.count(WHITE) >= 2) ? 1 : 0;
  if (features[WP2_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[2];
    features[WP2_RANK] = piece_sq/8;
    features[WP2_FILE] = piece_sq%8;
  }

  features[WP3_EXISTS] = (<PAWN>pos.count(WHITE) >= 3) ? 1 : 0;
  if (features[WP3_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[3];
    features[WP3_RANK] = piece_sq/8;
    features[WP3_FILE] = piece_sq%8;
  }

  features[WP4_EXISTS] = (<PAWN>pos.count(WHITE) >= 4) ? 1 : 0;
  if (features[WP4_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[4];
    features[WP4_RANK] = piece_sq/8;
    features[WP4_FILE] = piece_sq%8;
  }

  features[WP5_EXISTS] = (<PAWN>pos.count(WHITE) >= 5) ? 1 : 0;
  if (features[WP5_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[5];
    features[WP5_RANK] = piece_sq/8;
    features[WP5_FILE] = piece_sq%8;
  }

  features[WP6_EXISTS] = (<PAWN>pos.count(WHITE) >= 6) ? 1 : 0;
  if (features[WP6_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[6];
    features[WP6_RANK] = piece_sq/8;
    features[WP6_FILE] = piece_sq%8;
  }

  features[WP7_EXISTS] = (<PAWN>pos.count(WHITE) >= 7) ? 1 : 0;
  if (features[WP7_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[7];
    features[WP7_RANK] = piece_sq/8;
    features[WP7_FILE] = piece_sq%8;
  }

  features[WP8_EXISTS] = (<PAWN>pos.count(WHITE) >= 8) ? 1 : 0;
  if (features[WP8_EXISTS]) {
    Square piece_sq = pos.squares(WHITE)[8];
    features[WP8_RANK] = piece_sq/8;
    features[WP8_FILE] = piece_sq%8;
  }



  features[BQ1_EXISTS] = (<QUEEN>pos.count(BLACK) >= 1) ? 1 : 0;
  if (features[BQ1_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[1];
    features[BQ1_RANK] = piece_sq/8;
    features[BQ1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(QUEEN,BLACK);
    features[BQ1_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BR1_EXISTS] = (<ROOK>pos.count(BLACK) >= 1) ? 1 : 0;
  if (features[BR1_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[1];
    features[BR1_RANK] = piece_sq/8;
    features[BR1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(ROOK,BLACK);
    features[BR1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[BR2_EXISTS] = (<ROOK>pos.count(BLACK) >= 2) ? 1 : 0;
  if (features[BR2_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[2];
    features[BR2_RANK] = piece_sq/8;
    features[BR2_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(ROOK,BLACK);
    features[BR2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BB1_EXISTS] = (<BISHOP>pos.count(BLACK) >= 1) ? 1 : 0;
  if (features[BB1_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[1];
    features[BB1_RANK] = piece_sq/8;
    features[BB1_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(BISHOP,BLACK);
    features[BB1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[BB2_EXISTS] = (<BISHOP>pos.count(BLACK) >= 2) ? 1 : 0;
  if (features[BB2_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[2];
    features[BB2_RANK] = piece_sq/8;
    features[BB2_FILE] = piece_sq%8;
    Bitboard piece_attacks = pos.attacks_from(BISHOP,BLACK);
    features[BB2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BK1_EXISTS] = (<KNIGHT>pos.count(BLACK) >= 1) ? 1 : 0;
  if (features[BK1_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[1];
    features[BK1_RANK] = piece_sq/8;
    features[BK1_FILE] = piece_sq%8;
  }

  features[BK2_EXISTS] = (<KNIGHT>pos.count(BLACK) >= 2) ? 1 : 0;
  if (features[BK2_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[2];
    features[BK2_RANK] = piece_sq/8;
    features[BK2_FILE] = piece_sq%8;
  }


  features[BP1_EXISTS] = (<PAWN>pos.count(BLACK) >= 1) ? 1 : 0;
  if (features[BP1_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[1];
    features[BP1_RANK] = piece_sq/8;
    features[BP1_FILE] = piece_sq%8;
  }

  features[BP2_EXISTS] = (<PAWN>pos.count(BLACK) >= 2) ? 1 : 0;
  if (features[BP2_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[2];
    features[BP2_RANK] = piece_sq/8;
    features[BP2_FILE] = piece_sq%8;
  }

  features[BP3_EXISTS] = (<PAWN>pos.count(BLACK) >= 3) ? 1 : 0;
  if (features[BP3_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[3];
    features[BP3_RANK] = piece_sq/8;
    features[BP3_FILE] = piece_sq%8;
  }

  features[BP4_EXISTS] = (<PAWN>pos.count(BLACK) >= 4) ? 1 : 0;
  if (features[BP4_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[4];
    features[BP4_RANK] = piece_sq/8;
    features[BP4_FILE] = piece_sq%8;
  }

  features[BP5_EXISTS] = (<PAWN>pos.count(BLACK) >= 5) ? 1 : 0;
  if (features[BP5_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[5];
    features[BP5_RANK] = piece_sq/8;
    features[BP5_FILE] = piece_sq%8;
  }

  features[BP6_EXISTS] = (<PAWN>pos.count(BLACK) >= 6) ? 1 : 0;
  if (features[BP6_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[6];
    features[BP6_RANK] = piece_sq/8;
    features[BP6_FILE] = piece_sq%8;
  }

  features[BP7_EXISTS] = (<PAWN>pos.count(BLACK) >= 7) ? 1 : 0;
  if (features[BP7_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[7];
    features[BP7_RANK] = piece_sq/8;
    features[BP7_FILE] = piece_sq%8;
  }

  features[BP8_EXISTS] = (<PAWN>pos.count(BLACK) >= 8) ? 1 : 0;
  if (features[BP8_EXISTS]) {
    Square piece_sq = pos.squares(BLACK)[8];
    features[BP8_RANK] = piece_sq/8;
    features[BP8_FILE] = piece_sq%8;
  }


  //square-centric information
  //attack and defend maps
  for (i = 0; i < 64; i++)
  {
    features[SQUARE_OFF+i] = ;
  }

  //pawn information

  //something about king safety


}
