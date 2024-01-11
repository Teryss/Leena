# Leena

Chess engine written in C.

## Compiling

You must have GCC installed. Then you can simply ```make```.<br> 
> Note: in order to run this engine, your CPU must support [BMI2](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#BMI2_(Bit_Manipulation_Instruction_Set_2)) instruction set. <br>
Supported since Haswell and Evador by intel and AMD respectively.

## Feauters

### Board:
- Bitboards as board representation.
- Precalculated attack tables for non-sliding pieces.
- Precalculated sliding pieces attack tables using PEXT bitboards.

### Search:
- Copy-paste board with pseudo legal move generation.
- Alpha-beta pruning.
- Dynamically allocated Transposition table with always replace scheme.
- Quiesence search with depth limitation supported by delta pruning.

### Evaluation:
- Killer heuristics.
- Square piece tables.

<!-- ## Concepts explanation

### Bitboard
It's a 64-bit number used to represent pieces on board, where single bit represents a single piece. It's the most efficient way to represent a chess board. Modern processors have 64 bit registers and  bit operation are really cheap.<br>

For example we can represent all pawns on the board using two bitboards:
```
White pawns:
8   .  .  .  .  .  .  .  . 
7   .  .  .  .  .  .  .  . 
6   .  .  .  .  .  .  .  . 
5   .  .  .  .  .  .  .  . 
4   .  .  .  .  .  .  .  . 
3   .  .  .  .  .  .  .  . 
2   X  X  X  X  X  X  X  X 
1   .  .  .  .  .  .  .  . 
    A  B  C  D  E  F  G  H
```
```
Black pawns:
8   .  .  .  .  .  .  .  . 
7   X  X  X  X  X  X  X  X 
6   .  .  .  .  .  .  .  . 
5   .  .  .  .  .  .  .  . 
4   .  .  .  .  .  .  .  . 
3   .  .  .  .  .  .  .  . 
2   .  .  .  .  .  .  .  . 
1   .  .  .  .  .  .  .  . 
    A  B  C  D  E  F  G  H
```
```
Black pawns | White pawns:
8   .  .  .  .  .  .  .  . 
7   X  X  X  X  X  X  X  X 
6   .  .  .  .  .  .  .  . 
5   .  .  .  .  .  .  .  . 
4   .  .  .  .  .  .  .  . 
3   .  .  .  .  .  .  .  . 
2   X  X  X  X  X  X  X  X 
1   .  .  .  .  .  .  .  . 
    A  B  C  D  E  F  G  H
``` -->

## TODO
- Support UCI protocol.
- Principle variotions.
- Iterative deepening.
- Repetition detection.
- Evaluation enhancements
  - Mobility bonus
  - King safety
- Maybe more...