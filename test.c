#include <stdio.h>
#include <raylib.h>
#include "rayui.h"

int main(void)
{
    InitWindow(500, 500, "rayui test");

    float uiScale = 1.0f;

    char str[] = "Hello, World";

    Element box = MakeTextBox(
        RED,
        (Vector2) {100.0f, 100.0f},
        (Vector2) {200.0f, 200.0f},
        &uiScale,
        WHITE,
        (Vector2) {100.0f, 100.0f},
        str,
        10,
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &box, BUTTON_INPUT_STAY, BUTTON_OUTPUT_TOGGLE
    );

    Dragger test = MakeDragger(
        DRAGGER_PTR_HITBOX,
        (union DraggerHitbox) {.clickedPtr = &(btn.output)},
        DRAGGER_ELEM_TARGET,
        (union DraggerTarget) {.elem = &box},
        DRAGGER_NO_BOUNDS,
        (union DraggerBounds) {},
        DRAGGER_BOUND_FALSE,
        NULL,
        NULL
    );

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);
        UpdateDrag(&test);

        box.data.textbox->color = btn.output ? GREEN : RED;

        DrawElement(box);
//        uiScale *= 1.01;

        EndDrawing();
    }

    return 0;
}
