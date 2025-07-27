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

    Element mover = MakeBox(
        GRAY,
        (Vector2) {0.0f, 0.0f},
        (Vector2) {0.0f, 0.0f},
        NULL
    );

    Button btn = MakeElemButton(
        NULL, &mover, BUTTON_INPUT_NONE, BUTTON_OUTPUT_NONE
    );

    Vector2 dragTarget;

    float dragScale = 5.0f;
    Dragger test = MakeDragger(
        DRAGGER_PTR_HITBOX,
        (union DraggerHitbox) {.clickedPtr = &(btn.output)},
        DRAGGER_VECTORS_TARGET,
        (union DraggerTarget) {.vecs = {.pos = &dragTarget, .dim = NULL}},
        DRAGGER_NO_BOUNDS,
        (union DraggerBounds) {},
        DRAGGER_BOUND_FALSE,
        &dragScale,
        NULL
    );

    Grid elemGrid = {
        .pos = {0.0f, 0.0f},
        .numCols = 5,
        .numRows = 5,
        .colEnds = (float[]) {50.0f, 100.0f, 150.0f, 200.0f, 450.0f},
        .rowEnds = (float[]) {50.0f, 100.0f, 150.0f, 200.0f, 450.0f},
        .scale = NULL
    };

    dragTarget = (Vector2) {elemGrid.colEnds[2], elemGrid.rowEnds[2]};
    MoveElemToGrid(1, 1, 1, 1, elemGrid, &mover);
    MoveElemToGrid(3, 3, 3, 3, elemGrid, &box);
    MoveElemToGrid(3, 4, 3, 4, elemGrid, &bounds);

    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        ClearBackground(WHITE);

        UpdateButton(&btn);

        if(btn.output)btn.output = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if(!btn.output)btn.output =
            btn.mouseOn && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        UpdateDrag(&test);

        SetElemPos(&box, dragTarget);
        BoundDrag(
            DRAGGER_ELEM_TARGET,
            (union DraggerTarget) {.elem = &box},
            DRAGGER_ELEM_BOUNDS,
            (union DraggerBounds) {.elem = &bounds},
            DRAGGER_BOUND_BOX
        );

        if(!test.clicked)*(test.target.vecs.pos) = GetElemPos(box);

        box.data.textbox->color = btn.output ? GREEN : RED;

        DrawElement(bounds);
        DrawElement(mover);
        DrawElement(box);
        DrawPixel(dragTarget.x, dragTarget.y, BLACK);
//        uiScale *= 1.01;

        EndDrawing();
    }

    FreeElement(&bounds);
    FreeElement(&mover);
    FreeElement(&box);
    return 0;
}
