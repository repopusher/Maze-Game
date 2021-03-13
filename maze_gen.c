//Maze generation code was provided by Dr Victor Cionca, implementation of features below

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WALL 'w'
#define POTION '#'
#define NEEDED_POTIONS 3

struct maze{
    char **a; // 2D array supporting maze
    unsigned int w; // width
    unsigned int h; // height
    unsigned int cell_size; // number of chars per cell; walls are 1 char
};

/**
 * Represents a cell in the 2D matrix.
 */
struct cell{
    unsigned int x;
    unsigned int y;
};

/**
 * Stack structure using a list of cells.
 * At element 0 in the list we have NULL.
 * Elements start from 1 onwards.
 * top_of_stack represents the index of the top of the stack
 * in the cell_list.
 */
struct stack{
    struct cell *cell_list;
    unsigned int top_of_stack;
    unsigned int capacity;
};

/**
 * Initialises the stack by allocating memory for the internal list
 */
void init_stack(struct stack *stack, unsigned int capacity){
    stack->cell_list = (struct cell*)malloc(sizeof(struct cell)*(capacity+1));
    stack->top_of_stack = 0;
    stack->capacity = capacity;
}

void free_stack(struct stack *stack){
    free(stack->cell_list);
}

/**
 * Returns the element at the top of the stack and removes it
 * from the stack.
 * If the stack is empty, returns NULL
 */
struct cell stack_pop(struct stack *stack){
    struct cell cell = stack->cell_list[stack->top_of_stack];
    if (stack->top_of_stack > 0) stack->top_of_stack --;
    return cell;
}

/**
 * Pushes an element to the top of the stack.
 * If the stack is already full (reached capacity), returns -1.
 * Otherwise returns 0.
 */
int stack_push(struct stack *stack, struct cell cell){
    if (stack->top_of_stack == stack->capacity) return -1;
    stack->top_of_stack ++;
    stack->cell_list[stack->top_of_stack] = cell;
    return 0;
}

int stack_isempty(struct stack *stack){
    return stack->top_of_stack == 0;
}

//-----------------------------------------------------------------------------

void mark_visited(struct maze *maze, struct cell cell){
    maze->a[cell.y][cell.x] = 'v';
}

/**
 * Convert a cell coordinate to a matrix index.
 * The matrix also contains the wall elements and a cell might span
 * multiple matrix cells.
 */
int cell_to_matrix_idx(struct maze *m, int cell){
    return (m->cell_size+1)*cell+(m->cell_size/2)+1;
}

/**
 * Convert maze dimension to matrix dimension.
 */
int maze_dimension_to_matrix(struct maze *m, int dimension){
    return (m->cell_size+1)*dimension+1;
}

/**
 * Returns the index of the previous cell (cell - 1)
 */
int matrix_idx_prev_cell(struct maze *m, int cell_num){
    return cell_num - (m->cell_size+1);
}

/**
 * Returns the index of the next cell (cell + 1)
 */
int matrix_idx_next_cell(struct maze *m, int cell_num){
    return cell_num + (m->cell_size+1);
}

/**
 * Returns into neighbours the unvisited neighbour cells of the given cell.
 * Returns the number of neighbours.
 * neighbours must be able to hold 4 cells.
 */
int get_available_neighbours(struct maze *maze, struct cell cell, struct cell *neighbours){
    int num_neighbrs = 0;

    // Check above
    if ((cell.y > cell_to_matrix_idx(maze,0)) && (maze->a[matrix_idx_prev_cell(maze, cell.y)][cell.x] != 'v')){
        neighbours[num_neighbrs].x = cell.x;
        neighbours[num_neighbrs].y = matrix_idx_prev_cell(maze, cell.y);
        num_neighbrs ++;
    }

    // Check left
    if ((cell.x > cell_to_matrix_idx(maze,0)) && (maze->a[cell.y][matrix_idx_prev_cell(maze, cell.x)] != 'v')){
        neighbours[num_neighbrs].x = matrix_idx_prev_cell(maze, cell.x);
        neighbours[num_neighbrs].y = cell.y;
        num_neighbrs ++;
    }

    // Check right
    if ((cell.x < cell_to_matrix_idx(maze,maze->w-1)) && (maze->a[cell.y][matrix_idx_next_cell(maze, cell.x)] != 'v')){
        neighbours[num_neighbrs].x = matrix_idx_next_cell(maze, cell.x);
        neighbours[num_neighbrs].y = cell.y;
        num_neighbrs ++;
    }

    // Check below
    if ((cell.y < cell_to_matrix_idx(maze,maze->h-1)) && (maze->a[matrix_idx_next_cell(maze, cell.y)][cell.x] != 'v')){
        neighbours[num_neighbrs].x = cell.x;
        neighbours[num_neighbrs].y = matrix_idx_next_cell(maze, cell.y);
        num_neighbrs ++;
    }

    return num_neighbrs;
}


/**
 * Removes a wall between two cells.
 */
void remove_wall(struct maze *maze, struct cell a, struct cell b){
    int i;
    if (a.y == b.y){
        for (i=0;i<maze->cell_size;i++)
            maze->a[a.y-maze->cell_size/2+i][a.x-(((int)a.x-(int)b.x))/2] = ' ';
    }else{
        for (i=0;i<maze->cell_size;i++)
            maze->a[a.y-(((int)a.y-(int)b.y))/2][a.x-maze->cell_size/2+i] = ' ';
    }
}

/**
 * Fill all matrix elements corresponding to the cell
 */
void fill_cell(struct maze *maze, struct cell c, char value){
    int i,j;
    for (i=0;i<maze->cell_size;i++)
        for (j=0;j<maze->cell_size;j++)
            maze->a[c.y-maze->cell_size/2+i][c.x-maze->cell_size/2+j] = value;
}

/**
 * This function generates a maze of width x height cells.
 * Each cell is a square of cell_size x cell_size characters.
 * The maze is randly generated based on the supplied rand_seed.
 * Use the same rand_seed value to obtain the same maze.
 *
 * The function returns a struct maze variable containing:
 * - the maze represented as a 2D array (field a)
 * - the width (number of columns) of the array (field w)
 * - the height (number of rows) of the array (field h).
 * In the array, walls are represented with a 'w' character, while
 * pathways are represented with spaces (' ').
 * The edges of the array consist of walls, with the exception
 * of two openings, one on the left side (column 0) and one on
 * the right (column w-1) of the array. These should be used
 * as entry and exit.
 */
struct maze generate_maze(unsigned int width, unsigned int height, unsigned int cell_size, int rand_seed){
    int row, col, i;
    struct stack stack;
    struct cell cell;
    struct cell neighbours[4];  // to hold neighbours of a cell
    int num_neighbs;
    struct maze maze;
    maze.w = width;
    maze.h = height;
    maze.cell_size = cell_size;
    maze.a = (char**)malloc(sizeof(char*)*maze_dimension_to_matrix(&maze, height));

    // Initialise RNG
    srand(rand_seed);

    // Initialise stack
    init_stack(&stack, width*height);

    // Initialise the matrix with walls
    for (row=0;row<maze_dimension_to_matrix(&maze, height);row++){
        maze.a[row] = (char*)malloc(maze_dimension_to_matrix(&maze, width));
        memset(maze.a[row], WALL, maze_dimension_to_matrix(&maze, width));
    }

    // Select a rand position on a border.
    // Border means x=0 or y=0 or x=2*width+1 or y=2*height+1
    cell.x = cell_to_matrix_idx(&maze,0);
    cell.y = cell_to_matrix_idx(&maze,rand()%height);
    mark_visited(&maze, cell);
    stack_push(&stack, cell);

    while (! stack_isempty(&stack)){
        // Take the top of stack
        cell = stack_pop(&stack);
        // Get the list of non-visited neighbours
        num_neighbs = get_available_neighbours(&maze, cell, neighbours);
        if (num_neighbs > 0){
            struct cell next;
            // Push current cell on the stack
            stack_push(&stack, cell);
            // Select one rand neighbour
            next = neighbours[rand()%num_neighbs];
            // Mark it visited
            mark_visited(&maze, next);
            // Break down the wall between the cells
            remove_wall(&maze, cell, next);
            // Push new cell on the stack
            stack_push(&stack, next);
        }
    }

    // Finally, replace 'v' with spaces
    for (row=0;row<maze_dimension_to_matrix(&maze, height);row++)
        for (col=0;col<maze_dimension_to_matrix(&maze, width);col++)
            if (maze.a[row][col] == 'v'){
                cell.y = row;
                cell.x = col;
                fill_cell(&maze, cell, ' ');
            }

    // Select an entry point in the top right corner.
    // The first border cell that is available.
    for (row=0;row<maze_dimension_to_matrix(&maze, height);row++)
        if (maze.a[row][1] == ' ') { maze.a[row][0] = ' '; break; }

    // Select the exit point
    for (row=maze_dimension_to_matrix(&maze, height)-1;row>=0;row--)
        if (maze.a[row][cell_to_matrix_idx(&maze,width-1)] == ' ') {
            maze.a[row][maze_dimension_to_matrix(&maze, width)-1] = ' ';
            break;
        }

    maze.w = maze_dimension_to_matrix(&maze, maze.w);
    maze.h = maze_dimension_to_matrix(&maze, maze.h);

    // Add the potions inside the maze at three rand locations
    for (i=0;i<NEEDED_POTIONS;i++){
        do{
            row = rand()%(maze.h-1);
            col = rand()%(maze.w-1);
        }while (maze.a[row][col] != ' ');
        maze.a[row][col] = POTION;
    }

    return maze;
}

//Author: Tobias Lennon
//13/02/2020

//Initialize
int i, j, mazeWidth, mazeHeight, cellSize, mazeSeed, fogRadius, tester, entrance, playerHeight = 0, playerWidth = 0, potionCounter = 0
, startRow, endRow, startCol, endCol;
char movement, player = '@', space = ' ', potion = '#';

//Method simply loops through the 2d array that is the maze and prints.
void printMaze(struct maze mainMaze){
    printf("Potions: %d\n", potionCounter);
    for(i = 0; i < mainMaze.h; i++){
        for(j = 0; j < mainMaze.w; j++){
            printf("%c", mainMaze.a[i][j]);
        }
        printf("\n");       //one line of the maze is finished generating, moves the the next line
    }
}

//Method finds/prints the player at the entrance of the maze
void findEntrance(struct maze mainMaze){
    for (i = 0; i < mainMaze.h; i++){       //loops through the first column to find the space that represents the entrance
        if (mainMaze.a[i][0] == space){
            playerHeight = i;
            printf("%d, 0\n", i);
            mainMaze.a[playerHeight][playerWidth] = player; //Plyaer is now that emtpy space
        }
    }
}

//Method congratulates the user if they try to exit with 3 potions, it denies them exit if they don't have the required amount of potions.
int exitMaze(struct maze mainMaze){
    if(playerHeight == (mainMaze.h - 2) && (playerWidth + 1) == (mainMaze.w -1)){   //Checks if the player is trying to exit
        if(potionCounter == 3){     //Player has 3 potions and is allowed to exit the maze.
            playerWidth += 1; 
            mainMaze.a[playerHeight][playerWidth] = player;
            mainMaze.a[playerHeight][playerWidth - 1] = space;
            printMaze(mainMaze);
            printf("\nCongratulations you have collected all 3 potions and beaten the maze.");
            exit(0);
        }
        else{
            printf("You only have %d potions, you need 3 to escape the maze.", potionCounter);  //Prints if player has less then 3 potions
            return 0;
        }
    }
}

//Method prints the maze, but only in a radius around the player's current position.
void fog(struct maze mainMaze){

    //X = playerHeight, Y = playerWidth
    startRow = playerHeight - fogRadius;    //Finding the area to be printed
    startCol = playerWidth - fogRadius;     
    endRow = playerHeight + fogRadius;
    endCol = playerWidth + fogRadius;

    if (startRow < 0){                      //Checks top left corner of 2d array for out of bounds.
        startRow = 0;
    }
    if (startCol < 0){
        startCol = 0;
    }
    if(endRow > mainMaze.h - 1){            //Checks bottom right corner of 2d array for out of bounds.
        endRow = mainMaze.h - 1;
    }
    if(endCol > mainMaze.w- 1){
        endCol = mainMaze.w - 1;
    }
    
    printf("Potions: %d\n", potionCounter);     //printing potion counter
    for(i = startRow; i <= endRow; i++){
        for(j = startCol; j <= endCol; j++){
            printf("%c", mainMaze.a[i][j]);
        }
        printf("\n");
    }
}

void main(){

    //input
    printf("Maze width: ");
    scanf("%d", &mazeWidth);
    printf("Maze height: ");
    scanf("%d", &mazeHeight);
    printf("Maze cell size: ");
    scanf("%d", &cellSize);
    printf("Maze seed: ");
    scanf("%d", &mazeSeed);
    printf("Fog radius: ");
    scanf("%d", &fogRadius);
    
    struct maze mainMaze = generate_maze(mazeWidth, mazeHeight, cellSize, mazeSeed);    //Passing varibles into generate_maze()

    findEntrance(mainMaze); //calling findEntrance

    if(fogRadius > 0){
        fog(mainMaze);
    }
    else{
        printMaze(mainMaze);    //calling printMaze
    }    

    while(movement != 'e'){     //Looping character movement, can exit with char e

        switch (movement){      //switch statement for movement input

        case 'w':
            if(playerHeight > 0 && mainMaze.a[playerHeight - 1][playerWidth] != 'w'){   /*Checks if wall is not in front of the player and that they are within the bounds of the maze
                                                                                           if there is a wall, or out of bounds the player will not be able to move there.*/
                if(mainMaze.a[playerHeight - 1][playerWidth] == potion){    //Checks if a potion is in front of the player.
                    potionCounter += 1;                                     //Increments potions.
                }

                playerHeight -= 1;                                  //incrementing/decrementing playerHeight/playerWidth depending on the movement
                mainMaze.a[playerHeight][playerWidth] = player;     //satting player to the new position in the maze.
                mainMaze.a[playerHeight + 1][playerWidth] = space;  //setting space to the position the player was last,

                if(fogRadius > 0){                                  //If fog is > 0 for will be implemented
                    fog(mainMaze);                                  //calling fog()
                }
                else{
                    printMaze(mainMaze);                            //calling printmaze()
                }                              
            }
            break;

        case 'a':
            if(playerWidth > 0 && mainMaze.a[playerHeight][playerWidth - 1] != 'w'){

                if(mainMaze.a[playerHeight][playerWidth - 1] == potion){
                    potionCounter += 1;
                }

                playerWidth -= 1;
                mainMaze.a[playerHeight][playerWidth] = player;
                mainMaze.a[playerHeight][playerWidth + 1] = space;

                if(fogRadius > 0){
                    fog(mainMaze);
                }
                else{
                    printMaze(mainMaze);
                }
            }
            break;

        case 's':
            if(playerHeight < mainMaze.h && mainMaze.a[playerHeight + 1][playerWidth] != 'w'){

                if(mainMaze.a[playerHeight + 1][playerWidth] == potion){
                    potionCounter += 1;
                }

                playerHeight += 1;
                mainMaze.a[playerHeight][playerWidth] = player;
                mainMaze.a[playerHeight - 1][playerWidth] = space;

                if(fogRadius > 0){
                    fog(mainMaze);
                }
                else{
                    printMaze(mainMaze);
                }
            }
            break;

        case 'd':
            if(playerWidth < mainMaze.w - 1 && mainMaze.a[playerHeight][playerWidth + 1] != 'w'){

                i = exitMaze(mainMaze);         //Calling exit maze
                if(i == 0){
                    break;
                }

                if(mainMaze.a[playerHeight][playerWidth + 1] == potion){
                    potionCounter += 1;
                }

                playerWidth += 1;
                mainMaze.a[playerHeight][playerWidth] = player;
                mainMaze.a[playerHeight][playerWidth - 1] = space;

                if(fogRadius > 0){
                    fog(mainMaze);
                }
                else{
                    printMaze(mainMaze);
                }
            }
            break;
        }
        movement = getchar();   //Takes input
    }
}