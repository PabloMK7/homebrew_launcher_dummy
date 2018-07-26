#include "drawableObject.h"
#include "draw.h"

backgroundScreen_t  *newBackgroundObject(sprite_t *background)
{
    backgroundScreen_t  *ret;

    ret = (backgroundScreen_t *)calloc(1, sizeof(backgroundScreen_t));
    if (!ret) goto error;
    
    if (background) ret->background = background;
    ret->draw = drawBackground;
    return (ret);
error:
    return (NULL);
}

bool        drawBackground(void *self)
{
    backgroundScreen_t *bg;

    if (!self) goto error;
    bg = (backgroundScreen_t *)self;
    drawSprite(bg->background);
error:
    return (false);
}

drawableScreen_t    *newDrawableScreen(backgroundScreen_t *background)
{
    drawableScreen_t    *ret;

    ret = (drawableScreen_t *)calloc(1, sizeof(drawableScreen_t));
    if (!ret) goto error;
    if (background) ret->background = background;
    ret->draw = drawScreen;
    return (ret);
error:
    return (NULL);
}

bool        drawScreen(void *self)
{
	drawableScreen_t    *screen;

	if (!self) goto error;
	screen = (drawableScreen_t *)self;
	if (screen->background)
		screen->background->draw(screen->background);
	return (true);
error:
	return (false);
}