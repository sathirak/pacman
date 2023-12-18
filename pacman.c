#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <unistd.h>
#include <sqlite3.h>

#define ROWS 20
#define COLS 40

const char *db_file = "pcamn_record.db";
sqlite3 *db;

void init_database() {
    int rc = sqlite3_open(db_file, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    const char *create_table_query = "CREATE TABLE IF NOT EXISTS player_score (player_name TEXT, player_score INTEGER, player_time INTEGER);";
    rc = sqlite3_exec(db, create_table_query, 0, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);			
    }
}

void insert_player_info(const char *player_name, int player_score, int player_time) {
    char insert_query[200];
    sprintf(insert_query, "INSERT INTO player_score (player_name, player_score, player_time) VALUES ('%s', %d, %d);",
            player_name, player_score, player_time);

    int rc = sqlite3_exec(db, insert_query, 0, 0, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    }
}

void close_database() {
    sqlite3_close(db);
}

int score = 0;
int elapsed_seconds = 0;
int elapsed_minutes = 0;

int game_over = 0;
int pacman_row, pacman_col;
int ghost_row, ghost_col;
time_t start_time;

char pacman_char = 'C';
char ghost_char = 'X';

char player_name[50];

char board[ROWS][COLS];

const char* ghost_colors[] = {"\033[31m", "\033[32m", "\033[35m", "\033[34m"};
const int num_ghost_colors = sizeof(ghost_colors) / sizeof(ghost_colors[0]);

void initialize_board() {
	
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            board[i][j] = ' ';
        }
    }

    pacman_row = ROWS / 2;
    pacman_col = COLS / 2;
    board[pacman_row][pacman_col] = pacman_char;

    ghost_row = rand() % ROWS;
    ghost_col = rand() % COLS;
    board[ghost_row][ghost_col] = ghost_char;
    
    time(&start_time);
    
}

void hide_cursor() {
    printf("\033[?25l");
}

void show_cursor() {
    printf("\033[?25h");
}

void print_board() {
	
    hide_cursor();

    printf("\033[H");

    time_t current_time;
    time(&current_time);

    elapsed_seconds = difftime(current_time, start_time);
    elapsed_minutes = elapsed_seconds / 60;

    printf("\033[36mScore: %d\tTime: %02d:%02d\033[0m", score, elapsed_minutes, elapsed_seconds % 60);

    printf("\033[33m\t\t   %s\033[0m", player_name);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (board[i][j] == pacman_char) {
                printf("\033[33m%c \033[0m", pacman_char);
            } else if (board[i][j] == ghost_char) {
                int new_color_index;
                do {
                    new_color_index = rand() % num_ghost_colors;
                } while (new_color_index == 0);
                printf("%s%c \033[0m", ghost_colors[new_color_index], ghost_char);
            } else {
                printf("%c ", board[i][j]);
            }
        }
        printf("\n");
    }

    fflush(stdout);
    usleep(100000);

    show_cursor();
}

void move_pacman(char direction) {
    board[pacman_row][pacman_col] = ' ';

    switch (direction) {
        case 'w':
            pacman_row--;
            break;
        case 's':
            pacman_row++;
            break;
        case 'a':
            pacman_col--;
            break;
        case 'd':
            pacman_col++;
            break;
    }

    if (pacman_row < 0) pacman_row = ROWS - 1;
    if (pacman_row >= ROWS) pacman_row = 0;
    if (pacman_col < 0) pacman_col = COLS - 1;
    if (pacman_col >= COLS) pacman_col = 0;

    if (board[pacman_row][pacman_col] == ghost_char) {
        game_over = 1;
    }

    pacman_char = (pacman_char == 'C') ? 'O' : 'C';

    board[pacman_row][pacman_col] = pacman_char;
}



void move_ghost() {
    board[ghost_row][ghost_col] = ' ';

    int row_diff = ghost_row - pacman_row;
    int col_diff = ghost_col - pacman_col;

    int direction;

    if (abs(row_diff) > abs(col_diff)) {

        direction = (row_diff > 0) ? 1 : 0;
    } else {

        direction = (col_diff > 0) ? 3 : 2;
    }

    switch (direction) {
        case 0:
            ghost_row++;
            break;
        case 1:
            ghost_row--;
            break;
        case 2:
            ghost_col++;
            break;
        case 3:
            ghost_col--;
            break;
    }

    if (ghost_row < 0) ghost_row = ROWS - 1;
    if (ghost_row >= ROWS) ghost_row = 0;
    if (ghost_col < 0) ghost_col = COLS - 1;
    if (ghost_col >= COLS) ghost_col = 0;
    
    if (board[ghost_row][ghost_col] == pacman_char) {
        game_over = 1;
    }

    int new_color_index;
    do {
        new_color_index = rand() % num_ghost_colors;
    } while (new_color_index == 0);

    board[ghost_row][ghost_col] = ghost_char;
    printf("%s%c \033[0m", ghost_colors[new_color_index], ghost_char);
}


void game_menu() {

    system("cls");
    printf("\n\n\n");
    printf("\033[1;36m\t\t\t\t==================================\033[0m\n");
    printf("\033[1;36m\t\t\t\tWelcome to the Pacman Game! \033[0m %s", player_name);
    printf("\n\033[1;36m\t\t\t\t==================================\033[0m\n");
    printf("\n\n\t\t\t\t\t\033[1;36m1. Start Game\033[0m\n");
    printf("\n\n\t\t\t\t\t\033[1;36m2. Exit\033[0m\n");

    char choice;
    do {
        choice = _getch();
    } while (choice != '1' && choice != '2');

    if (choice == '2') {
        exit(0);
    }
}

void get_player_name() {
    printf("\n\n\t\t\t        Pacman Game!     ");
    printf("\n\n\t\t\t  Enter your name ===> ");
    scanf("%s", player_name);
}


int main() {

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, 11);

	get_player_name();
	init_database();

    char play_again;

    do {
        game_menu();

        initialize_board();
        game_over = 0;
        score = 0;
        time(&start_time);
        clock_t last_ghost_move = clock();
       
        while (!game_over) {
            print_board();

            char move;

            if (_kbhit()) {
                move = _getch();
                move_pacman(move);
            }

            clock_t current_time = clock();
            if ((double)(current_time - last_ghost_move) / CLOCKS_PER_SEC >= 0.2) {
                move_ghost();
                last_ghost_move = current_time;
            }

            score++;

            for (int i = 0; i < 10000000; i++) {}

            system("cls");
        }
        
        insert_player_info(player_name, score, elapsed_seconds);

        printf("\n\n\033[32m \t\t\t\tGame Over! %s \n\n\t\t\t\tYour score: %d\033[0m\n\n", player_name, score);
		printf("\t\t\t\t\033[36mTime: %02d:%02d\033[0m", score, elapsed_minutes, elapsed_seconds % 60);
        printf("\n\n\t\t\t\tRetry? [ Y / N ] ");

        play_again = _getch();

        if (play_again == 'y' || play_again == 'Y') {
        	
            system("cls");
            
        } else {
        	
        	system("cls");
		}

    } while (play_again == 'y' || play_again == 'Y');

    return 0;
}

