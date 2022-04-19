#include "btreestore.h"
#include "btree.h"


void *global_helper = NULL;
pthread_t test1;
pthread_t test2;
pthread_t test3;
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

void 
test_massive()
{
    void *helper = init_store(10, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t here[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    uint32_t big = 563433;

    for (int i = 0; i < big; i++) {
        btree_insert(i, 
                    &here, 
                    8, 
                    key, 
                    0x1234, 
                    helper); 
    }

    traversal(tree->root, 0);
    close_store(helper);
}

void
test_100()
{
    void *helper = init_store(10, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t here[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};
    btree_insert(5, 
                 &here, 
                 8, 
                 key, 
                 0x1234, 
                 helper); 
    for (int i = 0; i < 100; i++) {
        btree_insert(i, 
                    &here, 
                    8, 
                    key, 
                    0x1234, 
                    helper); 
    }

    traversal(tree->root, 0);
    close_store(helper);
    
    printf("\n");
}

void
test_10()
{
    void *helper = init_store(10, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t here[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 5000; i++) {
        int r = rand() % 5000;
        if (btree_insert(r, 
                    &here, 
                    8, 
                    key, 
                    0x1234, 
                    helper) == 0) { 
        printf("\ninserted %d\n", r);
        //traversal(tree->root, 0);
        //printf("\n\n\n");
        }
    }

/*
inserted 33
inserted 36
inserted 27
inserted 15
inserted 43
inserted 35
inserted 42
inserted 49
inserted 21
inserted 12
inserted 40
inserted 9
inserted 13

    btree_insert(33, &here, 8, key, 0x1234, helper); 
    btree_insert(36, &here, 8, key, 0x1234, helper); 
    btree_insert(27, &here, 8, key, 0x1234, helper); 
    btree_insert(15, &here, 8, key, 0x1234, helper); 
    btree_insert(43, &here, 8, key, 0x1234, helper); 
    btree_insert(35, &here, 8, key, 0x1234, helper); 
    btree_insert(42, &here, 8, key, 0x1234, helper); 
    btree_insert(49, &here, 8, key, 0x1234, helper); 
    btree_insert(21, &here, 8, key, 0x1234, helper); 
    btree_insert(12, &here, 8, key, 0x1234, helper); 
    btree_insert(40, &here, 8, key, 0x1234, helper); 
    btree_insert(9, &here, 8, key, 0x1234, helper); 
    btree_insert(13, &here, 8, key, 0x1234, helper); 
*/


    traversal(tree->root, 0);
    printf("\n");
    close_store(helper);
}

void 
test_del_three()
{
    void *helper = init_store(5, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 30; i++) {
        btree_insert(i, &plain, 8, key, 0x1234, helper); 
    }
/*
    for (int i = 0; i < 30; i++) {
        int r = rand() % 30;
        printf("deleting %d\n", r);
        btree_delete(r, helper);
    
    }
*/
    printf("before\n");
    traversal(tree->root, 0);
    printf("\n-------------\n");

    btree_delete(13, helper);
    printf("del 13\n");
    traversal(tree->root, 0);
    printf("\n-------------\n");

    btree_delete(16, helper);
    printf("del 16\n");
    traversal(tree->root, 0);
    printf("\n-------------\n");


    close_store(helper);
}

void
test_del_two()
{
    void *helper = init_store(3, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    btree_insert(20, &plain, 8, key, 0x1234, helper); 
    btree_insert(22, &plain, 8, key, 0x1234, helper); 
    btree_insert(1, &plain, 8, key, 0x1234, helper); 
    btree_insert(3, &plain, 8, key, 0x1234, helper); 
    btree_insert(4, &plain, 8, key, 0x1234, helper); 
    btree_insert(21, &plain, 8, key, 0x1234, helper); 
    btree_insert(80, &plain, 8, key, 0x1234, helper); 

    btree_insert(100, &plain, 8, key, 0x1234, helper); 
    btree_insert(23, &plain, 8, key, 0x1234, helper); 
    btree_insert(40, &plain, 8, key, 0x1234, helper); 
    btree_insert(69, &plain, 8, key, 0x1234, helper); 
    btree_insert(812, &plain, 8, key, 0x1234, helper); 

    btree_delete(23, helper);
    btree_delete(100, helper);
    btree_delete(812, helper);
    btree_delete(3, helper);
    btree_delete(1, helper);
    btree_delete(22, helper);

    traversal(tree->root, 0);
    close_store(helper);
    
    printf("\n");
}

void
test_del()
{
    void *helper = init_store(3, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    btree_insert(4, &plain, 8, key, 0x1234, helper); 
    btree_insert(21, &plain, 8, key, 0x1234, helper); 
    btree_insert(1, &plain, 8, key, 0x1234, helper); 
    btree_insert(3, &plain, 8, key, 0x1234, helper); 
    btree_insert(20, &plain, 8, key, 0x1234, helper); 
    btree_insert(22, &plain, 8, key, 0x1234, helper); 
    btree_insert(80, &plain, 8, key, 0x1234, helper); 

    btree_delete(20, helper);

    //struct node *nodes = malloc(sizeof(struct node) * 10000);
    //uint64_t n = btree_export(helper, &nodes);

    btree_delete(4, helper);
    //n = btree_export(helper, &nodes);

    traversal(tree->root, 0);
    close_store(helper);
    
    printf("\n");
}


void* 
thread1()
{
    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 200; i < 2000; i++) {
        btree_insert(i, &plain, 8, key, 0x1234, global_helper);  
    }  
}

void* 
thread2()
{
    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 200; i++) {
        btree_insert(i, &plain, 8, key, 0x1234, global_helper);  
    }   
}

void
test_multi()
{
    void *helper = init_store(4, 6);
    tree_t *tree = (tree_t *)helper;
    global_helper = helper;

	pthread_create(&test1, NULL, thread1, NULL);
    pthread_create(&test2, NULL, thread2, NULL);

	pthread_join(test1, NULL);
    pthread_join(test2, NULL);

    traversal(tree->root, 0);
    close_store(helper);    
    printf("\n-----------\n");
}

void 
test_del_four()
{
/*
Node: 4 21 
Node: 1 3 
Node: 20 
Node: 22 80 
*/
    void *helper = init_store(3, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    btree_insert(4, &plain, 8, key, 0x1234, helper); 
    btree_insert(21, &plain, 8, key, 0x1234, helper); 
    btree_insert(1, &plain, 8, key, 0x1234, helper); 
    btree_insert(3, &plain, 8, key, 0x1234, helper); 
    btree_insert(20, &plain, 8, key, 0x1234, helper); 
    btree_insert(22, &plain, 8, key, 0x1234, helper); 
    btree_insert(80, &plain, 8, key, 0x1234, helper); 

    traversal(tree->root, 0);   
    printf("\n-----------\n");
    btree_delete(20, helper);
    traversal(tree->root, 0);   
    printf("\n-----------\n");
    btree_delete(4, helper);
    traversal(tree->root, 0);   
    printf("\n-----------\n");
    btree_delete(21, helper);
    traversal(tree->root, 0);   
    printf("\n-----------\n");
    btree_delete(22, helper);


    traversal(tree->root, 0);   
    printf("\n-----------\n");

    close_store(helper); 
}

void
test_insert_and_del()
{
    void *helper = init_store(5, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 6000; i++) {
        int r = rand() % 6000;
        btree_insert(r, &plain, 8, key, 0x1234, helper); 
    }

/*
    btree_delete(29, helper);
    btree_delete(13, helper);
    btree_delete(21, helper);

    btree_delete(39, helper);
    btree_delete(24, helper);
    btree_delete(17, helper);


    btree_delete(38, helper);
    btree_delete(35, helper);
    btree_delete(33, helper);

    btree_delete(25, helper);
    btree_delete(5, helper);
*/


    for (int i = 0; i < 6000; i++) {
        int r = rand() % 6000;
        if (btree_delete(r, helper) != 1) {
            printf("---> deleted r : %d\n", r);
            printf("\n\n");
        }
    }

    printf("\n----------after----------\n");
    traversal(tree->root, 0);
    printf("\n\n\n");
    close_store(helper);   
}


void* 
threadB()
{
    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 2000; i++) {
        int r = rand() % 2000;
        btree_delete(r, global_helper); 
    }   
}


void* 
threadC()
{
    
    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 2000; i++) {
        int r = rand() % 2000;
        btree_delete(r, global_helper); 
    } 
}

void* 
threadA()
{
    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

    for (int i = 0; i < 2000; i++) {
        btree_insert(i, &plain, 8, key, 0x1234, global_helper);  
    } 

    for (int i = 0; i < 5000; i++) {
        int r = rand() % 5000;
        btree_delete(r, global_helper); 
        //printf("%d is deleted\n", r);
    } 
}

void
test_multi_two()
{
    void *helper = init_store(4, 6);
    tree_t *tree = (tree_t *)helper;
    global_helper = helper;

	pthread_create(&test1, NULL, threadA, NULL);
    pthread_create(&test2, NULL, threadB, NULL);
    pthread_create(&test3, NULL, threadC, NULL);

	pthread_join(test1, NULL);
    pthread_join(test2, NULL);
    pthread_join(test3, NULL);

    traversal(tree->root, 0);
    printf("\n-----------\n");
    close_store(helper);    
}

void
test_del_one()
{
    void *helper = init_store(3, 6);
    tree_t *tree = (tree_t *)helper;

    uint64_t plain[6] = {0,1,2,3,4,5};
    uint32_t key[4] = {0x414141, 0x414141, 0x414141, 0x414141};

/*
    for (int i = 0; i < 10; i++) {
        btree_insert(i, &plain, 8, key, 0x1234, helper); 
    }

    for (int i = 0; i < 6000; i++) {
        int r = rand() % 6000;
        if (btree_delete(r, helper) != 1) {
            printf("---> deleted r : %d\n", r);
            printf("\n\n");
        }
    }
*/
    btree_insert(0, &plain, 8, key, 0x1234, helper); 
    btree_insert(1, &plain, 8, key, 0x1234, helper); 
    btree_insert(2, &plain, 8, key, 0x1234, helper); 

    btree_delete(1, helper);

    printf("\n----------after----------\n");
    traversal(tree->root, 0);
    printf("\n\n\n");
    close_store(helper);   
}

int main() 
{
    //test_100();
    //test_10();
    //test_multi();
    
    //test_del_three();
    //test_del_four();
    //test_massive();
    
    test_del_one();

    //test_insert_and_del();
    
    //test_multi_two();

    return 0;
}