A work in progress chess engine written in C++17.

Bitboards, alpha-beta negamax, iterative deepening, killer and history heuristics,
and basic quiescence search. Quiescence searches captures, en passant,
promotions, and check evasions.

Build and run:

```sh
cmake -S . -B build
cmake --build build
./build/cheezy-engine
```

UCI commands: `uci`, `isready`, `ucinewgame`, `position startpos`,
`position fen`, `go depth <1-63>`, `stop`, and `quit`. Both position commands
accept a `moves` list. Clock controls, pondering, and engine options are not
implemented. Without a depth, `go` uses depth 63.

Example (wait for `bestmove` before entering `quit`):

```text
uci
isready
position startpos moves e2e4 e7e5
go depth 4
quit
```

TODO: Transposition table.
