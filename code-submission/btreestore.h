#ifndef BTREESTORE_H
#define BTREESTORE_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>

#define RAISE32 4294967296

/*
    General idea:
        - implement btree with a lock for each node
        - write a small module as a monitor for accesses to the node-data


    Btree:

    Branching factor b >= 2, determines num of children and keys:
        1)  n = max number of children internals nodes have before tree-restructure
            ceil(b / 2) <= n <= b
        2)  leaves have 0 children
        3)  root: 
                if root is leaf : n <= b
                if not leaf     : 2 <= n <= b
        4)  every node has n - 1 keys before key-restructure
    
    Ordering of keys/children
        1) keys separate children. 
        keys:           2    4          
        children      1   3    5
        
        inorder:    c0 < k0 < c1 < k1 < c2
        hence, children = n, keys = n - 1 (between)
*/


/*
    The value part of the dictionary
*/
struct info {
    uint32_t size;
    uint32_t key[4];
    uint64_t nonce;
    void * data;
};

/* 
    Format of the elements returned by btree_export()
*/
struct node {
    uint16_t num_keys;
    uint32_t * keys;
};

void * init_store(uint16_t branching, uint8_t n_processors);

void close_store(void * helper);

int btree_insert(uint32_t key, void * plaintext, size_t count, uint32_t encryption_key[4], uint64_t nonce, void * helper);

int btree_retrieve(uint32_t key, struct info * found, void * helper);

int btree_decrypt(uint32_t key, void * output, void * helper);

int btree_delete(uint32_t key, void * helper);

uint64_t btree_export(void * helper, struct node ** list);

void encrypt_tea(uint32_t plain[2], uint32_t cipher[2], uint32_t key[4]);

void decrypt_tea(uint32_t cipher[2], uint32_t plain[2], uint32_t key[4]);

void encrypt_tea_ctr(uint64_t * plain, uint32_t key[4], uint64_t nonce, uint64_t * cipher, uint32_t num_blocks);

void decrypt_tea_ctr(uint64_t * cipher, uint32_t key[4], uint64_t nonce, uint64_t * plain, uint32_t num_blocks);

#endif
