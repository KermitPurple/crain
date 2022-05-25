#include<math.h>
#include<time.h>
#include<ctype.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<ncurses.h>

#define _ARG3(a, b, c, ...) c
#define _NEW1(LVAL) _NEW2(LVAL, 1)
#define _NEW2(LVAL, COUNT) LVAL = calloc(COUNT, sizeof(*LVAL))
#define NEW(...) _ARG3(\
        __VA_ARGS__,\
        _NEW2,\
        _NEW1,\
    )(__VA_ARGS__)

#define CHARACTERS_LEN 94
const char CHARACTERS[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_-+={}|[]\\;:'\",<.>/?`~";
struct pos term_size;

typedef struct pos {
    int x, y;
} pos_t;

typedef struct trail{
    pos_t pos;
    char *chars;
    int len;
} trail_t;

int rand_between(int start, int end){
    return rand() % end + start;
}

char random_character(){
    return CHARACTERS[rand() % CHARACTERS_LEN];
}

char* random_chars(int len){
    if(len < 0){
        return NULL;
    }
    char *NEW(str, len + 1);
    int i = 0;
    while(i < len){
        str[i++] = random_character();
    }
    str[i] = '\0';
    return str;
}

int max(int a, int b){
    if(a > b)
        return a;
    return b;
}

void draw_trail(trail_t *trail){
    for(int i = 0; trail->pos.y - i >= 0; i++){
        if(trail->chars[i] == '\0'){
            mvaddch(trail->pos.y - i, trail->pos.x, ' ');
            break;
        }
        mvprintw(
            trail->pos.y - i,
            trail->pos.x,
            "%c",
            trail->chars[i]
        );
    }
}

void draw(trail_t *trails, int len){
    for(int i = 0; i < len; i++){
        draw_trail(trails + i);
    }
}

void update(trail_t *trails, int len){
    for(int i = 0; i < len; i++){
        trails[i].pos.y += 1;
        if(trails[i].pos.y - trails[i].len > term_size.y){
            trails[i].pos.y = 0;
            free(trails[i].chars);
            trails[i].len = rand_between(term_size.y * 0.2, term_size.y * 0.4);
            trails[i].chars = random_chars(trails[i].len);
        }
    }
}

trail_t *make_trails(){
    int len = term_size.x;
    trail_t *NEW(result, len);
    for(int i = 0; i < len; i++){
        result[i].pos.x = i;
        result[i].pos.y = rand_between(-30, term_size.y + 30);
        result[i].len = rand_between(term_size.y * 0.2, term_size.y * 0.4);
        result[i].chars = random_chars(result[i].len);
    }
    return result;
}

void free_trails(trail_t* trails){
    for(int i = 0; i < term_size.x; i++){
        free(trails[i].chars);
    }
    free(trails);
}

int main(){
    srand(time(0));
    initscr();
    curs_set(0); // hide cursor
    nodelay(stdscr, TRUE); // don't block input
    noecho(); // don't display gotten characters
    getmaxyx(stdscr, term_size.y, term_size.x); // calculate terminal size
    double desired_time = 0.1;
    move(1000, 1000);
    trail_t *trails = make_trails();
    while(tolower(getch()) != 'q'){
        clock_t begin = clock();
        int nexty, nextx;
        getmaxyx(stdscr, nexty, nextx);
        if(nextx != term_size.x || nexty != term_size.y){
            free_trails(trails);
            trails = make_trails();
            clear();
        }
        term_size = (pos_t){nextx, nexty};
        update(trails, term_size.x);
        draw(trails, term_size.x);
        refresh();
        double elapsed = (double)(clock() - begin) / CLOCKS_PER_SEC;
        if(elapsed < desired_time){
            usleep((desired_time - elapsed) * 1000000);
        }
    }
    curs_set(1); // show cursor
    endwin();
    return 0;
}
