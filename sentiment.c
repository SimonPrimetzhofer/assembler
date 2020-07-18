#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#define TEXT_BUFFER_SIZE 1000
#define PHRASES_BUFFER_SIZE 30


/* struct definitions */
struct phrase_dictionary {
    char *text;
    int score;
};

/* typedef */
typedef struct phrase_dictionary phrase_dictionary_t;

/* function definitions */
FILE *openFile(char *filename);
void print(phrase_dictionary_t **dictionary, int dictionarySize, int score);
int calculateScore(phrase_dictionary_t **dictionary, int dictionarySize);
void releaseMemory(phrase_dictionary_t **dictionary, int dictionarySize);

int main(int argc, char* argv[]){
    FILE *file = NULL;
    char printPhrases = '0';

    /* too less or too much parameters */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "\nmissing operand after './%s'\n\nUsage: ./%s[-v] phrase-file", argv[0], argv[0]);
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
    phrase_dictionary_t **dictionary = calloc(0, sizeof(phrase_dictionary_t *));
    int dictionarySize = 0;

    while(fgets(phrases_input_buffer, PHRASES_BUFFER_SIZE, file) != NULL) {
        dictionarySize++;
        dictionary = (phrase_dictionary_t **) realloc(dictionary, (dictionarySize) * sizeof(phrase_dictionary_t *));
        if(dictionary == NULL) {
            fprintf(stderr, "Memory for dictionary could not be reallocated!\n");
            exit(EXIT_FAILURE);
        }
        dictionary[dictionarySize - 1] = (phrase_dictionary_t *) calloc(1, sizeof(phrase_dictionary_t));
        if(dictionary[dictionarySize - 1] == NULL) {
            fprintf(stderr, "Memory for dictionary entry could not be reallocted!\n");
            exit(EXIT_FAILURE);
        }
        const char *delimiter = "\t";
        char *delimited = strtok(phrases_input_buffer, delimiter);

        if(strlen(delimited) > 30) {
            fprintf(stderr, "Phrase %s rejected due to length > 30!", delimited);
            exit(EXIT_FAILURE);
        }

        dictionary[dictionarySize - 1]->text = (char *) calloc(PHRASES_BUFFER_SIZE, sizeof(char));
        strcpy(dictionary[dictionarySize - 1]->text, delimited);

        delimited = strtok(NULL, delimiter);
        int numberPart = strtol(delimited, NULL, 10);
        if(numberPart != 0) {
            dictionary[dictionarySize - 1]->score = numberPart;
        }
    }

    /*int i;
    for(i=0;i<dictionarySize; i++) {
        printf("%s %d\n", dictionary[i]->text, dictionary[i]->score);
    }*/
    char text_input_buffer[TEXT_BUFFER_SIZE];
    phrase_dictionary_t **occurrenceDictionary = calloc(1, sizeof(phrase_dictionary_t *));
    int occurrenceDictionarySize = 0;

    if(occurrenceDictionary == NULL) {
        fprintf(stderr, "Could not allocate memory!");
        exit(EXIT_FAILURE);
    }

    while(fgets(text_input_buffer, TEXT_BUFFER_SIZE, stdin) != NULL) {
        fprintf(stdout, "%s\n", text_input_buffer);


    }

    int score = calculateScore(occurrenceDictionary, occurrenceDictionarySize);

    /* print */
    print((printPhrases == '1' ? occurrenceDictionary : NULL), occurrenceDictionarySize, score);

    releaseMemory(dictionary, dictionarySize);
    releaseMemory(occurrenceDictionary, occurrenceDictionarySize);

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

void print(phrase_dictionary_t **dictionary, int dictionarySize, int score) {
    printf("{");

    /* print items */
    if(dictionary != NULL) {
        printf("\"matches\": [\n");
        int i;
        for(i = 0; i < dictionarySize; i++) {
            printf("{\"phrase\": \"%s\", \"score\": %d}", dictionary[i]->text, dictionary[i]->score);
            if(i != (dictionarySize - 1))
                printf(",\n");
        }
        printf("],\n");
    }
    printf("\"score\": %d}\n", score);
}

int calculateScore(phrase_dictionary_t **dictionary, int dictionarySize) {
    int i;
    int sum = 0;

    for(i = 0; i < dictionarySize; i++) {
        sum += dictionary[i]->score;
    }

    return sum;
}

void releaseMemory(phrase_dictionary_t **dictionary, int dictionarySize) {
    int i;
    for(i = 0; i < dictionarySize; i++) {
        free(dictionary[i]->text);
        free(dictionary[i]);
    }
    free(dictionary);
}
