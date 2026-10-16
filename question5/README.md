# Question 5 — Embedded Firmware Log Compression Utility

As company which deploys bunch of embedded monitoring devices in remote locations, transmiting diagnostic logs can be resource-intense so our task is to compresses before transimission and restores diagnostic logs via `Huffman coding` (lossless compression).

## Build and run

```bash
gcc main.c question5.c -o huffman
./huffman
```

we will be using `device_log.txt` in the same directory which contains sample (randomly generated).

Our Menu
```
1. Compress log
2. Decompress log  
3. Verify restored log  
4. Exit  
```

## Design

- Counts character frequencies and builds a Huffman tree
- Encodes the file with generated binary codes
- Stores frequency table and bitstream in the `.huff` file so the tree can be rebuilt
- Verification compares original and restored bytes exactly
