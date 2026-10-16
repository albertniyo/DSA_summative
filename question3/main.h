#ifndef MAIN_H
#define MAIN_H

#define lim_servers 32
#define lim 64

typedef struct {
    char names[lim_servers][lim];
    int matrix[lim_servers][lim_servers];
    int count;
} graph;

void init(graph *g);
void build_def(graph *g);

int find_server(const graph *g, const char *name);
int add_server(graph *g, const char *name);
int remove_server(graph *g, const char *name);

int add_dep(graph *g, const char *from, const char *to);
int remove_dep(graph *g, const char *from, const char *to);
void query_server(const graph *g, const char *name);

void display(const graph *g);
void analyzer();

#endif
