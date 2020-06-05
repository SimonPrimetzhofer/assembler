#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE_ARGS 1
#define INPUT_BUFFER_LENGTH 50
/* struct definitions */
struct person {
    char *lastname;
    char *firstname;
    int year;
    int month;
    int day;
};

struct traversal_node {
    int id;
    struct person info;
    char visited;
    struct traversal_node *nextFriend;
};

/* typedefs */
typedef struct person person_t;
typedef struct traversal_node traversal_node_t;

/* Function definitions */
static void handleError(FILE *file, char *message);
static void addRelationship(traversal_node_t *person1, traversal_node_t* person2);
static void printData(traversal_node_t **nodes, int count);

int main(int argc, char **argv){

    if(argc != 4) {
        fprintf(stderr, "Exactly three input parameters have to be passed! Not %d\n", argc);
        return EXIT_FAILURE;
    }
    argv++;

    const char *filePath = *(argv);
    FILE *data = fopen(filePath, "r");

    if(data == NULL){
        fprintf(stderr, "File path is not valid!\n");
        return EXIT_FAILURE;
    }
    printf("%s\n", filePath);

    argv++;
    const int startId = strtol(*(argv), NULL, 10);
    /* CHECK STARTID */
    if(startId == 0) {
        handleError(data, "StartID is not a valid number!\n");
        return EXIT_FAILURE;
    }
    printf("%d\n", startId);

    argv++;
    const int depth = strtol(*(argv), NULL, 10);
    if(depth == 0) {
        handleError(data, "Depth is not a valid number!\n");
        return EXIT_FAILURE;
    }
    printf("%d\n", depth);

    /* Reading */
    traversal_node_t **traversal_nodes = (traversal_node_t **) malloc(sizeof(traversal_node_t *));
    if(traversal_nodes == NULL) {
        return EXIT_FAILURE;
    }
    int count = 0;
    int elementsCount = 0;
    char input_buffer[INPUT_BUFFER_LENGTH];
    const char *delimiter = ",";
    char readRelationships = '0';

    for(count = 0;;count++) {

        /* receive line */
        int ret = fscanf(data, "%s", input_buffer);

        if(ret == EOF)
            break;

        /* Check for delimiter line which contains at least one ~ */
        char *tilde = strchr(input_buffer, (int) '~');
        if(tilde != NULL) {
            readRelationships = '1';
        }

        /* reading people finished, not get relationships */
        if(readRelationships == '1' && tilde == NULL){
            printf("current relationline %s\n", input_buffer);

        } else if(readRelationships == '0') { /* read another person | else if, because the tilde line should be skipped */
            printf("current personline %s\n", input_buffer);
            elementsCount++;
            /* reallocate memory */
            traversal_nodes = (traversal_node_t **) realloc(traversal_nodes, sizeof(traversal_node_t *) * elementsCount);
            traversal_nodes[count] = (traversal_node_t *) malloc(sizeof(traversal_node_t));
            if(traversal_nodes[count] == NULL){
                return EXIT_FAILURE;
            }

            traversal_nodes[count]->id = count + 1;

            /* split input buffer by ',' */
            char *delimited = strtok(input_buffer, delimiter);
            /* get string parts split by ',' */
            while(delimited != NULL) {
                char *strPart;
                int numberPart = strtol(delimited, &strPart, 10);

                if(*strPart != '\0') {
                    printf("stringpart %s\n", strPart);
                    /* Check if firstname or lastname has to be set */
                    if(traversal_nodes[count]->info.firstname == NULL) {
                        traversal_nodes[count]->info.firstname = (char *) malloc(strlen(strPart));
                        strcpy(traversal_nodes[count]->info.firstname, strPart);
                    }
                    else if(traversal_nodes[count]->info.lastname == NULL) {
                        traversal_nodes[count]->info.lastname = (char *) malloc(strlen(strPart));
                        strcpy(traversal_nodes[count]->info.lastname, strPart);
                    }
                    /* else error */
                } else if(numberPart != 0) {
                    /*if(traversal_nodes[count].info.year == NULL)*/
                }
                delimited = strtok(NULL, delimiter);
            }
        }
    }

    /* all lines read */

    /* traverse */

    /* sort */

    /* print */
    printData(traversal_nodes, elementsCount);

    free(traversal_nodes);
    fclose(data);
    return EXIT_SUCCESS;
}

static void addRelationship(traversal_node_t *person1, traversal_node_t* person2) {
}

static void printData(traversal_node_t **nodes, int count) {
    printf("%d\n", count);
    int index;
    for(index = 0; index < count; index++) {
        printf("%d %s %s\n", nodes[index]->id, nodes[index]->info.firstname, nodes[index]->info.lastname);
    }
}

static void handleError(FILE *data, char *message) {
    if(data != NULL){
        fclose(data);
    }
    fprintf(stderr, "%s", message);
}
