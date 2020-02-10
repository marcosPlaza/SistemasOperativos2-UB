#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "red-black-tree.h"
#include "tree-to-mmap.h"
#include "dbfnames-mmap.h"

#define MAXCHAR 100

/*
* Utilizaremos dos semaforos para controlar el incremento de los indices y el incremento del numero de coincidencias
*/
sem_t *sem_index;
sem_t *sem_data;

/**
 *
 *  Given a file, insert the words it contains into a tree.  We assume that
 *  each line contains a word and that words are not repeated. 
 *
 */
void index_dictionary_words(rb_tree *tree, FILE *fp)
{
  char *s, word[MAXCHAR];

  node_data *n_data;

  while (fgets(word, MAXCHAR, fp))
  {
    // Remove '\n' from string
    word[strlen(word)-1] = 0;

    // In the last line of the dictionary
    // an empty string is read
    if (strlen(word) > 0) {
      // Copy static string to dynamic string
      s = malloc(strlen(word)+1);
      strcpy(s, word);

      // Insert into tree
      n_data = malloc(sizeof(node_data));

      n_data->key = s;
      n_data->num_times = 0;
      insert_node(tree, n_data);
    }
  }
}

/**
 *
 *  Given a line with words, extract them and transform them to
 *  lowercase. We search each word in the tree. If it is found,
 *  we increase the associated counter. Otherwise nothing is done
 *  for that word.
 *
 */
void index_words_line(rb_tree *tree, char *line)
{
  node_data *n_data;

  int i, j, is_word, len_line;
  char paraula[MAXCHAR];

  i = 0;

  len_line = strlen(line);

  /* Search for the beginning of a candidate word */

  while ((i < len_line) && (isspace(line[i]) || ((ispunct(line[i])) && (line[i] != '#')))) i++; 

  /* This is the main loop that extracts all the words */

  while (i < len_line)
  {
    j = 0;
    is_word = 1;

    /* Extract the candidate word including digits if they are present */

    do {

      if ((isalpha(line[i])) || (line[i] == '\''))
        paraula[j] = line[i];
      else 
        is_word = 0;

      j++; i++;

      /* Check if we arrive to an end of word: space or punctuation character */

    } while ((i < len_line) && (!isspace(line[i])) && (!(ispunct(line[i]) && (line[i]!='\'') && (line[i]!='#'))));

    /* If word insert in list */

    if (is_word) {

      /* Put a '\0' (end-of-word) at the end of the string*/
      paraula[j] = 0;

      /* Search for the word in the tree */
      n_data = find_node(tree, paraula);

      if (n_data != NULL){
      	/* Establecemos region critica al incrementar el numero de coincidencias */
      	sem_wait(sem_data);
        n_data->num_times++;
      	sem_post(sem_data);
      }
    }

    /* Search for the beginning of a candidate word */

    while ((i < len_line) && (isspace(line[i]) || ((ispunct(line[i])) && (line[i] != '#')))) i++; 

  } /* while (i < len_line) */
}

/**
 *
 *  Given a file, this funcion reads the lines it contains and uses function
 *  index_words_line to index all dictionary words it contains.
 *
 */
void process_file(rb_tree *tree, char *fmmap_names, int index)
{
  FILE *fp;
  char *fname, line[MAXCHAR];

  fname = get_dbfname_from_mmap(fmmap_names, index);
  fp = fopen(fname, "r");
  if (!fp) {
    printf("Could not open %s\n", fname);
    exit(1);
  }
  
  printf("Processing %s\n", fname);

  while (fgets(line, MAXCHAR, fp))
    index_words_line(tree, line);

  fclose(fp);
}

/*
* Como la key debe ser accesible a todos los procesos la mapeamos en memoria y inicialicamos el semaforo 
* con la key respectiva
*/
sem_t *init_sem_key(){
  sem_t *sem;
  sem = (sem_t *)mmap(0, sizeof(sem_t), PROT_WRITE | PROT_READ, 
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (sem == MAP_FAILED) {
        printf("Map failed. Errno = %d\n", errno);
        exit(1);
    }
  sem_init(sem,1,1);
  
  return sem;
}

/*
* Este metodo inicializa el indice del lector de ficheros y lo mapea en memoria 
* para que todos los procesos puedan acceder
*/
int *init_index_file(){
    int *index;
    
    index = mmap(0, sizeof(int), PROT_WRITE | PROT_READ, 
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (index == MAP_FAILED) {
        printf("Map failed. Errno = %d\n", errno);
        exit(1);
    }
    
    *index = 0;    
    return index;
}

/*
* Incrementamos el indice del lector de ficheros y devolvemos el valor actual 
*/
int next_index_file(int *index){
    int i;
    
    sem_wait(sem_index);
    
    i = *index;
    
    *index = i+1;
    
    sem_post(sem_index);
    
    return i;
}

/**
 *
 *  Construct the tree given a dictionary file and a 
 *  database with files to process.
 *
 */
rb_tree *create_tree(char *fname_dict, char *fname_db)
{
  FILE *fp_dict, *fp_db;

  rb_tree *tree;
  int max_process = 2; //EL maximo de procesos que vamos a utilizar para leer los ficheros de la base de datos
  int num_files, num_process, i, *index;
  char *mmap_node_data, *fmmap_names;
  
  fp_dict = fopen(fname_dict, "r");
  if (!fp_dict) {
    printf("Could not open dictionary file %s\n", fname_dict);
    return NULL;
  }

  fp_db = fopen(fname_db, "r");
  if (!fp_db) {
    printf("Could not open dabase file %s\n", fname_db);
    return NULL;
  }

  /* Allocate memory for tree */
  tree = (rb_tree *) malloc(sizeof(rb_tree));

  /* Initialize the tree */
  init_tree(tree);

  /* Index dictionary words */
  index_dictionary_words(tree, fp_dict);

  /* Serialize the tree */
  mmap_node_data = serialize_node_data_to_mmap(tree);
  
  /* Serialize the database file */
  fmmap_names = dbfnames_to_mmap(fp_db);

  /* Read the number of files the database contains */
  num_files = * ((int *) fmmap_names);

  /* Inicializamos el indice que vamos a usar para seleccionar los ficheros a procesar */
  index = init_index_file();
  
  /* Inicializamos cada semaforo */
  sem_index = init_sem_key();
  sem_data = init_sem_key();
  
  /* Read database files */
  for (num_process = 0; num_process<max_process; num_process++){
      if(fork() == 0){
        i = next_index_file(index);
        
        while(i<num_files) {
            /* Process file */
            process_file(tree, fmmap_names, i);
            i = next_index_file(index);
        }

        /* Necesitamos cerrar ficheros deserializar y hacer el free del arbol */
        fclose(fp_dict);
        fclose(fp_db);

        deserialize_node_data_from_mmap(tree, mmap_node_data); 
        delete_tree(tree);
        free(tree);

        exit(0);
    }
  }

  /* Esperamos a que todos los hijos terminen de ejecutarse */
  for (num_process=0; num_process<max_process; num_process++){
    wait(NULL);
  }
  
  /* Close files */
  fclose(fp_dict);
  fclose(fp_db);

  /* Deserializamos el arbol y el fichero mapeado */
  deserialize_node_data_from_mmap(tree, mmap_node_data);  
  dbfnames_munmmap(fmmap_names);

  /* Return created tree */
  return tree;
}
