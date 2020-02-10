#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>  
#include "red-black-tree.h"


#define MAXCHAR 100

/**
 * Function to see if the mode is legit
 * r: we analyze the file for words that are in our dictionary tree
 * w: we generate or expand the tree
 */
void isAcceptedMode(char mode){    
    if (mode != 'r' && mode != 'w'){
        printf("----\"%c\" is not an accepted mode----\n",mode);
        printf("->Modes availables:\n\'r\': analyze file\n\'w\': generate dictionary tree\n");
        exit(1);
    }
}


/*
 * Function to see if the word is already at the tree, if it's not there and 
 * the mode is 'w' (write) this word will be append 
 */
void process_word(char *word, rb_tree *tree, char mode){
    node_data *n_data;
    n_data = find_node(tree, word); 
    
    isAcceptedMode(mode);
    
    if (n_data != NULL) {

        //If the key is in the tree increment 'num' 
        n_data->num_times++;
    } else if (mode == 'w') {

        //If the key is not in the tree, allocate memory for the data
        // and insert in the tree 

        n_data = malloc(sizeof(node_data));
        n_data->key = malloc(strlen(word)+1); //we allocate the exact memory we need to store the word

        //This is the key by which the node is indexed in the tree 
        //We make a copy of the actual value
        strcpy(n_data->key,word);

        //This is additional information that is stored in the tree 
        n_data->num_times = 0;

        //We insert the node in the tree 
        insert_node(tree, n_data);
    }
}

/*
* This function determines which of the symbols (included on ispunct funcion) we want to allow on a word
*
* If a word is followed by a "." we must pick the word: This code is written in C.
* We will take: "This", "code", "is", "written", "in", "C".
*
* But if a word is followed by a "#": This code is written in C#
* We will take: "This", "code", "is", "written", "in" however the "C#" is refused.
*
* So, we consider that the punctuation signs we must allow (not ot discard a word) are: ./,/:/;/?/!/(/)
*/
int valid_ortographical_signs(char sign){
    if(sign == '.' || sign == ',' || sign == ':' || sign == ';' || sign == '?' || sign == '!' || sign == '(' || sign == ')') return 1;
    return 0;
}

/*
* Process a line of the plain text file and internally uses process_word function to look for coincidences
*/
void process_line(char *line, rb_tree *tree, char mode)
{
    int i, j, is_word, len_line, fromdash;
    char paraula[MAXCHAR];
    i = 0;
    len_line = strlen(line);
    
    isAcceptedMode(mode);

/* Search for the beginning of a candidate word taking into account unvalid symbols at the begining*/
    while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i]) && !isalpha(line[i+1])))) i++;

    /* This is the main loop that extracts all the words */
    while (i < len_line)
    {
        j = 0;
        is_word = 1;
        fromdash = 0; /* If a word contains a '-' (it can happens in our implementation) means that is a composed word. We will remove the symbol later */

        /* Won't extract the candidate word if:
        *   - contains a number
        *   - contains any simbol except '\'' (apostrophe) -> We will consider that if there is a contiguous valid ortographical
        *       sign (seen on the previous function) we can extract the word
        *   - contains diaresis or accent
        */

        do {
            if ((isalpha(line[i]) || line[i] == '\'')) // We will take the apostroph of the word
                paraula[j] = line[i];
            else if (line[i] == '-')
                fromdash = 1;
            else
                is_word = 0; // We won't take the word if contains an unvalid symbol
            
            j++; i++;

        /* Check if we arrive to an end of word: space or valid punctuation character (valid_ortographical_signs or '-') */
        } while ((i < len_line) && (!isspace(line[i])) && (!(valid_ortographical_signs(line[i]) || line[i] == '-') || line[i] == '\'' ));

        /* If word insert in list */
        if (is_word) {
            if(fromdash){ // If we are in the case of a word containing a '-' we will remove it
                int cnt = 0;
                while(cnt<j){
                    paraula[cnt] = paraula[cnt+1];
                    cnt++;
                }
                memmove(&paraula[j], &paraula[j+1], j);
                j--;
            }

            /* Put a '\0' (end-of-word) at the end of the string*/
            paraula[j] = 0;

            /* Look for any coincidences */
            process_word(paraula,tree,mode);
        }

        /* Search for the beginning of a candidate word taking into account unvalid symbols at the begining*/
        while ((i < len_line) && (isspace(line[i]) || (ispunct(line[i]) && !isalpha(line[i+1])))) i++;
    } 
}

/*
* This function reads the file which contains all the files to analyze
*/
void process_file(char *file, rb_tree *tree, char mode){
    FILE *fp;
    char line[500];
    
    //we delete the newline char so we can read the path
    file = strtok(file, "\n");
    fp = fopen(file, "r");
    
    if (!fp) {
        printf("%s",file);
        printf("Could not open file\n");
        exit(1);
    }
    
    isAcceptedMode(mode);

    while (fgets(line, MAXCHAR, fp)) process_line(line, tree, mode);
    
    fclose(fp);
}

rb_tree *create_tree(char *dict, char *dades)
{
    FILE *fp;
    int count = 0, i;
    char path[500];
    rb_tree *tree;

    if (dict == NULL || dades == NULL) {
        printf("Insert dictionary and data base directories\n");
        exit(1);
    }
    
    fp = fopen(dades, "r");
    if (!fp) {
        printf("Could not open file\n");
        exit(1);
    }

    /* Random seed */
    srand(time(NULL));

    /* Allocate memory for tree */
    tree = (rb_tree *) malloc(sizeof(rb_tree));

    /* Initialize the tree */
    init_tree(tree);

    //we get the first line (number of files to read)
    count = atoi(fgets(path,MAXCHAR,fp));

    //We generate the dictionary tree
    process_file(dict,tree,'w');

    i = 0;
    while (i<count){
        if (fgets(path,MAXCHAR,fp)!=NULL)
            process_file(path, tree, 'r');

        i++;
    }

    fclose(fp);

    /* Print and delete the tree */
    printf("\nTree with size %d was created\n",tree->size);
    
    return tree;
}

void save_tree(rb_tree *tree, char *name, int magic){
    FILE *fp;
    
    fp = fopen(name, "w");
    fwrite(&magic, sizeof(int), 1, fp);
    save_tree_disk(tree,fp);
    
    fclose(fp);
}

rb_tree *load_tree(char *name, int magic){
    FILE *fp;
    int r = 0, number = 0, it = 0;
    char paraula[MAXCHAR];
    rb_tree *tree;
    node_data *n_data;
    
    fp = fopen(name, "r");
    if (!fp) {
        printf("Could not open file\n");
        exit(1);
    }
    
    r = fread(&number, sizeof(int), 1, fp);
    
    if (r == 0 || number!=magic){
        fclose(fp);
        exit(1);
    }
    
    
    /* Random seed */
    srand(time(NULL));

    /* Allocate memory for tree */
    tree = (rb_tree *) malloc(sizeof(rb_tree));

    /* Initialize the tree */
    init_tree(tree);
    
    r = fread(&it, sizeof(int), 1, fp);
    
    for (int i = 0; i<it; i++){
        //Initialize the data
        n_data = malloc(sizeof(node_data));
        
        //we read the word length and allocate the allocate the memory
        r = fread(&number, sizeof(int), 1, fp);       
        n_data->key = malloc(number+1);
        
        //we read the word and save the word
        r = fread(paraula,1,number,fp);
        paraula[number]=0;
        strcpy(n_data->key,paraula);
        
        //wee read the number of times the word appear
        r = fread(&number,sizeof(int),1,fp);
        n_data->num_times = number;
        
        insert_node(tree,n_data);
        
    }
    
    fclose(fp);
    
    return tree;
}
