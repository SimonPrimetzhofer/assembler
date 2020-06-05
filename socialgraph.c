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
    int friendcount;
    struct traversal_node **friends;
};

/* typedefs */
typedef struct person person_t;
typedef struct traversal_node traversal_node_t;

/* Function definitions */
static void handleError(FILE *file, char *message);
static void addRelationship(traversal_node_t *person1, traversal_node_t* person2);
static void printData(traversal_node_t **nodes, int count);
static void printFriendsByNode(traversal_node_t *node);

int compareFunction(const void *first, const void *second) {
    traversal_node_t *node1 = *(traversal_node_t **)first;
    traversal_node_t *node2 = *(traversal_node_t **)second;

    int cmpLastname = strcmp(node1->info.lastname, node2->info.lastname);
    if(cmpLastname == 0) {
        int cmpFirstname = strcmp(node1->info.firstname, node2->info.firstname);
        if(cmpFirstname == 0) {
            int cmpYear = node1->info.year - node2->info.year;
            if(cmpYear == 0) {
                int cmpMonth = node1->info.month - node2->info.month;
                if(cmpMonth == 0) {
                    int cmpDay = node1->info.day - node2->info.day;
                    if(cmpDay == 0) {
                        int cmpFriends = node1->friendcount - node2->friendcount;
                        return cmpFriends;
                    }
                    return cmpDay;
                }
                return cmpMonth;
            }
            return cmpYear;
        }
        return cmpFirstname;
    }

    return cmpLastname;
}

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

    argv++;
    const int startId = strtol(*(argv), NULL, 10);
    /* CHECK STARTID */
    if(startId == 0) {
        handleError(data, "StartID is not a valid number!\n");
        return EXIT_FAILURE;
    }

    argv++;
    const int depth = strtol(*(argv), NULL, 10);
    if(depth == 0) {
        handleError(data, "Depth is not a valid number!\n");
        return EXIT_FAILURE;
    }

    /* Reading */
    traversal_node_t **traversal_nodes = (traversal_node_t **) malloc(sizeof(traversal_node_t *));
    if(traversal_nodes == NULL) {
        return EXIT_FAILURE;
    }
    int count = 0;
    int elementsCount = 0;
    char input_buffer[INPUT_BUFFER_LENGTH];
    const char *peopleDelimiter = ",";
    const char *relationshipDelimiter = "<->";
    char readRelationships = '0';

    for(count = 0;;count++) {

        /* receive line */
        int ret = fscanf(data, "%s", input_buffer);

        if(ret == EOF){
            if(readRelationships == '0'){
                fprintf(stderr, "~ delimiter line is missing!");
                return EXIT_FAILURE;
            }
            break;
        }

        /* Check for delimiter line which contains at least one ~ */
        char *tilde = strchr(input_buffer, (int) '~');
        if(tilde != NULL) {
            readRelationships = '1';
        }

        /* reading people finished, not get relationships */
        if(readRelationships == '1' && tilde == NULL){
            char *delimited = strtok(input_buffer, relationshipDelimiter);
            int left = strtol(delimited, NULL, 10);
            if(left < 1) return EXIT_FAILURE;
            delimited = strtok(NULL, relationshipDelimiter);
            int right = strtol(delimited, NULL, 10);
            if(right < 1) return EXIT_FAILURE;

            if(traversal_nodes[left-1]->friends == NULL) {
                 /* allocate space for first element, if not done yet */
                traversal_nodes[left-1]->friends = (traversal_node_t **) malloc(sizeof(traversal_node_t *));
            }
            /* add new friend to count */
            traversal_nodes[left-1]->friends[traversal_nodes[left-1]->friendcount] = traversal_nodes[right-1];
            traversal_nodes[left-1]->friendcount++;



        } else if(readRelationships == '0') { /* read another person | else if, because the tilde line should be skipped */
            elementsCount++;
            /* reallocate memory */
            traversal_nodes = (traversal_node_t **) realloc(traversal_nodes, sizeof(traversal_node_t *) * elementsCount);
            traversal_nodes[count] = (traversal_node_t *) malloc(sizeof(traversal_node_t));

            if(traversal_nodes == NULL || traversal_nodes[count] == NULL){
                return EXIT_FAILURE;
            }

            traversal_nodes[count]->id = count + 1;

            /* split input buffer by ',' */
            char *delimited = strtok(input_buffer, peopleDelimiter);
            /* get string parts split by ',' */
            while(delimited != NULL) {
                char *strPart;
                int numberPart = strtol(delimited, &strPart, 10);

                if(*strPart != '\0') {
                    /* Check if firstname or lastname has to be set */
                    if(traversal_nodes[count]->info.firstname == NULL) {
                        traversal_nodes[count]->info.firstname = (char *) malloc(strlen(strPart));
                        strcpy(traversal_nodes[count]->info.firstname, strPart);
                    }
                    else if(traversal_nodes[count]->info.lastname == NULL) {
                        traversal_nodes[count]->info.lastname = (char *) malloc(strlen(strPart));
                        strcpy(traversal_nodes[count]->info.lastname, strPart);
                    }
                    else return EXIT_FAILURE;

                } else if(numberPart != 0) {
                    if(traversal_nodes[count]->info.year == 0) {
                        traversal_nodes[count]->info.year = numberPart;
                    } else if(traversal_nodes[count]->info.month == 0) {
                        traversal_nodes[count]->info.month = numberPart;
                    } else if(traversal_nodes[count]->info.day == 0) {
                        traversal_nodes[count]->info.day = numberPart;
                    } else return EXIT_FAILURE;
                }
                delimited = strtok(NULL, peopleDelimiter);
            }
        }
    }

    /* all lines read */
    if(startId > elementsCount) {
        fprintf(stderr, "Element with id %d does not exist!\n", startId);
        return EXIT_FAILURE;
    }

    /* traverse */
    if(depth > 0) {

    }

    /* sort */
    qsort(traversal_nodes, elementsCount, sizeof(traversal_node_t *), compareFunction);

    /* print */
    printData(traversal_nodes, elementsCount);

    free(traversal_nodes);
    fclose(data);
    return EXIT_SUCCESS;
}

static void addRelationship(traversal_node_t *person1, traversal_node_t* person2) {
}

static void printData(traversal_node_t **nodes, int count) {
    int index;
    for(index = 0; index < count; index++) {
        char *friendsWord = nodes[index]->friendcount == 1 ? "friend: " : nodes[index]->friendcount == 0 ? "friends" : "friends: ";
        printf("[%d] %s %s, born %d-%d-%d has %d %s", nodes[index]->id, nodes[index]->info.firstname, nodes[index]->info.lastname,
                                                                       nodes[index]->info.year, nodes[index]->info.month, nodes[index]->info.day,
                                                                       nodes[index]->friendcount, friendsWord);
        printFriendsByNode(nodes[index]);
    }
}

static void printFriendsByNode(traversal_node_t *node) {
    if(node->friendcount < 1) {
        printf("\n");
        return;
    }
    int index;
    for(index = 0; index < node->friendcount - 1; index++) {
        printf("%d, ", node->friends[index]->id);
    }
    printf("%d\n", node->friends[index]->id);
}

static void handleError(FILE *data, char *message) {
    if(data != NULL){
        fclose(data);
    }
    fprintf(stderr, "%s", message);
}
