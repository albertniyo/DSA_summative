#ifndef MAIN_H
#define MAIN_H

#define lim_cmd 50
#define lim 64
#define try 2

typedef struct node {
    char *command;
    struct node *left;
    struct node *right;
} bst;

typedef struct {
    bst *root;
    int count;
} cmdreg;

void init(cmdreg *reg);
int load_cmd(cmdreg *reg, const char *filename);

int insert_cmd(cmdreg *reg, const char *command);
bst *search_cmd(bst *root, const char *command);
void print_cmd(bst *root);

void f_suggestion(bst *root, const char *input, char *best, int *best_dist);
void clear_reg(cmdreg *reg);

void log_unknown(const char *command);
void run_cmd();

#endif
