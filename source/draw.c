#include "draw.h"

static DVLB_s           *vshader_dvlb;
static shaderProgram_s  program;
static int              uLoc_projection;
static textVertex_s     *textVtxArray;
static int              textVtxArrayPos;
static drawTarget_t     top;
static drawTarget_t     bottom;
static bool             frameStarted = false;
static gfxScreen_t      currentScreen = -1;

#define TEXT_VTX_ARRAY_COUNT (8 * 1024)

#define TEX_MIN_SIZE 64

//Grabbed from: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
unsigned int nextPow2(unsigned int v)
{
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return (v >= TEX_MIN_SIZE ? v : TEX_MIN_SIZE);
}

static void addTextVertex(float vx, float vy, float vz, float tx, float ty)
{
    textVertex_s    *vtx;

    vtx = &textVtxArray[textVtxArrayPos++];
    vtx->position[0] = vx;
    vtx->position[1] = vy;
    vtx->position[2] = 0.0f;
    vtx->texcoord[0] = tx;
    vtx->texcoord[1] = ty;
}

static void resetC3Denv() {
	C3D_TexEnv *env;
	for (int i = 0; i < 4; i++) {
		env = C3D_GetTexEnv(i);
		C3D_TexEnvInit(env);
	}
}

static void bindImageGreyScale(C3D_Tex *texture, u32 texture_color) {
	//((0.3 * R) + (0.59 * G) + (0.11 * B)). -> 0xFF1C964C
	C3D_TexEnv  *env;
	u32 greyMask = 0xFF1C964C;
	resetC3Denv();

	C3D_TexBind(0, texture);
	env = C3D_GetTexEnv(0);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_TEXTURE0, GPU_CONSTANT, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, 0, 0);
	C3D_TexEnvOpRgb(env, 0, 0, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	C3D_TexEnvColor(env, texture_color);
	env = C3D_GetTexEnv(1);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_CONSTANT, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, 0, 0);
	C3D_TexEnvOpRgb(env, 0, 0, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	C3D_TexEnvColor(env, greyMask);
	C3D_TexEnvBufUpdate(C3D_RGB, 0b0010);
	env = C3D_GetTexEnv(2);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_PREVIOUS, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, 0, 0);
	C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_R, GPU_TEVOP_RGB_SRC_G, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	env = C3D_GetTexEnv(3);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_PREVIOUS, GPU_PREVIOUS_BUFFER, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_PREVIOUS, 0, 0);
	C3D_TexEnvOpRgb(env, 0, GPU_TEVOP_RGB_SRC_B, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
}

static void bindTexture(C3D_Tex *texture, u32 texture_color)
{
	C3D_TexEnv  *env;
	resetC3Denv();

	C3D_TexBind(0, texture);
	env = C3D_GetTexEnv(0);
	C3D_TexEnvBufUpdate(C3D_RGB, 0);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_TEXTURE0, GPU_CONSTANT, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, 0, 0);
	C3D_TexEnvOpRgb(env, 0, 0, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
	C3D_TexEnvFunc(env, C3D_Alpha, GPU_REPLACE);
	C3D_TexEnvColor(env, texture_color);
}

void setSpritePos(sprite_t *sprite, float posX, float posY)
{
    if (!sprite) return;
    sprite->posX = posX;
    sprite->posY = posY;
}

void drawSprite(sprite_t *sprite)
{
    float       height;
    float       width;
    float       u;
    float       v;
    float       x;
    float       y;
    int         arrayIndex;
    C3D_Tex     *texture;
    if (!frameStarted) return;

    if (!sprite || sprite->isHidden) return;
    texture = &sprite->texture;
    height = sprite->height;
    width = sprite->width;
    x = sprite->posX;
    y = sprite->posY;
    u = width / (float)texture->width;
    v = height / (float)texture->height;

	width = floor(width * sprite->amount);
	u *= sprite->amount;

    C3D_BufInfo *bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, textVtxArray, sizeof(textVertex_s), 2, 0x10);
    //Set the vertices
    arrayIndex = textVtxArrayPos;
    addTextVertex(x, y + height, sprite->depth, 0.0f, v); //left bottom
    addTextVertex(x + width, y + height, sprite->depth, u, v); //right bottom
    addTextVertex(x, y, sprite->depth, 0.0f, 0.0f); //left top
    addTextVertex(x + width, y, sprite->depth, u, 0.0f); //right top

	//Bind the sprite's texture
	if (sprite->isGreyedOut) {
		bindImageGreyScale(texture, sprite->drawColor);
	}
	else {
		bindTexture(texture, sprite->drawColor);
	}

    //Draw 
    C3D_DrawArrays(GPU_TRIANGLE_STRIP, arrayIndex, 4);
}

void drawRectangle(rectangle_t *rectangle)
{
	float       height;
	float       width;
	float       x;
	float       y;
	int         arrayIndex;
	C3D_TexEnv  *env;

    if (!frameStarted) return;

	if (!rectangle) return;
	height = rectangle->height;
	width = ceil(rectangle->width * rectangle->amount);
	x = rectangle->posX;
	y = rectangle->posY;

	C3D_BufInfo *bufInfo = C3D_GetBufInfo();
	BufInfo_Init(bufInfo);
	BufInfo_Add(bufInfo, textVtxArray, sizeof(textVertex_s), 2, 0x10);
	//Set the vertices
	arrayIndex = textVtxArrayPos;
	addTextVertex(x, y + height, rectangle->depth, 0.0f, 1.f); //left bottom
	addTextVertex(x + width, y + height, rectangle->depth, 1.f, 1.f); //right bottom
	addTextVertex(x, y, rectangle->depth, 0.0f, 0.0f); //left top
	addTextVertex(x + width, y, rectangle->depth, 1.f, 0.0f); //right top

	resetC3Denv();
	env = C3D_GetTexEnv(0);
	C3D_TexBind(0, &(rectangle->sprite->texture));
	C3D_TexEnvBufUpdate(C3D_RGB, 0);
	C3D_TexEnvSrc(env, C3D_RGB, GPU_TEXTURE0, 0, 0);
	C3D_TexEnvSrc(env, C3D_Alpha, GPU_CONSTANT, 0, 0);
	C3D_TexEnvOpRgb(env, 0, 0, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
	C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
	C3D_TexEnvColor(env, 0xFFFFFFFF);

	//Draw 
	C3D_DrawArrays(GPU_TRIANGLE_STRIP, arrayIndex, 4);
}

sprite_t *newSprite(int width, int height)
{
    sprite_t    *sprite;
    C3D_Tex     *texture;
    bool        result;

    //Alloc the sprite
    sprite = (sprite_t *)calloc(1, sizeof(sprite_t));
    if (!sprite) goto allocError;
    texture = &sprite->texture;

    //Create and init the sprite's texture
    result = C3D_TexInit(texture, nextPow2(width), nextPow2(height), GPU_RGBA8);
    if (!result) goto texInitError;
    //C3D_TexSetWrap(texture, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
    texture->param = GPU_TEXTURE_MAG_FILTER(GPU_LINEAR) | GPU_TEXTURE_MIN_FILTER(GPU_LINEAR)
        | GPU_TEXTURE_WRAP_S(GPU_CLAMP_TO_BORDER) | GPU_TEXTURE_WRAP_T(GPU_CLAMP_TO_BORDER);

    sprite->width = (float)width;
    sprite->height = (float)height;
	sprite->drawColor = 0xFFFFFFFF;
	sprite->isGreyedOut = false;
	sprite->isHidden = false;
	sprite->depth = 0.0f;
	sprite->amount = 1.f;
    return (sprite);
texInitError:
    free(sprite);
allocError:
    return (NULL);
}

void deleteSprite(sprite_t *sprite)
{
    if (!sprite) return;
    C3D_TexDelete(&sprite->texture);
    free(sprite);
    sprite = NULL;
}

static void sceneInit(void)
{
    C3D_AttrInfo    *attrInfo;

    // Load the vertex shader, create a shader program and bind it
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_len);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");

    // Configure attributes for use with the vertex shader
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
                                                   
    // Compute the projection matrix
    Mtx_OrthoTilt(&top.projection, 0.0f, 400.0f, 240.0f, 0.0f, 0.0f, 1.0f, true);
    Mtx_OrthoTilt(&bottom.projection, 0.0f, 320.0f, 240.0f, 0.0f, 0.0f, 1.0f, true);

    // Configure depth test to overwrite pixels with the same depth (needed to draw overlapping glyphs)
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);

    // Create the text vertex array
    textVtxArray = (textVertex_s*)linearAlloc(sizeof(textVertex_s)*TEXT_VTX_ARRAY_COUNT);
}

static void sceneExit(void)
{
    // Free the textures

    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}

void drawInit(void)
{
    C3D_RenderTarget *target;

    //Init Citro3D
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the top render target
    target = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(target, GFX_TOP, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    top.target = target;

    // Initialize the bottom render target
    target = C3D_RenderTargetCreate(240, 320, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
	C3D_RenderTargetClear(target, C3D_CLEAR_ALL, CLEAR_COLOR, 0);
    C3D_RenderTargetSetOutput(target, GFX_BOTTOM, GFX_LEFT, DISPLAY_TRANSFER_FLAGS);
    bottom.target = target;

    // Initialize the scene
    sceneInit();
}

void drawExit(void)
{
    sceneExit();
    C3D_Fini();
}

void setTextColor(u32 color)
{
#ifndef CITRA
    C3D_TexEnv  *env;
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_CONSTANT, 0, 0);
    C3D_TexEnvSrc(env, C3D_Alpha, GPU_TEXTURE0, GPU_CONSTANT, 0);
	C3D_TexEnvOpRgb(env, 0, 0, 0);
	C3D_TexEnvOpAlpha(env, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    C3D_TexEnvFunc(env, C3D_Alpha, GPU_MODULATE);
    C3D_TexEnvColor(env, color);
#endif
}

void updateScreen(void)
{
    if (frameStarted) {
        C3D_FrameEnd(0);
        frameStarted = false;
    }
    textVtxArrayPos = 0;
    currentScreen = -1;
}

void setScreen(gfxScreen_t screen)
{
    if (!frameStarted)
    {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        frameStarted = true;
    }
    if (screen == currentScreen) return;
    currentScreen = screen;
    if (screen == GFX_TOP)
    {
        C3D_FrameDrawOn(top.target);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &top.projection);
    }
    else if (screen == GFX_BOTTOM)
    {
        C3D_FrameDrawOn(bottom.target);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &bottom.projection);
    }
    else return;
}
