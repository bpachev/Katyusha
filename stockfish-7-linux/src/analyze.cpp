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

#include "analyze.h"
#include "movegen.h"
#include "cnpy.h"
#include "time.h"
#include "KatyushaEngine.h"

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


void output_game_evals(ostream& out, string& mov_str)
{
 out << analyze_game(mov_str) << "\n";
}

void save_pos_features(Position& pos, float * dest)
{
  int featurevec[Analyze::NB_FEATURES];
  Analyze::Katyusha_pos_rep(pos, featurevec);
  for (int i = 0; i < Analyze::NB_FEATURES; i++) dest[i] = (float)featurevec[i];
}

void output_feature_pos(ofstream& out, Position& pos, int * featurevec)
{
  Analyze::Katyusha_pos_rep(pos, featurevec);
  out << featurevec[0];
  for (int i = 1; i < Analyze::NB_FEATURES; i++) out << "," << featurevec[i];
  out << endl;
}

//we pass in a featurevec so that we don't have to reallocate memory over and over again
void output_feature_game(ofstream& out, string mov_str, int * featurevec)
{
//  cout << mov_str << endl;
  SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);

   istringstream is(mov_str);

   string token;
   Move m;
   Position pos(StartFEN, false, Threads.main());
   output_feature_pos(out, pos, featurevec);

   // Parse move list
   while (is >> token && (m = UCI::to_move(pos, token)) != MOVE_NONE)
   {
       SetupStates->push(StateInfo());
       pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));
       output_feature_pos(out, pos, featurevec);
   }
   out << endl;
}

}//namespace

/*
 Reads in a file of games, and writes the static evaluation move-by-move to the given outputfile.
*/
void Analyze::evaluate_game_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  evaluate_game_list(infile, ofile);
}

void Analyze::evaluate_game_list(string infile, string ofile)
{
  process_game_list(infile, ofile, output_game_evals);
  return;
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
      output_game_evals(out, mov_str);
      mov_str = "";
    }
  }

  if (mov_str.length())
  {
    output_game_evals(out, mov_str);
  }

  f.close();
  out.close();

}

void Analyze::process_game_list(string infile, string ofile, void(*game_func)(ostream&, string&))
{
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
      game_func(out, mov_str);
      mov_str = "";
    }
  }

  if (mov_str.length())
  {
    game_func(out, mov_str);
  }

  f.close();
  out.close();

}

#define MAX_RAND_MOVES 3
#define MAX_PUNISHMENT_MOVES 3


void Analyze::gen_training_set(string infile, string ofile, int npositions)
{
  cout << "infile " << infile << " ofile " << ofile << "npos " << npositions << endl;
  fstream f;
  f.open(infile);
  string line;
  string mov_str;

  //enable Stockfish
  KatyushaEngine::deactivate();

  srand(time(NULL));

  float * training_features = (float*)malloc(sizeof(float)*npositions*(NB_FEATURES));
  float * training_evals = (float*)malloc(sizeof(float) * npositions);
  if (!training_features || !training_evals) cout << "Memory alloc error. " << endl;
  int curPos = 0;

  while (getline(f, line))
  {
    if (line.length())
    {
      mov_str += line;
      mov_str += " "; //so there will be a space between moves
    }
    else
    {
      if (curPos%10000 == 0) cout << "Processing Position " << curPos << endl;
      Position pos(StartFEN, false, Threads.main());
      Move m;
      string token;
      vector<string> move_strs;

      istringstream is(mov_str);
      while (is >> token)
      {
        move_strs.push_back(token);
      }

      int nmoves = move_strs.size();
      if (!nmoves) continue;
      int mnum = rand() % nmoves;

//      cout << analyze_game(mov_str) << endl;
      SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);
      for (int k = 0; k < mnum; k++)
      {
        if ((m = UCI::to_move(pos, move_strs[k])) != MOVE_NONE)
        {
          SetupStates->push(StateInfo());
          pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));
        }
        // if we encountered an invalid move, break out
        else break;
      }

//      cout << "moved to move num " << mnum << "total moves " << move_strs.size() << endl;

//      Position& origPos = pos;
//      cout << pos << "opchecks " << pos.checkers() << endl;
      if (rand()%2) {
      random_capture(pos);
      save_pos_features(pos, training_features+NB_FEATURES*curPos);
      training_evals[curPos] = (float)centipawn_evaluate(pos);
  //    cout << "First Evaluation " << pos << "pchecks "<< pos.checkers() << endl;
    //  if (pos.checkers()) cout << "UhOHp"<<endl;
//      cout << "Orig Pos" << origPos << endl;
      }
      else {
        random_moves(pos, rand()%MAX_RAND_MOVES, rand()%MAX_PUNISHMENT_MOVES);
          save_pos_features(pos, training_features+NB_FEATURES*curPos);
          training_evals[curPos] = (float)centipawn_evaluate(pos);
      }
      if (++curPos >= npositions) break;



      mov_str = "";
    }
  }
  f.close();

  cout << "Writing to outfile" << endl;

  //now write to outfile
  const unsigned int feature_shape[] = {(unsigned int)curPos, (unsigned int)NB_FEATURES};
  int feature_dims = 2;
  const unsigned int evals_shape[] = {(unsigned int)curPos};
  int evals_dims = 1;

  cout << "total " << curPos << "positions out of requested " << npositions << endl;
  cnpy::npz_save(ofile, "training_x", training_features, feature_shape, feature_dims, "a");
  cout << "got training x " << endl;
  cnpy::npz_save(ofile, "training_y", training_evals, evals_shape, evals_dims, "a");
  cout << "got training y" << endl;
}

//make moves random moves, followed by punishment_moves of good play
void Analyze::random_moves(Position& pos, int moves, int punishment_moves)
{
//  SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);
  cout << "doing random " << moves << " punishment " << punishment_moves << endl;
  StateInfo st[moves+punishment_moves];
  for (int i = 0; i < moves; i++)
  {
    size_t nmoves = MoveList<LEGAL>(pos).size();
    if (!nmoves) return; //UH-OH, there are no legal moves
    int move_num = rand() % nmoves;
    int j = 0;
    for (const auto& m : MoveList<LEGAL>(pos))
    {
      if (j == move_num)
      {
  //      SetupStates->push(StateInfo());
//        pos.do_move(m, SetupStates->top(), pos.gives_check(m, CheckInfo(pos)));
        pos.do_move(m, st[i], pos.gives_check(m, CheckInfo(pos)));
        break;
      }
      j++;
    }
  }

// have Stockfish play against itself for punishment_moves moves, using 1-ply lookahead static evaluation
// The idea is for bad captures to happen and be punished

 for (int i = 0; i < punishment_moves; i++)
 {
    Value v = VALUE_DRAW;
    Move bestm = MOVE_NONE;
    Value bestVal = VALUE_MATED_IN_MAX_PLY;
    //  StateInfo st;
    //  SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);
    //StateInfo st[1];
    for (auto const& m : MoveList<LEGAL>(pos))
    {
    //  SetupStates->push(StateInfo());
      pos.do_move(m, st[moves+i], pos.gives_check(m, CheckInfo(pos)));
      v = Eval::evaluate<true>(pos);
      if (v > bestVal) {
        bestVal = v;
        bestm = m;
      }
      pos.undo_move(m);
    }
    //  SetupStates->push(StateInfo());
    if (bestm != MOVE_NONE) pos.do_move(bestm, st[moves+i], pos.gives_check(bestm, CheckInfo(pos)));
  }

//  play_moves(pos, punishment_moves);
}

void naive_lookahead(Position& pos)
{
  Value v = VALUE_DRAW;
  Move bestm = MOVE_NONE;
  Value bestVal = VALUE_DRAW;
//  StateInfo st;
//  SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);
  StateInfo st[1];
  for (auto const& m : MoveList<LEGAL>(pos))
  {
  //  SetupStates->push(StateInfo());
    pos.do_move(m, st[0], pos.gives_check(m, CheckInfo(pos)));
    v = Eval::evaluate<true>(pos);
    if (v > bestVal) {
      bestVal = v;
      bestm = m;
    }
    pos.undo_move(m);
  }
//  SetupStates->push(StateInfo());
  pos.do_move(bestm, st[0], pos.gives_check(bestm, CheckInfo(pos)));
}

void Analyze::play_moves(Position& pos, int moves)
{
//  cout << "playing " << moves << " moves" << endl;
  for (int i = 0; i < moves; i++)
  {
    naive_lookahead(pos);
  }
}

void Analyze::random_capture(Position& pos)
{
//  cout << " doing random capture " << endl;
  if (pos.checkers())
  {
    random_moves(pos, 1,0);
    return;
  }

  size_t captures = MoveList<CAPTURES>(pos).size();
  if (captures == 0) goto end_random_capture;

  {
  int cap_num = rand()%captures;
  int j = 0;
//  SetupStates = Search::StateStackPtr(new std::stack<StateInfo>);
  StateInfo st[1];
  for (const auto& m : MoveList<CAPTURES>(pos))
  {
    if (j == cap_num)
    {
      //the cool thing about this is that j will not be incremented, so the next move will be tested for legality
      if (!pos.legal(m, pos.pinned_pieces(pos.side_to_move() ) ) ) continue;
//      SetupStates->push(StateInfo());
      pos.do_move(m, st[0], pos.gives_check(m, CheckInfo(pos)));
      return;
    }
    j++;
  }
  }
  //if we got here, there were no legal captures, or we chose an illegal capture after the first legal one
  //in this case, just make a random move with random punishment
end_random_capture:
  random_moves(pos, 1, rand() % 2);
}


/*void Analyze::process_pos_list(string infile, string ofile)
{

}*/

void Analyze::evaluate_pos_list(string infile, string ofile)
{
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

/*

*/
void Analyze::evaluate_pos_list(istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  evaluate_pos_list(infile, ofile);
}

// print progress out every 10000 games
#define GAME_CHECKPOINT 10000

//Take a file of games and write the feauture representations to the outfile (in csv format)
void Analyze::feature_game_list(string infile, string ofile)
{
  int games_processed = 0;
  int featurevec[NB_FEATURES];
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
      output_feature_game(out, mov_str, featurevec);
      mov_str = "";
      games_processed++;
    }
    if (games_processed && games_processed % GAME_CHECKPOINT == 0) cout << "Processed " << games_processed << " games" << endl;
  }

  if (mov_str.length())
  {
    output_feature_game(out, mov_str, featurevec);
  }

  f.close();
  out.close();
}

void Analyze::feature_game_list(std::istringstream& is)
{
  string infile;
  string ofile;
  if ( !(is >> infile) || !(is >> ofile)) return;
  feature_game_list(infile, ofile);
}

//void Analyze::feature_pos_list(string infile, string ofile){}
//void Analyze::feature_pos_list(std::istringstream& is){}




#define FEATURES_PER_LINE 5
void Analyze::print_pos_rep(Position& pos)
{
  int features[NB_FEATURES];
  vector<string> feature_names = {
    "SIDE_TO_MOVE", "WCASTLE_OO", "WCASTLE_OOO","BCASTLE_OO",
    "BCASTLE_OOO",  "NUM_WQ",  "NUM_WR",  "NUM_WB", "NUM_WN",
    "NUM_WP", "NUM_BQ", "NUM_BR", "NUM_BB", "NUM_BN", "NUM_BP",
    "WK_RANK", "WK_FILE", "BK_RANK", "BK_FILE",
    "WQ1_EXISTS"," WQ1_RANK"," WQ1_FILE"," WQ1_MIN_DEFENDER"," WQ1_MIN_ATTACKER",
    " WQ1_SQUARES"," WR1_EXISTS"," WR1_RANK"," WR1_FILE"," WR1_MIN_DEFENDER"," WR1_MIN_ATTACKER"," WR1_SQUARES",
    " WR2_EXISTS"," WR2_RANK"," WR2_FILE"," WR2_MIN_DEFENDER"," WR2_MIN_ATTACKER"," WR2_SQUARES",
    " WB1_EXISTS"," WB1_RANK"," WB1_FILE"," WB1_MIN_DEFENDER"," WB1_MIN_ATTACKER"," WB1_SQUARES",
    " WB2_EXISTS"," WB2_RANK"," WB2_FILE"," WB2_MIN_DEFENDER"," WB2_MIN_ATTACKER"," WB2_SQUARES",
    " WK1_EXISTS"," WK1_RANK"," WK1_FILE"," WK1_MIN_DEFENDER"," WK1_MIN_ATTACKER"," WK2_EXISTS",
    " WK2_RANK"," WK2_FILE"," WK2_MIN_DEFENDER"," WK2_MIN_ATTACKER"," WP1_EXISTS",
    " WP1_RANK"," WP1_FILE"," WP1_MIN_DEFENDER"," WP1_MIN_ATTACKER"," WP2_EXISTS",
    " WP2_RANK"," WP2_FILE"," WP2_MIN_DEFENDER"," WP2_MIN_ATTACKER",
    " WP3_EXISTS"," WP3_RANK"," WP3_FILE"," WP3_MIN_DEFENDER"," WP3_MIN_ATTACKER",
    " WP4_EXISTS"," WP4_RANK"," WP4_FILE"," WP4_MIN_DEFENDER"," WP4_MIN_ATTACKER",
    " WP5_EXISTS"," WP5_RANK"," WP5_FILE"," WP5_MIN_DEFENDER"," WP5_MIN_ATTACKER",
    " WP6_EXISTS"," WP6_RANK"," WP6_FILE"," WP6_MIN_DEFENDER"," WP6_MIN_ATTACKER",
    " WP7_EXISTS"," WP7_RANK"," WP7_FILE"," WP7_MIN_DEFENDER"," WP7_MIN_ATTACKER",
    " WP8_EXISTS"," WP8_RANK"," WP8_FILE"," WP8_MIN_DEFENDER"," WP8_MIN_ATTACKER",
    " BQ1_EXISTS"," BQ1_RANK"," BQ1_FILE"," BQ1_MIN_DEFENDER"," BQ1_MIN_ATTACKER"," BQ1_SQUARES",
    " BR1_EXISTS"," BR1_RANK"," BR1_FILE"," BR1_MIN_DEFENDER"," BR1_MIN_ATTACKER"," BR1_SQUARES",
    " BR2_EXISTS"," BR2_RANK"," BR2_FILE"," BR2_MIN_DEFENDER"," BR2_MIN_ATTACKER"," BR2_SQUARES",
    " BB1_EXISTS"," BB1_RANK"," BB1_FILE"," BB1_MIN_DEFENDER"," BB1_MIN_ATTACKER"," BB1_SQUARES",
    " BB2_EXISTS"," BB2_RANK"," BB2_FILE"," BB2_MIN_DEFENDER"," BB2_MIN_ATTACKER"," BB2_SQUARES",
    " BK1_EXISTS"," BK1_RANK"," BK1_FILE"," BK1_MIN_DEFENDER"," BK1_MIN_ATTACKER",
    " BK2_EXISTS"," BK2_RANK"," BK2_FILE"," BK2_MIN_DEFENDER"," BK2_MIN_ATTACKER",
    " BP1_EXISTS"," BP1_RANK"," BP1_FILE"," BP1_MIN_DEFENDER"," BP1_MIN_ATTACKER",
    " BP2_EXISTS"," BP2_RANK"," BP2_FILE"," BP2_MIN_DEFENDER"," BP2_MIN_ATTACKER",
    " BP3_EXISTS"," BP3_RANK"," BP3_FILE"," BP3_MIN_DEFENDER"," BP3_MIN_ATTACKER",
    " BP4_EXISTS"," BP4_RANK"," BP4_FILE"," BP4_MIN_DEFENDER"," BP4_MIN_ATTACKER",
    " BP5_EXISTS"," BP5_RANK"," BP5_FILE"," BP5_MIN_DEFENDER"," BP5_MIN_ATTACKER",
    " BP6_EXISTS"," BP6_RANK"," BP6_FILE"," BP6_MIN_DEFENDER"," BP6_MIN_ATTACKER",
    " BP7_EXISTS"," BP7_RANK"," BP7_FILE"," BP7_MIN_DEFENDER"," BP7_MIN_ATTACKER",
    " BP8_EXISTS"," BP8_RANK"," BP8_FILE"," BP8_MIN_DEFENDER"," BP8_MIN_ATTACKER"
  };
  Katyusha_pos_rep(pos, features);
  int i;

  cout << endl;
  for (i = 0; i < ATTACK_SQUARE_OFF; i++)
  {
    cout << " " << feature_names[i] << " " << features[i];
    if (i % FEATURES_PER_LINE == 0)  cout << endl;
  }
  cout << endl;

  for (i = 0; i < 64; i++)
  {
    if (i%8 == 0) cout << endl;
    cout << " " << features[ATTACK_SQUARE_OFF+i];
  }

  cout << endl;

  int off = DEFEND_SQUARE_OFF;
  for (int j = 0; j < 64; j++)
  {
    if (j%8 == 0) cout << endl;
    cout << " " << features[off+j];
  }
  cout << endl;
}


//extract a feature representation from the position pos

//utility function to convert Stockfish's internal feature representation into a feature vector I can use to train Katusha.
// NOTE: this feature representation is based on that used by Giraffe, with some tweaks.
//features is the array in which to store the result
void Analyze::Katyusha_pos_rep(const Position& pos, int * features)
{
  std::memset(features, 0, sizeof(int)*NB_FEATURES);

  //tempo
  Color side = pos.side_to_move();
  features[SIDE_TO_MOVE] = side;

  //castling rights
  features[WCASTLE_OO] = (pos.can_castle(WHITE_OO)) ? 1 : 0;
  features[WCASTLE_OOO] = pos.can_castle(WHITE_OOO) ? 1 : 0;
  features[BCASTLE_OO] = pos.can_castle(BLACK_OO) ? 1 : 0;
  features[BCASTLE_OOO] = pos.can_castle(BLACK_OOO) ? 1 : 0;

  //material configuration
  features[NUM_WQ] = pos.count<QUEEN>(WHITE);
  features[NUM_WR] = pos.count<ROOK>(WHITE);
  features[NUM_WB] = pos.count<BISHOP>(WHITE);
  features[NUM_WN] = pos.count<KNIGHT>(WHITE);
  features[NUM_WP] = pos.count<PAWN>(WHITE);
  features[NUM_BQ] = pos.count<QUEEN>(BLACK);
  features[NUM_BR] = pos.count<ROOK>(BLACK);
  features[NUM_BB] = pos.count<BISHOP>(BLACK);
  features[NUM_BN] = pos.count<KNIGHT>(BLACK);
  features[NUM_BP] = pos.count<PAWN>(BLACK);

  //piece-centric information

  //king position information
  Square wk_square = pos.square<KING>(WHITE);
  features[WK_RANK] = wk_square /8;
  features[WK_FILE] = wk_square % 8;
  //pos.attacks_from(wk_square, KING) & attacks();


  Square bk_square = pos.square<KING>(BLACK);
  features[BK_RANK] = bk_square / 8;
  features[BK_FILE] = bk_square % 8;
//  features[BK_DANGER_SQUARES] = ;



  //this consists of coordinates, existence, and mobility
  //NOTE: the below code is generated using a python script
  // I am trying to produce reasonably efficient code that is quick to develop and avoids bugs
  features[WQ1_EXISTS] = (pos.count<QUEEN>(WHITE) >= 1) ? 1 : 0;
  if (features[WQ1_EXISTS]) {
    Square piece_sq = pos.squares<QUEEN>(WHITE)[0];
    features[WQ1_RANK] = piece_sq/8;
    features[WQ1_FILE] = piece_sq%8;
    features[WQ1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq, WHITE);
    features[WQ1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
    Bitboard piece_attacks = pos.attacks_from<QUEEN>(piece_sq);
    features[WQ1_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WR1_EXISTS] = (pos.count<ROOK>(WHITE) >= 1) ? 1 : 0;
  if (features[WR1_EXISTS]) {
    Square piece_sq = pos.squares<ROOK>(WHITE)[0];
    features[WR1_RANK] = piece_sq/8;
    features[WR1_FILE] = piece_sq%8;
    features[WR1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WR1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
    Bitboard piece_attacks = pos.attacks_from<ROOK>(piece_sq);
    features[WR1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[WR2_EXISTS] = (pos.count<ROOK>(WHITE) >= 2) ? 1 : 0;
  if (features[WR2_EXISTS]) {
    Square piece_sq = pos.squares<ROOK>(WHITE)[1];
    features[WR2_RANK] = piece_sq/8;
    features[WR2_FILE] = piece_sq%8;
    features[WR2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WR2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
    Bitboard piece_attacks = pos.attacks_from<ROOK>(piece_sq);
    features[WR2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WB1_EXISTS] = (pos.count<BISHOP>(WHITE) >= 1) ? 1 : 0;
  if (features[WB1_EXISTS]) {
    Square piece_sq = pos.squares<BISHOP>(WHITE)[0];
    features[WB1_RANK] = piece_sq/8;
    features[WB1_FILE] = piece_sq%8;
    features[WB1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WB1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
    Bitboard piece_attacks = pos.attacks_from<BISHOP>(piece_sq);
    features[WB1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[WB2_EXISTS] = (pos.count<BISHOP>(WHITE) >= 2) ? 1 : 0;
  if (features[WB2_EXISTS]) {
    Square piece_sq = pos.squares<BISHOP>(WHITE)[1];
    features[WB2_RANK] = piece_sq/8;
    features[WB2_FILE] = piece_sq%8;
    features[WB2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WB2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
    Bitboard piece_attacks = pos.attacks_from<BISHOP>(piece_sq);
    features[WB2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[WK1_EXISTS] = (pos.count<KNIGHT>(WHITE) >= 1) ? 1 : 0;
  if (features[WK1_EXISTS]) {
    Square piece_sq = pos.squares<KNIGHT>(WHITE)[0];
    features[WK1_RANK] = piece_sq/8;
    features[WK1_FILE] = piece_sq%8;
    features[WK1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WK1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WK2_EXISTS] = (pos.count<KNIGHT>(WHITE) >= 2) ? 1 : 0;
  if (features[WK2_EXISTS]) {
    Square piece_sq = pos.squares<KNIGHT>(WHITE)[1];
    features[WK2_RANK] = piece_sq/8;
    features[WK2_FILE] = piece_sq%8;
    features[WK2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WK2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }


  features[WP1_EXISTS] = (pos.count<PAWN>(WHITE) >= 1) ? 1 : 0;
  if (features[WP1_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[0];
    features[WP1_RANK] = piece_sq/8;
    features[WP1_FILE] = piece_sq%8;
    features[WP1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP2_EXISTS] = (pos.count<PAWN>(WHITE) >= 2) ? 1 : 0;
  if (features[WP2_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[1];
    features[WP2_RANK] = piece_sq/8;
    features[WP2_FILE] = piece_sq%8;
    features[WP2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP3_EXISTS] = (pos.count<PAWN>(WHITE) >= 3) ? 1 : 0;
  if (features[WP3_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[2];
    features[WP3_RANK] = piece_sq/8;
    features[WP3_FILE] = piece_sq%8;
    features[WP3_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP3_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP4_EXISTS] = (pos.count<PAWN>(WHITE) >= 4) ? 1 : 0;
  if (features[WP4_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[3];
    features[WP4_RANK] = piece_sq/8;
    features[WP4_FILE] = piece_sq%8;
    features[WP4_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP4_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP5_EXISTS] = (pos.count<PAWN>(WHITE) >= 5) ? 1 : 0;
  if (features[WP5_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[4];
    features[WP5_RANK] = piece_sq/8;
    features[WP5_FILE] = piece_sq%8;
    features[WP5_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP5_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP6_EXISTS] = (pos.count<PAWN>(WHITE) >= 6) ? 1 : 0;
  if (features[WP6_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[5];
    features[WP6_RANK] = piece_sq/8;
    features[WP6_FILE] = piece_sq%8;
    features[WP6_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP6_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP7_EXISTS] = (pos.count<PAWN>(WHITE) >= 7) ? 1 : 0;
  if (features[WP7_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[6];
    features[WP7_RANK] = piece_sq/8;
    features[WP7_FILE] = piece_sq%8;
    features[WP7_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP7_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }

  features[WP8_EXISTS] = (pos.count<PAWN>(WHITE) >= 8) ? 1 : 0;
  if (features[WP8_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(WHITE)[7];
    features[WP8_RANK] = piece_sq/8;
    features[WP8_FILE] = piece_sq%8;
    features[WP8_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,WHITE);
    features[WP8_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,BLACK);
  }



  features[BQ1_EXISTS] = (pos.count<QUEEN>(BLACK) >= 1) ? 1 : 0;
  if (features[BQ1_EXISTS]) {
    Square piece_sq = pos.squares<QUEEN>(BLACK)[0];
    features[BQ1_RANK] = piece_sq/8;
    features[BQ1_FILE] = piece_sq%8;
    features[BQ1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BQ1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
    Bitboard piece_attacks = pos.attacks_from<QUEEN>(piece_sq);
    features[BQ1_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BR1_EXISTS] = (pos.count<ROOK>(BLACK) >= 1) ? 1 : 0;
  if (features[BR1_EXISTS]) {
    Square piece_sq = pos.squares<ROOK>(BLACK)[0];
    features[BR1_RANK] = piece_sq/8;
    features[BR1_FILE] = piece_sq%8;
    features[BR1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BR1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
    Bitboard piece_attacks = pos.attacks_from<ROOK>(piece_sq);
    features[BR1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[BR2_EXISTS] = (pos.count<ROOK>(BLACK) >= 2) ? 1 : 0;
  if (features[BR2_EXISTS]) {
    Square piece_sq = pos.squares<ROOK>(BLACK)[1];
    features[BR2_RANK] = piece_sq/8;
    features[BR2_FILE] = piece_sq%8;
    features[BR2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BR2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
    Bitboard piece_attacks = pos.attacks_from<ROOK>(piece_sq);
    features[BR2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BB1_EXISTS] = (pos.count<BISHOP>(BLACK) >= 1) ? 1 : 0;
  if (features[BB1_EXISTS]) {
    Square piece_sq = pos.squares<BISHOP>(BLACK)[0];
    features[BB1_RANK] = piece_sq/8;
    features[BB1_FILE] = piece_sq%8;
    features[BB1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BB1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
    Bitboard piece_attacks = pos.attacks_from<BISHOP>(piece_sq);
    features[BB1_SQUARES] = popcount<Max15>(piece_attacks);
  }

  features[BB2_EXISTS] = (pos.count<BISHOP>(BLACK) >= 2) ? 1 : 0;
  if (features[BB2_EXISTS]) {
    Square piece_sq = pos.squares<BISHOP>(BLACK)[1];
    features[BB2_RANK] = piece_sq/8;
    features[BB2_FILE] = piece_sq%8;
    features[BB2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BB2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
    Bitboard piece_attacks = pos.attacks_from<BISHOP>(piece_sq);
    features[BB2_SQUARES] = popcount<Max15>(piece_attacks);
  }


  features[BK1_EXISTS] = (pos.count<KNIGHT>(BLACK) >= 1) ? 1 : 0;
  if (features[BK1_EXISTS]) {
    Square piece_sq = pos.squares<KNIGHT>(BLACK)[0];
    features[BK1_RANK] = piece_sq/8;
    features[BK1_FILE] = piece_sq%8;
    features[BK1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BK1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BK2_EXISTS] = (pos.count<KNIGHT>(BLACK) >= 2) ? 1 : 0;
  if (features[BK2_EXISTS]) {
    Square piece_sq = pos.squares<KNIGHT>(BLACK)[1];
    features[BK2_RANK] = piece_sq/8;
    features[BK2_FILE] = piece_sq%8;
    features[BK2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BK2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }


  features[BP1_EXISTS] = (pos.count<PAWN>(BLACK) >= 1) ? 1 : 0;
  if (features[BP1_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[0];
    features[BP1_RANK] = piece_sq/8;
    features[BP1_FILE] = piece_sq%8;
    features[BP1_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP1_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP2_EXISTS] = (pos.count<PAWN>(BLACK) >= 2) ? 1 : 0;
  if (features[BP2_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[1];
    features[BP2_RANK] = piece_sq/8;
    features[BP2_FILE] = piece_sq%8;
    features[BP2_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP2_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP3_EXISTS] = (pos.count<PAWN>(BLACK) >= 3) ? 1 : 0;
  if (features[BP3_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[2];
    features[BP3_RANK] = piece_sq/8;
    features[BP3_FILE] = piece_sq%8;
    features[BP3_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP3_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP4_EXISTS] = (pos.count<PAWN>(BLACK) >= 4) ? 1 : 0;
  if (features[BP4_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[3];
    features[BP4_RANK] = piece_sq/8;
    features[BP4_FILE] = piece_sq%8;
    features[BP4_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP4_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP5_EXISTS] = (pos.count<PAWN>(BLACK) >= 5) ? 1 : 0;
  if (features[BP5_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[4];
    features[BP5_RANK] = piece_sq/8;
    features[BP5_FILE] = piece_sq%8;
    features[BP5_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP5_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP6_EXISTS] = (pos.count<PAWN>(BLACK) >= 6) ? 1 : 0;
  if (features[BP6_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[5];
    features[BP6_RANK] = piece_sq/8;
    features[BP6_FILE] = piece_sq%8;
    features[BP6_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP6_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP7_EXISTS] = (pos.count<PAWN>(BLACK) >= 7) ? 1 : 0;
  if (features[BP7_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[6];
    features[BP7_RANK] = piece_sq/8;
    features[BP7_FILE] = piece_sq%8;
    features[BP7_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP7_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }

  features[BP8_EXISTS] = (pos.count<PAWN>(BLACK) >= 8) ? 1 : 0;
  if (features[BP8_EXISTS]) {
    Square piece_sq = pos.squares<PAWN>(BLACK)[7];
    features[BP8_RANK] = piece_sq/8;
    features[BP8_FILE] = piece_sq%8;
    features[BP8_MIN_DEFENDER] = pos.simple_min_attacker(piece_sq,BLACK);
    features[BP8_MIN_ATTACKER] = pos.simple_min_attacker(piece_sq,WHITE);
  }





  //square-centric information
  //attack and defend maps
  Square sq;
  for (sq = SQ_A1;  sq <= SQ_H8; ++sq)
  {
    features[ATTACK_SQUARE_OFF+sq] = pos.simple_min_attacker(sq, ~side);
  }

  for (sq = SQ_A1;  sq <= SQ_H8; ++sq)
  {
    features[DEFEND_SQUARE_OFF+sq] = pos.simple_min_attacker(sq, side);
  }

  //file-based pawn information
  //for each file, count how many white pawns are on that file, and how many black pawns
  File pfile;
  Bitboard wpawns = pos.pieces(WHITE, PAWN);
  Bitboard bpawns = pos.pieces(BLACK, PAWN);
  for (pfile = FILE_A; pfile <= FILE_H; ++pfile)
  {
    features[WHITE_PAWN_FILE+pfile] = popcount<Max15>(wpawns&file_bb(pfile));
    features[BLACK_PAWN_FILE+pfile] = popcount<Max15>(bpawns&file_bb(pfile));
  }


}
