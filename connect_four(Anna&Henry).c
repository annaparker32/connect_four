/* Anna Parker & Henry Swaffield
*--c4playerAH2.c
*
* code contains a connect-4 player
*
* to run: make c4playerAH2
*  ./match.py c4playerAH2 [other player's name]
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define ROWS 6
#define COLS 7

// global variable for the board
// uses 0 for empty cells, 1 and 2 for player 1 and 2
int board[ROWS][COLS] = {{0}};

//Four counting, to determine who goes first: 
int numO = 0;
int numX = 0;

// maps board contents 0, 1, 2 to characters
char sym[] = {' ', 'X', 'O'};

// simple assert function, quits program with error message if assertion fails
void assert(int cond, char *msg)
{
    if (!cond) {
	fprintf(stderr, "failed assertion %s\n", msg);
	exit(1);
    }
}

// seed random number generator based on on current time in microseconds and board state
void randseed() {
    struct timeval t1;
    gettimeofday(&t1, NULL);
    unsigned int s = t1.tv_usec * t1.tv_sec;
    int row, col;
    for (col = 0; col < COLS; col++) {
	for (row = 0; row < ROWS; row++) {
	    int b = board[row][col];
	    s += (b+1) * (13*row + 37*col);
	}
    }
    srand(s);
}

// read encoded board from stdin:
// 7 strings of 'X' and 'O', representing columns from bottom up
// followed by single X/O representing player to move
// returns player num (1=X, 2=O)
int read_board()
{
    int row = 0, col = 0;
    char c;
    while (col < COLS) {
	c = getchar();
	if (c == 'X' || c == 'O') {
	    assert(row < ROWS, "row < ROWS");
	    board[row++][col] = (c == 'X' ? 1 : 2);

        //new code: allows to count the chars...
        if (c == 'O') {
            numO++;
        }

        else if (c == 'X') {
            numX++;
        }

	} else if (c == '\n') {
	    row = 0;
	    col++;
	} else {
	    assert(0, "input symbols must be 'X', 'O', or '\\n'");
	}
    }
    // read one more symbol indicating whose move it is
    c = getchar();
    assert(c == 'X' || c == 'O', "last input symbol must 'X' or 'O'");
    randseed(); // seed random number generator based on board state and current time
    return (c == 'X' ? 1 : 2);
}

// prints board (for debugging)
void print_board()
{
    int row, col;
    for (row = ROWS-1; row >= 0; row--) {
	printf("| ");
	for (col = 0; col < COLS; col++)
	    printf("%c ", sym[board[row][col]]);
	printf("|\n");
    }
    printf("+---------------+\n");
    printf("  0 1 2 3 4 5 6  \n");
}


//  Bulk of new code starts here:


// Returns the count of chars in a contiguous horizontal 
// sequence surrouding the given location.
// if it returns a value >= 3, then there is a potential
// win or loss.
int checkSurroundingHorizontal(int boardC[][COLS], int row, int col, char current, int distance) {
    //returns the location of the play to block danger
    int countMatchLeft = 0;
    int countMatchRight = 0;
    int i;
    for (i = 1; i < distance; i++) {
        if (col-i < 0) {
           break;
        } else if (sym[boardC[row][col-i]] == current) {
            countMatchLeft++;
        } 
        else { 
            break;  //ends loop - streak done.
        }
    }

    for (i = 1; i < distance; i++) {
        if (col+i > 6) {
           break;
        } else if (sym[boardC[row][col+i]] == current) {
                countMatchRight++;
        } else {
            break; //ends loop
        }
    }
    return countMatchLeft + countMatchRight;
}


// Counts and return contiguous streaks of specified character on each column vertically
//returns this contiguous streak length found on each column
int checkSurroundingVertical(int boardC[][COLS], int row, int col, char current, int distance) {
    //returns the number around...
    int matchCount = 0;
    int i;
    for (i = 1; i < distance; i++) {
        if (row-i < 0) {
           continue;
        } else {
            if (sym[boardC[row-i][col]] == current) {
                matchCount++;
            } else {
                matchCount = 0;
            }
        }
    }
    return matchCount;
}


// Counts the length of the longest diagonal the the current location is currently surrounded
// by, i.e. one less than the length of such a diagonal if the player moved there.
int checkSurroundingDiagonal(int boardC[][COLS], int row, int col, char current, int distance) {
    int countMatchLeftDown = 0;
    int countMatchRightUp = 0;

    int countMatchLeftUp = 0;
    int countMatchRightDown = 0;

    int i;

    // Diagonal with slope = 1;
    for (i = 1; i < distance; i++) {
        if (col-i < 0 || row - i < 0) {
           break;
        } else if (sym[boardC[row-i][col-i]] == current) {
            countMatchLeftDown++;
        } 
        else { 
            break;  //ends loop - streak done.
        }
    }

    for (i = 1; i < distance; i++) {
        if (col+i > 6 || row + i > 5) {
           break;
        } else if (sym[boardC[row+i][col+i]] == current) {
                countMatchRightUp++;
        } else {
            break; //ends loop
        }
    }

    //checking other diagonal, slope = -1
    for (i = 1; i < distance; i++) {
        if (col-i < 0 || row + i > 5) {
           break;
        } else if (sym[boardC[row+i][col-i]] == current) {
            countMatchLeftUp++;
        } 
        else { 
            break;  //ends loop - streak done.
        }
    }

    for (i = 1; i < distance; i++) {
        if (col+i > 6 || row - i < 0) {
           break;
        } else if (sym[boardC[row-i][col+i]] == current) {
                countMatchRightDown++;
        } else {
            break; //ends loop
        }
    }

    //return the length of the longest diagonal found;
    if (countMatchLeftDown + countMatchRightUp > countMatchLeftUp + countMatchRightDown) {
        return countMatchLeftDown + countMatchRightUp;
    } else {
        return countMatchLeftUp + countMatchRightDown;
    }
}

// Checksetup checks whether a potential move will provide and immediate opportunity
// for the opponent to win. Doing so may prevent the player from making a move that
// would ensure their defeat.
// Simulate the future board, and see if there are any long contiguous strings formed.
// 
// returns 1 if moving in the specified location would ensure the players defeat
// i.e. a bad move, otherwise it returns 0.
int checkSetup(int boardC[][COLS], char current, char other, int row, int col) {
    int boardFuture[ROWS][COLS];
    int i, j;

    for (i = ROWS-1; i >= 0; i--) {
        for (j = 0; j < COLS; j++) {
            boardFuture[i][j] = boardC[i][j];
        }}

    int move = 1;   // 'X'

    if (current == 'O') {
        move = 2;
    }

    //updating the board:
    boardFuture[row][col] = move;

    //checking to see if the move would setup a loss:

    // this check was missing and was the cause of our downfall!: Seg faults...
    if (row == 5) {
        return 0;
    }

    if (checkSurroundingHorizontal(boardFuture, (row + 1), col, other, 4) >= 3) {
        return 1;
    }    

    if (checkSurroundingVertical(boardFuture, (row + 1), col, other, 4) >= 3) {
        return 1;
    }

    if (checkSurroundingDiagonal(boardFuture, (row + 1), col, other, 4) >= 3) {
        return 1;
    }

    return 0;
}

// check board, performs most of the computations, and returns the player's move
//
// Step 1: determine where locations are that would allow for immediate victory
// if they exist move there, and win game. Done.
//
// Step 2: determine where the opponent has an opportunity to win on the next move,
// if they exist move there, to prevent immediate defeat. Done.
//
// Step 3: calculate which move would simultanesouly maximize our strings, and also
// block the opponents, by adding up the various horizontal and diagonal counts.
//
// Step 4: Check whether the move would setup the opponent for an immediate win.
// if it would, switch to the next column, and check again (repeats up to 4 times).
//
// Step 5: ensure the selected column will not overflow.
// then return the selected move.
int checkBoard(char current, char other) {
    int row, col;
    int a, b, c, d, e, f;
    int maxDanger = 0;  //stores which column has the highest threat level... moves there
    int maxThreat = 0;  //stores the "threat level for each column"
    int moveCol = 3;    //will change based on board
    int rowVals[COLS] = {0};
    for (row = ROWS-1; row >= 0; row--) {
        for (col = 0; col < COLS; col++) {

            //only consider possible locations: ones that have pieces under them
            // and which are also empty: Max of 7 considered per move.
            if((board[row][col] == 0) && (row == 0 || board[row-1][col] != 0)) {
                
                maxThreat = 0;
                rowVals[col] = row;

                //these moves will win the game:

                d = checkSurroundingHorizontal(board, row, col, current, 4);
                if (d >= 3) {
                    return col+1;
                }
                e = checkSurroundingVertical(board, row, col, current, 4);
                if (e >= 3) {
                    return col+1;
                }
                f = checkSurroundingDiagonal(board, row, col, current, 4);
                if (f >= 3) {
                    return col+1;
                }

                //These essntially only will return if there is "DANGER"
                //these will block a potential immediate loss:

                a = checkSurroundingHorizontal(board, row, col, other, 4);
                if (a >= 3) {
                    return col+1;
                }
                b = checkSurroundingVertical(board, row, col, other, 4);
                if (b >= 3) {
                    return col+1;
                }
                c = checkSurroundingDiagonal(board, row, col, other, 4);
                if (c >= 3) {
                    return col+1;
                }

                //preventing illegal moves:
                // if the column is filled, don't consider this column:
                if (row >= 5) {
                    continue;
                }


                // Consider the developing threats from the horizontal and diagonal
                maxThreat = a + c + d + f;

                // Columns towards the middle are more dangerous, so we assign a weighting to them:
                if (col == 3) {
                    maxThreat += 3;
                } else if (col == 2 || col == 4) {
                    maxThreat += 2;
                } else if (col == 1 || col == 5) {
                    maxThreat += 1;
                }

                if (maxThreat > maxDanger) {
                    moveCol = col;
                    maxDanger = maxThreat;
                }
            }
        }
    }

    //After we have guessed a best move we check to make sure it would not immediately cause our loss:
    //the process repeats 4 times, so if there are somehow that many bad moves, they will be skipped

    // by the time four coloumn choices could be dangerous the game is over...

    if (checkSetup(board, current, other, rowVals[moveCol], moveCol) == 1) {
        //printf("%s %d\n", "WE HAVE A PROBLEM...", moveCol);
        moveCol  = (moveCol % 6) + 1;
    
        //ensures it is a legal move...
        while (board[5][moveCol] != 0) {
            moveCol  = (moveCol % 6) + 1;
        }

         //give it a second guess...
         if (checkSetup(board, current, other, rowVals[moveCol], moveCol) == 1) {
            // printf("%s %d\n", "WE HAVE A PROBLEM...2", moveCol);
             moveCol  = (moveCol % 6) + 1; // so that we don't make a move that will trigger loss
         
            //ensures it is a legal move...
            while (board[5][moveCol] != 0) {
                moveCol  = (moveCol % 6) + 1;
            }
            
            //third check...
            if (checkSetup(board, current, other, rowVals[moveCol], moveCol) == 1) {
               // printf("%s %d\n", "WE HAVE A PROBLEM...3", moveCol);
                moveCol  = (moveCol % 6) + 1; // so that we don't make a move that will trigger loss
            
                //ensures it is a legal move...
                while (board[5][moveCol] != 0) {
                    moveCol  = (moveCol % 6) + 1;
                }
            
                //fourth check...
                if (checkSetup(board, current, other, rowVals[moveCol], moveCol) == 1) {
                   // printf("%s %d\n", "WE HAVE A PROBLEM...4", moveCol);
                    moveCol  = (moveCol % 6) + 1; // hopefully the next column is not dangerous...
                }
            }
        }
    }


    //Finally, we ensure it is a legal move
    //this should only be necessary when there have been 4 checksetups
    //TODO: make sure this mod check thing is right...
    while (board[5][moveCol] != 0) {
        moveCol  = (moveCol % 6) + 1;
    }

    // return the move, to be printed.


    return (moveCol + 1);
}


// make a move looking at our chars and theirs
int make_move() {
    char current;
    char other;

    int choice;
    
    if (numX > numO) {
        current = 'O';
        other = 'X';
    } else {
        current = 'X';
        other = 'O';
    }
   
    //somehow it is returning 0...
    choice = checkBoard(current, other);

    if (choice == 0) {
        printf("%s\n", "WE HAVE A PROBLEM OUT OF BOUNDS");
    }

   return choice;
}

// readboad, and make the move:
int main()
{

    int choice;
    read_board();

    choice = make_move();

    if (choice <= 0 || choice > 6){
            printf("%s\n", "WE HAVE A PROBLEM OUT OF BOUNDS");
    }

    printf("%d\n", choice);
    
    return 0;
}
