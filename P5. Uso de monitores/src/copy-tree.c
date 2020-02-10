#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "red-black-tree.h"
#include "copy-tree.h" 


/**
 *  
 * Copy the given tree, recursive part. 
 *
 */

void copy_tree_recursive(rb_tree *copy, node *x){
    node_data *n_data;
    
    if (x != NIL){
        copy_tree_recursive(copy, x->left);
        
        n_data = malloc(sizeof(node_data));
        n_data->key = malloc(strlen(x->data->key)+1);
        
        strcpy(n_data->key,x->data->key);
        n_data->num_times = 0;
        
        insert_node(copy,n_data);
        
        copy_tree_recursive(copy, x->right);
    }
}


/**
 *
 *  Copy the given tree. 
 *
 */

rb_tree *copy_tree(rb_tree *tree){
    rb_tree *copy = NULL;

    if (tree->root != NIL){
        copy = (rb_tree *) malloc(sizeof(rb_tree));
        init_tree(copy);

        copy_tree_recursive(copy, tree->root);
    }
    
    return copy;
}
