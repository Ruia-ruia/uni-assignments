#include "btreestore.h"
#include "btree.h"

bnode_t*
insert(void *helper, int32_t key, struct info *value)
{   
    uint32_t median, med_idx;
    bnode_t *target, *root, *left, *right;
    tree_t *tree = (tree_t *)helper;   
    root = tree->root;
/*
    Check if this is an empty tree
*/
    if (root == NULL) {
        target = new_tree_node(tree);
        target->keys[target->keys_count++] = new_key_node(key, value);
        tree->root = target;
        return target;
    }
/*
    First, follow the searching algorithm to search for K in the tree. 
    It is an error if K already exists in the tree.
    Identify the leaf node that would contain K.
*/
    target = find_key_node(key, root); 

    keyn_t *ret = target->keys[target->idx];
    if (target->idx < target->keys_count 
        && ret != NULL && ret->key == key) {
        // key was found so we return NULL
        return NULL;
    }
/*
    Insert K and V into the node, maintaining the ordering of keys
*/
    keyn_t *old_key = NULL;
    old_key = target->keys[target->idx];
    
    memmove(&target->keys[target->idx + 1], 
            &target->keys[target->idx], 
            sizeof(*(target->keys)) * (target->keys_count - target->idx));
    target->keys[target->idx] = new_key_node(key, value);
    target->keys_count += 1;
/*
    If the new number of keys in the node does not exceed the limit b - 1, 
    the insertion algorithm is complete
*/
    if (target->keys_count <= (tree->branching - 1)) {
        return target;
    }
/*
    If the number of keys exceeds the limit b - 1, 
    identify the median of the keys including the new key, Kmedian.
    If the total number of keys is even, 
    choose the smaller of the two middle keys as Kmedian. 
*/
    med_idx = floor((target->keys_count) / 2);
    if (tree->branching % 2 == 0) {
        med_idx -= 1;
    }
    median = target->keys[med_idx]->key;
    
    //printf("the median is %d\n", median);
/*
    Split the node into two new sibling nodes of the same parent 
    with the new left node containing all keys < Kmedian and the
    new right node containing all keys > Kmedian.
    Move Kmedian into the parent node’s keys. 
    Note that Kmedian now separates the two new nodes.
*/
    left = new_tree_node(tree);
    right = new_tree_node(tree);
    
    split(tree, target, left, right, median, med_idx);

    return target;
}

void
split(tree_t *tree,
      bnode_t *from,
      bnode_t *left,
      bnode_t *right,
      int32_t median, 
      int32_t med_idx)
{
    bnode_t *new_left, *new_right;
    bool root_on = false;

    one_two(tree, from, left, right, med_idx);

    if (left->parent->keys_count > (tree->branching - 1)) {
        med_idx = floor((left->parent->keys_count) / 2);
        if (tree->branching % 2 == 0) {
            med_idx -= 1;
        }
        median = left->parent->keys[med_idx]->key;
        bnode_t *new_left = new_tree_node(tree);
        bnode_t *new_right = new_tree_node(tree);

        split(tree, left->parent, new_left, new_right, median, med_idx);
    }
}

void
one_two(tree_t *tree, 
        bnode_t *from, 
        bnode_t *left, 
        bnode_t *right, 
        int32_t med_idx)
{
/*
    Split the node into two new sibling nodes of the
    same parent with the new left node containing
    all keys < Kmedian and the new right node
    containing all keys > Kmedian.
*/
    bnode_t *parent = from->parent;
    if (parent == NULL && tree->root == from) {
        // we are at root
        parent = new_tree_node(tree);
        tree->root = parent;
        tree->root->leaf = false;
    }
    left->parent = parent;
    right->parent = parent;
    
/*
    Copy keys and children
*/
    int ix = 0;
    int j = 0;
    for (int i = 0; i < from->keys_count; i++) {
        if (from->keys[i]->key < from->keys[med_idx]->key) {
            bnode_t *one = from->kids[j++];
            bnode_t *two = from->kids[j++];
            if (one != NULL) {
                one->parent = left;
                left->kids[ix++] = one;
            }
            if (two != NULL) {
                two->parent = left;
                left->kids[ix++] = two;
            }
            left->keys[left->keys_count++] = from->keys[i];
        }
    }
    ix = 0;
    j = med_idx + 1;
    for (int i = 0; i < from->keys_count; i++) {
        if (from->keys[i]->key > from->keys[med_idx]->key) {
            bnode_t *one = from->kids[j++];
            bnode_t *two = from->kids[j++];
            if (one != NULL) {
                one->parent = right;
                right->kids[ix++] = one;
            }
            if (two != NULL) {
                two->parent = right;
                right->kids[ix++] = two;
            }
            right->keys[right->keys_count++] = from->keys[i];
        }
    }
    // copy median to parent
    int32_t idx = find_key_array(from->keys[med_idx]->key, parent);
    memmove(&parent->keys[idx + 1],
            &parent->keys[idx],
            sizeof(*(parent->keys)) * (parent->keys_count - idx));

    parent->keys[idx] = from->keys[med_idx];
    parent->keys_count += 1;
    parent->kids[idx] = left;

    memmove(&parent->kids[idx + 1],
            &parent->kids[idx],
            sizeof(*(parent->kids)) * (parent->keys_count - idx));

    parent->kids[idx + 1] = right;

    if (parent->kids[0] != NULL) {
        parent->leaf = false;
    } else {
        if (tree->root != parent) {
            parent->leaf = true;
        }
    }

    if (left->kids[0] != NULL) {
        left->leaf = false;
    } else {
        left->leaf = true;
    }
    if (right->kids[0] != NULL) {
        right->leaf = false;
    } else {
        right->leaf = true;
    }

    // garbage collect
    free(from->kids);
    free(from->keys); 
    free(from);
}