# Fast Nonogram Solver

The Android game Nonogram Katana features some interesting large (up to 80x80) nonogram puzzles. Most nonogram solvers I've seen cannot handle puzzles of this size. Hence I decided to roll out my own nonogram solver.

## Usage

Create a text file (see sample_input folder for examples) with the following content:

The first line consists of two numbers, the number of rows and number of columns.

The next n lines contain the list of blocks in each row.

The next m lines contain the list of blocks in each column.

Example:

```
2 4
1 2
2 1
2
1
1
2
```

Corresponds to the nonogram

```
    2 1 1 2
1 2 ■ □ ■ ■
2 1 ■ ■ □ ■
```

Then invoke the program with

```
./nonogram input.txt
```



## The Algorithm

Suppose you have a line that looks like this:

```
2 2 □□□□□□
```

There are several possible ways to place the two blocks:

```
1. ■■□■■□
2. ■■□□■■
3. □■■□■■
```

But however you place them, the second and fifth cells are always filled. Hence we conclude that these two cells must be filled.

Now suppose that by solving other lines in the puzzle, we find that the third cell must also be filled. There is only one way to place these blocks that is compatible with this new piece of information, namely the third one in the list above. Thus we know that the sixth cell must be filled, and the remaining two cells must be empty.

This is the fundamental idea of the algorithm: we generate all possible ways to place the blocks compatible with current known information, and find commonalities between them.

However, for large nonogram puzzles, the number of compatible configurations grows exponentially with the number of blocks. A naive implementation of the above idea would not suffice.

We use a **dynamic programming** approach to generate all possible configurations. Consider the following problem: Given the list of blocks in a line, and some partial information about the line (some cells are definitely filled, some are definitely empty), *is it possible to place the **last** n blocks entirely into the **last** m cells of the line, in a way that is consistent with given information, such that cell i is filled (or empty)?* Here "consistent" means, if it is given that one of the last m cells must be filled, then it must be part of one of the last n blocks; and if it is given that a cell must be empty, then it must not be part of any block.

We use F(n, m, i) to denote a particular instance of this problem, which asks whether cell i could be filled. We also use E(n, m, i) to denote the counterpart which asks whether cell i could be empty. If we take n to be the total number of blocks, and m the total length of the line, then these two values provide the following information:

|                    | F(n, m, i) = true     | F(n, m, i) = false   |
| ------------------ | --------------------- | -------------------- |
| E(n, m, i) = true  | Ambiguous             | Cell i must be empty |
| E(n, m, i) = false | Cell i must be filled | Contradiction        |

We now show how to compute these values recursively. First we look at the m-th rightmost cell in the line. If this cell must be filled, then it must be part of the last n blocks. In fact, it must be the first cell in the n-th rightmost block. This effectively fixes the position of the n-th rightmost block, as long as it is not obstructed by other cells that must be empty.

If the m-th rightmost cell need not be filled, then there are two possibilities. Either we place the n-th rightmost block right at the m-th rightmost cell, or we place it further to the right. The former case is identical to the case above, and the latter case is handled by F(n, m - 1, i).

Having placed the n-th rightmost block, we consider the (n - 1) remaining blocks. If n = 1 then all cells after this block must be empty. Check the provided information to make sure none of them must be filled. If n > 1 then the cell right after the block must be empty, and the rest of the blocks are handled by F(n - 1, m - len - 1, i) where len is the length of the block.

This allows us to compute all F(n, m, i) in time O(nm^2).

## The Implementation

The program scans the puzzle in iterations. During each iteration, it looks at each row and column, and uses the above algorithm to deduce more cells from known information. If each iteration reveals at least one new cell (so-called "True Nonograms" in Nonogram Katana), then the program completes within N iterations where N is the total number of cells in the puzzle. In practice, a true nonogram of size 80 x 75 can be solved within 0.8 seconds on my laptop (Intel i5-1135G7).

Since nonogram is NP-complete, there are many cases where the above procedure would not reveal anything new. In this case the program resorts to recursive trial-and-error to solve the puzzle. Execution time varies wildly depending on the difficulty of the puzzle. The current hardest puzzle in Nonogram Katana takes around 28 minutes on my laptop. Suggestions are welcome to optimize the program for these difficult puzzles.

My program only supports black-and-white nonograms. However the above ideas should be fairly easy to generalize to multi-color nonograms.