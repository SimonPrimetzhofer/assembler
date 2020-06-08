#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINE_ARGS 1
#define NAME_LENGTH 20
#define INPUT_BUFFER_LENGTH 50

/* struct definitions */

/* holds the personal information of a person */
/* lastname and firstname are allocated dynamically later on */
struct person {
    char *lastname;
    char *firstname;
    int year;
    int month;
    int day;
};

/* represents one person with it's friends and additional data */
struct traversal_node {
    int id;
    struct person info;
    char visited;
    int friendcount;
    struct traversal_node **friends;
};

/* typedefs for better readability later on */
typedef struct person person_t;
typedef struct traversal_node traversal_node_t;

/* Function definitions */
static FILE *openFile(const char *file_path, const char *permissions);
static void close_file(FILE *file);
static void handleError(char *message);
static void freeMemory();

static int addRelationship(traversal_node_t *person1, traversal_node_t* person2);
static void traverseDepth(traversal_node_t *node, int depth);
static void sort();

static void printData();
static void printFriendsByNode(traversal_node_t *node);


/*
    purpose: comparator function for qsort
    author: simon primetzhofer
    input: passed in from qsort function
           first - pointer to first element as void *
           second - pointer to second element as void *
    output: integer, which indicates the difference between first and second

*/
int compareFunction(const void *first, const void *second) {
    /* cast void-pointer (any type) to the specific traversal_node_t pointer */
    traversal_node_t *node1 = *(traversal_node_t **)first;
    traversal_node_t *node2 = *(traversal_node_t **)second;

    int cmpLastname = strcmp(node1->info.lastname, node2->info.lastname);
    if(cmpLastname == 0) {
        int cmpFirstname = strcmp(node1->info.firstname, node2->info.firstname);
        if(cmpFirstname == 0) {
            int cmpYear = node2->info.year - node1->info.year;
            if(cmpYear == 0) {
                int cmpMonth = node2->info.month - node1->info.month;
                if(cmpMonth == 0) {
                    int cmpDay = node2->info.day - node1->info.day;
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

/*
    purpose: read data from text-file
             create set of structs from file data
             find all friends of certain people up to a certain depth
             sort people by certain criteria
             print result
    author: simon primetzhofer
    input: argument count, arguments
            startId: id of person to start traversing
            depth: depth of traversing based on startId
    output: EXIT_FAILURE - arguments not valid, allocation errors, format errors, ...
            EXIT_SUCCESS - no errror occured and program executed successfully

*/

/* Global variables */
FILE *file = NULL;
traversal_node_t **traversal_nodes = NULL;
int traversal_node_count = 0;

int main(int argc, char **argv){

    /* exactly 4 parameters have to be passed
        program_name (socialgraph.c)
        file_name
        startId
        depth
    */
    if(argc != 4) {
        fprintf(stderr, "Exactly three input parameters have to be passed! Not %d\n", argc);
        return EXIT_FAILURE;
    }
    argv++;

    /* get filepath from input params */
    const char *file_path = *(argv);
    /* open file with read permissions */
    file = openFile(file_path, "r");

    argv++;
    const int startId = strtol(*(argv), NULL, 10);
    /* CHECK STARTID */
    if(startId == 0) {
        handleError("StartID is not a valid number!\n");
        return EXIT_FAILURE;
    }

    argv++;
    const int depth = strtol(*(argv), NULL, 10);
    if(depth == 0) {
        handleError("Depth is not a valid number!\n");
        return EXIT_FAILURE;
    }
    /* depth has to be bigger than one - but i guess that >= 1 is correct, since three_in 1 1 cannot produce the expected result! */
    if(depth < 1) {
        handleError("Depth has to be bigger than one!\n");
        return EXIT_FAILURE;
    }

    /* Reading */
    traversal_nodes = (traversal_node_t **) calloc(1, sizeof(traversal_node_t *));
    if(traversal_nodes == NULL) {
        fprintf(stderr, "Memory could not be allocated!\n");
        return EXIT_FAILURE;
    }
    int count = 0;
    char input_buffer[INPUT_BUFFER_LENGTH];
    const char *peopleDelimiter = ",";
    const char *relationshipDelimiter = "<->";
    char readRelationships = '0';

    for(count = 0;;count++) {

        /* receive line */
        int ret = fscanf(file, "%s", input_buffer);

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

        /* reading people finished, now get relationships */
        if(readRelationships == '1' && tilde == NULL){ /* tilde == NULL checks, if the current line is not the tilde line */
            char *delimited = strtok(input_buffer, relationshipDelimiter);
            int left = strtol(delimited, NULL, 10);
            if(left < 1) return EXIT_FAILURE;
            delimited = strtok(NULL, relationshipDelimiter);
            int right = strtol(delimited, NULL, 10);
            if(right < 1) return EXIT_FAILURE;

            if(traversal_nodes[left-1]->friends == NULL) {
                 /* allocate space for first element, if not done yet */
                traversal_nodes[left-1]->friends = (traversal_node_t **) calloc(1, sizeof(traversal_node_t *));
                if(traversal_nodes[left-1]->friends == NULL) printf("oida");
            }
            if(traversal_nodes[right-1]->friends == NULL) {
                /* allocate space for first element, if not done yet */
                traversal_nodes[right-1]->friends = (traversal_node_t **) calloc(1, sizeof(traversal_node_t *));
                if(traversal_nodes[right-1]->friends == NULL) printf("oida");
            }
            /* add new friend to count */
            traversal_nodes[left-1]->friends[traversal_nodes[left-1]->friendcount] = traversal_nodes[right-1];
            traversal_nodes[right-1]->friends[traversal_nodes[right-1]->friendcount] = traversal_nodes[left-1];
            traversal_nodes[left-1]->friendcount++;
            traversal_nodes[right-1]->friendcount++;

        } else if(readRelationships == '0') { /* read another person | else if, because the tilde line should be skipped */
            traversal_node_count++;
            /* reallocate memory */
            traversal_nodes = (traversal_node_t **) realloc(traversal_nodes, sizeof(traversal_node_t *) * traversal_node_count);
            if(traversal_nodes == NULL) {
                handleError("Nodes pointer could not be reallocated!");
                return EXIT_FAILURE;
            }
            traversal_nodes[count] = (traversal_node_t *) calloc(1, sizeof(traversal_node_t));
            if(traversal_nodes[count] == NULL) {
                handleError("New node could not be added!");
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
                        traversal_nodes[count]->info.firstname = (char *) calloc(1, sizeof(char) * NAME_LENGTH);
                        strncpy(traversal_nodes[count]->info.firstname, strPart, NAME_LENGTH);
                    }
                    else if(traversal_nodes[count]->info.lastname == NULL) {
                        traversal_nodes[count]->info.lastname = (char *) calloc(1, sizeof(char) * NAME_LENGTH);
                        strncpy(traversal_nodes[count]->info.lastname, strPart, NAME_LENGTH);
                    }
                    else {
                        handleError("no space for name!");
                        return EXIT_FAILURE;
                    }

                } else if(numberPart != 0) {
                    if(traversal_nodes[count]->info.year == 0) {
                        traversal_nodes[count]->info.year = numberPart;
                    } else if(traversal_nodes[count]->info.month == 0) {
                        traversal_nodes[count]->info.month = numberPart;
                    } else if(traversal_nodes[count]->info.day == 0) {
                        traversal_nodes[count]->info.day = numberPart;
                    }else {
                        handleError("no space for number!");
                        return EXIT_FAILURE;
                    }
                }
                delimited = strtok(NULL, peopleDelimiter);
            }
        }
    }

    /* all lines read */
    if(startId > traversal_node_count) {
        fprintf(stderr, "Element with id %d does not exist!\n", startId);
        return EXIT_FAILURE;
    }

    /* traverse */
    traversal_node_t *startNode = traversal_nodes[startId - 1];
    if(startNode == NULL) {
        handleError("Element at startId does not exist!\n");
        return EXIT_FAILURE;
    }
    startNode->visited = '1';
    traverseDepth(startNode, depth);


    /* sort */
    sort();

    /* print */
    printData();

    freeMemory();

    fclose(file);
    return EXIT_SUCCESS;
}

/*
    purpose: do cleanup measures on error
    author: simon primetzhofer
    input: message string
    output: void -> terminates program

*/
static void handleError(char *message) {
    /* cleanup */
    close_file(file);
    freeMemory();

    /* print message */
    fprintf(stderr, "%s", message);

    /* exit program */
    exit(EXIT_FAILURE);
}

/*
    purpose: open a file and check for success
    author: simon primetzhofer
    input: file_path -> path to file, permissions: r/w/...
    ouput: FILE pointer or program terminates in handlerror

*/
static FILE *openFile(const char *file_path, const char *permissions) {
    /* open file with given name */
    FILE *file = fopen(file_path, permissions);

    /* check, if file was opened successfully */
    if(file == NULL){
        handleError("file_path is not valid!");
    }
    return file;
}

/*
    purpose: close file
    author: simon primetzhofer
    input: FILE pointer
    output: nothing

*/
static void close_file(FILE *file) {
    int status = fclose(file);

    /* fclose returns EOF on error
        no further handling of EOF case... there is nothing more to do
    */
    if(status == EOF)
        fprintf(stderr, "File could not be closed!\n");
}

/*
    purpose: free allocated memory
    author: simon primetzhofer
    input: none
    output: none
*/
static void freeMemory() {
    int index;
    /* loop for freeing dynamically allocated memory */
    for(index = 0; index < traversal_node_count; index++) {
        /* Free first- and lastname */
        free(traversal_nodes[index]->info.lastname);
        free(traversal_nodes[index]->info.firstname);

        /* traversal_nodes[index]->friends do not have to be free'd here
           since they free themselves */
        /* free traversal_node itself */
        free(traversal_nodes[index]);
    }

    /* free array of traversal_nodes */
    free(traversal_nodes);
}


static int addRelationship(traversal_node_t *person1, traversal_node_t* person2) {
    return 0;
}

/*
    purpose: traverse nodes from given startId up to a certain depth and set visited
    author: simon primetzhofer
    input: node to traverse further into depth
    output: void -> only set visited
    assertion: depth is >= 1 (criteria from assignment)
*/
static void traverseDepth(traversal_node_t *node, int depth) {
    /* if depth is reached, quit */
    /* example:
        depth = 3 -> start_node is visited -> call friends with depth - 1
        depth = 2 -> friends of friends visited -> call friends of friends with initial depth -2
        depth = 1 -> friends of friends of friends -> call with depth = 0
        quit
    */
    if(depth < 1) return;

    /* iterate over the current node's friends
        set them visited
        do the same for the friend's friends, but depth - 1
    */
    int index;
    for(index = 0; index < node->friendcount; index++) {
        /* check if friendnode was already visited */
        if(node->friends[index]->visited != '1') {
            node->friends[index]->visited = '1';
            traverseDepth(node->friends[index], depth-1);
        }
    }
}

/*
    purpose: call library-function qsort
    author: simon primetzhofer
    input: none
    output: none, since qsort doesn't return anything
*/
static void sort() {
    /*
        traversal_nodes: element to be sorted by qsort
        traversal_node_count: number of elements in traversal_nodes
        size of one element which is sorted
        comparator function
    */
    qsort(traversal_nodes, traversal_node_count, sizeof(traversal_node_t *), compareFunction);
}

/*
    purpose: print nodes (id, name, date of birth, friendcount, ids of friends
    author: simon primetzhofer
    input: none
    output: none
*/
static void printData() {
    int index;
    for(index = 0; index < traversal_node_count; index++) {
        if(traversal_nodes[index]->visited == '1') {
            char *friendsWord = traversal_nodes[index]->friendcount == 1 ? "friend: " : traversal_nodes[index]->friendcount == 0 ? "friends" : "friends: ";
            printf("[%d] %s %s, born %d-%d-%d has %d %s", traversal_nodes[index]->id, traversal_nodes[index]->info.firstname, traversal_nodes[index]->info.lastname,
                                                                       traversal_nodes[index]->info.year, traversal_nodes[index]->info.month, traversal_nodes[index]->info.day,
                                                                       traversal_nodes[index]->friendcount, friendsWord);
            printFriendsByNode(traversal_nodes[index]);
        }
    }
}

/*
    purpose: print a node's friends
    author: simon primetzhofer
    input: node which provides its friends
    output: none
*/
static void printFriendsByNode(traversal_node_t *node) {
    /* if a person has no friends, just print newline */
    if(node->friendcount < 1) {
        printf("\n");
        return;
    }

    /* print ids with , */
    int index;
    for(index = 0; index < node->friendcount - 1; index++) {
        printf("%d, ", node->friends[index]->id);
    }
    /* after the last id, print newline */
    printf("%d\n", node->friends[index]->id);
}
