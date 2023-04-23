#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


#define CACHE_SIZE 32
#define CACHE_LINES 8
#define BLOCK_SIZE 4
#define NUM_SETS 2

typedef struct {
    bool valid[NUM_SETS];
    bool dirty[NUM_SETS];
    unsigned long tag[NUM_SETS];
    int lru[NUM_SETS];
} CacheLine;

void cacheInit(CacheLine *cache) {
    for (int i = 0; i < CACHE_SIZE / NUM_SETS; i++) {
        for (int j = 0; j < NUM_SETS; j++) {
            cache[i].valid[j] = false;
            cache[i].dirty[j] = false;
            cache[i].tag[j] = 0;
            cache[i].lru[j] = j;
        }
    }
}

void cacheAccess(unsigned long address, CacheLine *cache, int *numHits, int *numAccesses) {
    int setIndex = (address / BLOCK_SIZE) % NUM_SETS;
    unsigned long tag = address / (BLOCK_SIZE * NUM_SETS);
    int lruIndex = (cache[setIndex].lru[0] < cache[setIndex].lru[1]) ? 0 : 1;
    bool hit = false;

    for (int i = 0; i < NUM_SETS; i++) {
        if (cache[setIndex].valid[i] && cache[setIndex].tag[i] == tag) {
            hit = true;
            cache[setIndex].lru[i] = 1 - lruIndex;
            (*numHits)++;
            break;
        }
    }

    if (!hit) {
        (*numAccesses)++;
        if (cache[setIndex].valid[lruIndex] && cache[setIndex].dirty[lruIndex]) {
            // Write back to memory
            cache[setIndex].dirty[lruIndex] = false;
        }

        // Update cache line with new tag
        cache[setIndex].valid[lruIndex] = true;
        cache[setIndex].dirty[lruIndex] = false;
        cache[setIndex].tag[lruIndex] = tag;
        cache[setIndex].lru[lruIndex] = 1 - lruIndex;
    }
}

typedef struct {
    unsigned int tag;
    int valid;
    int lru;
} cache_line_t;
cache_line_t cache[BLOCK_SIZE][CACHE_SIZE / BLOCK_SIZE];
void init_cache() {
    int i, j;
    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < CACHE_SIZE / BLOCK_SIZE; j++) {
            cache[i][j].valid = 0;
            cache[i][j].lru = j;
        }
    }
}
int search_cache(unsigned int addr) {
    int i, j, hit = 0;
    unsigned int tag = addr / (CACHE_SIZE / BLOCK_SIZE);

    for (i = 0; i < BLOCK_SIZE; i++) {
        for (j = 0; j < CACHE_SIZE / BLOCK_SIZE; j++) {
            if (cache[i][j].valid && cache[i][j].tag == tag) {
                hit = 1;
                cache[i][j].lru = j;
            } else if (!cache[i][j].valid) {
                cache[i][j].valid = 1;
                cache[i][j].tag = tag;
                cache[i][j].lru = j;
            }
        }
    }

    if (hit) {
        return 1;
    } else {
        int min_lru = CACHE_SIZE, min_i = 0, min_j = 0;
        for (i = 0; i < BLOCK_SIZE; i++) {
            for (j = 0; j < CACHE_SIZE / BLOCK_SIZE; j++) {
                if (cache[i][j].valid && cache[i][j].lru < min_lru) {
                    min_lru = cache[i][j].lru;
                    min_i = i;
                    min_j = j;
                }
            }
        }
        cache[min_i][min_j].tag = tag;
        cache[min_i][min_j].lru = CACHE_SIZE / BLOCK_SIZE - 1;
        return 0;
    }
}


typedef struct {
    bool valid;
    int tag;
    int lru_counter;
} cache_line;
int find_cache_line(cache_line *cache3, int tag) {
    for (int i = 0; i < CACHE_LINES; i++) {
        if (cache3[i].valid && cache3[i].tag == tag) {
            return i;
        }
    }
    return -1;
}
int find_lru_cache_line(cache_line *cache3) {
    int lru_line = 0;
    for (int i = 0; i < CACHE_LINES; i++) {
        if (cache3[i].lru_counter < cache3[lru_line].lru_counter) {
            lru_line = i;
        }
    }
    return lru_line;
}
void access_cache(cache_line *cache3, int address, int *hits, int *accesses) {
    (*accesses)++;
    int tag = address / CACHE_SIZE;
    int line_index = find_cache_line(cache3, tag);
    if (line_index == -1) {
        int lru_line = find_lru_cache_line(cache3);
        cache3[lru_line].valid = true;
        cache3[lru_line].tag = tag;
        cache3[lru_line].lru_counter = CACHE_LINES;
    } else {
        (*hits)++;
        cache3[line_index].lru_counter = CACHE_LINES;
        for (int i = 0; i < CACHE_LINES; i++) {
            if (i != line_index && cache3[i].valid) {
                cache3[i].lru_counter--;
            }
        }
    }
}



int main() {
  //Direct-mapped set-asscoiative  
    unsigned int address10;
    int i, cache_index, hit_count = 0, access_count = 0;
    int cache5[CACHE_LINES] = {-1, -1, -1, -1, -1, -1, -1, -1};

    FILE* fp = fopen("traces.txt", "r");
    if (fp == NULL) {
        printf("Failed to open file\n");
        exit(EXIT_FAILURE);
    }
    while (fscanf(fp, "%x", &address10) == 1) {
        cache_index = (address10 / BLOCK_SIZE) % CACHE_LINES;
        access_count++;

        if (cache5[cache_index] == address10) {
            hit_count++;
        } else {
            cache5[cache_index] = address10;
        }
    }
    float hit_rate = (float) hit_count / (float) access_count * 100;
      printf("\nDirect-mapped set-associative cache with LRU replacement policy:\n");
    printf("Number of hits: %d\n", hit_count);
    printf("Number of total accesses: %d\n", access_count);
    printf("Hit rate: %.2f%%\n", hit_rate);
    fclose(fp);
  
    char line[256];
    int total_accesses = 0;
    int cache_hits = 0;
    char *line2 = NULL;
    size_t len = 0;
    ssize_t read;
    cache_line cache3[CACHE_LINES];
    for (int i = 0; i < CACHE_LINES; i++) {
        cache3[i].valid = false;
        cache3[i].tag = 0;
        cache3[i].lru_counter = 0;
    }
    int hits = 0;
    int accesses = 0;
    CacheLine cache[CACHE_SIZE / NUM_SETS];
    cacheInit(cache);
    unsigned long address3;
    int numHits = 0, numAccesses = 0;

//2-way set-asscoiative  
    fp = fopen("traces.txt", "r");
    while (fscanf(fp, "%lx", &address3) != EOF) {
        cacheAccess(address3, cache, &numHits, &numAccesses);
    }
    fclose(fp);
    double hitRate = (double) numHits / numAccesses * 100;
    printf("\n2-way set-associative cache with LRU replacement policy:\n");
    printf("Number of hits: %d\n", numHits);
    printf("Number of total accesses: %d\n", numAccesses);
    printf("Hit rate: %.2f%%\n", hitRate);

//4-way set-asscoiative  
    fp = fopen("traces.txt", "r");
    if (fp == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        unsigned int addr;
        sscanf(line, "%x", &addr);
        total_accesses++;
        cache_hits += search_cache(addr);
    }
    fclose(fp);  
    printf("\n4-way set-associative cache with LRU replacement policy:\n");
    printf("Number of hits: %d\n", cache_hits);
    printf("Number of total accesses: %d\n", total_accesses - cache_hits);
    printf("Hit rate: %.2f%%\n", (float)cache_hits / total_accesses * 100);

  //fully set-asscoiative  
    fp = fopen("traces.txt", "r");
    if (fp == NULL) {
        printf("Error opening file\n");
        return 1;
    }
    while ((read = getline(&line2, &len, fp)) != -1) {
        int address = strtol(line, NULL, 16);
        access_cache(cache3, address, &hits, &accesses);
    }
    printf("\nFully associative cache with LRU replacement policy:\n");
    printf("Number of hits: %d\n", hits);
    printf("Number of total accesses: %d\n", accesses);
    printf("Hit rate: %.2f%%\n", (float)hits/accesses*100);
    fclose(fp);
    if (line2) {
        free(line2);
    }
    return 0;
}
