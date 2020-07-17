#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_BUFFER_SIZE 1000
#define PHRASES_BUFFER_SIZE 100


/* struct definitions */

/* function definitions */
FILE *openFile(char *filename);

int main(int argc, char* argv[]){
    FILE *file = NULL;
    char printPhrases = '0';

    /* too less or too much parameters */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "\nmissing operand after './sentiment'\n\nUsage: ./sentiment [-v] phrase-file");
        exit(EXIT_SUCCESS);
    }

    argv++;

    /* handle input params */

    if(argc == 2) {
        file = openFile(*(argv));
    } else if(argc == 3) {
        if(strcmp(*(argv), "-v") == 0) {
            printPhrases = '1';
            argv++;
            file = openFile(*(argv));
        }
    }

    /* read phrases */
    char phrases_input_buffer[PHRASES_BUFFER_SIZE];

    while(fgets(phrases_input_buffer, PHRASES_BUFFER_SIZE, file) != NULL) {
        char currentPhrase[50];
        int currentScore = 0;
        sscanf(phrases_input_buffer, "%s\t%d", currentPhrase, &currentScore);

        fprintf(stdout, "phrase %s, score %d\n", currentPhrase, currentScore);
    }
    /*printf("\n");

    while(fgets(input_buffer, TEXT_BUFFER_SIZE, stdin) != NULL) {
        fprintf(stdout, "%s\n", input_buffer);

    }*/

    return EXIT_SUCCESS;
}

FILE *openFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        fprintf(stderr, "\n%s is not a valid file!", filename);
        exit(EXIT_FAILURE);
    }

    return file;
}
