#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static huff *create_node(char ch, int freq,
                             huff *left, huff *right) {
    huff *node = (huff *)malloc(sizeof(huff));
    if (node == NULL) {
        return NULL;
    }
    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;

    return node;
}

static void free_tree(huff *root) {
    if (root == NULL) {
        return;
    }
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

static int is_leaf(const huff *node) {
    return node != NULL && node->left == NULL && node->right == NULL;
}

static int find_min_index(huff **nodes, int count) {
    int i;
    int min_i = -1;

    for (i = 0; i < count; i++) {
        if (nodes[i] == NULL) {
            continue;
        }
        if (min_i == -1 || nodes[i]->freq < nodes[min_i]->freq) {
            min_i = i;
        }
    }

    return min_i;
}

static huff *build_tree(unsigned int freq[alph]) {
    huff *nodes[alph];
    int count = 0;
    int i;
    huff *left;
    huff *right;
    huff *parent;
    int a, b;

    for (i = 0; i < alph; i++) {
        if (freq[i] > 0) {
            nodes[count] = create_node((unsigned char)i, freq[i], NULL, NULL);
            if (nodes[count] == NULL) {
                return NULL;
            }
            count++;
        }
    }

    if (count == 0) {
        return NULL;
    }

    if (count == 1) {
        parent = create_node(0, nodes[0]->freq, nodes[0], NULL);
        return parent;
    }

    while (1) {
        a = find_min_index(nodes, count);
        left = nodes[a];
        nodes[a] = NULL;

        b = find_min_index(nodes, count);
        if (b == -1) {
            return left;
        }
        right = nodes[b];
        nodes[b] = NULL;

        parent = create_node(0, left->freq + right->freq, left, right);
        if (parent == NULL) {
            free_tree(left);
            free_tree(right);
            return NULL;
        }

        nodes[a] = parent;
    }
}

static void build_codes(huff *root, char *code, int depth,
                        char codes[alph][lim]) {
    if (root == NULL) {
        return;
    }

    if (is_leaf(root)) {
        if (depth == 0) {
            strcpy(codes[root->ch], "0");
        } else {
            code[depth] = '\0';
            strcpy(codes[root->ch], code);
        }
        return;
    }

    if (root->left != NULL) {
        code[depth] = '0';
        build_codes(root->left, code, depth + 1, codes);
    }
    if (root->right != NULL) {
        code[depth] = '1';
        build_codes(root->right, code, depth + 1, codes);
    }
}

static void print_freqs(unsigned int freq[alph]) {
    int i;

    puts("\ncharacter frequencies");
    printf("%-12s %s\n", "Character", "Frequency");
    for (i = 0; i < alph; i++) {
        if (freq[i] == 0) {
            continue;
        }
        if (i == '\n') {
            printf("%-12s %u\n", "'\\n'", freq[i]);
        } else if (i == '\t') {
            printf("%-12s %u\n", "'\\t'", freq[i]);
        } else if (i == ' ') {
            printf("%-12s %u\n", "' '", freq[i]);
        } else if (i >= 32 && i < 127) {
            printf("'%c'%-9s %u\n", (char)i, "", freq[i]);
        } else {
            printf("0x%02X%-8s %u\n", i, "", freq[i]);
        }
    }
}

static unsigned char *read_file(const char *path, size_t *out_size) {
    FILE *fp;
    unsigned char *data;
    long size;

    fp = fopen(path, "rb");
    if (fp == NULL) {
        printf("%s could not open\n", path);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        printf("%s could not seek\n", path);
        return NULL;
    }

    size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        printf("could not determine size of %s\n", path);
        return NULL;
    }
    rewind(fp);

    data = (unsigned char *)malloc((size_t)size + 1);
    if (data == NULL) {
        fclose(fp);
        puts("memory allocation failed");
        return NULL;
    }

    if (size > 0 && fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        printf("failed to read %s\n", path);
        return NULL;
    }

    data[size] = '\0';
    fclose(fp);
    *out_size = (size_t)size;

    return data;
}

int compress_log(const char *input_path, const char *output_path) {
    unsigned char *data;
    size_t size;
    unsigned int freq[alph] = {0};
    char codes[alph][lim];
    char scratch[lim];
    huff *root;
    FILE *out;
    size_t i;
    size_t bit_count = 0;
    size_t byte_index = 0;
    unsigned char current = 0;
    int bit_pos = 0;
    unsigned char *bitstream;
    size_t max_bits;
    size_t compressed_bytes;
    double ratio;

    data = read_file(input_path, &size);
    if (data == NULL) {
        return 0;
    }

    if (size == 0) {
        puts("log file is empty");
        free(data);
        return 0;
    }

    for (i = 0; i < size; i++) {
        freq[data[i]]++;
    }

    print_freqs(freq);

    root = build_tree(freq);
    if (root == NULL) {
        puts("failed to build Huffman tree");
        free(data);
        return 0;
    }

    memset(codes, 0, sizeof(codes));
    build_codes(root, scratch, 0, codes);

    puts("\nHuffman codes:");
    for (i = 0; i < alph; i++) {
        if (freq[i] == 0) {
            continue;
        }
        if (i == '\n') {
            printf("'\\n' -> %s\n", codes[i]);
        } else if (i >= 32 && i < 127) {
            printf("'%c' -> %s\n", (char)i, codes[i]);
        } else {
            printf("0x%02X -> %s\n", (unsigned int)i, codes[i]);
        }
    }

    max_bits = size * lim;
    bitstream = (unsigned char *)calloc((max_bits / 8) + 1, 1);
    if (bitstream == NULL) {
        puts("memory allocation failed");
        free_tree(root);
        free(data);
        return 0;
    }

    for (i = 0; i < size; i++) {
        const char *code = codes[data[i]];
        size_t k;
        for (k = 0; code[k] != '\0'; k++) {
            if (code[k] == '1') {
                current |= (unsigned char)(1 << (7 - bit_pos));
            }
            bit_pos++;
            bit_count++;
            if (bit_pos == 8) {
                bitstream[byte_index++] = current;
                current = 0;
                bit_pos = 0;
            }
        }
    }
    if (bit_pos > 0) {
        bitstream[byte_index++] = current;
    }

    out = fopen(output_path, "wb");
    if (out == NULL) {
        printf("could not create %s\n", output_path);
        free(bitstream);
        free_tree(root);
        free(data);
        return 0;
    }

    {
        unsigned int unique = 0;
        unsigned char ch;

        for (i = 0; i < alph; i++) {
            if (freq[i] > 0) {
                unique++;
            }
        }

        {
            unsigned int original_size_u = (unsigned int)size;
            unsigned int bit_count_u = (unsigned int)bit_count;

            fwrite("HUFF", 1, 4, out);
            fwrite(&original_size_u, sizeof(unsigned int), 1, out);
            fwrite(&unique, sizeof(unsigned int), 1, out);
            for (i = 0; i < alph; i++) {
                if (freq[i] > 0) {
                    ch = (unsigned char)i;
                    fwrite(&ch, 1, 1, out);
                    fwrite(&freq[i], sizeof(unsigned int), 1, out);
                }
            }
            fwrite(&bit_count_u, sizeof(unsigned int), 1, out);
            fwrite(bitstream, 1, byte_index, out);
            fclose(out);

            compressed_bytes = 4 + sizeof(unsigned int) + sizeof(unsigned int) +
                               (unique * (1 + sizeof(unsigned int))) +
                               sizeof(unsigned int) + byte_index;
        }
    }
    ratio = (1.0 - ((double)compressed_bytes / (double)size)) * 100.0;

    printf("\noriginal size:     %zu bytes\n", size);
    printf("compressed size:   %zu bytes\n", compressed_bytes);
    printf("compression ratio: %.2f%%\n", ratio);
    printf("saved compressed file: %s\n", output_path);

    free(bitstream);
    free_tree(root);
    free(data);

    return 1;
}

int decompress_log(const char *input_path, const char *output_path) {
    FILE *in;
    FILE *out;
    char magic[4];
    unsigned int original_size;
    unsigned int freq[alph];
    unsigned int unique;
    unsigned int u;
    unsigned int bit_count;
    unsigned char *bitstream;
    size_t byte_count;
    size_t file_pos;
    long file_end;
    huff *root;
    huff *node;
    unsigned int bits_read = 0;
    unsigned int written = 0;
    size_t i;
    int bit;

    memset(freq, 0, sizeof(freq));

    in = fopen(input_path, "rb");
    if (in == NULL) {
        printf("could not open %s\n", input_path);
        return 0;
    }

    if (fread(magic, 1, 4, in) != 4 || memcmp(magic, "HUFF", 4) != 0) {
        puts("invalid compressed file format");
        fclose(in);
        return 0;
    }

    if (fread(&original_size, sizeof(unsigned int), 1, in) != 1 ||
        fread(&unique, sizeof(unsigned int), 1, in) != 1) {
        printf("corrupted header in %s\n", input_path);
        fclose(in);
        return 0;
    }

    if (unique > alph) {
        puts("invalid frequency table");
        fclose(in);
        return 0;
    }

    for (u = 0; u < unique; u++) {
        unsigned char ch;
        unsigned int f;

        if (fread(&ch, 1, 1, in) != 1 ||
            fread(&f, sizeof(unsigned int), 1, in) != 1) {
            puts("corrupted frequency table");
            fclose(in);
            return 0;
        }
        freq[ch] = f;
    }

    if (fread(&bit_count, sizeof(unsigned int), 1, in) != 1) {
        puts("corrupted bitstream header");
        fclose(in);
        return 0;
    }

    file_pos = (size_t)ftell(in);
    fseek(in, 0, SEEK_END);

    file_end = ftell(in);
    fseek(in, (long)file_pos, SEEK_SET);
    byte_count = (size_t)file_end - file_pos;

    bitstream = (unsigned char *)malloc(byte_count ? byte_count : 1);
    if (bitstream == NULL) {
        puts("memory allocation failed");
        fclose(in);
        return 0;
    }

    if (byte_count > 0 && fread(bitstream, 1, byte_count, in) != byte_count) {
        puts("failed reading compressed data");
        free(bitstream);
        fclose(in);
        return 0;
    }
    fclose(in);

    root = build_tree(freq);
    if (root == NULL) {
        puts("failed to rebuild Huffman tree");
        free(bitstream);
        return 0;
    }

    out = fopen(output_path, "wb");
    if (out == NULL) {
        printf("could not create %s\n", output_path);
        free_tree(root);
        free(bitstream);
        return 0;
    }

    node = root;
    for (i = 0; i < byte_count && written < original_size; i++) {
        for (bit = 7; bit >= 0 && bits_read < bit_count && written < original_size; bit--) {
            int bit_val = (bitstream[i] >> bit) & 1;

            if (root->right == NULL) {
                fputc(root->left->ch, out);
                written++;
                bits_read++;
                continue;
            }

            node = bit_val ? node->right : node->left;
            if (node == NULL) {
                puts("invalid bitstream");
                fclose(out);
                free_tree(root);
                free(bitstream);
                return 0;
            }

            if (is_leaf(node)) {
                fputc(node->ch, out);
                written++;
                node = root;
            }
            bits_read++;
        }
    }

    fclose(out);
    free_tree(root);
    free(bitstream);

    if (written != original_size) {
        printf("decompressed size mismatch (%u vs %u).\n",
               written, original_size);
        return 0;
    }

    printf("Decompressed to %s (%u bytes).\n", output_path, written);
    return 1;
}

int verify_logs(const char *original_path, const char *restored_path) {
    unsigned char *a;
    unsigned char *b;
    size_t size_a;
    size_t size_b;

    a = read_file(original_path, &size_a);
    if (a == NULL) {
        return 0;
    }
    b = read_file(restored_path, &size_b);
    if (b == NULL) {
        free(a);
        return 0;
    }

    if (size_a == size_b && memcmp(a, b, size_a) == 0) {
        puts("Verification: SUCCESS");
        puts("restored file and original are identical");
        free(a);
        free(b);

        return 1;
    }

    puts("Verification: FAILED");
    puts("restored file and original differs");
    free(a);
    free(b);

    return 0;
}

void huffman() {
    char choice[16];

    puts("Embedded Firmware Log Compression Utility");
    puts("1. Compress log");
    puts("2. Decompress log");
    puts("3. Verify restored log");
    puts("4. Exit");

    while (1) {
        printf("\nChoice: ");
        if (fgets(choice, sizeof(choice), stdin) == NULL) {
            break;
        }

        switch (choice[0]) {
        case '1':
            compress_log("device_log.txt", "device_log.huff");
            break;
        case '2':
            if (decompress_log("device_log.huff", "device_log_restored.txt")) {
                puts("\nautomatic verification...");
                verify_logs("device_log.txt", "device_log_restored.txt");
            }
            break;
        case '3':
            verify_logs("device_log.txt", "device_log_restored.txt");
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
