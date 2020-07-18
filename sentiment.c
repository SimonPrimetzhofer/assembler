#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define TEXT_BUFFER_SIZE 1000
#define PHRASES_BUFFER_SIZE 30


/* struct definitions */
struct phrase_dictionary {
    char *text;
    int numOfWords;
    int score;
};

/* typedef */
typedef struct phrase_dictionary phrase_dictionary_t;

/* function definitions */
FILE *openFile(char *filename);
void print(phrase_dictionary_t **dictionary, int dictionarySize, int score);
int calculateScore(phrase_dictionary_t **dictionary, int dictionarySize);
int strcasecmp(const char *s1, const char *s2);
void releaseMemory(phrase_dictionary_t **dictionary, int dictionarySize);
phrase_dictionary_t *searchWordInDictionary(phrase_dictionary_t **dictionary, int dictionarySize, char *word);

int main(int argc, char* argv[]){
    FILE *file = NULL;
    char printPhrases = '0';

    /* too less or too much parameters */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "\nmissing operand after '%s'\n\nUsage: %s [-v] phrase-file\n", argv[0], argv[0]);
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

    /* read phrases line by line */
    while(fgets(phrases_input_buffer, PHRASES_BUFFER_SIZE, file) != NULL) {

        /* increase dictionary and check if allocation has worked*/
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

        /* split by tabulator to extract text and score */
        const char *delimiter = "\t";
        char *delimited = strtok(phrases_input_buffer, delimiter);

        /* phrases with more than 30 characters are rejected and throw an error */
        if(strlen(delimited) > PHRASES_BUFFER_SIZE) {
            fprintf(stderr, "Phrase %s rejected due to length > 30!", delimited);
            exit(EXIT_FAILURE);
        }

        dictionary[dictionarySize - 1]->text = (char *) calloc(PHRASES_BUFFER_SIZE, sizeof(char));
        if(dictionary[dictionarySize - 1]->text == NULL) {
            fprintf(stderr, "Memory could not be allocated!");
            exit(EXIT_FAILURE);
        }
        /* copy phrase to dictionary */
        strcpy(dictionary[dictionarySize - 1]->text, delimited);

        /* count number of words */
        int wordCount = 1;
        int i;
        for(i = 0;i < strlen(delimited); i++) {
            if(delimited[i] == ' ' && delimited[i + 1] != ' ')
                wordCount++;
        }
        dictionary[dictionarySize - 1]->numOfWords = wordCount;

        /* get score of phrase (part right of tabulator) */
        delimited = strtok(NULL, delimiter);
        int numberPart = strtol(delimited, NULL, 10);
        if(numberPart != 0) {
            dictionary[dictionarySize - 1]->score = numberPart;
        }
    }

    /* create space to store occurrences */
    phrase_dictionary_t **occurrenceDictionary = calloc(0, sizeof(phrase_dictionary_t *));
    int occurrenceDictionarySize = 0;

    if(occurrenceDictionary == NULL) {
        fprintf(stderr, "Could not allocate memory!");
        exit(EXIT_FAILURE);
    }

    char currentWord[PHRASES_BUFFER_SIZE];
    char **wordContainer = calloc(0, sizeof(char *));
    int containerSize = 0;

    /* read word by word from stdin */
    while(scanf(" %49[^ \t\n]", currentWord) != EOF) {
        containerSize++;
        /* wordcontainer holds all words */
        wordContainer = realloc(wordContainer, containerSize * (sizeof(char *)));
        if(wordContainer == NULL) {
            fprintf(stderr, "Memory could not be reallocated!");
            exit(EXIT_FAILURE);
        }
        wordContainer[containerSize - 1] = calloc(strlen(currentWord), sizeof(char));
        if(wordContainer[containerSize - 1] == NULL) {
            fprintf(stderr, "Memory could not be allocated!");
            exit(EXIT_FAILURE);
        }
        strcpy(wordContainer[containerSize - 1], currentWord);
    }

    int i;
    for(i = 0; i < containerSize; i++) {
        /* check, if words from wordcontainer occur in the dictionary */
        const phrase_dictionary_t *entry = searchWordInDictionary(dictionary, dictionarySize, wordContainer[i]);
        if(entry != NULL) {
            if(entry->numOfWords == 1) {
                occurrenceDictionarySize++;
                occurrenceDictionary = realloc(occurrenceDictionary, occurrenceDictionarySize * sizeof(phrase_dictionary_t *));
                if(occurrenceDictionary == NULL) {
                    fprintf(stderr, "Memory could not be reallocated!");
                    exit(EXIT_FAILURE);
                }
                occurrenceDictionary[occurrenceDictionarySize - 1] = calloc(1, sizeof(phrase_dictionary_t));
                if(occurrenceDictionary[occurrenceDictionarySize - 1] == NULL) {
                    fprintf(stderr, "Memory could not be allocated!");
                    exit(EXIT_FAILURE);
                }
            }

            /* check if dictionary entry consists of more than one word */
            if(entry->numOfWords > 1) {
                if(i + entry->numOfWords > containerSize)
                    continue;
                int index;
                for(index = i + 1; index < i + entry->numOfWords; index++) {
                    if(strstr(entry->text, wordContainer[index]) != NULL) {
                        /* special handling of multi-word phrases */
                        occurrenceDictionarySize++;
                        occurrenceDictionary = realloc(occurrenceDictionary, occurrenceDictionarySize * sizeof(phrase_dictionary_t *));
                        if(occurrenceDictionary == NULL) {
                            fprintf(stderr, "Memory could not be reallocated!");
                            exit(EXIT_FAILURE);
                        }
                        occurrenceDictionary[occurrenceDictionarySize - 1] = calloc(1, sizeof(phrase_dictionary_t));
                        if(occurrenceDictionary[occurrenceDictionarySize - 1] == NULL) {
                            fprintf(stderr, "Memory could not be allocated!");
                            exit(EXIT_FAILURE);
                        }
                        memcpy(occurrenceDictionary[occurrenceDictionarySize - 1], entry, sizeof(phrase_dictionary_t));
                    }
                }

            } else {
                memcpy(occurrenceDictionary[occurrenceDictionarySize - 1], entry, sizeof(phrase_dictionary_t));
            }
        }
    }

    /* calculate score */
    int score = calculateScore(occurrenceDictionary, occurrenceDictionarySize);

    /* print */
    print((printPhrases == '1' ? occurrenceDictionary : NULL), occurrenceDictionarySize, score);

    /* free dictionary */
    releaseMemory(dictionary, dictionarySize);

    /* free wordcontainer */
    for(i = 0; i < containerSize; i++) {
        free(wordContainer[i]);
    }
    free(wordContainer);

    return EXIT_SUCCESS;
}

/* open a file */
FILE *openFile(char *filename) {
    FILE *file = fopen(filename, "r");
    if(file == NULL) {
        fprintf(stderr, "\n%s is not a valid file!", filename);
        exit(EXIT_FAILURE);
    }

    return file;
}

/* print phrases in valid json format */
void print(phrase_dictionary_t **dictionary, int dictionarySize, int score) {
    /* opening bracket */
    printf("{");

    /* print items as json array */
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
    /* score is printed always */
    printf("\"score\": %d}\n", score);
}

/* calculate score of given dictionary */
int calculateScore(phrase_dictionary_t **dictionary, int dictionarySize) {
    int i;
    int sum = 0;

    for(i = 0; i < dictionarySize; i++) {
        sum += dictionary[i]->score;
    }

    return sum;
}

/* search for word inside of a dictionary */
phrase_dictionary_t *searchWordInDictionary(phrase_dictionary_t **dictionary, int dictionarySize, char *word) {
    int i;
    for(i = 0; i < dictionarySize; i++) {
        if(strcasecmp(dictionary[i]->text, word) == 0) {
            return dictionary[i];
        } else if(dictionary[i]->numOfWords > 1) { /* for handling multi-word phrases */
            if(strstr(dictionary[i]->text, word) != NULL) {
                return dictionary[i];
            }
        }
    }
    return NULL;
}

/* free memory of a dictionary */
void releaseMemory(phrase_dictionary_t **dictionary, int dictionarySize) {
    int i;
    for(i = 0; i < dictionarySize; i++) {
        free(dictionary[i]->text);
        free(dictionary[i]);
    }
    free(dictionary);
}
