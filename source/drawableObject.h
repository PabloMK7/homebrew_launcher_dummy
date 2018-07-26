#ifndef DRAWABLEOBJECT_H
#define DRAWABLEOBJECT_H
#define MAX_ELEMENTS 15

#include "draw.h"

typedef struct  drawableObject_s
{
    bool        (*draw)(void *self);
}               drawableObject_t;

typedef struct   backgroundScreen_s
{
    /* herited from drawableObject_t */
    bool        (*draw)(void *self);

    sprite_t     *background;
    sprite_t     *headerText;
    sprite_t     *footerText;

}                backgroundScreen_t;

typedef struct  drawableScreen_s
{
    /* herited from drawableObject_t */
    bool        (*draw)(void *self);

    backgroundScreen_t  *background;
    int                 elementsCount;
    int                 elementList[MAX_ELEMENTS];
}               drawableScreen_t;

backgroundScreen_t  *newBackgroundObject(sprite_t *background);
bool        drawBackground(void *self);

drawableScreen_t    *newDrawableScreen(backgroundScreen_t *background);
bool        drawScreen(void *self);

#endif
