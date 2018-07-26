#include "graphics.h"
#include "drawableObject.h"


sprite_t         *bottomSprite = NULL;
sprite_t         *topSprite = NULL;
drawableScreen_t *botScreen = NULL;
drawableScreen_t *topScreen = NULL;

void    initUI(void)
{
    backgroundScreen_t *bg;

	newSpriteFromPNG(&topSprite, "romfs:/top.png");
	newSpriteFromPNG(&bottomSprite, "romfs:/bottom.png");

	setSpritePos(topSprite, 0, 0);
	setSpritePos(bottomSprite, 0, 0);

	topSprite->drawColor = 0xFF000000;
	bottomSprite->drawColor = 0xFF000000;
    
    bg = newBackgroundObject(bottomSprite);
    botScreen = newDrawableScreen(bg);
    bg = newBackgroundObject(topSprite);
    topScreen = newDrawableScreen(bg);

    updateUI();
}

void setExitMode() {
	botScreen->background->background->isGreyedOut = true;
	topScreen->background->background->isGreyedOut = true;
}

void handleFadeIn(s64 time, u64 tot) {
	float ratio = (float)(tot - time) / tot;
	u8 col = (u8)(ratio * 0xff);
	u32 finalcol = (0xFF << 24) | (col << 16) | (col << 8) | (col);
	topScreen->background->background->drawColor = finalcol;
	botScreen->background->background->drawColor = finalcol;
}


void    exitUI(void)
{
    deleteSprite(bottomSprite);
    deleteSprite(topSprite);
}

static inline void drawUITop(void)
{
    setScreen(GFX_TOP);
    
    topScreen->draw(topScreen);
}

static inline void drawUIBottom(void)
{
    setScreen(GFX_BOTTOM);
    
    botScreen->draw(botScreen);
}

int   updateUI(void)
{
    hidScanInput();
    drawUITop();
    drawUIBottom();
    updateScreen();
    return (1);
}