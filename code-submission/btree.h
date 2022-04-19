#define NODELIM 0x100000

typedef struct tree {
    pthread_mutex_t lock;
    pthread_cond_t cv;
    bool locked;
    uint16_t branching;
    struct tree_node *root;
} tree_t;

typedef struct key_node {
    uint32_t key;
    struct info *val;
} keyn_t;

typedef struct tree_node {
    pthread_mutex_t lock;
    bool leaf;
    uint32_t idx;
    uint32_t keys_count;
    keyn_t **keys;

    struct tree_node **kids;

    struct tree_node *next;
    struct tree_node *prev;
    struct tree_node *head;             // entry into children linked list
    struct tree_node *parent;           
} bnode_t;

typedef struct push {
    bnode_t *store[NODELIM];
    uint32_t idx;
} stack_t;

/*
    Node operations
*/
bnode_t*
new_tree_node(tree_t *tree);

keyn_t*
new_key_node(int32_t key, struct info *value);

int32_t 
find_key_array(int32_t key, bnode_t *node);

bnode_t*
find_index_child(int32_t index, bnode_t *container);

bnode_t*
find_key_node(int32_t key, bnode_t *container);

/*
    Linked list operations
*/
void
remove_node(bnode_t *parent, bnode_t *child);

void
replace_node(bnode_t *parent, bnode_t *child, bnode_t *replace);

void 
add_to_end(bnode_t *head, bnode_t *append);

void
find_left_right(bnode_t *target,
                int32_t key, 
                int32_t *l_idx, 
                int32_t *r_idx);

bnode_t*
get_rightmost(bnode_t *root);

/*
    Insert operations
*/
bnode_t*
insert(void *helper, int32_t key, struct info *value);

void
split(tree_t *tree,
      bnode_t *from,
      bnode_t *left,
      bnode_t *right,
      int32_t median, int32_t med_idx);

void
split_relink(tree_t *tree, 
             bnode_t *from, 
             bnode_t *parent,
             bnode_t *left, 
             bnode_t *right, 
             int32_t med_idx);

void
one_two(tree_t *tree, 
        bnode_t *from, 
        bnode_t *left, 
        bnode_t *right, 
        int32_t med_idx);

/*
    Delete operations
*/
int32_t
delete(void *helper, int32_t key);

void
reached_root(tree_t *tree);

void 
del_leaf(bnode_t *target);

bnode_t*
del_internal(bnode_t *target, keyn_t *found_key);

void
suitable_sibling(bnode_t *target, 
                bnode_t *sibling, 
                uint32_t idx, bool left);

bnode_t*
merge(tree_t *tree, bnode_t *target, bnode_t *sibling);

void 
lateral_merge(tree_t *tree, 
              bnode_t *target, 
              bnode_t *sibling, 
              uint32_t idx, 
              uint32_t min);

void 
node_internal(bnode_t *target, 
              bool left);

void 
below_min_keys(tree_t *tree, 
               bnode_t *target, 
               uint32_t min,
               uint32_t key);

void
splice_siblings(bnode_t *parent, 
                bnode_t *merged, 
                bnode_t *target, 
                bnode_t *sibling);

void
splice_children(bnode_t *parent, 
                bnode_t *merged, 
                bnode_t *target, 
                bnode_t *sibling);

void
update_parent(bnode_t *parent, 
              bnode_t *merged, 
              bnode_t *target, 
              bnode_t *sibling);


/*
    Output
*/
void 
print_keys(bnode_t *node);

void 
traversal(bnode_t *node, int i);

