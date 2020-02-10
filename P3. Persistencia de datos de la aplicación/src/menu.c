/**
 *
 * Practica 3 
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

#define MAXCHAR      100
#define MAGIC_NUMBER 0x01234567

/**
 * 
 *  Menu
 * 
 */

int menu() 
{
    char str[5];
    int opcio;
    char *r;
    
    printf("\n\nMenu\n\n");
    printf(" 1 - Creacio de l'arbre\n");
    printf(" 2 - Emmagatzemar arbre a disc\n");
    printf(" 3 - Llegir arbre de disc\n");
    printf(" 4 - Consultar informacio de l'arbre\n");
    printf(" 5 - Sortir\n\n");
    printf("   Escull opcio: ");

    r = fgets(str, 5, stdin);
    if(r == NULL)
        return 5;
    opcio = atoi(str); 

    return opcio;
}

/**
 * 
 *  Main procedure
 *
 */

int main(int argc, char **argv)
{
    char str1[MAXCHAR], str2[MAXCHAR];
    int opcio;
    rb_tree *tree = NULL;

    if (argc != 1)
        printf("Opcions de la linia de comandes ignorades\n");

    do {
        opcio = menu();
        printf("\n\n");

        char *r;
        switch (opcio) {
            case 1:
                printf("Fitxer de diccionari de paraules: ");
                r = fgets(str1, MAXCHAR, stdin);
                str1[strlen(str1)-1]=0;

                printf("Fitxer de base de dades: ");
                r = fgets(str2, MAXCHAR, stdin);
                str2[strlen(str2)-1]=0;
                
                if (r!= NULL){
                    if(tree != NULL){
                        delete_tree(tree);
                        free(tree);
                        printf("\nWarning! The previous tree was deleted to create a new one!\n");
                    }
                    tree = create_tree(str1,str2);
                }
                
                break;

            case 2:
                if(tree != NULL){
                    printf("Nom de fitxer en que es desara l'arbre: ");
                    r = fgets(str1, MAXCHAR, stdin);
                    str1[strlen(str1)-1]=0;
                    save_tree(tree,str1, MAGIC_NUMBER);
		    printf("\nThe tree was saved successfully on file %s\n",str1);
                }else printf("Please, generate or load a tree before save it\n");

                break;

            case 3:
                printf("Nom del fitxer que conte l'arbre: ");
                r = fgets(str1, MAXCHAR, stdin);
                str1[strlen(str1)-1]=0;

                if(tree != NULL){
                    delete_tree(tree);
                    free(tree);
                    printf("\nWarning! The previous tree was deleted to load one from file!\n");
                }
                
                tree = load_tree(str1,MAGIC_NUMBER);

                break;

            case 4:
		if(tree != NULL){
		        printf("Paraula a buscar o polsa enter per saber la paraula que apareix mes vegades: ");
		        r = fgets(str1, MAXCHAR, stdin);
		        str1[strlen(str1)-1]=0;
			
			int option = 0;
			if(strlen(str1) != 0) option = 1;
			find_ncoincidences_word(tree, option, r);
		}else printf("\nFirst you need to create a tree\n");
                break;

            case 5:

                if(tree!=NULL){
                    delete_tree(tree);
                    free(tree);
                }

		printf("\nFins aviat!\n");

                break;

            default:
                printf("Opcio no valida\n");

        } /* switch */
    }
    while (opcio != 5);

    return 0;
}

