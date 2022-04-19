#include "btreestore.h"
#include "btree.h"

void 
print_keys(bnode_t *node)
{
    for (int i = 0; i < node->keys_count; i++) {
        if (node->keys[i] == NULL) {
            continue;
        }

        if (node->keys_count > 1) {
            if (i < node->keys_count - 1) {
                if (node->keys[i]->key > node->keys[i + 1]->key) {
                    printf("ordered wrong in print_keys\n");
                    int32_t *p = NULL;
                    *p = 3;
                }
            }
        }

        assert(node->keys[0] != NULL);
        printf("%d ", node->keys[i]->key);
    }
}

void 
traversal(bnode_t *node, int j) 
{
    if (node != NULL) {
        printf("\n");
        print_keys(node);

        for (int i = 0; i < (node->keys_count + 1); i++) {
            traversal(node->kids[i], j);
        }
    }
}