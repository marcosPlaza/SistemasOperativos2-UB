#pragma once 
#include "red-black-tree.h"

void isAcceptedMode(char mode);
void process_word(char *word, rb_tree *tree, char mode);
int valid_ortographical_signs(char sign); 
void process_line(char *line, rb_tree *tree, char mode);
void process_file(char *file, rb_tree *tree, char mode);
void save_tree(rb_tree *tree, char *name, int magic);
rb_tree *load_tree(char *name, int magic);
rb_tree *create_tree(char *dict, char *dades);

