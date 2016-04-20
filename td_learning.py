import numpy as np
import chess
from chess import uci
from sys import argv, exit
import network_arch as na
from itertools import izip


max_batches = 1000
batch_size = 16
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
    katyusha.go(movetime=100)
    for i in xrange(num_moves):
        katyusha.position(board)
        kinfo = katyusha.go(depth=5)
        with katyusha_info:
            if "pos_rep" in dir(katyusha):
#                print i, katyusha.foo, katyusha.ponder, kinfo.ponder
                pos_reps.append(katyusha.pos_rep)
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
#        print board

    return res, pos_reps

def td_errors(evals, lam=.7):
    errors = np.zeros(len(evals))
    diffs = evals[1:]-evals[:-1]
    N = len(evals)
    for t in xrange(N):
        for j in xrange(t,N-1):
            errors[t] += diffs[j] * lam ** (j-t)
    return errors

def propigate_errors(model, reps, errors):
    training_dict = na.make_training_dict(reps, errors)
    current_evals = model.predict(training_dict)["out"]
    n_pos = len(errors)
    print "l1 loss "+str(np.sum(np.abs(errors))/n_pos/50.)
    print "l2 loss " + str(np.sum(errors**2)/n_pos/2500.)
    #I cannot directly back-propigate an arbitrary error signal
    #I can, however force the model to fit features to outputs
    #So, I obtain the model's current output for the inputs in consideration
    #Next, I add the desired errors, and voila, it works!
    #This is inefficient, but I'm not in the mood to extend Keras
    #Tee-hee-heee
    training_dict["out"] += np.ravel(current_evals)
    #print training_dict["out"].shape, all_errors.shape, all_errors
    model.train_on_batch(training_dict)



def other_td_update_network(model,evals_list, lam=.7):
    """
    Updates the model weights to match the temporal difference loss
    This only does updates for the starting position
    """
    total_positions = len(evals_list)
    all_errors = np.zeros(total_positions)
    cur_pos = 0
    all_reps = np.zeros((total_positions, na.num_features))
    for evals, pos_reps in evals_list:
    #    print evals,pos_reps
        #how many moves were acutally played
        num_moves = len(pos_reps)
        #get error signals
        all_errors[cur_pos] = td_errors(evals[:num_moves], lam=lam)[0]
        all_reps[cur_pos] = np.fromstring(pos_reps[0], sep=",")
        cur_pos += 1
    propigate_errors(model, all_reps, all_errors)


def td_update_network(model, evals_list, lam=.7):
    """
    Updates the model weights to match the temporal difference loss
    """
    game_lens = [len(el[1]) for el in evals_list]
    total_positions = sum(game_lens)
    all_errors = np.zeros(total_positions)
    cur_pos = 0
    all_reps = np.zeros((total_positions, na.num_features))
    for evals, pos_reps in evals_list:
    #    print evals,pos_reps
        #how many moves were acutally played
        num_moves = len(pos_reps)
        #get error signals
        all_errors[cur_pos:cur_pos+num_moves] = td_errors(evals[:num_moves], lam=lam)
        for pos_rep in pos_reps:
            all_reps[cur_pos] = np.fromstring(pos_rep, sep=",")
            cur_pos += 1

    training_dict = na.make_training_dict(all_reps, all_errors)
    current_evals = model.predict(training_dict)["out"]
    n_pos = len(all_errors)
    print "l1 loss "+str(np.sum(np.abs(all_errors))/n_pos/50.)
    print "l2 loss " + str(np.sum(all_errors**2)/n_pos/2500.)
    #I cannot directly back-propigate an arbitrary error signal
    #I can, however force the model to fit features to outputs
    #So, I obtain the model's current output for the inputs in consideration
    #Next, I add the desired errors, and voila, it works!
    #This is inefficient, but I'm not in the mood to extend Keras
    #Tee-hee-heee
    training_dict["out"] += np.ravel(current_evals)
    #print training_dict["out"].shape, all_errors.shape, all_errors
    model.train_on_batch(training_dict)

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
    weightsfile = argv[2]

    temp_npz_file = "temp_td_weights.npz"
    na.model.load_weights(weightsfile)
    na.save_as_npz(temp_npz_file)
    info_handler = uci.InfoHandler()
    katyusha.info_handlers.append(info_handler)
    katyusha.setoption({"Katyusha_Learning":True, "weightsfile":temp_npz_file})
    pos_file = open(argv[3], "r")
    fen_arr = np.array([line.strip() for line in pos_file])
    mask = np.arange(len(fen_arr))
    np.random.shuffle(mask)
    fen_arr = fen_arr[mask]

    num_moves = 12
    for batch_num in xrange(max_batches):
        print "On Batch "+str(batch_num)
        evals_list = []
        for i in xrange(batch_size):
            print str(i)+" out of "+str(batch_size)
            fen = fen_arr[batch_num*batch_size + i]
            if not len(fen):
                break
            try:
                evals, reps = extract_evals(katyusha, info_handler, startFen=fen, num_moves=num_moves)
                evals /= 100. #convert from centipawns to pawns
                evals_list.append((evals, reps))
            except ValueError:
                print "Encountered ValueError, not sure why"
                continue
#        td_update_network(na.model, evals_list)
        other_td_update_network(na.model, evals_list)
        na.model.save_weights("td_"+weightsfile, overwrite=True)
        na.save_as_npz(temp_npz_file)
        katyusha.setoption({"weightsfile":temp_npz_file})
        #TODO send command to katyusha to update weights
