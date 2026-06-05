# Color Puzzle Game Solver

A command-line C++23 solver for the daily tube-sorting puzzles posted on
[r/ColorPuzzleGame](https://www.reddit.com/r/ColorPuzzleGame/).

The puzzle consists of colored stacks in tubes. A move pours the active run of
same-colored pieces from the bottom of one tube into another tube. The goal is to
reach a state where every non-empty tube is full and contains only one color.

## Puzzle rules modeled by the solver

The solver uses the following rules:

- The bottom of a tube is the end of that tube's input line.
- A move selects a source tube and a destination tube.
- The source tube must be non-empty.
- The solver moves the whole active color run from the bottom of the source tube.
  For example, if the bottom of a tube is `RRR`, all three reds move together.
- The destination tube may be empty, or its bottom color must match the moving run.
- The destination tube must have enough free space for the entire run.
- A solved state is one where every tube is either empty or full of a single
  color.

## How it works

The default solver performs a breadth-first search over game states. Because BFS
explores states in increasing move count, the first solution found is a shortest
solution under the move rules above.

Visited states are stored in a game_state_set using game_state_token, so equivalent 
states are not expanded repeatedly. A game_state_token represents each tube as a 
two-byte value: a four-element tuple of four-bit color values, with each color 
occupying one hexadecimal digit. Because the physical order of the tubes does 
not affect whether a state is solvable, tokens are canonicalized by sorting these 
two-byte tube representations before they are stored or compared.

The solver also supports trying specific two-color openings. In that mode, it
first consolidates active runs of the first opening color into tube 11, then
active runs of the second opening color into tube 12, and then runs the normal
BFS solver from that new state.

The `catalog` command tries all two-color openings available from the active
bottom colors of the non-empty puzzle tubes, reports the solvable ones, and sorts
them by solution length.

## Input format

The input is a plain text file with one tube per line.

Each alphabetic character is interpreted as a color. Non-alphabetic characters
are ignored, so you may add spaces or punctuation for readability.

The supported color codes are:

| Code | Color |
| ---- | ----- |
| `G`  | green |
| `L`  | lilac |
| `P`  | purple |
| `B`  | blue |
| `W`  | white |
| `C`  | cyan |
| `M`  | magenta |
| `O`  | orange |
| `Y`  | yellow |
| `R`  | red |

The last color character on a line is treated as the bottom of that tube. 

For example:

```text
[RGBY]
[ORCM]
[LLPG]
[WWCB]
...
[]
[]
```

In the first line above, `Y` is the bottom color of tube 1.

The file must contain exactly 20 tube lines. For the current daily-puzzle setup, this is
expected to include the empty tubes as blank lines or lines containing no alphabetic chatacters.

## Usage

```sh
color_puzzle_solver <puzzle.txt> [command]
```

### Find a shortest solution

```sh
color_puzzle_solver puzzle.txt
```

Example output:

```text
best solution is 32 moves:
3 -> 11
5 -> 12
...
```

Moves are printed using one-based tube numbers.

### Catalog solvable openings

```sh
color_puzzle_solver puzzle.txt catalog
```

Example output:

```text
R-G is solvable in 31 moves.
B-Y is solvable in 32 moves.
```

### Force a specific opening

```sh
color_puzzle_solver puzzle.txt RG
```

The opening argument is read from its first and last characters, so both `RG`
and `R-G` work.

If the requested opening is solvable, the program prints the full move sequence.
Otherwise it reports that there is no solution with that opening.

## Build notes

This project uses C++23 features, including ranges, `std::print`, and
`std::format`. Use a compiler and standard library with support for those
features.
