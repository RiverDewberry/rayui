#include <stdio.h>
#include <raylib.h>
#include "rayui.h"

int main(void)
{
    InitWindow(500, 500, "rayui test");

    float uiScale = 1.0f;

    Element box = MakeTextBox(
        RED,
        (Vector2) {100.0f, 100.0f},
        (Vector2) {200.0f, 200.0f},
        &uiScale,
        WHITE,
        (Vector2) {100.0f, 100.0f},
        "Hello, World!",
        10,
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &box, BUTTON_INPUT_DOWN, BUTTON_OUTPUT_TOGGLE
    );

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);

        box.data.textbox->color = btn.output ? GREEN : RED;

        DrawElement(box);
//        uiScale *= 1.01;

        EndDrawing();
    }

    return 0;
}
