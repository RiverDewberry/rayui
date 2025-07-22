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
        "0",
        10,
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &box, BUTTON_INPUT_NONE, BUTTON_OUTPUT_NONE
    );

    SetTargetFPS(30);

    char outputOne = 0, outputTwo = 0;

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);

        outputOne = GetButtonOutput(
            &btn, BUTTON_INPUT_STAY, BUTTON_OUTPUT_FRAME, outputOne
        );

        outputTwo = GetButtonOutput(
            &btn, BUTTON_INPUT_STAY, BUTTON_OUTPUT_SELECT, outputTwo
        );

        box.data.textbox->color = outputTwo ? GREEN : RED;
        (box.data.textbox->text)[0] = 49;

        DrawElement(box);
//        uiScale *= 1.01;

        EndDrawing();
    }

    return 0;
}
