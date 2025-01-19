#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_TREE_HT 100

// Huffman tree node
struct MinHeapNode {
    char data;
    unsigned freq; // as freq never be -ve keep it unsigned so 1 bit will be reduced
    struct MinHeapNode *left, *right;
};

// MinHeap ( to peek the smallest freq character from freq set)
struct MinHeap {
    unsigned size;
    unsigned capacity;
    struct MinHeapNode** array;
};

// Create a new node
struct MinHeapNode* createNode(char data, unsigned freq) {
    struct MinHeapNode* temp = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
    temp->data = data;
    temp->freq = freq;
    temp->left = temp->right = NULL;
    return temp;
}

// Create a MinHeap
struct MinHeap* createMinHeap(unsigned capacity) {
    struct MinHeap* minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    minHeap->size = 0;
    minHeap->capacity = capacity;
    minHeap->array = (struct MinHeapNode**)malloc(minHeap->capacity * sizeof(struct MinHeapNode*));
    return minHeap;
}

// Swap nodes
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) {
    struct MinHeapNode* t = *a;
    *a = *b;
    *b = t;
}

// MinHeapify
void minHeapify(struct MinHeap* minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left]->freq < minHeap->array[smallest]->freq)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right]->freq < minHeap->array[smallest]->freq)
        smallest = right;

    if (smallest != idx) {
        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);
        minHeapify(minHeap, smallest);
    }
}

// Extract minimum value node
struct MinHeapNode* extractMin(struct MinHeap* minHeap) {
    struct MinHeapNode* temp = minHeap->array[0];
    minHeap->array[0] = minHeap->array[minHeap->size - 1];
    --minHeap->size;
    minHeapify(minHeap, 0);
    return temp;
}

// Insert a node
void insertMinHeap(struct MinHeap* minHeap, struct MinHeapNode* minHeapNode) {
    ++minHeap->size;
    int i = minHeap->size - 1;

    while (i && minHeapNode->freq < minHeap->array[(i - 1) / 2]->freq) {
        minHeap->array[i] = minHeap->array[(i - 1) / 2];
        i = (i - 1) / 2;
    }

    minHeap->array[i] = minHeapNode;
}

// Build MinHeap
void buildMinHeap(struct MinHeap* minHeap) {
    int n = minHeap->size - 1;
    for (int i = (n - 1) / 2; i >= 0; --i)
        minHeapify(minHeap, i);
}

// Check if leaf node
int isLeaf(struct MinHeapNode* root) {
    return !(root->left) && !(root->right);
}

// Create and build MinHeap
struct MinHeap* createAndBuildMinHeap(char data[], int freq[], int size) {
    struct MinHeap* minHeap = createMinHeap(size);

    for (int i = 0; i < size; ++i)
        minHeap->array[i] = createNode(data[i], freq[i]);

    minHeap->size = size;
    buildMinHeap(minHeap);

    return minHeap;
}

// Build Huffman Tree
struct MinHeapNode* buildHuffmanTree(char data[], int freq[], int size) {
    struct MinHeapNode *left, *right, *top;
    struct MinHeap* minHeap = createAndBuildMinHeap(data, freq, size);

    while (minHeap->size > 1) {
        left = extractMin(minHeap);
        right = extractMin(minHeap);

        top = createNode('$', left->freq + right->freq);
        top->left = left;
        top->right = right;

        insertMinHeap(minHeap, top);
    }

    return extractMin(minHeap);
}

// Store Huffman codes in a map
void storeCodes(struct MinHeapNode* root, int arr[], int top, char codes[256][MAX_TREE_HT]) {
    if (root->left) {
        arr[top] = 0;
        storeCodes(root->left, arr, top + 1, codes);
    }

    if (root->right) {
        arr[top] = 1;
        storeCodes(root->right, arr, top + 1, codes);
    }

    if (isLeaf(root)) {
        for (int i = 0; i < top; ++i)
            codes[(unsigned char)root->data][i] = arr[i] + '0';
        codes[(unsigned char)root->data][top] = '\0';
    }
}

// Write compressed binary file
void writeCompressedFile(char* inputFile, char* compressedFile, char codes[256][MAX_TREE_HT]) {
    FILE* input = fopen(inputFile, "r");
    FILE* output = fopen(compressedFile, "wb");

    if (!input || !output) {
        printf("Error opening files!\n");
        return;
    }

    unsigned char buffer = 0;
    int bitCount = 0;
    char ch;

    while ((ch = fgetc(input)) != EOF) {
        char* code = codes[(unsigned char)ch];
        for (int i = 0; code[i] != '\0'; ++i) {
            buffer = (buffer << 1) | (code[i] - '0');
            bitCount++;
            if (bitCount == 8) {
                fwrite(&buffer, 1, 1, output);
                buffer = 0;
                bitCount = 0;
            }
        }
    }

    if (bitCount > 0) {
        buffer = buffer << (8 - bitCount);  // Pad remaining bits with 0s
        fwrite(&buffer, 1, 1, output);
    }

    fclose(input);
    fclose(output);
}

// Decode compressed file
void decodeCompressedFile(char* compressedFile, char* outputFile, struct MinHeapNode* root) {
    FILE* input = fopen(compressedFile, "rb");
    FILE* output = fopen(outputFile, "w");

    if (!input || !output) {
        printf("Error opening files!\n");
        return;
    }

    struct MinHeapNode* currentNode = root;
    unsigned char buffer;
    int bitCount = 8;

    while (fread(&buffer, 1, 1, input)) {
        for (int i = 7; i >= 0; --i) {
            int bit = (buffer >> i) & 1;
            currentNode = bit ? currentNode->right : currentNode->left;

            if (isLeaf(currentNode)) {
                fputc(currentNode->data, output);
                currentNode = root;
            }
        }
    }

    fclose(input);
    fclose(output);
}

// Main function
int main() {
    char inputFile[] = "example.txt";
    char compressedFile[] = "compressed.bin";
    char decodedFile[] = "decoded.txt";

    FILE* file = fopen(inputFile, "r");
    if (!file) {
        printf("File not found!\n");
        return 1;
    }

    int freq[256] = {0};
    char ch;
    while ((ch = fgetc(file)) != EOF)
        freq[(unsigned char)ch]++;

    fclose(file);

    char data[256];
    int frequency[256];
    int size = 0;

    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            data[size] = (char)i;
            frequency[size] = freq[i];
            size++;
        }
    }

    struct MinHeapNode* root = buildHuffmanTree(data, frequency, size);

    char codes[256][MAX_TREE_HT] = {0};
    int arr[MAX_TREE_HT], top = 0;

    storeCodes(root, arr, top, codes);

    writeCompressedFile(inputFile, compressedFile, codes);
    decodeCompressedFile(compressedFile, decodedFile, root);

    printf("Compression and decompression completed.\n");
    return 0;
}
