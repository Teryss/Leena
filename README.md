# Leena

Chess engine written in C. <br>

## Compiling

You must have GCC installed. Then you can simply ```make```.<br> 
> Note: in order to run this engine, your CPU must support [BMI2](https://en.wikipedia.org/wiki/X86_Bit_manipulation_instruction_set#BMI2_(Bit_Manipulation_Instruction_Set_2)) instruction set <br>
Supported since Haswell and Evador by intel and AMD respectively.

## Feauters

### Board:
- Bitboards as board representation.
- Precalculated attack tables for non-sliding pieces.
- Precalculated sliding pieces attack tables using PEXT bitboards.

### Search:
- Alpha-beta pruning.
- Transposition table with always replace scheme.
- Quiesence search with depth limitation supported by delta pruning.

### Evaluation:
- Killer heuristics.
- Square piece tables.

## TODO
- Support UCI protocol.
- Iterative deepening.
- Evaluation enhancements
  - Mobility bonus
  - King safety
- Maybe more...