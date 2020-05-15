#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "recipes.h"

#define RECIPE_LENGTH 214

/* function definitions */
static int produceItem(recipe_t* recipe);
static int computeIngredients(ingredient_list_node_t* ingredient, int amount);

/* struct definition */
struct productionStorageItem {
    int leftoverCount;
    int producedCount;
    recipe_t recipe;
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

/*

    Store data during recursion -> do not modify the data from recipe.c!
    Only add time in recursion -> no printing -> has to be done afterwards
    Print afterwards

*/

/* Produce item by recipe */
static int produceItem(recipe_t* recipe) {
    if(item_count % recipe->yield == 0)
        return item_count * (recipe->time + computeIngredients(recipe->ingredients, 1));
    fprintf(stdout, "leftover part %s\n", recipe->name);
    return (item_count + item_count % recipe->yield)/recipe->yield * (recipe->time + computeIngredients(recipe->ingredients, 1));
}

/* Calculate ticks for producing the subelements */
static int computeIngredients(ingredient_list_node_t* ingredient, int amount) {
    /* Last ingredient in list was reached */
    printf("%s %d\n", ingredient->item->name, ingredient->amount);
    /* Check, if raw material was reached */
    if(ingredient->item->ingredients == NULL) {
        printf("%s: %d\n", ingredient->item->name, item_count * amount);
        return ingredient->item->time; /* return 0 */
    }
    /* check if next ingredient is not there */
    if(ingredient->next == NULL) {
        /*return ingredient->amount * (ingredient->item->time + computeIngredients(ingredient->item->ingredients, 1));*/
        return ingredient->item->time + (ingredient->amount * produceItem(ingredient->item));
    }

    /* ingredient->amount evtl. removen */
    /*return*/ /*ingredient->amount */ /*(ingredient->item->time + computeIngredients(ingredient->item->ingredients, 1)) + computeIngredients(ingredient->next, 1);*/
    return ingredient->item->time + (produceItem(ingredient->item) * ingredient->amount) + ingredient->next->item->time + (ingredient->next->amount * produceItem(ingredient->next->item));
}
