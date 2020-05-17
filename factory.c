#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "recipes.h"

#define RECIPE_LENGTH 214

/* function definitions */
static recipe_t *findRecipe(char *name);
static int produceItem(recipe_t* recipe, int amount);
static void prepareStorage();
static void printStorage(char c);

/* struct definitions */
struct productionStorageItem { /* Storing intermediate results of an item */
    int leftoverCount;
    int producedCount;
    const recipe_t *recipe;
};

/* Global variables */
static char* item_name;
static long int item_count;
static struct productionStorageItem storage[RECIPE_LENGTH];

int main(int argc, char **argv) {
    /* Handle input params */
    /* Check, if number of arguments is valid */
    if(argc < 2 || argc > 3) {
        /* Invalid number of arguments */
        fprintf(stderr, "Invalid number of program arguments (%d)! Required: 1 or 2", (argc-1));
        return EXIT_FAILURE;
    }
    /* increment argv pointer to get to the second input element - the first one is the program name */
    argv++;

    /* Get item_name from input params */
    item_name = *(argv);

    /* item_count was provided */
    if(argc == 3) {
        /* Get to item_count element */
        argv++;
        /* parse long int from input param string */
        item_count = strtol(*argv, NULL, 10);
        /* Check, if a valid item_count was submitted */
        if(item_count < 1 || item_count > 255) {
            fprintf(stderr, "%s is not a valid item_count! Required: 1 <= x <= 255", *argv);
            return EXIT_SUCCESS;
        }
    }else item_count = 1; /* Default item count = 1 */

    /* Lookup recipe */
    recipe_t* found_recipe = findRecipe(item_name);
    /* Check, if recipe was found */
    if(found_recipe == NULL) {
        fprintf(stderr, "Recipe of %s not found!", item_name);
        return EXIT_FAILURE;
    }

    /* Prepare storage for producing the item */
    prepareStorage();

    /* Produce item and calculate ticks */
    int ticks = produceItem(found_recipe, item_count);

    /* Print output */
    printStorage('1'); /* print leftover materials */
    printf("\n");
    printStorage('0'); /* print used materials */

    printf("\n%d ticks", ticks); /* print overall ticks */

    return EXIT_SUCCESS;
}

/*
    purpose: prepare storage space for storing intermediate results
             initialise counter variables with 0 and store the recipe
             from same index of recipes.c recipes
    input: none
    output: none
*/
static void prepareStorage() {
    int i;
    for(i = 0; i < RECIPE_LENGTH; i++) {
        storage[i].leftoverCount = 0;
        storage[i].producedCount = 0;
        storage[i].recipe = &recipes[i];
    }
}

/*
    purpose: Find recipe in recipes.c recipes[]
    input: name of the recipe
    output: pointer to recipe or NULL, if the recipe was not found
    external dependency: strcmp for comparing recipe names
*/
static recipe_t *findRecipe(char *name) {
    int i;
    for(i = 0;i < RECIPE_LENGTH; i++) {
        if(strcmp(item_name, recipes[i].name) == 0) {
            return &recipes[i];
        }
    }
    return NULL;
}

/*
    purpose: find storage space for item
    input: name of recipe
    output: pointer to struct productionStorageItem or NULL, if storage space was not found
*/
static struct productionStorageItem *findStoragePlace(char *name){
    int i;
    for(i = 0;i < RECIPE_LENGTH; i++) {
        if(strcmp(name, storage[i].recipe->name) == 0) {
            return &storage[i];
        }
    }
    return NULL;
}

/*
    purpose: print storage data to stdout (leftover or used materials
    input: character 1 or 0 -> 1 = leftover materials, 0 = used materials
    output: none
*/
static void printStorage(char leftOver) {
    int i;
    char printHeader = '1'; /* variable for checking, if header has to be printed */

    for(i = 0; i < RECIPE_LENGTH; i++) {
        /* Only print leftover items which have a value > 0 */
        if(leftOver == '1' && storage[i].leftoverCount > 0){
            /* print header, if in the first iteration */
            if(printHeader == '1'){
                printHeader = '0';
                fprintf(stdout, "%s\n", leftOver == '1' ? "leftover materials" : "used materials");
            }
            fprintf(stdout, "%s: %d\n", storage[i].recipe->name, storage[i].leftoverCount);
        }
        else if(leftOver == '0' && storage[i].producedCount > 0) {
            if(printHeader == '1'){
                printHeader = '0';
                fprintf(stdout, "%s\n", leftOver == '1' ? "leftover materials" : "used materials");
            }
            fprintf(stdout, "%s: %d\n", storage[i].recipe->name, storage[i].producedCount);
        }
    }
}

/*

*/
static int produceItem(recipe_t* recipe, int amount){

    int ticks = 0;
    struct productionStorageItem* storageItem = findStoragePlace(recipe->name);
    ingredient_list_node_t *currentIngredient = recipe->ingredients;

    /* Calculate yield - check if amount is a multiple of yield */
    int yieldRest = amount % recipe->yield;
    /* amount is not a multiple of yield */
    if(amount % recipe->yield != 0) {

        /* amount == yieldRest, if amount is smaller than yield -> get difference then */
        if(amount == yieldRest)
            yieldRest = recipe->yield - amount;
        /* Add rest to amount to reach amount % yield == 0 for division */
        amount += yieldRest;
        /* yieldRest is added to the item's leftover count */
        storageItem->leftoverCount += yieldRest;
    }
    /* Divide by yield to get correct amount values */
    amount /= recipe->yield;

    /* Check, if the current item has no further ingredients -> is a raw material */
    if(currentIngredient == NULL){
        storageItem->producedCount += amount;
        return recipe->time * amount; /* equals return 0, since recipe->time of a raw material is 0 */
    }

    /* Add overall ticks of parent item */
    ticks += amount * recipe->time;

    /* Iterate over next elements in the ingredients list */
    while(currentIngredient->next != NULL) {
        /* Add ticks of ingredient */
        ticks += produceItem(currentIngredient->item, amount * currentIngredient->amount);
        /* Next element */
        currentIngredient = currentIngredient->next;
    }
    /* Last element */
    ticks += produceItem(currentIngredient->item, amount * currentIngredient->amount);

    return ticks;
}

