

pieces = ["QUEEN", "ROOK", "BISHOP", "KNIGHT", "PAWN"]

mobilePieces = pieces[:3]
pieceNums = [1, 2, 2, 2, 8]

colors = ["WHITE", "BLACK"]
nfeatures_list = []

def print_feature(name, val, indent = ""):
    print indent+"features["+name+"] = "+val+";"
    nfeatures_list.append(name)

for color in colors:
    for piece, num in zip(pieces, pieceNums):
        for i in xrange(1, num+1):
            piece_pref = color[0] + piece[0] + str(i)
            exists =  piece_pref + "_EXISTS"
            #existence
            print "features[" + exists + "] = (<" + piece + ">pos.count("+color+") >= "+str(i)+") ? 1 : 0;"
            #square to rank is /8, square to file is % 8

            rnk, fle = piece_pref+"_RANK", piece_pref+"_FILE"
            print "if (features["+exists+"]) {"
            print "  Square piece_sq = pos.squares("+color+")["+str(i)+"];"
            print "  features["+rnk+"] = piece_sq/8;"
            print "  features["+fle+"] = piece_sq%8;"
            nfeatures_list.extend([exists, rnk, fle])
            if piece in mobilePieces:
                print "  Bitboard piece_attacks = pos.attacks_from("+piece+","+color+");"
                mob_feature = piece_pref+"_SQUARES"
                #I'm lazy (or rather strapped for time, So I'm only giving the network (for now) the total squares controlled)
                print_feature(mob_feature, "popcount<Max15>(piece_attacks)", indent = "  ")
            print "}\n"
        print
    print

if piece in mobilePieces:
    print "  features[] = ;"
    print "  features[] = ;"
    print "  features[] = ;"
    print "  features[] = ;"
