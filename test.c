#include <stdio.h>
#include <raylib.h>
#include "rayui.h"

int main(void)
{
    InitWindow(500, 500, "rayui test");

    float uiScale = 1.0f;

    char str[] = "Hello,\nWorld";

    Element box = MakeTextBox(
        RED,
        (Vector2) {150.0f, 150.0f},
        (Vector2) {50.0f, 50.0f},
        &uiScale,
        WHITE,
        (Vector2) {10.0f, 10.0f},
        str,
        10,
        NULL
    );

    Element bounds = MakeBox(
        BLUE,
        (Vector2) {150.0f, 150.0f},
        (Vector2) {300.0f, 300.0f},
        NULL
    );

    Element mover = MakeBox(
        GRAY,
        (Vector2) {50.0f, 50.0f},
        (Vector2) {50.0f, 50.0f},
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &mover, BUTTON_INPUT_NONE, BUTTON_OUTPUT_NONE
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

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);

        if(btn.output)btn.output = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if(!btn.output)btn.output =
            btn.mouseOn && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        UpdateDrag(&test);

        box.data.textbox->color = btn.output ? GREEN : RED;

        DrawElement(bounds);
        DrawElement(mover);
        DrawElement(box);
//        uiScale *= 1.01;

        EndDrawing();
    }

    return 0;
}
