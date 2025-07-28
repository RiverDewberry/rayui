#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include "rayui.h"

int main(void)
{
    InitWindow(500, 500, "rayui test");

    float uiScale = 1.0f;

    char str[13];

    Element box = MakeTextBox(
        RED,
        (Vector2) {0.0f, 0.0f},
        (Vector2) {0.0f, 0.0f},
        &uiScale,
        WHITE,
        (Vector2) {10.0f, 10.0f},
        str,
        10,
        NULL
    );

    Element bounds = MakeBox(
        BLUE,
        (Vector2) {0.0f, 0.0f},
        (Vector2) {0.0f, 0.0f},
        NULL
    );

    Element smallBox = MakeBox(
        BLACK,
        (Vector2) {0.0f, 0.0f},
        (Vector2) {6.0f, 6.0f},
        NULL
    );

    Element mover = MakeBox(
        GRAY,
        (Vector2) {0.0f, 0.0f},
        (Vector2) {0.0f, 0.0f},
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &mover, BUTTON_INPUT_DOWN, BUTTON_OUTPUT_SELECT
    );

    Button textBtn = MakeElemButton(
        NULL, &smallBox, BUTTON_INPUT_FOCUS, BUTTON_OUTPUT_SELECT
    );

    TextInput textInput = MakeTextInput(
        box.data.textbox->text,
        12, &textBtn.output,
        1,
        (struct CharRange[]) {{' ', '~'}}
    );

    float dragScale = 5.0f;
    Dragger test = MakeDragger(
        DRAGGER_PTR_HITBOX,
        (union DraggerHitbox) {.clickedPtr = &(btn.output)},
        DRAGGER_ELEM_TARGET,
        (union DraggerTarget) {.elem = &box},
        DRAGGER_ELEM_BOUNDS,
        (union DraggerBounds) {.elem = &bounds},
        DRAGGER_BOUND_BOX,
        &dragScale,
        NULL
    );

    Grid elemGrid = {
        .pos = {0.0f, 0.0f},
        .numCols = 5,
        .numRows = 5,
        .colEnds = (float[]) {50.0f, 100.0f, 150.0f, 200.0f, 450.0f},
        .rowEnds = (float[]) {50.0f, 100.0f, 150.0f, 180.0f, 450.0f},
        .scale = NULL
    };

    MoveElemToGrid(1, 1, 1, 1, elemGrid, &mover);
    MoveElemToGrid(3, 3, 3, 3, elemGrid, &box);
    MoveElemToGrid(3, 4, 3, 4, elemGrid, &bounds);

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);
        UpdateButton(&textBtn);
        UpdateTextInput(&textInput);

        if(textInput.curLen == 0)
        {
            if(textInput.focused)*(box.data.textbox->text) = 0;
            else strcpy(box.data.textbox->text, "Enter text");
        }

        box.dim.x =
            (20.0f * uiScale) +
            ((float) MeasureText(textInput.output, 10));

        if(!textInput.focused)
        {
            UpdateDrag(&test);
            SetElemPos(&smallBox, GetElemPos(box));
            smallBox.pos.x += 2.0f;
            smallBox.pos.y += 22.0f;
        } else {
            SetElemPos(&box, GetElemPos(smallBox));
            box.pos.x -= 2.0f;
            box.pos.y -= 22.0f;
            UpdateDrag(&test);
        }

        box.data.textbox->color = btn.output ? GREEN : RED;
        smallBox.data.box->color = textBtn.output ? WHITE : BLACK;

        DrawElement(bounds);
        DrawElement(mover);
        DrawElement(box);
        DrawElement(smallBox);
//        uiScale *= 1.01;

        EndDrawing();
    }

    FreeElement(&bounds);
    FreeElement(&mover);
    FreeElement(&box);
    return 0;
}
