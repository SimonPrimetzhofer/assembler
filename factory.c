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

/* struct definition */
struct productionStorageItem {
    int leftoverCount;
    int producedCount;
    const recipe_t *recipe;
};

/* Global variables */
static char* item_name;
static int item_count;
static struct productionStorageItem storage[RECIPE_LENGTH];

int main(int argc, char **argv) {
    /* Handle input params */
    /* Check, if number of arguments is valid */
    if(argc < 2 || argc > 3) {
        /* Invalid number of arguments */
        fprintf(stderr, "Invalid number of program arguments (%d)! Required: 1 or 2", (argc-1));
        return EXIT_FAILURE;
    }
    argv++;

    /* Get item_name from input params */
    item_name = *(argv);
    /* fprintf(stdout, "%s\n", item_name); */
    argv++;

    /* item_count was provided */
    if(argc == 3) {
        item_count = strtol(*argv, NULL, 10);
        /* Check, if a valid item_count was submitted */
        if(item_count < 1) {
            fprintf(stderr, "%s is not a valid item_count! Required: int >= 1", *argv);
            return EXIT_SUCCESS;
        }
    }else item_count = 1;

    /* Lookup recipe */
    recipe_t* found_recipe = findRecipe(item_name);

    if(found_recipe == NULL) {
        fprintf(stderr, "Recipe of %s not found!", item_name);
        return EXIT_FAILURE;
    }

    /* Prepare storage for producing the item */
    prepareStorage();

    /* Produce item and calculate ticks */
    int ticks = produceItem(found_recipe, item_count);

    /* Print output */
    printStorage('1');
    printf("\n");
    printStorage('0');

    printf("\n%d ticks", ticks);

    return EXIT_SUCCESS;

}

static recipe_t *findRecipe(char *name) {
    int i = 0;
    for(i = 0;i < RECIPE_LENGTH; i++) {
        if(strcmp(item_name, recipes[i].name) == 0) {
            return &recipes[i];
        }
    }
    return NULL;
}

static struct productionStorageItem *findStoragePlace(char *name){
    int i = 0;
    for(i = 0;i < RECIPE_LENGTH; i++) {
        if(strcmp(name, storage[i].recipe->name) == 0) {
            return &storage[i];
        }
    }
    return NULL;
}

static void printStorage(char leftOver) {
    int i;
    char printHeader = '1';

    for(i = 0; i < RECIPE_LENGTH; i++) {
        if(leftOver == '1' && storage[i].leftoverCount > 0){
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

static void prepareStorage() {
    int i;
    for(i = 0; i < RECIPE_LENGTH; i++) {
        storage[i].leftoverCount = 0;
        storage[i].producedCount = 0;
        storage[i].recipe = &recipes[i];
    }
}

/*

    Store data during recursion -> do not modify the data from recipe.c!
    Only add time in recursion -> no printing -> has to be done afterwards
    Print afterwards

*/

static int produceItem(recipe_t* recipe, int amount){

    struct productionStorageItem* storageItem = findStoragePlace(recipe->name);
    printf("%s %s\n", storageItem->recipe->name, recipe->name);

    /* Item is in storage and therefore, check leftover parts */
    if(storageItem == NULL) {
        return 0;
    }
    printf("yield: %d\n", recipe->yield);
    /* Check, if leftover parts are there */
    int yieldQuotient = 1;
    if(storageItem->leftoverCount > 0)
        yieldQuotient = amount % recipe->yield;

    /* Yield Quotient is not a divisor of amount*/
    if(amount % recipe->yield != 0) {
        amount += yieldQuotient;
        amount /= recipe->yield;
        storageItem->leftoverCount += yieldQuotient;
    }

    printf("amount: %d, yieldquo: %d\n", amount, yieldQuotient);

    /* No further ingredients */
    if(recipe->ingredients == NULL) {
        printf("no furhter ingredients: %s %d\n", recipe->name, amount * recipe->time);
        printf("storage item name: %s\n", storageItem->recipe->name);
        storageItem->producedCount += amount;
        return amount * recipe->time;
    }

    if(recipe->ingredients->next == NULL) {
        printf("last ingredient: %s %d\n", recipe->name, amount);
        return (amount * recipe->time)
            + produceItem(recipe->ingredients->item, amount * recipe->ingredients->amount);
    }

    printf("ois nu do: %s\n", recipe->name);

    return ((amount * recipe->time) + produceItem(recipe->ingredients->next->item, recipe->ingredients->next->amount * amount))
        + ((amount * recipe->time) + produceItem(recipe->ingredients->item, recipe->ingredients->amount * amount));

}

