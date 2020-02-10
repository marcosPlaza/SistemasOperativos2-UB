#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "red-black-tree.h"
#include "update-tree.h" 


/**
 *  
 *  Update the given tree with the values of the copy, recursive part.
 *
 */

void update_tree_recursive(rb_tree *tree, node *x, pthread_mutex_t *mutex_update){
    node_data *n_data;
    
    if (x != NIL){
        update_tree_recursive(tree, x->left, mutex_update);
        
        if(x->data->num_times != 0){
            n_data = find_node(tree, x->data->key);
            
            pthread_mutex_lock(mutex_update);
            n_data->num_times += x->data->num_times;
            pthread_mutex_unlock(mutex_update);
        }
        update_tree_recursive(tree, x->right, mutex_update);
    }
}


/**
 *
 *  Update the given tree with the values of the copy. 
 *
 */

void update_tree(rb_tree *tree, rb_tree *copy, pthread_mutex_t *mutex_update){
    
    if (copy->root != NIL){
        update_tree_recursive(tree, copy->root, mutex_update);
    }
    
    return;
} 
