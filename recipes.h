#ifndef RECIPES_H
#define RECIPES_H

typedef const struct recipe recipe_t;
typedef const struct ingredient_list_node ingredient_list_node_t;

struct recipe {
    char* name;
    int time;
    int yield;
    ingredient_list_node_t* ingredients;
};

struct ingredient_list_node {
    int amount;
    recipe_t* item;
    ingredient_list_node_t* next;
};

extern const recipe_t recipes[214];

#endif /* RECIPES_H */
