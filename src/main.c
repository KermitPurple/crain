#include<math.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#ifndef _WIN32
#include<sys/ioctl.h>
#endif

#define _ARG3(a, b, c, ...) c
#define _NEW1(LVAL) _NEW2(LVAL, 1)
#define _NEW2(LVAL, COUNT) LVAL = calloc(COUNT, sizeof(*LVAL))
#define NEW(...) _ARG3(\
        __VA_ARGS__,\
        _NEW2,\
        _NEW1,\
    )(__VA_ARGS__)

#define CSI "\e["
#define RGB CSI "38;2;"
#define HIDE CSI "?25l"
#define SHOW CSI "?25h"
#define MOVE "H"
#define RESET CSI "0m"
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

pos_t get_terminal_size(){
#ifdef _WIN32
    fputs("Shanye hasn't implemented this for windows yet");
    exit(1);
#else
    struct winsize ws;
    ioctl(0, TIOCGWINSZ, &ws);
    return (pos_t){ws.ws_col, ws.ws_row};
#endif
}

void move(int x, int y) {
    printf(CSI "%d;%d" MOVE, x, y);
}

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
        move(trail->pos.y - i, trail->pos.x);
        if(trail->chars[i] == '\0'){
            putchar(' ');
            break;
        }
        printf(
            CSI RGB "0;%d;0m%c",
            max(0, 255 - i * (255 / trail->len)),
            trail->chars[i]
        );
    }
}

void draw(trail_t *trails, int len){
    for(int i = 0; i < len; i++){
        draw_trail(trails + i);
    }
}

// TODO get a better name
void set_chars(trail_t *trail){
    trail->len = rand_between(term_size.y * 0.2, term_size.y * 0.4);
    trail->chars = random_chars(trail->len);
}

void update(trail_t *trails, int len){
    for(int i = 0; i < len; i++){
        trails[i].pos.y += 1;
        if(trails[i].pos.y - trails[i].len > term_size.y){
            trails[i].pos.y = 0;
            free(trails[i].chars);
            set_chars(trails + i);
        }
    }
}

trail_t *make_trails(){
    int len = term_size.x;
    trail_t *NEW(result, len);
    for(int i = 0; i < len; i++){
        result[i].pos.x = i;
        result[i].pos.y = rand_between(-30, term_size.y + 30);
        set_chars(result + i);
    }
    return result;
}

void free_trails(trail_t* trails){
    for(int i = 0; i < term_size.x; i++){
        free(trails[i].chars);
    }
    free(trails);
}

void cleanup(){
    system("clear");
    printf(SHOW RESET);
    exit(0);
}

int main(){
    srand(time(0));
    signal(SIGINT, cleanup);
    term_size = get_terminal_size();
    double desired_time = 0.1;
    trail_t *trails = make_trails();
    printf(HIDE);
    system("clear");
    while(1){
        clock_t begin = clock();
        pos_t next_size = get_terminal_size();
        if(next_size.x != term_size.x || next_size.y != term_size.y){
            free_trails(trails);
            term_size = next_size;
            trails = make_trails();
            system("clear");
        }
        update(trails, term_size.x);
        draw(trails, term_size.x);
        double elapsed = (double)(clock() - begin) / CLOCKS_PER_SEC;
        if(elapsed < desired_time){
            usleep((desired_time - elapsed) * 1000000);
        }
    }
    return 0;
}
