import data

for k1 in data.perft_my.keys():
    if k1 not in data.perft_stock.keys():
        print(k1 + "not in stockfish")

for k1 in data.perft_stock.keys():
    if k1 not in data.perft_my.keys():
        print(k1 + "not in mine")

for k1 in data.perft_my.keys():
    if (data.perft_my[k1] != data.perft_stock[k1]):
        print(k1, data.perft_my[k1], data.perft_stock[k1], "diff:", data.perft_my[k1] - data.perft_stock[k1])