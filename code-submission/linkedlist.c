#include "btreestore.h"
#include "btree.h"

void
remove_node(bnode_t *parent, bnode_t *child)
{
    if (parent != NULL && parent->head == child) {
        parent->head = child->next;
        return;
    } 
    if (child->next != NULL) {
        child->next->prev = child->prev;
    }
    if (child->prev != NULL) {
        child->prev->next = child->next;
    }
}

void
replace_node(bnode_t *parent, bnode_t *child, bnode_t *replace)
{
    replace->prev = child->prev;
    replace->next = child->next;
    replace->parent = child->parent;

    if (parent->head == child) {
        parent->head = replace;
    } 
    if (child->next != NULL) {
        child->next->prev = replace;
    }
    if (child->prev != NULL) {
        child->prev->next = replace;
    }
}

bnode_t*
get_rightmost(bnode_t *root)
{
    if (root->leaf == true) {
        return root;
    }
    bnode_t *cursor = root->head;
    while (cursor->next != NULL) {
        cursor = cursor->next;
    }
    return get_rightmost(cursor);
}

void
find_left_right(bnode_t *target,
                int32_t key_n, 
                int32_t *l_idx, 
                int32_t *r_idx)
{
    int32_t key;

    bnode_t *parent = target->parent;
    if (parent == NULL) {
        printf("parent was null in find_left_right\n");
        *l_idx = -1;
        *r_idx = -1;
        return;
    }
    *l_idx = -1;
    *r_idx = -1;

    // check sorted
    for (int i = 0; i < (parent->keys_count - 1); i++) {
        if (parent->keys[i]->key > parent->keys[i + 1]->key) {
            printf("ordered wrong in find_left_right\n");
            int32_t *p = NULL;
            *p = 3;
        }
    }

    if (*l_idx == -1 && target->prev != NULL) {
        int32_t smallest_left = 
        target->prev->keys[target->prev->keys_count - 1]->key;
        for (int i = 0; i < parent->keys_count; i += 1) {
            if (parent->keys[i]->key > smallest_left) {
                *l_idx = i;
                break;
            }
        }
    }
    if (*r_idx == -1 && target->next != NULL) {
        int32_t smallest_right = target->next->keys[0]->key;
        for (int i = parent->keys_count - 1; i >= 0; i -= 1) {
            if (parent->keys[i]->key < smallest_right) {
                *r_idx = i;
                break;
            }
        }
    }
}

/*
int32_t
count_index_list(bnode_t *target)
{
    bnode_t *parent = target->parent;
    bnode_t *cursor = parent->head;
    int i = 0;
    while (cursor->next != NULL) {
        cursor = cursor->next;
        i += 1;
    }
    return i;
}
*/

/*
void
find_left_right(bnode_t *target,
                int32_t key_n, 
                int32_t *l_idx, 
                int32_t *r_idx)
{
    int32_t key;
    bnode_t *left;
    bnode_t *right;
    bnode_t *parent;

    left = target->prev;
    right = target->next;
    parent = target->parent;

    *r_idx = -1;

    int32_t index = count_index_list(target);
    printf("index of %d is %d\n", key_n, index - 1);

    if ((index - 1) >= target->keys_count) {
        *r_idx = -1;
    }
    *l_idx = index - 1;

    if (*l_idx == -1 && target->prev != NULL) {
        printf("returning l_idx == -1 weirdly\n");
    }
    if (*r_idx == -1 && target->next != NULL) {
        printf("returning r_idx == -1 weirdly\n");
    }
}
*/
/*
void
find_left_right(bnode_t *target,
                int32_t key_n, 
                int32_t *l_idx, 
                int32_t *r_idx)
{
    int32_t key;
    bnode_t *left;
    bnode_t *right;
    bnode_t *parent;

    left = target->prev;
    right = target->next;
    parent = target->parent;

    *l_idx = -1;
    *r_idx = -1;

    if (parent == NULL) {
        printf("parent was null in find left right\n");
        return;
    }

    if (target->keys_count != 0) {
        key = target->keys[target->keys_count - 1]->key;
    } else {
        key = key_n;
    }
// left case
    if (left != NULL) {
        for (int i = 0; i < parent->keys_count; i++) {
            if (parent->keys[i]->key > left->keys[left->keys_count - 1]->key
                && parent->keys[i]->key < key) {
                    *l_idx = i;
                    break;
            }
        }
    }
// right case
    if (right != NULL) {
        for (int i = 0; i < parent->keys_count; i++) {
            if (parent->keys[i]->key > key 
                && parent->keys[i]->key < right->keys[0]->key) {
                    *r_idx = i;
                    break;
            }
        }
    }

    if (*l_idx == -1 && target->prev != NULL) {
        printf("returning l_idx == -1 weirdly\n");
    }
    if (*r_idx == -1 && target->next != NULL) {
        printf("returning r_idx == -1 weirdly\n");
    }
}
*/


void 
add_to_end(bnode_t *head, bnode_t *append)
{
    bnode_t *cursor = head;
    if (cursor != NULL) {
        while (cursor->next != NULL) {
            cursor = cursor->next;
        }
        cursor->next = append;
        append->prev = cursor;
        append->prev->next = append;
        append->parent = head->parent;
        append->next = NULL;
    }
}