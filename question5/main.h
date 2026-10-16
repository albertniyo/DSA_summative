#ifndef MAIN_H
#define MAIN_H

#define alph 256
#define lim 256

typedef struct node {
    unsigned char ch;
    unsigned int freq;
    struct node *left;
    struct node *right;
} huff;

int compress(const char *input_path, const char *output_path);

int decompress(const char *input_path, const char *output_path);

int verify(const char *original_path, const char *restored_path);

void huffman();

#endif
