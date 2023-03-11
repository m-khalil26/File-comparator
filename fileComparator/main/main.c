#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "holdall.h"
#include <ctype.h>

#define WORD_LENGTH_MAX 63 //correction
#define TAB_OCC_MOT_MAX 1024

size_t str_hashfun(const char *s);
int rfree(void *ptr);
int scptr_display(int *nb_mot, const char *s, char *cptr);

//argc  recoit le nombre d'elements de lentree standard
//argv est un tableau qui stock les adresses des elements de lentree sstandard
//(tableau vers chaine de char*)char *tab_occ_mot =
// malloc(TAB_OCC_MOT_MAX*(sizeof(char)));

int main(int argc, char **argv) {
  int nb_fichier_ap = 10;
  int word_length_max = WORD_LENGTH_MAX;
  int majuscules = 0;
  int ponctuation = 0;
  int i = 1;
  /*-------------------Gestion des options------------------------*/
  if (i != argc && argv[i][0] == '-' && argv[i][1] == '-'
      && strlen(argv[i]) < 7) { //pour traiter --help, mais si ce n'est pas --help, il va continuer a lire le code
    if (strcmp(argv[i], "--help") == 0) {
      printf("Voici WS !!\n");
      printf("Insérez autant de fichier que vous le souhaitez \n");
      printf("Et vous aurez tout les mots partagés de ces fichiers!\n");
      printf("Voici une liste d'options que vous pouvez utiliser :\n");
      printf("\n");
      printf(
          "\t-t : Suivi d'un nombre entre 0 et 99 pour indiquer le nombre de mots à produire\n");
      printf(
          "\t-i : Suivi d'un nombre entre 0 et 99 pour indiquer la longueur des mots à afficher\n");
      printf("\t-u : Pour afficher tout les mots en majuscule\n");
      printf("\t-p : Pour ne pas considerer la ponctuation\n");
      printf("\t - : Pour inserer une phrase directement à la place du -\n");
      printf("\n");
      puts("IMPORTANT: VEUILLEZ RELANCER LE PROGRAMME POUR L'UTILISER\n");
      return EXIT_SUCCESS;
    }
  }
  while (i != argc && argv[i][0] == '-' && strlen(argv[i]) == 2) { //pour tester les options -u, -p..etc
    if (isdigit(argv[i + 1][0]) && !isdigit(argv[i + 1][1])) { //si l'element qui suit l'option est un chiffre
      if (strcmp(argv[i], "-t") == 0) {
        nb_fichier_ap = (*argv[i + 1] - 48); //48 est le code ascii de 0, et
                                             // *argv[i+1]
                                             //renvoie le code ascii du chiffre
        printf("\nnombre de mots à produire %d\n", nb_fichier_ap);
      }
      if (strcmp(argv[i], "-i") == 0) { //specifier la longueur maximale des mots
        word_length_max = (*argv[i + 1] - 48);
      }
      i++;
    }
    if (isdigit(argv[i + 1][0]) && isdigit(argv[i + 1][1])) {
      if (strcmp(argv[i], "-t") == 0) {
        nb_fichier_ap = (argv[i + 1][0] - 48) * 10 + (argv[i + 1][1] - 48);
        printf("\nnombre de mots à produire %d\n", nb_fichier_ap);
      }
      if (strcmp(argv[i], "-i") == 0) {
        word_length_max = (argv[i + 1][0] - 48) * 10 + (argv[i + 1][1] - 48);
      }
      i++;
    }
    if (strcmp(argv[i], "-u") == 0) {
      majuscules = 1;
    }
    if (strcmp(argv[i], "-p") == 0) {
      ponctuation = 1;
    }
    i++;
  }
  if (i == argc) { //si jamais on est sorti de la boucle while et que i=argc cela veut dire que l'on a insere que des -machin, donc toutes des options
    fprintf(stderr,
        "Toutes les entrées sont des options, veuillez inserez au moins deux fichiers\n");
    return EXIT_FAILURE;
  }
  if (argc - i < 2) { //si jamais on est sorti de la boucle while et que le nombre des elements qu'on ait mis sur l'entrée standard est i-1, cela veut dire que i=argc-1, donc on a inseré un seule element qui n'est pas une option
    fprintf(stderr, "Veuillez inserer au moins deux fichiers\n"); //on aurait pu mettre if(i==argc-1)
    return EXIT_FAILURE;
  }
  /*-----------------------Declarations----------------------------*/
  void *f[argc - i]; //argc-i car quand on sort de la boucle while i aura le nombre des options+l'executable, et pour se debarasser de ces derniers, on les soustrait a argc
  //ln taille du tableau des pointeurs de fichiers
  int ln = 0;
  char *tab_occ_mot = malloc((TAB_OCC_MOT_MAX + 1) * (sizeof(char)));
   if (tab_occ_mot == NULL) { //correction
    fprintf(stderr, "erreur d'allocation"); //correction
    return EXIT_FAILURE;
  }
  *(tab_occ_mot + TAB_OCC_MOT_MAX) = '\0';
  memset(tab_occ_mot, '-', (TAB_OCC_MOT_MAX) * sizeof(char));
  hashtable *ht = hashtable_empty((int (*)(const void *, const void *))strcmp,
      (size_t (*)(const void *))str_hashfun);
  holdall *has = holdall_empty();
  char c1;
  char c2;
  if (ht == NULL
      || has == NULL) {
    return EXIT_FAILURE;
  }
  for (int j = i; j < argc; j++) { //on aurait pu utiliser directement i au lieu de j
    /*-----------------Gestion de l'entrée standard---------------------*/
    if (strcmp(argv[j], "-") == 0) {
      char **tab_mot_entree = malloc(WORD_LENGTH_MAX * sizeof(char *));
      char s[WORD_LENGTH_MAX + 1];
      while (scanf("%s", s) == 1) {
        if (strlen(s) == (size_t) WORD_LENGTH_MAX) {
          fprintf(stderr, "maximum atteint.\n");
        }
      }
      int z = 0;
      int debut = 0;
      while (s[debut] != '\0') {
        int index = 0;
        char *p = s;
        while (!isspace(p[index]) && p[index] != '\0') {
          index++;
        }
        memcpy(tab_mot_entree[z], p + debut, (size_t) (index - debut));
        debut = index;
        z++;
      }
      printf("bonne lecture de l'entrée standard\n");
    } else {
      /*-------------------Ouvertue des fichiers------------------------*/
      f[ln] = fopen(argv[j], "r");
      if (f[ln] == NULL) {
        fprintf(stderr, "erreur ouverture fichier %d \n", ln);
        return EXIT_FAILURE;
      }
      ln++;
    }
  }
  /*-----------------------Lecture des fichiers----------------------*/
  for (int i = 0; i < ln; i++) { // pour passer entre les fichiers
    while (!feof(f[i])) { //lire les mots
      char *w1 = malloc(sizeof(char) * (long unsigned int) WORD_LENGTH_MAX + 1);
      *(w1 + WORD_LENGTH_MAX) = '\0';
      char *p = w1;
      //pour passer entre les mots
      int taille_m = 0;
      while (taille_m < (int) WORD_LENGTH_MAX) { //pour passer entre
        //les caracteres
        c1 = (char) fgetc(f[i]);
        c2 = (char) fgetc(f[i]);
        if (c2 == EOF) {
          break;
        } else {
          ungetc(c2, f[i]);
          if (majuscules == 1) {
            c1 = (char) toupper(c1);
          }
          if (ponctuation == 1) {
            if (ispunct(c1)) {
              p = '\0';
              break;
            }
          }
          (*p) = c1;
          ++p;
          ++taille_m;
          if (isspace(c1)) {
            *(p - 1) = '\0';
            break;
          }
        }
      }
      /*---------------------Ajout dans les structures-------------------*/
      char *w = malloc(sizeof(char) * (long unsigned int) word_length_max + 1);
      *(w + word_length_max) = '\0';
      memcpy(w, w1, (long unsigned int) word_length_max * sizeof(char)); //pour justement couper les mots depassant la longueur mùaximale
      *(w + word_length_max) = '\0';
      free(w1);
      char *cptr = (char *) hashtable_search(ht, w);
      if (cptr != NULL) {
        //SI LE MOT EXISTE
        (*(cptr + ln))++; //mise a jour multiplicité
        *(cptr + i) = 'x';
      } else { //SI LE MOT EXISTE PAS
        char *s = malloc(strlen(w) + 1);
        if (s == NULL) {
          return EXIT_FAILURE;
        }
        strcpy(s, w);
        char *cptr1 = tab_occ_mot + ((int) holdall_count(has)) * (ln + 1);
        if (hashtable_add(ht, s, cptr1) == NULL) {
          free(s);
          return EXIT_FAILURE;
        }
        *(cptr1 + ln) = '1';
        *(cptr1 + i) = 'x';
        if (holdall_put(has, s) != 0) {
          free(s);
          return EXIT_FAILURE;
          free(w);
        }
      }
      free(w);
    }
  }
  /*-----------------------Tri et affichage----------------------------*/
  //holdall_sort(has, ((int (*)(const void *, const void *))strcmp));
  if (holdall_apply_context2(has,
      ht, (void *(*)(void *, void *))hashtable_search,
      (void *) &nb_fichier_ap,
      (int (*)(void *, void *, void *))scptr_display) != 0) {
    return EXIT_FAILURE;
  }
  /*---------------------Libération de l'espace mémoire---------------------*/
  free(tab_occ_mot);
  if (has != NULL) {
    holdall_apply(has, rfree);
  }
  holdall_dispose(&has);
  hashtable_dispose(&ht);
  /*-----------------------Fermeture des fichiers------------------------*/
  while (ln >= 1) {
    int ferme = fclose(f[ln - 1]);
    if (ferme) {
      fprintf(stderr, "erreur de fermeture");
      return EXIT_FAILURE;
    }
    --ln;
  }
}

/*-------------------------------Fonctions---------------------------------*/
size_t str_hashfun(const char *s) {
  size_t h = 0;
  for (const unsigned char *p = (const unsigned char *) s; *p != '\0'; ++p) {
    h = 37 * h + *p;
  }
  return h;
}

int scptr_display(int *nb_mot, const char *s, char *cptr) {
  int c = (int) 'c';
  int k = 0;
  int j = 0;
  while (!isdigit(c) && c != '\0') {
    if (c == 'x') {
      j++;
    }
    c = (int) *(cptr + k);
    k++;
  }
  if (j > 1) {
    char occ[k + 1];
    int nb_m = *nb_mot;
    while (nb_m > 0) {
      nb_m--;
      *nb_mot = nb_m;
      memcpy(occ, cptr, (long unsigned int) (k-1) * sizeof(char));
      return printf("%s\t%c\t%s\n", occ, (char) c, s) < 0;
    }
  }
  return EXIT_SUCCESS;
}

int rfree(void *ptr) {
  free(ptr);
  return 0;
}
