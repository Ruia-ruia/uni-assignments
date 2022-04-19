#include "btreestore.h"
#include "btree.h"

void *
init_store(uint16_t branching, uint8_t n_processors) 
{
/*
    Set up the data structures used for the tree
*/
    tree_t *tree = malloc(sizeof(tree_t));
    memset(tree, 0, sizeof(tree_t));
    tree->branching = branching;
    pthread_mutex_init(&tree->lock, NULL);
    if (pthread_cond_init(&tree->cv, NULL) != 0) {
        printf("condition variable init failed\n");
    }

    return tree;
}

void
close_keys(bnode_t *root)
{
    for (int i = 0; i < root->keys_count; i++) {
        if (root->keys[i] == NULL) {
            continue;
        }
        free(root->keys[i]->val->data);
        free(root->keys[i]->val);
        free(root->keys[i]);
    }
    free(root->keys);
}

void 
print_2(bnode_t *node, int32_t *idx, stack_t *stacked)
{
    stacked->store[*idx] = node;
    *idx = *idx + 1;
    close_keys(node);
}

void
close_tree(bnode_t *root, int32_t *idx, stack_t *stacked)
{   
    if (root != NULL) {
        print_2(root, idx, stacked);

        for (int i = 0; i < (root->keys_count + 1); i++) {
            close_tree(root->kids[i], idx, stacked);
        }
    }
}

void 
close_store(void * helper) 
{
    tree_t *tree = (tree_t *)helper;

    stack_t *stacked = malloc(sizeof(stack_t));
    int32_t *idx = malloc(sizeof(int32_t));
    *idx = 0;
    
    pthread_mutex_lock(&tree->lock);
    close_tree(tree->root, idx, stacked);
    for (int i = 0; i < (*idx); i++) {
        free(stacked->store[i]->kids);
        free(stacked->store[i]);
    }
    pthread_mutex_unlock(&tree->lock);

    free(stacked);
    free(idx);
    pthread_mutex_destroy(&tree->lock);
    free(tree);

    return;
}

uint64_t 
round_64(size_t count) 
{
    uint64_t rem = count % 8;
    if (rem == 0) {
        return count;
    }
    return count + 8 - rem;
}

int 
btree_delete(uint32_t key, void * helper) 
{
    // Your code here
    tree_t *tree = (tree_t *)helper;

    pthread_mutex_lock(&tree->lock);

/*
    while(tree->locked == 1) {
        // sleep on condition variable:
        pthread_cond_wait(&tree->cv, &tree->lock);
    }
    tree->locked = 1;
*/
    int err = delete(helper, key);
    
    tree->locked = 0;
  //  pthread_cond_signal(&tree->cv);
    pthread_mutex_unlock(&tree->lock);
    
    return err;
}

int btree_insert(uint32_t key, 
                 void * plaintext, 
                 size_t count, 
                 uint32_t encryption_key[4], 
                 uint64_t nonce, 
                 void * helper) 
{
/*
    Insert key into your B tree with the given plaintext data of size count bytes. You must encrypt the contents
    of plaintext using a single instance of the TEA-CTR algorithm with the parameters given in encryption_key
    and nonce and store the encrypted result in your tree. You cannot store the pointer plaintext itself in your
    tree, it may not remain valid. Return 0 if the insertion is successful. Return 1 if the key already exists. For error
    cases, the B tree should remain unchanged and ready for further requests.
*/
    tree_t *tree = (tree_t *)helper;

    pthread_mutex_lock(&tree->lock);
/*
    while(tree->locked == 1) {
        // sleep on condition variable:
        pthread_cond_wait(&tree->cv, &tree->lock);
    }
    tree->locked = 1;
*/
    struct info *value = malloc(sizeof(struct info));
    value->size = count;
    value->nonce = nonce;
    memcpy(&value->key, encryption_key, sizeof(uint32_t) * 4);
    
    uint64_t real_count = round_64(count);
    value->data = malloc(sizeof(uint64_t) * real_count);
    void *plain = malloc(real_count); 
    memcpy(plain, plaintext, count);
    encrypt_tea_ctr(plain, value->key, value->nonce, value->data, real_count / 8); 
    
    if (insert(helper, key, value) != NULL) {
        free(plain);

        tree->locked = 0;
        //pthread_cond_signal(&tree->cv);
        pthread_mutex_unlock(&tree->lock);
        return 0;
    }
    free(value->data);
    free(value);
    free(plain);
    
    tree->locked = 0;
    //pthread_cond_signal(&tree->cv);
    pthread_mutex_unlock(&tree->lock);

    return 1;
}

int btree_retrieve(uint32_t key, struct info * found, void * helper) 
{
    tree_t *tree = (tree_t *)helper;
    bnode_t *target;

        pthread_mutex_lock(&tree->lock);

    target = find_key_node(key, tree->root); 
    keyn_t *ret = target->keys[target->idx];
    if (target->idx < target->keys_count 
        && ret != NULL && ret->key == key) {
        // key was found
        memcpy(found, ret->val, sizeof(struct info));
            pthread_mutex_unlock(&tree->lock);

        return 0;
    }
        pthread_mutex_unlock(&tree->lock);


    return 1;
}

int btree_decrypt(uint32_t key, void * output, void * helper) 
{
    tree_t *tree = (tree_t *)helper;


    struct info *found = malloc(sizeof(struct info));
    int err = btree_retrieve(key, found, helper);
                    pthread_mutex_lock(&tree->lock);

    if (err == 1) {
        free(found);
        pthread_mutex_unlock(&tree->lock);

        return err;
    }
    uint64_t real_count = round_64(found->size);
    void *stage = malloc(real_count);
    decrypt_tea_ctr(found->data, found->key, found->nonce, stage, real_count / 8);
    // truncate
    memcpy(output, stage, found->size);
    free(stage);
    free(found);

    pthread_mutex_unlock(&tree->lock);


    return 0;
}

void 
store_tree(bnode_t *node, int32_t *idx, stack_t *stacked)
{
    stacked->store[*idx] = node;
    *idx = *idx + 1;
}

void
get_tree(bnode_t *root, int32_t *idx, stack_t *stacked)
{   
    if (root != NULL) {
        store_tree(root, idx, stacked);

        for (int i = 0; i < (root->keys_count + 1); i++) {
            get_tree(root->kids[i], idx, stacked);
        }
    }
}

struct node*
get_store(void * helper, int32_t *num) 
{
    tree_t *tree = (tree_t *)helper;

    stack_t *stacked = malloc(sizeof(stack_t));
    int32_t *idx = malloc(sizeof(int32_t));
    *idx = 0;
    
    struct node *nodes = malloc(sizeof(struct node) * NODELIM);

    get_tree(tree->root, idx, stacked);

    for (int i = 0; i < (*idx); i++) {
        nodes[i].keys = malloc(sizeof(uint32_t) * tree->branching);
        nodes[i].num_keys = stacked->store[i]->keys_count;
        for (int j = 0; j < nodes[i].num_keys; j++) {
            // printf("\n--> %d is the key\n", stacked->store[i]->keys[j]->key);
            nodes[i].keys[j] = stacked->store[i]->keys[j]->key;
        }
    }
    *num = *idx;
    free(stacked);
    free(idx);

    return nodes;
}

uint64_t btree_export(void * helper, struct node ** list) {
/*
    Export the B tree as an array of struct node (described below). The array must contain the nodes in your B
    tree, ordered as a preorder traversal. Each node in your tree corresponds to a struct node which contains the
    number of keys it has and an ordered array of the keys. Your function must cause *list to point to the first
    element of your array. Your array must be dynamically allocated; the caller will call free() on your array. The
    keys array of each struct node should also be dynamically allocated; the caller will call free() on each of
    these pointers. The return value must be the number of nodes in your B tree. If the B tree has no nodes, then
    *list can be NULL and free() will not be called.
*/
    tree_t *tree = (tree_t *)helper;
    int32_t num = 0;
    pthread_mutex_lock(&tree->lock);
    *list = get_store(helper, &num);
    pthread_mutex_unlock(&tree->lock);

    return num;
}