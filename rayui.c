#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include "rayui.h"

void CenterCamera(Camera2D *cam)
{
    cam->offset = (Vector2){GetScreenWidth()/2.0f, GetScreenHeight()/2.0f};
}

void Zoom(float *fl, float min, float max, float speed)
{
    *fl += GetMouseWheelMove() * speed * *fl;
    if(*fl < min)*fl = min;
    if(*fl > max)*fl = max;
}

Element MakeBox(
    Color color,
    Vector2 pos,
    Vector2 dim,
    float *scale
)
{
    Element retVal = {
        .pos = pos,
        .dim = dim,
        .scale = scale,
        .type = BOX,
        .data.box = malloc(sizeof(struct ElementBoxData))
    };

    retVal.data.box->color = color;

    return retVal;
}

Element MakeTextBox(
    Color color,
    Vector2 pos,
    Vector2 dim,
    float *scale,
    Color textColor,
    Vector2 textPos,
    char *text,
    int fontSize,
    float *fontScale
)
{
    Element retVal = {
       .pos = pos,
       .dim = dim,
       .scale = scale,
       .type = TEXTBOX,
       .data.textbox = malloc(sizeof(struct ElementTextboxData))
    };

    retVal.data.textbox->color = color;
    retVal.data.textbox->textColor = textColor;
    retVal.data.textbox->text = text;
    retVal.data.textbox->textPos = textPos;
    retVal.data.textbox->fontScale = fontScale;
    retVal.data.textbox->fontSize = fontSize;

    return retVal;
}

Element MakeImageBox(
    Color color,
    Vector2 pos,
    Vector2 dim,
    float *scale,
    Texture2D texture,
    Rectangle source,
    Vector2 origin,
    float rotation
)
{
    Element retVal = {
       .pos = pos,
       .dim = dim,
       .scale = scale,
       .type = IMAGEBOX,
       .data.imagebox = malloc(sizeof(struct ElementImageboxData))
    };

    retVal.data.imagebox->texture = texture;
    retVal.data.imagebox->source = source;
    retVal.data.imagebox->origin = origin;
    retVal.data.imagebox->rotation = rotation;
    retVal.data.imagebox->tint = color;

    return retVal;
}


void FreeElement(Element *elem)
{
    switch(elem->type)
    {
        case BOX:
            free(elem->data.box);
            break;

        case TEXTBOX:
            free(elem->data.textbox);
            break;

        case IMAGEBOX:
            free(elem->data.imagebox);
            break;

        case NOELEM:
            printf("WARNING: attempted double-free of element\n");
            break;
    }
    elem->type = NOELEM;
}

void DrawElement(Element elem)
{
    float scale = (elem.scale == NULL) ? 1.0f : *(elem.scale);

    //used later
    float fontScale;

    switch(elem.type)
    {
        case BOX:
            DrawRectangle(
                (int)(elem.pos.x * scale),
                (int)(elem.pos.y * scale),
                (int)(elem.dim.x * scale),
                (int)(elem.dim.y * scale),
                elem.data.box->color
            );
            break;

        case TEXTBOX:
            DrawRectangle(
                (int)(elem.pos.x * scale),
                (int)(elem.pos.y * scale),
                (int)(elem.dim.x * scale),
                (int)(elem.dim.y * scale),
                elem.data.textbox->color
            );

            fontScale = (elem.data.textbox->fontScale == NULL) ? 
               1.0f : *(elem.data.textbox->fontScale);

            DrawText(
                elem.data.textbox->text,
                (int)((
                     elem.data.textbox->textPos.x + elem.pos.x
                ) * scale),
                (int)((
                     elem.data.textbox->textPos.y + elem.pos.y
                ) * scale),
                (int)(elem.data.textbox->fontSize * fontScale),
                elem.data.textbox->textColor
            );
            break;

        case IMAGEBOX:
            DrawTexturePro(
                elem.data.imagebox->texture,
                elem.data.imagebox->source,
                (Rectangle) {
                    elem.pos.x * scale,
                    elem.pos.y * scale,
                    elem.dim.x * scale,
                    elem.dim.y * scale
                },
                (Vector2) {
                    elem.data.imagebox->origin.x,
                    elem.data.imagebox->origin.y
                },
                elem.data.imagebox->rotation,
                elem.data.imagebox->tint
            );
            break;

        case NOELEM:
            printf("WARNING: attempted to draw freed element\n");
            break;
    }
}

void UpdateButton(Button *btn)
{
    Vector2 mousePos = GetMousePosition();

    if(btn->cam != NULL)mousePos = GetScreenToWorld2D(mousePos, *(btn->cam));

    char prevMouseOn = btn->mouseOn;

    float hitboxScale;
    switch(btn->hitboxType)
    {
        case ELEM_BUTTON:

            hitboxScale = (btn->hitbox.elem->scale == NULL)?
                1.0f : *(btn->hitbox.elem->scale);

            btn->mouseOn = CheckCollisionPointRec(
                mousePos,
                (Rectangle) {
                    btn->hitbox.elem->pos.x * hitboxScale,
                    btn->hitbox.elem->pos.y * hitboxScale,
                    btn->hitbox.elem->dim.x * hitboxScale,
                    btn->hitbox.elem->dim.y * hitboxScale
                }
            );
            break;

        case REC_BUTTON:
            btn->mouseOn = CheckCollisionPointRec(
                mousePos,
                *(btn->hitbox.rec)
            );
            break;
    }

    if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        btn->mouseDownOver = btn->mouseOn;
        btn->mouseLeft = 0;
        btn->mouseEntered = 0;
    }
    if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        btn->mouseUpOver = btn->mouseOn;

    if(!btn->mouseLeft)btn->mouseLeft = btn->mouseDownOver && !btn->mouseOn;
    if(!btn->mouseEntered)
        btn->mouseEntered = !btn->mouseDownOver && btn->mouseOn;

    //if the element is clicked on based on the input method
    char clicked = 0;

    //if the button is clicked off of
    char unclicked = 0;

    switch(btn->inputStyle)
    {
        case BUTTON_INPUT_ON_AND_OFF:
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                if(btn->mouseDownOver && btn->mouseUpOver)
                    clicked = 1;
                else if(!btn->mouseUpOver && !btn->mouseDownOver)
                    unclicked = 1;
            }
            break;

        case BUTTON_INPUT_STAY:
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                if(btn->mouseDownOver && btn->mouseUpOver && !btn->mouseLeft)
                    clicked = 1;
                else if(
                    !btn->mouseDownOver &&
                    !btn->mouseUpOver &&
                    !btn->mouseEntered
                )
                    unclicked = 1;
            }
            break;

        case BUTTON_INPUT_OFF:
            if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
            {
                if(btn->mouseOn)clicked = 1;
                else unclicked = 1;
            }
            break;

        case BUTTON_INPUT_ON:
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if(btn->mouseOn)clicked = 1;
                else unclicked = 1;
            }
            break;

        case BUTTON_INPUT_DOWN: 
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                clicked = btn->mouseOn;
            else if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
                unclicked = btn->mouseOn;
            else if(IsMouseButtonDown(MOUSE_LEFT_BUTTON))
            {
                clicked = !prevMouseOn && btn->mouseOn;
                unclicked = prevMouseOn && !btn->mouseOn;
            }


        case BUTTON_INPUT_NONE:
            break;
    }

    switch(btn->outputStyle)
    {
        case BUTTON_OUTPUT_FRAME:
            btn->output = clicked;
            break;

        case BUTTON_OUTPUT_SELECT:
            if(clicked)btn->output = 1;
            if(unclicked)btn->output = 0;
            break;

        case BUTTON_OUTPUT_TOGGLE:
            if(clicked)btn->output ^= 1;
            break;

        case BUTTON_OUTPUT_NONE:
            break;
    }
}

Button MakeElemButton(
    Camera2D *cam,
    Element *hitbox,
    enum ButtonInputStyle inStyle,
    enum ButtonOutputStyle outStyle
)
{
    return (Button) {
        .mouseEntered = 0,
        .mouseLeft = 0,
        .mouseDownOver = 0,
        .mouseUpOver = 0,
        .output = 0,
        .mouseOn = 0,
        .hitboxType = ELEM_BUTTON,
        .inputStyle = inStyle,
        .outputStyle = outStyle,
        .hitbox.elem = hitbox,
        .cam = cam,
    };
}

Button MakeRectButton(
    Camera2D *cam,
    Rectangle *hitbox,
    enum ButtonInputStyle inStyle,
    enum ButtonOutputStyle outStyle
)
{
    return (Button) {
        .mouseEntered = 0,
        .mouseLeft = 0,
        .mouseDownOver = 0,
        .mouseUpOver = 0,
        .output = 0,
        .mouseOn = 0,
        .hitboxType = REC_BUTTON,
        .inputStyle = inStyle,
        .outputStyle = outStyle,
        .hitbox.rec = hitbox,
        .cam = cam,
    };
}
