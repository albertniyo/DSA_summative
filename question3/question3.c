#include "main.h"

#include <stdio.h>
#include <string.h>

void init(graph *g) {
    int i, j;

    g->count = 0;
    for (i = 0; i < lim_servers; i++) {
        g->names[i][0] = '\0';
        for (j = 0; j < lim_servers; j++) {
            g->matrix[i][j] = 0;
        }
    }
}

int find_server(const graph *g, const char *name) {
    int i;

    for (i = 0; i < g->count; i++) {
        if (strcmp(g->names[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_server(graph *g, const char *name) {
    if (name == NULL || name[0] == '\0') {
        puts("invalid server name");
        return 0;
    }

    if (find_server(g, name) != -1) {
        printf("server %s already exists\n", name);
        return 0;
    }

    if (g->count >= lim_servers) {
        puts("server limit reached");
        return 0;
    }

    strncpy(g->names[g->count], name, lim - 1);
    g->names[g->count][lim - 1] = '\0';
    g->count++;
    printf("server %s added\n", name);
    return 1;
}

int remove_server(graph *g, const char *name) {
    int idx = find_server(g, name);
    int last;
    int i;

    if (idx == -1) {
        printf("server %s not found\n", name);
        return 0;
    }

    last = g->count - 1;
    if (idx != last) {
        for (i = 0; i < g->count; i++) {
            if (i == idx) {
                continue;
            }
            g->matrix[idx][i] = g->matrix[last][i];
            g->matrix[i][idx] = g->matrix[i][last];
        }
        g->matrix[idx][idx] = g->matrix[last][last];
        strcpy(g->names[idx], g->names[last]);
    }

    for (i = 0; i < g->count; i++) {
        g->matrix[last][i] = 0;
        g->matrix[i][last] = 0;
    }
    g->names[last][0] = '\0';
    g->count--;

    printf("server %s removed\n", name);
    return 1;
}

int add_dep(graph *g, const char *from, const char *to) {
    int a = find_server(g, from);
    int b = find_server(g, to);

    if (a == -1 || b == -1) {
        puts("servers must exist to create dependency");
        return 0;
    }

    if (a == b) {
        puts("server cannot depend on itself");
        return 0;
    }

    if (g->matrix[a][b] == 1) {
        puts("dependency already exists");
        return 0;
    }

    g->matrix[a][b] = 1;
    printf("dependency added: \t%s -> %s\n", from, to);
    return 1;
}

int remove_dep(graph *g, const char *from, const char *to) {
    int a = find_server(g, from);
    int b = find_server(g, to);

    if (a == -1 || b == -1) {
        puts("one/both servers do not exist");
        return 0;
    }

    if (g->matrix[a][b] == 0) {
        puts("dependency does not exist");
        return 0;
    }

    g->matrix[a][b] = 0;
    printf("dependency removed: \t%s -> %s\n", from, to);
    return 1;
}

void query_server(const graph *g, const char *name) {
    int idx = find_server(g, name);
    int i;
    int found;

    if (g->count == 0) {
        puts("graph is empty");
        return;
    }

    if (idx == -1) {
        printf("server %s not found\n", name);
        return;
    }

    printf("\nserver %s\n", name);

    puts("\ndirect dependencies");
    found = 0;
    for (i = 0; i < g->count; i++) {
        if (g->matrix[idx][i] == 1) {
            printf("  %s\n", g->names[i]);
            found = 1;
        }
    }
    if (!found) {
        puts("  (none)");
    }

    printf("\nservices depending on %s:\n", name);
    found = 0;
    for (i = 0; i < g->count; i++) {
        if (g->matrix[i][idx] == 1) {
            printf("  %s\n", g->names[i]);
            found = 1;
        }
    }
    if (!found) {
        puts("  (none)");
    }
}

void display(const graph *g) {
    int i, j;

    if (g->count == 0) {
        printf("graph is empty.\n");
        return;
    }

    puts("\nadjacency Matrix\n");
    printf("%-22s", "");
    for (i = 0; i < g->count; i++) {
        printf("[%d] ", i);
    }
    puts("");

    for (i = 0; i < g->count; i++) {
        printf("[%d] %-18s", i, g->names[i]);
        for (j = 0; j < g->count; j++) {
            printf(" %d  ", g->matrix[i][j]);
        }
        puts("");
    }

    puts("\nindex legend:");
    for (i = 0; i < g->count; i++) {
        printf("  [%d] %s\n", i, g->names[i]);
    }
}

static void add_server_bcg(graph *g, const char *name) {
    if (find_server(g, name) != -1 || g->count >= lim_servers || name[0] == '\0') {
        return;
    }
    strncpy(g->names[g->count], name, lim - 1);
    g->names[g->count][lim - 1] = '\0';
    g->count++;
}

static void add_dep_bcg(graph *g, const char *from, const char *to) {
    int a = find_server(g, from);
    int b = find_server(g, to);

    if (a >= 0 && b >= 0 && a != b) {
        g->matrix[a][b] = 1;
    }
}

void build_dep(graph *g) {
    add_server_bcg(g, "web server");
    add_server_bcg(g, "API server");
    add_server_bcg(g, "auth server");
    add_server_bcg(g, "db server");
    add_server_bcg(g, "cache server");
    add_server_bcg(g, "backup server");

    add_dep_bcg(g, "web server", "API server");
    add_dep_bcg(g, "web server", "auth server");
    add_dep_bcg(g, "API server", "db server");
    add_dep_bcg(g, "API server", "cache server");
    add_dep_bcg(g, "auth server", "db server");
    add_dep_bcg(g, "cache server", "db server");
    add_dep_bcg(g, "db server", "backup server");

    printf("%d servers loaded in our newtwork\n", g->count);
}

static void read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    buf[strcspn(buf, "\n")] = '\0';
}

void analyzer() {
    graph g;
    char cmd[16];
    char name[lim];
    char from[lim];
    char to[lim];

    init(&g);
    puts("Server Dependency Network Analyzer");
    build_dep(&g);
    puts("commands: \n- a(add) \n- r(remove) \n- c(create dep) \n- d(delete dep) \n- q(query) \n- m(matrix) \n- x(exit)");

    while (1) {
        printf("\n~# ");
        if (fgets(cmd, sizeof(cmd), stdin) == NULL) {
            break;
        }

        switch (cmd[0]) {
        case 'a':
            printf("server name: ");
            read_line(name, sizeof(name));
            add_server(&g, name);
            break;
        case 'r':
            printf("server name: ");
            read_line(name, sizeof(name));
            remove_server(&g, name);
            break;
        case 'c':
            printf("from server: ");
            read_line(from, sizeof(from));

            printf("to server: ");
            read_line(to, sizeof(to));
            add_dep(&g, from, to);
            break;
        case 'd':
            printf("from server: ");
            read_line(from, sizeof(from));

            printf("to server: ");
            read_line(to, sizeof(to));
            remove_dep(&g, from, to);
            break;
        case 'q':
            printf("server name: ");
            read_line(name, sizeof(name));
            query_server(&g, name);
            break;
        case 'm':
            display(&g);
            break;
        case 'x':
            puts("exiting...");
            return;
        default:
            puts("invalid command,\nuse a, r, c, d, q, m, or x");
            break;
        }
    }
}
