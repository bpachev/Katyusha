import numpy as np
import chess
from chess import uci
from sys import argv, exit
import network_arch as na
from itertools import izip


max_batches = 1
batch_size = 1
#TD-learning rate
lam = .7


def extract_evals(katyusha, info_handler, startFen=None, num_moves=12):
    """
    Have Katyusha play num_moves moves against itself.
    Get katyusha's evaluation of the board between moves.
    (Will probably need to fix signs on evals, to fix relative to absolute)
    """
    res = np.zeros(num_moves)
    board = chess.Board(fen = startFen) if startFen is not None else chess.Board()
    if board.is_game_over():
        print "Got a starting position that is already terminal."
        return res

    katyusha_info = info_handler
    pos_reps = []
    for i in xrange(num_moves):
        if "pos_rep" in dir(katyusha):
            pos_reps.append(katyusha.pos_rep)
        katyusha.position(board)
        kinfo = katyusha.go(movetime = 1000)
        with katyusha_info:
            if 1 in katyusha_info.info["score"]:
                cp = katyusha_info.info["score"][1].cp
                mate = katyusha_info.info["score"][1].mate
                if cp is None:
                    print "Checkmate (probably) found"
                    res[i:] = res[max(i-1,0)]
                    break
                else:
                    if board.turn == chess.BLACK:
                        cp *= -1
                    res[i] = cp
            else:
                print "Bad Info "
                res[i:] = res[max(i-1,0)]
                break

        board.push(kinfo.bestmove)
        if board.is_game_over():
            #If the game is cut short, then all the last evaluations should be the same as the last real one.
            #This ensures minimal noisy temporal differences
            res[i+1:] = res[i]
            break
        print board

    return res, pos_reps

def td_update_network(model, evals_list, lam=.7):
    """
    Updates the model weights to match the temporal difference loss
    """
    game_lens = [len(el[1]) for el in evals_list]
    total_positions = sum(game_lens)
    all_evals = np.zeros(total_positions)
    cur_pos = 0
    all_reps = np.zeros((total_positions, na.total_inputs))

    for evals, pos_reps in evals_list:
        for val, pos_rep in izip(evals, pos_reps):
            all_evals[cur_pos] = val
            all_reps[cur_pos] = np.fromstring(pos_rep, sep=",")
    
    model.train_on_batch()


if __name__ == "__main__":
    usage = "usage katyushabinaryfile weightsfile positions"
    if len(argv) < 4:
        print usage
        exit(1)

    katyusha = None
    try:
        katyusha = uci.popen_engine(argv[1])
        katyusha.uci()
    except e:
        print "error initializing engine"
        print e
        exit(1)

    na.model.load_weights(argv[2])
    info_handler = uci.InfoHandler()
    katyusha.info_handlers.append(info_handler)
    katyusha.setoption({"Katyusha_Learning":True})
    pos_file = open(argv[3], "r")
    for batch_num in xrange(max_batches):
        evals_list = []
        for i in xrange(batch_size):
            fen = pos_file.readline()
            if not len(fen):
                break
            evals, reps = extract_evals(katyusha, info_handler, startFen=fen, num_moves=12)
            evals_list.append((evals, reps))
