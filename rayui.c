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

Dragger MakeDragger(
    enum DraggerHitboxType hitboxType,
    union DraggerHitbox  hitbox,
    enum DraggerTargetType targetType,
    union DraggerTarget target,
    enum DraggerBoundsType boundsType,
    union DraggerBounds bound,
    enum DraggerBoundStyle boundStyle,
    float *scale,
    Camera2D *cam
)
{
    return (Dragger) {
        .cam = cam,
        .scale = scale,
        .clicked = 0,
        .prevClicked = 0,
        .clickedPos = (Vector2) {0.0f, 0.0f},
        .prevMousePos = (Vector2) {0.0f, 0.0f},
        .boundStyle = boundStyle,
        .bound = bound,
        .boundsType = boundsType,
        .hitboxType = hitboxType,
        .hitbox = hitbox,
        .targetType = targetType,
        .target = target
    };
}

void BoundDrag(
    enum DraggerTargetType targetType,
    union DraggerTarget target,
    enum DraggerBoundsType boundsType,
    union DraggerBounds bound,
    enum DraggerBoundStyle boundStyle
){

    if(boundsType == DRAGGER_NO_BOUNDS)return;
    if(boundStyle == DRAGGER_BOUND_FALSE)return;

    Vector2 pos, dim;

    float elemTargetScale;
    switch(targetType)
    {
        case DRAGGER_VECTORS_TARGET:
            pos = *(target.vecs.pos);
            dim = (target.vecs.dim == NULL) ?
                (Vector2) {0.0f, 0.0f} : *(target.vecs.dim);
            break;

        case DRAGGER_ELEM_TARGET:
            elemTargetScale = (target.elem->scale == NULL) ?
                1.0f : *(target.elem->scale);

            pos.x = target.elem->pos.x * elemTargetScale;
            pos.y = target.elem->pos.y * elemTargetScale;
            dim.x = target.elem->dim.x * elemTargetScale;
            dim.y = target.elem->dim.y * elemTargetScale;
            break;

        case DRAGGER_REC_TARGET:
            pos.x = target.rec->x;
            pos.y = target.rec->y;
            dim.x = target.rec->width;
            dim.y = target.rec->height;
            break;
    }

    Vector2 boundPos, boundDim;

    float elemBoundsScale;
    switch(boundsType)
    {
        case DRAGGER_REC_BOUNDS:
            boundPos.x = bound.rec->x;
            boundPos.y = bound.rec->y;
            boundDim.x = bound.rec->width;
            boundDim.y = bound.rec->height;
            break;

        case DRAGGER_ELEM_BOUNDS:
            elemBoundsScale = (bound.elem->scale == NULL) ?
                1.0f : *(bound.elem->scale);

            boundPos.x = bound.rec->x * elemBoundsScale;
            boundPos.y = bound.rec->y * elemBoundsScale;
            boundDim.x = bound.rec->width * elemBoundsScale;
            boundDim.y = bound.rec->height * elemBoundsScale;
            break;

        case DRAGGER_NO_BOUNDS:
            printf(
                "Oh fuck, something went wrong! It's probably my fault, but "
                "if it isn't, you are sooo fucking cooked.\n"
            );
            break;
    }

    switch(boundStyle)
    {
        case DRAGGER_BOUND_BOX:
            if(pos.x < boundPos.x)pos.x = boundPos.x;
            if(pos.y < boundPos.y)pos.y = boundPos.y;
            if(pos.x + dim.x > boundPos.x + boundDim.x)
                pos.x = boundPos.x + boundDim.x - dim.x;
            if(pos.y + dim.y > boundPos.y + boundDim.y)
                pos.y = boundPos.y + boundDim.y - dim.y;
            break;

        case DRAGGER_BOUND_TOUCH:
            if(pos.x < boundPos.x - dim.x)pos.x = boundPos.x - dim.x;
            if(pos.y < boundPos.y - dim.y)pos.y = boundPos.y - dim.y;
            if(pos.x > boundPos.x + boundDim.x)
                pos.x = boundPos.x + boundDim.x;
            if(pos.y > boundPos.y + boundDim.y)
                pos.y = boundPos.y + boundDim.y;
            break;

        case DRAGGER_BOUND_FALSE:
            //if you get here, wtf are you doing???
            break;

    }

    switch(targetType)
    {
        case DRAGGER_VECTORS_TARGET:
            *(target.vecs.pos) = pos;
            if(target.vecs.dim == NULL)break;
            *(target.vecs.dim) = dim;
            break;

        case DRAGGER_ELEM_TARGET:
            elemTargetScale = (target.elem->scale == NULL) ?
                1.0f : *(target.elem->scale);

            target.elem->pos.x = pos.x / elemTargetScale;
            target.elem->pos.y = pos.y / elemTargetScale;
            target.elem->dim.x = dim.x / elemTargetScale;
            target.elem->dim.y = dim.y / elemTargetScale;
            break;

        case DRAGGER_REC_TARGET:
            target.rec->x = pos.x;
            target.rec->y = pos.y;
            target.rec->width = dim.x;
            target.rec->height = dim.y;
            break;
    }
}

void UpdateDrag(Dragger *drag)
{
    Vector2 mousePos = GetMousePosition();
    Vector2 prevMousePos = drag->prevMousePos;
    if(drag->cam != NULL)
    {
        mousePos = GetScreenToWorld2D(
        mousePos, *(drag->cam)
        );

        prevMousePos = GetScreenToWorld2D(
        prevMousePos, *(drag->cam)
        );
    };

    float hitboxElemScale;
    switch(drag->hitboxType)
    {
        case DRAGGER_ELEM_HITBOX:

            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                hitboxElemScale = (drag->hitbox.elem->scale == NULL) ?
                    1.0f : *(drag->hitbox.elem->scale);

                drag->clicked = CheckCollisionPointRec(
                    mousePos, (Rectangle) {
                        drag->hitbox.elem->pos.x * hitboxElemScale,
                        drag->hitbox.elem->pos.y * hitboxElemScale,
                        drag->hitbox.elem->dim.x * hitboxElemScale,
                        drag->hitbox.elem->dim.y * hitboxElemScale
                    });
            } else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                drag->clicked = 0;

            break;

        case DRAGGER_REC_HITBOX:

            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                drag->clicked = CheckCollisionPointRec(
                    mousePos, *(drag->hitbox.rec)
                );
            } else if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                drag->clicked = 0;

            break;

        case DRAGGER_PTR_HITBOX:

            drag->clicked = *(drag->hitbox.clickedPtr);

            break;

        case DRAGGER_NO_HITBOX:
            break;
    }

    float dragScale = (drag->scale == NULL) ? 1.0f : *(drag->scale);

    if(drag->clicked)
    {
        if(!drag->prevClicked)
        {
            drag->clickedPos = mousePos;
        } else {

            float targetElemScale;
            switch(drag->targetType)
            {
                case DRAGGER_ELEM_TARGET:

                    targetElemScale = (drag->target.elem->scale == NULL) ?
                        1.0f : *(drag->target.elem->scale);

                    drag->target.elem->pos.x -=
                        (prevMousePos.x - drag->clickedPos.x) *
                        dragScale / targetElemScale;
                    drag->target.elem->pos.y -=
                        (prevMousePos.y - drag->clickedPos.y) *
                        dragScale / targetElemScale;

                    drag->target.elem->pos.x +=
                        (mousePos.x - drag->clickedPos.x) *
                        dragScale / targetElemScale;
                    drag->target.elem->pos.y +=
                        (mousePos.y - drag->clickedPos.y) *
                        dragScale / targetElemScale;
                    break;

                case DRAGGER_REC_TARGET:
                    drag->target.rec->x -=
                        (prevMousePos.x - drag->clickedPos.x) *
                        dragScale;
                    drag->target.rec->y -=
                        (prevMousePos.y - drag->clickedPos.y) *
                        dragScale;

                    drag->target.rec->x +=
                        (mousePos.x - drag->clickedPos.x) * dragScale;
                    drag->target.rec->y +=
                        (mousePos.y - drag->clickedPos.y) * dragScale;
                    break;

                case DRAGGER_VECTORS_TARGET:
                    drag->target.vecs.pos->x -=
                        (prevMousePos.x - drag->clickedPos.x) *
                        dragScale;
                    drag->target.vecs.pos->y -=
                        (prevMousePos.y - drag->clickedPos.y) *
                        dragScale;

                    drag->target.vecs.pos->x +=
                        (mousePos.x - drag->clickedPos.x) *
                        dragScale;
                    drag->target.vecs.pos->y +=
                        (mousePos.y - drag->clickedPos.y) *
                        dragScale;
                    break;
            }
        }
    }

    drag->prevMousePos = GetMousePosition();
    drag->prevClicked = drag->clicked;

    BoundDrag(
        drag->targetType,
        drag->target,
        drag->boundsType,
        drag->bound,
        drag->boundStyle
    );
}

void UpdateButton(Button *btn)
{
    Vector2 mousePos = GetMousePosition();

    if(btn->cam != NULL)mousePos = GetScreenToWorld2D(mousePos, *(btn->cam));

    btn->mouseWasOn = btn->mouseOn;

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

    if(
        (btn->inputStyle == BUTTON_INPUT_NONE) ||
        (btn->outputStyle == BUTTON_OUTPUT_NONE)
    )return;

    btn->output = GetButtonOutput(
        btn, btn->inputStyle, btn->outputStyle, btn->output
    );
}

char GetButtonOutput(
    Button *btn,
    enum ButtonInputStyle inputStyle,
    enum ButtonOutputStyle outputStyle,
    char prevOutput
)
{
    if(
        (inputStyle == BUTTON_INPUT_NONE) ||
        (outputStyle == BUTTON_OUTPUT_NONE)
    )return prevOutput;

    //if the element is clicked on based on the input method
    char clicked = 0;

    //if the button is clicked off of
    char unclicked = 0;

    switch(inputStyle)
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
                clicked = !btn->mouseWasOn && btn->mouseOn;
                unclicked = btn->mouseWasOn && !btn->mouseOn;
            }
            break;

        case BUTTON_INPUT_FOCUS:
            unclicked = btn->mouseWasOn && !btn->mouseOn;
            clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && btn->mouseOn;
            break;

        case BUTTON_INPUT_NONE:
            break;
    }

    switch(outputStyle)
    {
        case BUTTON_OUTPUT_FRAME:
            return clicked;
            break;

        case BUTTON_OUTPUT_SELECT:
            if(clicked)return 1;
            if(unclicked)return 0;
            break;

        case BUTTON_OUTPUT_TOGGLE:
            if(clicked)return prevOutput ^ 1;
            break;

        case BUTTON_OUTPUT_NONE:
            break;
    }

    return prevOutput;
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
        .mouseWasOn = 0,
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
        .mouseWasOn = 0,
        .hitboxType = REC_BUTTON,
        .inputStyle = inStyle,
        .outputStyle = outStyle,
        .hitbox.rec = hitbox,
        .cam = cam,
    };
}

void MoveRecToGrid(
    int rowStart,
    int rowEnd,
    int colStart,
    int colEnd,
    Grid grid,
    Rectangle *rec
)
{
    float gridScale = (grid.scale == NULL) ? 1.0f : *(grid.scale);

    rec->x = grid.pos.x * gridScale;
    rec->y = grid.pos.y * gridScale;
    if(colStart != 0) rec->x += grid.colEnds[colStart - 1] * gridScale;
    if(rowStart != 0) rec->y += grid.rowEnds[rowStart - 1] * gridScale;
    rec->width = (grid.colEnds[colEnd] * gridScale) - rec->x;
    rec->height = (grid.rowEnds[rowEnd] * gridScale) - rec->y;
}

void MoveElemToGrid(
    int rowStart,
    int rowEnd,
    int colStart,
    int colEnd,
    Grid grid,
    Element *elem
)
{
    float elemScale = (elem->scale == NULL) ? 1.0f : *(elem->scale);

    Rectangle rec = {
        .x = elem->pos.x * elemScale,
        .y = elem->pos.y * elemScale,
        .width = elem->dim.x * elemScale,
        .height = elem->dim.y * elemScale,
    };

    MoveRecToGrid(rowStart, rowEnd, colStart, colEnd, grid, &rec);

    elem->pos.x = rec.x / elemScale;
    elem->pos.y = rec.y / elemScale;
    elem->dim.x = rec.width / elemScale;
    elem->dim.y = rec.height / elemScale;
}

void SetElemPos(Element *elem, Vector2 pos)
{
    float scale = (elem->scale == NULL) ? 1.0f : *(elem->scale);
    elem->pos.x = pos.x / scale;
    elem->pos.y = pos.y / scale;
}

void SetElemDim(Element *elem, Vector2 dim)
{
    float scale = (elem->scale == NULL) ? 1.0f : *(elem->scale);
    elem->dim.x = dim.x / scale;
    elem->dim.y = dim.y / scale;
}

Vector2 GetElemPos(Element elem)
{
    float scale = (elem.scale == NULL) ? 1.0f : *(elem.scale);
    return (Vector2) {elem.pos.x * scale, elem.pos.y * scale};
}

Vector2 GetElemDim(Element elem)
{
    float scale = (elem.scale == NULL) ? 1.0f : *(elem.scale);
    return (Vector2) {elem.dim.x * scale, elem.dim.y * scale};
}

TextInput MakeTextInput
(
    char *output,
    unsigned int maxLen,
    char *focusPtr,
    char charRangesNum,
    struct CharRange *allowedChars  
)
{
    return (TextInput) {
        .output = output,
        .maxLen = maxLen,
        .focused = 0,
        .focusPtr = focusPtr,
        .charRangesNum = charRangesNum,
        .allowedChars = allowedChars,
        .curLen = 0
    };
}

void UpdateTextInput(TextInput *input)
{
    if(input->focusPtr != NULL)input->focused = *(input->focusPtr);
    if(!input->focused)return;
    int temp;

    if(IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE))
    {
        if(input->curLen == 0)return;
        input->curLen--;
        input->output[input->curLen] = 0;
    }

    while((temp = GetCharPressed()) != 0)
    {
        if(input->curLen >= input->maxLen)return;
        char isValid = 0;

        for(int i = input->charRangesNum; i-- > 0;)
        {
            isValid |=
                (input->allowedChars[i].min <= temp) &&
                (input->allowedChars[i].max >= temp);

            if(isValid)break;
        }

        if(isValid)
        {
            input->output[input->curLen] = temp;
            input->curLen++;
            input->output[input->curLen] = 0;
        }
    }
}
