#include "main.h"

#include <stdio.h>
#include <string.h>

void init(roadnet *net) {
    int i, j;

    net->count = 0;
    for (i = 0; i < lim_loc; i++) {
        net->names[i][0] = '\0';
        for (j = 0; j < lim_loc; j++) {
            net->dist[i][j] = (i == j) ? 0 : inf;
        }
    }
}

int find_loc(const roadnet *net, const char *name) {
    int i;

    for (i = 0; i < net->count; i++) {
        if (strcmp(net->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

static int add_loc(roadnet *net, const char *name) {
    int idx = find_loc(net, name);

    if (idx != -1) {
        return idx;
    }
    if (net->count >= lim_loc) {
        return -1;
    }

    strncpy(net->names[net->count], name, lim - 1);
    net->names[net->count][lim - 1] = '\0';
    net->count++;

    return net->count - 1;
}

static void add_road(roadnet *net, const char *a, const char *b, int weight) {
    int i = add_loc(net, a);
    int j = add_loc(net, b);

    if (i < 0 || j < 0 || weight < 0) {
        return;
    }

    net->dist[i][j] = weight;
    net->dist[j][i] = weight;
}

void build_def(roadnet *net) {
    add_road(net, "central warehouse", "distribution center", 8);
    add_road(net, "central warehouse", "district hospital", 14);
    add_road(net, "distribution center", "health center A", 5);
    add_road(net, "distribution center", "health center B", 11);
    add_road(net, "health center A", "district hospital", 7);
    add_road(net, "health center B", "health center C", 3);
    add_road(net, "health center C", "district hospital", 6);
    add_road(net, "health center B", "district hospital", 9);
    add_road(net, "health Center A", "health center C", 4);
}

void list_loc(const roadnet *net) {
    int i;

    puts("\nlocations:");
    for (i = 0; i < net->count; i++) {
        printf("  %d. %s\n", i + 1, net->names[i]);
    }
}

static int dijkstra(const roadnet *net, int src, int dst,
                    int *path, int *path_len) {
    int dist[lim_loc];
    int prev[lim_loc];
    int visited[lim_loc];
    int i, u, v, count;
    int min_dist;

    for (i = 0; i < net->count; i++) {
        dist[i] = inf;
        prev[i] = -1;
        visited[i] = 0;
    }
    dist[src] = 0;

    for (count = 0; count < net->count; count++) {
        u = -1;
        min_dist = inf;
        for (i = 0; i < net->count; i++) {
            if (!visited[i] && dist[i] < min_dist) {
                min_dist = dist[i];
                u = i;
            }
        }

        if (u == -1) {
            break;
        }

        visited[u] = 1;

        for (v = 0; v < net->count; v++) {
            if (!visited[v] && net->dist[u][v] < inf &&
                dist[u] + net->dist[u][v] < dist[v]) {
                dist[v] = dist[u] + net->dist[u][v];
                prev[v] = u;
            }
        }
    }

    if (dist[dst] >= inf) {
        return -1;
    }

    if (path != NULL && path_len != NULL) {
        *path_len = 0;
        for (v = dst; v != -1; v = prev[v]) {
            path[(*path_len)++] = v;
        }
    }

    return dist[dst];
}

void shortest_route(const roadnet *net, const char *start, const char *dest) {
    int src = find_loc(net, start);
    int dst = find_loc(net, dest);
    int path[lim_loc];
    int path_len = 0;
    int total;
    int i;

    if (start[0] == '\0' || dest[0] == '\0') {
        puts("location name cannot be empty");
        return;
    }
    if (src == -1) {
        printf("starting location not found: %s\n", start);
        return;
    }
    if (dst == -1) {
        printf("destination not found: %s\n", dest);
        return;
    }
    if (src == dst) {
        puts("start and end destination are the same");
        printf("\nshortest route:\n%s\n", net->names[src]);
        puts("\ntotal distance is 0 km");
        return;
    }

    total = dijkstra(net, src, dst, path, &path_len);
    if (total < 0) {
        puts("cannot be reached");
        return;
    }

    puts("\nshortest route");
    for (i = path_len - 1; i >= 0; i--) {
        printf("%s", net->names[path[i]]);
        if (i > 0) {
            printf(" -> ");
        }
    }
    printf("\ntotal distance is %d km\n", total);
}

static void show_route_distance(const roadnet *net, const char *start,
                                const char *dest) {
    int src = find_loc(net, start);
    int dst = find_loc(net, dest);
    int total;

    if (start[0] == '\0' || dest[0] == '\0') {
        puts("loc name cannot be empty");
        return;
    }
    if (src == -1) {
        printf("starting location  %s not found\n", start);
        return;
    }
    if (dst == -1) {
        printf("destination %s not found\n", dest);
        return;
    }
    if (src == dst) {
        puts("total distance is 0 km");
        return;
    }

    total = dijkstra(net, src, dst, NULL, NULL);
    if (total < 0) {
        puts("destination cannot be reached");
        return;
    }

    printf("total distance is %d km\n", total);
}

static void read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

void routing(void) {
    roadnet net;
    char choice[16];
    char start[lim];
    char dest[lim];

    init(&net);
    build_def(&net);

    puts("Emergency Medical Supply Routing");
    puts("1. Display locations");
    puts("2. Find shortest route");
    puts("3. Display route distance");
    puts("4. Exit");

    while (1) {
        printf("\nChoice: ");
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            break;
        }

        switch (choice[0]) {
        case '1':
            list_loc(&net);
            break;
        case '2':
            printf("starting location: ");
            read_line(start, sizeof(start));

            printf("destination location: ");
            read_line(dest, sizeof(dest));

            shortest_route(&net, start, dest);
            break;
        case '3':
            printf("starting location: ");
            read_line(start, sizeof(start));

            printf("destination location: ");
            read_line(dest, sizeof(dest));

            show_route_distance(&net, start, dest);
            break;
        case '4':
            puts("exiting...");
            return;
        default:
            puts("Invalid choice, \nenter 1, 2, 3, or 4");
            break;
        }
    }
}
