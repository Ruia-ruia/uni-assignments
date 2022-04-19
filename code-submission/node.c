#include "btreestore.h"
#include "btree.h"


bnode_t*
new_tree_node(tree_t *tree)
{
    bnode_t *new_node;
/*
    Construct a new tree node for the btree.
    Reserve space for its keys and children arrays
    Return it to the caller
*/
    new_node = malloc(sizeof(bnode_t));
    memset(new_node, 0, sizeof(bnode_t));
    new_node->keys = malloc(sizeof(keyn_t) * (tree->branching * 2));
    memset(new_node->keys, 0, sizeof(keyn_t) * (tree->branching * 2));
    new_node->leaf = true;

    new_node->kids = malloc(sizeof(bnode_t *) * (tree->branching * 2));
    memset(new_node->kids, 0, sizeof(bnode_t *) * (tree->branching * 2));

    pthread_mutex_init(&new_node->lock, NULL);

    return new_node;
}

keyn_t*
new_key_node(int32_t key, struct info *value)
{
    keyn_t *new = malloc(sizeof(keyn_t));
    new->key = key;
    new->val = value;
    return new;
}

int32_t 
find_key_array(int32_t key, bnode_t *node)
{
    int32_t below, above, middle;

    if (node->keys_count > 1) {
        for (int i = 0; i < (node->keys_count - 1); i++) {
            if (node->keys[i]->key > node->keys[i + 1]->key) {
                printf("ordered wrong in find_key_array\n");
                int32_t *p = NULL;
                *p = 3;
            }
        }
    }

    above = node->keys_count;

    for (below = -1; (below + 1) < above; ) {
        middle = (below + above) / 2;
        if (node->keys[middle]->key == key) {
            return middle;
        } else if (node->keys[middle]->key < key) {
                below = middle;
        } else {
            above = middle;
        }
    }

    return above;
}

bnode_t*
find_index_child(int32_t index, bnode_t *container)
{   
    return container->kids[index];
}

bnode_t*
find_key_node(int32_t key, bnode_t *container)
{
/*
    Find K or where K would be if it were in the tree
*/
    int32_t index;
    bnode_t *child;

    if (container != NULL) {
        index = find_key_array(key, container);
        if (index < container->keys_count
            && container->keys[index]->key == key) {
                container->idx = index;   // we didn't find K but we need this index
                return container;
        } else {
            if (container->leaf == true) {
                container->idx = index;   // we found k, need to check this in caller
                return container;
            }
            child = find_index_child(index, container);
            if (child == NULL) {
                printf("No child at index %d\n", index);
                return NULL;
            }
            return find_key_node(key, child);
        }
    } else {
        return NULL;
    }
}