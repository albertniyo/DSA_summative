#ifndef MAIN_H
#define MAIN_H

#define lim_loc 16
#define lim 64
#define inf 999999

typedef struct {
    char names[lim_loc][lim];
    int dist[lim_loc][lim_loc];
    int count;
} roadnet;

void init(roadnet *net);
void build_def(roadnet *net);

int find_loc(const roadnet *net, const char *name);
void list_loc(const roadnet *net);

void shortest_route(const roadnet *net, const char *start, const char *dest);
void routing();

#endif
