#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "recipes.h"

#define RECIPE_LENGTH 214

/* function definitions */
static int produceItem(recipe_t* recipe);

/* Global variables */
static char* item_name;
static int item_count;

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
    int i;
    recipe_t* found_recipe = NULL;
    for(i = 0;i < RECIPE_LENGTH; i++) {
        if(strcmp(item_name, recipes[i].name) == 0) {
            found_recipe = &recipes[i];
            break;
        }
    }

    if(found_recipe == NULL) {
        fprintf(stderr, "Recipe of %s not found!", item_name);
        return EXIT_FAILURE;
    }

    printf("used materials\n");
    int ticks = produceItem(found_recipe);
    printf("\n\n%d ticks", ticks);

    return EXIT_SUCCESS;

}

static int produceItem(recipe_t* recipe) {
    fprintf(stdout, "%s: %d", recipe->ingredients->item->name, recipe->ingredients->amount);

    if(recipe->ingredients->next == NULL){
        return recipe->time + (recipe->ingredients->item->time * recipe->ingredients->amount);
    }

    return recipe->time + (recipe->ingredients->item->time * recipe->ingredients->amount) + produceItem(recipe->ingredients->next->item);
}
