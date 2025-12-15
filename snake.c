#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BOARD_WIDTH 30
#define BOARD_HEIGHT 20
#define INITIAL_SNAKE_LENGTH 3

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

#define CONTINUE 0
#define QUIT 1
#define RESET 2

#define DELAY 150000
// 150000 microseconds ~ 150ms

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point *body;
    int length;
    int direction;
} Snake;

typedef struct {
    Point food;
    int score;
    int game_over;
} GameState;

void settings() {
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    curs_set(0);
    srand(time(NULL));
}

void place_food(Snake *snake, GameState *game) {
    int position_valid = 0;
    
    while (!position_valid) {
        game->food.x = rand() % BOARD_WIDTH;
        game->food.y = rand() % BOARD_HEIGHT;
        
        position_valid = 1;
        for (int i = 0; i < snake->length; i++) {
            if (snake->body[i].x == game->food.x && 
                snake->body[i].y == game->food.y) {
                position_valid = 0;
                break;
            }
        }
    }
}

void setup_game(Snake *snake, GameState *game) {
    if (snake->body == NULL) {
        snake->body = malloc(BOARD_WIDTH * BOARD_HEIGHT * sizeof(Point));
    }
    
    snake->length = INITIAL_SNAKE_LENGTH;
    snake->direction = RIGHT;
    
    int center_x = BOARD_WIDTH / 2;
    int center_y = BOARD_HEIGHT / 2;
    
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = center_x - i;
        snake->body[i].y = center_y;
    }
    
    game->score = 0;
    game->game_over = 0;
    
    game->food.x = center_x + 5;
    game->food.y = center_y;
}

void restart_game(Snake *snake, GameState *game) {
    snake->length = INITIAL_SNAKE_LENGTH;
    snake->direction = RIGHT;
    
    int center_x = BOARD_WIDTH / 2;
    int center_y = BOARD_HEIGHT / 2;
    
    for (int i = 0; i < snake->length; i++) {
        snake->body[i].x = center_x - i;
        snake->body[i].y = center_y;
    }
    
    game->score = 0;
    game->game_over = 0;
    
    place_food(snake, game);
}

int handle_movement(Snake *snake) {
    int key = getch();
    
    switch (key) {
        case 'w':
            if (snake->direction != DOWN) {
                snake->direction = UP;
            }
            break;
        case 's':
            if (snake->direction != UP) {
                snake->direction = DOWN;
            }
            break;
        case 'a':
            if (snake->direction != RIGHT) {
                snake->direction = LEFT;
            }
            break;
        case 'd':
                if (snake->direction != LEFT) {
                snake->direction = RIGHT;
            }
            break;
        case 'q':
            return QUIT;
        case 'r':
            return RESET;
    }
    return CONTINUE;
}

void move_snake(Snake *snake, GameState *game) {
    Point next_head = snake->body[0];
    
    switch (snake->direction) {
        case UP:
            next_head.y--;
            break;
        case RIGHT:
            next_head.x++;
            break;
        case DOWN:
            next_head.y++;
            break;
        case LEFT:
            next_head.x--;
            break;
    }
    
    if (next_head.x < 0 || next_head.x >= BOARD_WIDTH ||
        next_head.y < 0 || next_head.y >= BOARD_HEIGHT) {
        game->game_over = 1;
        return;
    }
    
    int ate_food = 0;
    if (next_head.x == game->food.x && next_head.y == game->food.y) {
        ate_food = 1;
        game->score++;
        snake->length++;
    }
    
    int collision_check_length = ate_food ? snake->length : snake->length - 1;
    for (int i = 0; i < collision_check_length; i++) {
        if (snake->body[i].x == next_head.x && 
            snake->body[i].y == next_head.y) {
            game->game_over = 1;
            return;
        }
    }
    
    for (int i = snake->length - 1; i > 0; i--) {
        snake->body[i] = snake->body[i - 1];
    }
    snake->body[0] = next_head;
    
    if (ate_food) {
        place_food(snake, game);
    }
}

void draw_game(Snake *snake, GameState *game) {
    clear();
    
    for (int x = 0; x < BOARD_WIDTH + 2; x++) {
        mvprintw(0, x, "#");
    }
    
    for (int y = 1; y <= BOARD_HEIGHT; y++) {
        mvprintw(y, 0, "#");
        mvprintw(y, BOARD_WIDTH + 1, "#");
    }
    
    for (int x = 0; x < BOARD_WIDTH + 2; x++) {
        mvprintw(BOARD_HEIGHT + 1, x, "#");
    }
    
    for (int i = 0; i < snake->length; i++) {
        char segment_char;
        if (i == 0) {
            segment_char = 'O';
        } else {
            segment_char = 'o';
        }
        mvprintw(snake->body[i].y + 1, snake->body[i].x + 1, "%c", segment_char);
    }
    
    mvprintw(game->food.y + 1, game->food.x + 1, "*");
    
    mvprintw(BOARD_HEIGHT + 3, 0, "score: %d", game->score);
    mvprintw(BOARD_HEIGHT + 4, 0, "controls: WASD to move, q to quit");
    
    if (game->game_over) {
        mvprintw(BOARD_HEIGHT + 6, 0, "game over, press r to restart, q to quit");
    }
    
    refresh();
}

int main() {
    Snake snake;
    GameState game;
    int should_quit = 0;
    int is_first_game = 1;
    
    snake.body = NULL;
    
    settings();
    
    while (!should_quit) {
        if (is_first_game) {
            setup_game(&snake, &game);
            place_food(&snake, &game);
            is_first_game = 0;
        } else {
            restart_game(&snake, &game);
        }
        
        while (!game.game_over) {
            int input_action = handle_movement(&snake);
            if (input_action == QUIT) {
                should_quit = 1;
                break;
            }
            
            move_snake(&snake, &game);
            draw_game(&snake, &game);
            
            usleep(DELAY);
        }
        
        if (should_quit) break;
        
        draw_game(&snake, &game);
        
        nodelay(stdscr, FALSE);
        flushinp();
        int key;
        while (1) {
            key = getch();
            if (key == 'q' || key == 'Q') {
                should_quit = 1;
                break;
            } else if (key == 'r' || key == 'R') {
                flushinp();
                nodelay(stdscr, TRUE);
                break;
            }
        }
    }
    
    free(snake.body);
    endwin();
    
    return 0;
}
