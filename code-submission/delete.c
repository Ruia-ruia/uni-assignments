#include "btreestore.h"
#include "btree.h"

int32_t
delete(void *helper, int32_t key)
{
    bnode_t *target, *leaf_target, *root, 
            *parent, *left, *right;
    keyn_t *found_key;
    tree_t *tree = (tree_t *)helper;   
    root = tree->root;

    if (root == NULL) {
        // tree is empty
        return 1;
    }
/*
    First, follow the searching algorithm to search for K in the tree. 
    It is an error if K does not exist in the tree
*/
    target = find_key_node(key, root); 

    if (target == NULL) {
        printf("key wasn't found %d\n", key);

        return 1;
    }
    found_key = target->keys[target->idx];
    if (found_key == NULL || found_key->key != key) {
        return 1;

    }
    // check 
    //traversal(tree->root);


// Leaf and Internal cases for swap-and-null operation
    if (target->leaf == false) {
        leaf_target = del_internal(target, found_key);
    } else {
        del_leaf(target);
        leaf_target = target;
    }
/*
    If the leaf node still satisfies the minimum number
    of keys, the deletion algorithm is complete.
*/
    uint32_t min = ceil((tree->branching - 1) / 2);
    assert(min > 0);
    if (leaf_target->keys_count >= min) {
        free(found_key->val->data);
        free(found_key->val);
        free(found_key);
        
        return 0;
    }  
    below_min_keys(tree, leaf_target, min, key);


    free(found_key->val->data);
    free(found_key->val);
    free(found_key);

    return 0;
}

void
reached_root(tree_t *tree)
{
/*
If the root node is reached and has less than the minimum number of keys (recall the minimum number
of keys for the root specifically is 1), simply delete the root node. The new root is the merged child node
produced from step 6. The height of the entire tree therefore decreases by 1.
*/
    bnode_t *old_root = tree->root;
    bnode_t *new_root = tree->root->head;

    if (new_root == NULL) {
        // we got down to just one node
        return;
    }
    free(old_root->keys);
    free(old_root);

    tree->root = new_root;
    new_root->parent = NULL;
}

void 
del_leaf(bnode_t *target)
{
    // remove the found_key in target->keys if target was a leaf
    assert(target->idx >= 0 && target->idx < target->keys_count);

    if (target->keys_count > 0) {
        memmove(&target->keys[target->idx], 
                &target->keys[target->idx + 1], 
                sizeof(*(target->keys)) * (target->keys_count - target->idx));
    } else {
        target->keys[target->idx] = NULL;
    }
    target->keys_count -= 1;
}

bnode_t*
del_internal(bnode_t *target, keyn_t *found_key)
{
/*
    If the node containing K is an internal node, 
    suppose that C is the left of the 2 child nodes that K separates.
    Swap K with the largest key (Knew) in the entire subtree rooted at C. 
    Note that Knew must reside in a leaf node, so after the swap K now resides in this leaf node. 
    Note that now Knew also correctly separates the same 2 child nodes as K did
*/
    bnode_t *left, *right;

    left = find_index_child(target->idx, target);
    
    assert(target->idx < target->keys_count);
    assert(target->idx >= 0);

    right = get_rightmost(left);// recurse to right-most leaf from left
    // swap K and the right-most key
    target->keys[target->idx] = right->keys[right->keys_count - 1];
/*
If K was in an internal node, note that K is now in a leaf node 
and this is identical to the case where K was originally in a leaf node. 
Delete K from the leaf node.
*/
    right->keys[right->keys_count - 1] = NULL;
    right->keys_count -= 1;

    return right;
}


void
suitable_sibling(bnode_t *target, bnode_t *sibling, uint32_t idx, bool left)
{
    keyn_t *K_sib = target->parent->keys[idx]; 
    keyn_t *key_target; 
    
    // if left then largest
    if (left) {
        key_target = sibling->keys[sibling->keys_count - 1];
        sibling->keys[sibling->keys_count - 1] = NULL;
    } else {
    // if right then smallest
        key_target = sibling->keys[0];
        memmove(&sibling->keys[0], 
                &sibling->keys[1], 
                sizeof(*(sibling->keys)) * (sibling->keys_count));
    }

    target->parent->keys[idx] = key_target; 
    
    int32_t key_idx = find_key_array(K_sib->key, target); 
    memmove(&target->keys[key_idx + 1], 
            &target->keys[key_idx], 
            sizeof(*(target->keys)) * (target->keys_count - key_idx));
    target->keys[key_idx] = K_sib; 
    target->keys_count += 1;
    sibling->keys_count -= 1;

    if (target->head != NULL) {
        node_internal(target, left);
    }

}

bnode_t*
merge(tree_t *tree, bnode_t *target, bnode_t *sibling)
{
    bnode_t *merged = new_tree_node(tree);
    bnode_t *parent = target->parent;
    bnode_t *left = target->prev;
    bnode_t *right = target->next;

    if (sibling == left) {
        // copy keys across
        memmove(merged->keys, 
                sibling->keys, 
                sizeof(keyn_t *) * sibling->keys_count);
        merged->keys_count += sibling->keys_count;
        // then move the target's keys into the new node
        memmove(&merged->keys[sibling->keys_count], 
                target->keys, 
                sizeof(keyn_t *) * target->keys_count);
        merged->keys_count += target->keys_count;

        // relink sibling and parent
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);      

        // relink children
        merged->head = sibling->head;
        bnode_t *cursor = sibling->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = target->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }
    } else if (right == sibling) {
        // copy keys 
        memmove(merged->keys, 
                target->keys, 
                sizeof(keyn_t *) * (target->keys_count));
        merged->keys_count += target->keys_count;
        memmove(&merged->keys[target->keys_count], 
                sibling->keys, 
                sizeof(keyn_t *) * sibling->keys_count);
        merged->keys_count += sibling->keys_count;

        // relink sibling and parent    
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);

        // relink children
        merged->head = target->head;
        bnode_t *cursor = target->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = sibling->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }
    }

    // reset parent 
    if (merged->parent != NULL) {
        merged->parent->head->prev = NULL;
        if (sibling->prev == NULL) {
            merged->parent->head = merged;
        }
    }

    // reset parent of children
    if (merged->head != NULL) {
        merged->head->next->prev = merged->head;
        bnode_t *cur = merged->head;
        while (cur != NULL) {
            cur->parent = merged;
            cur = cur->next;
        }
    }

    // make sure it's linked
    if (target->next != NULL) {
        target->next->prev = merged;
    }
    if (target->prev != NULL) {
        target->prev->next = merged;
    }

    // make sure we get the right peer?
    if (sibling == target->prev) {
        merged->next = target->next;
    } else {
        merged->prev = target->prev;
    }

    // reset leaves
    if (merged->head == NULL) {
        merged->leaf = true;
    } else {
        merged->leaf = false;
    }

    free(target->keys);
    free(sibling->keys);
    sibling->prev = NULL;

    free(target);
    free(sibling);

    return merged;
}

void
splice_siblings(bnode_t *parent, 
                bnode_t *merged, 
                bnode_t *target, 
                bnode_t *sibling)
{
    bnode_t *cursor = parent->head;
    assert(cursor != NULL);
// standard relink
    bnode_t *node;

    if (target->next == sibling) {
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);
    } else {
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);
    }

    if (target->next != NULL) {
        target->next->prev = merged;
    }
    if (target->prev != NULL) {
        target->prev->next = merged;
    }
    if (sibling == target->prev) {
        merged->next = target->next;
    } else {
        merged->prev = target->prev;
    }
}

void
splice_children(bnode_t *parent, 
                bnode_t *merged, 
                bnode_t *target, 
                bnode_t *sibling)
{
    bnode_t *cursor;

    if (merged->head != NULL) {
        //merged->head->next->prev = merged->head;
        bnode_t *cur = merged->head;
        while (cur != NULL) {
            cur->parent = merged;
            cur = cur->next;
        }
    }

    if (sibling == target->prev) {
// left case
        merged->head = sibling->head;
        bnode_t *cursor = sibling->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = target->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }

    } else if (sibling == target->next) {
// right case
        merged->head = target->head;
        bnode_t *cursor = target->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = sibling->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }

    } else {
        printf("Error in splice children\n");
    }
}

void
update_parent(bnode_t *parent, 
              bnode_t *merged, 
              bnode_t *target, 
              bnode_t *sibling)
{
/*
    The case where we merged with the head of a list
*/
    if (target->parent->head == target 
        || target->parent->head == sibling) {
        target->parent->head = merged;
        merged->parent = target->parent;
        merged->prev = NULL;
    }
    if (merged->parent != NULL) {
        merged->parent->head->prev = NULL;
        if (sibling->prev == NULL) {
            merged->parent->head = merged;
        }
    }
    if (merged->head != NULL) {
        //merged->head->next->prev = merged->head;
        bnode_t *cur = merged->head;
        while (cur != NULL) {
            cur->parent = merged;
            cur = cur->next;
        }
    }
}
/*
bnode_t*
merge(tree_t *tree, bnode_t *target, bnode_t *sibling)
{
    bnode_t *merged;

    if (sibling == target->prev) {
        // left sibling case
        merged = new_tree_node(tree);
        // move the left sibling's keys into the new node
        memmove(merged->keys, 
                sibling->keys, 
                sizeof(keyn_t *) * sibling->keys_count);
        merged->keys_count += sibling->keys_count;
        // then move the target's keys into the new node
        memmove(&merged->keys[sibling->keys_count], 
                target->keys, 
                sizeof(keyn_t *) * target->keys_count);
        merged->keys_count += target->keys_count;
        // relink as appropriate
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);

        merged->head = sibling->head;
        bnode_t *cursor = sibling->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = target->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }
//
    } else if (target->next == sibling) {
        // right sibling case
        merged = new_tree_node(tree);
        memmove(merged->keys, 
                target->keys, 
                sizeof(keyn_t *) * (target->keys_count));
        merged->keys_count += target->keys_count;
        memmove(&merged->keys[target->keys_count], 
                sibling->keys, 
                sizeof(keyn_t *) * sibling->keys_count);
        merged->keys_count += sibling->keys_count;
        // relink
        merged->next = target->next;
        merged->parent = target->parent;
        remove_node(target->parent, target);
        replace_node(target->parent, sibling, merged);

        merged->head = target->head;
        bnode_t *cursor = target->head;
        if (cursor != NULL) {
            while (cursor->next != NULL) {
                cursor = cursor->next;
            }
            cursor->next = sibling->head;
            if (cursor->next != NULL) {
                cursor->next->prev = cursor;
            }
            if (cursor->prev != NULL)  {
                cursor->prev->next = cursor;
            }
        }
    }
    if (merged->parent != NULL) {
        merged->parent->head->prev = NULL;
        if (sibling->prev == NULL) {
            merged->parent->head = merged;
        }
    }
    if (merged->head != NULL) {
        //merged->head->next->prev = merged->head;
        bnode_t *cur = merged->head;
        while (cur != NULL) {
            cur->parent = merged;
            cur = cur->next;
        }
    }
    if (target->next != NULL) {
        target->next->prev = merged;
    }
    if (target->prev != NULL) {
        target->prev->next = merged;
    }

    if (sibling == target->prev) {
        merged->next = target->next;
    } else {
        merged->prev = target->prev;
    }
    if (merged->head == NULL) {
        merged->leaf = true;
    } else {
        merged->leaf = false;
    }

    free(target->keys);
    free(sibling->keys);
    sibling->prev = NULL;

    free(target);
    free(sibling);

    return merged;
}
*/

void 
lateral_merge(tree_t *tree, 
              bnode_t *target, 
              bnode_t *sibling, 
              uint32_t idx, 
              uint32_t min)
{
    //fprintf(stderr, "Step 4 : Entered lateral_merge\n");
    
    bnode_t *parent = target->parent;
/*
    Move all keys and children from the sibling to the target node.
*/
    bnode_t *ret = merge(tree, target, sibling);

/*
    Additionally, move the parent key that separates the target node 
    and the merged sibling, into the target node. 
*/
    keyn_t *S_key = parent->keys[idx];
    //fprintf(stderr, "The sep key is %d\n", S_key->key);
/*
    Shift parent's keys left by one
*/
    uint32_t n_idx = find_key_array(S_key->key, ret);

    memmove(&parent->keys[idx], 
            &parent->keys[idx + 1], 
            sizeof(*(parent->keys)) * (parent->keys_count - idx));
    parent->keys_count -= 1;
/*
    Shift ret's (the new merged node) keys right by one
*/
    // ret->keys_count += 1;

    memmove(&ret->keys[n_idx + 1],
            &ret->keys[n_idx], 
            sizeof(*(ret->keys)) * (ret->keys_count - n_idx));
    ret->keys[n_idx] = S_key;
    ret->keys_count += 1;
    //printf("stored %d in %d\n", S_key->key, n_idx);
/*
    The parent has now lost a key. If it has less than the 
    minimum number, repeat steps 4-6 recursively up the tree. 
    If it satisfies the minimum number of keys, the algorithm is complete.
*/
    //fprintf(stderr, "parent count: %d\n", parent->keys_count);

    if (ret->parent->keys_count < min) {
        if (ret->parent == tree->root && ret->parent->keys_count > 0) {
            return; 
        } else {
            below_min_keys(tree, ret->parent, min, S_key->key);
        }
    }
}

void 
node_internal(bnode_t *target, 
              bool left)
{
/*
For future recursive steps, step 4 may need to be performed on a target node that is internal as follows. 

If the left sibling has more than the minimum number of keys, let Cleft be its “largest” child, which is separated
in the left sibling by Kleftchild. 

Note that, due to the insertion algorithm, all keys in Cleft are also < Kleft.

Then, according to step 4, Kleft is moved into the target node, and Kleftchild is moved into the parent. 

If the target node is internal, also move the child (and subtree rooted at) Cleft so that it is now the leftmost child of
the target node separated by the new key Kleft. 

Likewise if we are considering the right sibling, let Cright be the “smallest” child of the right sibling, 
which is separated by Krightchild. Kright is moved into the target node, Krightchild is moved to the parent, 
and Cright (and its subtree) is moved to become the rightmost child of the target node, separated by Kright. 
Again, if this step completes successfully, the deletion algorithm is complete
*/
    bnode_t *sibling;

    if (! left) {
        // right case
        sibling = target->next;
        bnode_t *right_sub = sibling->head;
        remove_node(sibling, right_sub);

        assert(right_sub->keys[0] != NULL);

        add_to_end(target->head, right_sub);
        
        right_sub->parent = target;

        sibling->head->prev = NULL;

    } else if (left) {
        // left case

        sibling = target->prev;
        bnode_t *cursor = sibling->head;
        while (cursor->next != NULL) {
            if (cursor->parent != cursor->next->parent) {
                printf("cursor->parent != cursor->next->parent\n");
            }
            cursor = cursor->next;
        }
        bnode_t *left_sub = cursor;
        remove_node(sibling, left_sub);
        
        left_sub->parent = target;

        left_sub->next = target->head;

        if (left_sub->next != NULL) {
            left_sub->next->prev = left_sub;
        }

        target->head = left_sub;
        left_sub->prev = NULL;

        sibling->head->prev = NULL;  
        
    }
}

void 
below_min_keys(tree_t *tree, 
               bnode_t *target, 
               uint32_t min,
               uint32_t key)
{
    // printf("below_min_keys with key: %d\n", key);
/*
    Check recursion to root
*/
    if (target->parent == NULL) {
        if (target->keys_count < min) {
           // fprintf(stderr, "Reached root with recursion\n");
        }
        reached_root(tree);
        return;
    }
/*
    After deletion, the leaf node (call this the target node) may now have less than the minimum number of
    keys permitted. Suppose that the target node is separated by the keys Kleft and Kright in the parent. 
*/
    int32_t l_key_idx;
    int32_t r_key_idx;

    
    find_left_right(target, key, &l_key_idx, &r_key_idx);
/*
    Firstly, check if the immediate left sibling of the target node (by order) has more than the minimum number of
    keys. If it does, in the parent replace Kleft with the largest key in the left sibling (call this Kleftchild), and
    move Kleft into the target node so that it has the minimum number of keys required.
*/
    int32_t sib_idx;
    bnode_t *sibling = NULL; 
    bnode_t *left = target->prev;                                     // left sibling
    bnode_t *right = target->next;                                   // right sibling


    if (left != NULL && left->keys_count > min) {
        sibling = left;
        if (l_key_idx != -1) {
// step 5
            sib_idx = l_key_idx;
            suitable_sibling(target, sibling, sib_idx, true);
            return;
        } else {
            printf("l_key_index == -1\n");
        }

    } else if (right != NULL && right->keys_count > min) {

        sibling = right;
        if (r_key_idx != -1) {
// step 5
            sib_idx = r_key_idx;
            suitable_sibling(target, sibling, sib_idx, false);
        }
    } else {
/*
    Left
*/
        if (left != NULL) {
            sibling = left;
            if (l_key_idx != -1) {
                sib_idx = l_key_idx;
                lateral_merge(tree, target, sibling, sib_idx, min);
            }
        } else if (right != NULL) {
/*
    Right
*/
            sibling = right;
            if (r_key_idx != -1) {
                sib_idx = r_key_idx;
                lateral_merge(tree, target, sibling, sib_idx, min);
            }
        } else {
            printf("No option worked? %d\n", key);
            //usleep(2000);
        }
    }
}