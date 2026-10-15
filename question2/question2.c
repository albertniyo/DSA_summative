#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


void init(cmdreg *reg) {
    reg->root = NULL;
    reg->count = 0;
}

static bst *create_node(const char *command) {
    bst *node = (bst *)malloc(sizeof(bst));

    if (node == NULL) return NULL;

    node->command = (char *)malloc(strlen(command) + 1);

    if (node->command == NULL) {
        free(node);
        return NULL;
    }

    strcpy(node->command, command);

    node->left = NULL;
    node->right = NULL;

    return node;
}

static bst *insert_node(bst *root, const char *command, int *inserted) {
    int tmp;

    if (root == NULL) {
        bst *node = create_node(command);
        if (node != NULL) {
            *inserted = 1;
        }
        return node;
    }

    tmp = strcmp(command, root->command);
    if (tmp < 0) {
        root->left = insert_node(root->left, command, inserted);
    } else if (tmp > 0) {
        root->right = insert_node(root->right, command, inserted);
    }

    return root;
}

int insert_cmd(cmdreg *reg, const char *command) {
    int inserted = 0;

    if (reg->count >= lim_cmd) {
        printf("limit (%d) reached \nSkipping: %s\n",
               lim_cmd, command);

        return 0;
    }

    reg->root = insert_node(reg->root, command, &inserted);
    if (inserted) {
        reg->count++;
    }
    return inserted;
}

int load_cmd(cmdreg *reg, const char *filename) {
    FILE *fp;
    char line[lim];
    int curr = 0;

    fp = fopen(filename, "r");

    if (fp == NULL) {
        puts("error opening file");

        return 0;
    }

    while (fgets(line, sizeof(line), fp) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';

        if (line[0] == '\0') {
            continue;
        }
        if (insert_cmd(reg, line)) {
            curr++;
        }
    }

    fclose(fp);
    printf("%d command(s) approved\n", curr);

    return curr;
}

bst *search_cmd(bst *root, const char *command) {
    int tmp;

    if (root == NULL) return NULL;

    tmp = strcmp(command, root->command);

    if (tmp == 0) {
        return root;
    }
    if (tmp < 0) {
        return search_cmd(root->left, command);
    }

    return search_cmd(root->right, command);
}

void print_cmd(bst *root) {
    if (root == NULL) return;

    print_cmd(root->left);

    printf("%s\n", root->command);
    print_cmd(root->right);
}

static int min(int a, int b, int c) {
    if (a <= b && a <= c) {
        return a;
    }
    if (b <= a && b <= c) {
        return b;
    }

    return c;
}

static int edit_distance(const char *a, const char *b) {
    int len_a = (int)strlen(a);
    int len_b = (int)strlen(b);
    int i, j;
    int *prev;
    int *curr;
    int result;

    prev = (int *)malloc((size_t)(len_b + 1) * sizeof(int));
    curr = (int *)malloc((size_t)(len_b + 1) * sizeof(int));

    if (prev == NULL || curr == NULL) {
        free(prev);
        free(curr);

        return 999;
    }

    for (j = 0; j <= len_b; j++) {
        prev[j] = j;
    }

    for (i = 1; i <= len_a; i++) {
        curr[0] = i;
        for (j = 1; j <= len_b; j++) {
            if (a[i - 1] == b[j - 1]) {
                curr[j] = prev[j - 1];
            } else {
                curr[j] = 1 + min(prev[j], curr[j - 1], prev[j - 1]);
            }
        }
        for (j = 0; j <= len_b; j++) {
            prev[j] = curr[j];
        }
    }

    result = prev[len_b];
    free(prev);
    free(curr);

    return result;
}

void f_suggestion(bst *root, const char *input, char *best, int *best_dist) {
    int dist;

    if (root == NULL) {
        return;
    }

    f_suggestion(root->left, input, best, best_dist);

    dist = edit_distance(input, root->command);
    if (dist < *best_dist) {
        *best_dist = dist;
        strcpy(best, root->command);
    }

    f_suggestion(root->right, input, best, best_dist);
}

static void free_tree(bst *root) {
    if (root == NULL) return;

    free_tree(root->left);
    free_tree(root->right);
    free(root->command);
    free(root);
}

void clear_reg(cmdreg *reg) {
    free_tree(reg->root);
    reg->root = NULL;
    reg->count = 0;
    printf("commands registry cleared\n");
}

static void free_reg(cmdreg *reg) {
    free_tree(reg->root);
    
    reg->root = NULL;
    reg->count = 0;
}

static void write_cmd(FILE *fp, bst *root) {
    if (root == NULL) {
        return;
    }
    write_cmd(fp, root->left);
    fprintf(fp, "%s\n", root->command);
    write_cmd(fp, root->right);
}

static void save_session(const cmdreg *reg) {
    FILE *fp = fopen("session_cmds.txt", "w");

    if (fp == NULL) {
        puts("error in saving the commands");
        return;
    }

    fprintf(fp, "%d command(s) saved in command registry\n", reg->count);
    write_cmd(fp, reg->root);

    fclose(fp);
    printf("Session saved to session_cmds.txt\n");
}

void log_unknown(const char *command) {
    FILE *fp;
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestr[32];

    if (tm_info == NULL) {
        strcpy(timestr, "unknown-time");
    } else {
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm_info);
    }

    fp = fopen("audit.log", "a");
    if (fp == NULL) {
        fprintf(stderr, "error writing to audit.log\n");
        return;
    }

    fprintf(fp, "%s | UNKNOWN | %s\n", timestr, command);

    fclose(fp);
}

static void read_line(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }

    buf[strcspn(buf, "\n")] = '\0';
}

static void verify_command(cmdreg *reg) {
    char input[lim];
    char suggestion[lim];
    int best_dist = 999;

    printf("enter a command: ");
    read_line(input, sizeof(input));

    if (input[0] == '\0') {
        puts("empty input");
        return;
    }

    if (search_cmd(reg->root, input) != NULL) {
        puts("command approved");
        return;
    }

    puts("unknow command");
    log_unknown(input);

    suggestion[0] = '\0';
    f_suggestion(reg->root, input, suggestion, &best_dist);
    if (best_dist > 0 && best_dist <= try && suggestion[0] != '\0') {
        printf("Did you mean: %s?\n", suggestion);
    }
}

void run_cmd() {
    cmdreg reg;
    char command[16];

    init(&reg);
    printf("Network Device Command Registry\n");
    load_cmd(&reg, "commands.txt");
    puts("Commands: \n- v(verify) \n- p(print) \n- s(size) \n- x(clear) \n- q(quit)\n");

    while (1) {
        printf("\n~# ");
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }

        switch (command[0]) {
        case 'v':
            verify_command(&reg);
            break;
        case 'p':
            if (reg.root == NULL) {
                puts("registry is empty");
            } else {
                puts("approved commands on alphabetical order:");
                print_cmd(reg.root);
            }
            break;
        case 's':
            printf("stored commands: %d\n", reg.count);
            break;
        case 'x':
            clear_reg(&reg);
            break;
        case 'q':
            puts("saving session and quitting...");
            save_session(&reg);
            free_reg(&reg);

            return;
        default:
            puts("invalid option try menu option, \ntryv, p, s, x, or q");
            break;
        }
    }

    free_reg(&reg);
}
