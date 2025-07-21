#ifndef RAYUI_H
#define RAYUI_H
#include <raylib.h>

//centers the camera
void CenterCamera(Camera2D *cam);

//zooms the given float based on mouse wheel movement, speed, and bounds
void Zoom(float *fl, float min, float max, float speed);

//a grid on which elements or rectangles can be placed
typedef struct Grid {
    Vector2 pos;
    int numRows;
    int numCols;
    float *rowEnds;
    float *colEnds;
    //if scale is NULL, then it is treated as 1.0f
    float *scale;
} Grid;

struct ElementBoxData {
    Color color;
};

struct ElementTextboxData {
    Color color;
    Color textColor;
    char *text;
    Vector2 textPos;
    int fontSize;
    float *fontScale;
};

struct ElementImageboxData {
    Texture2D texture;
    Rectangle source;
    Vector2 origin;
    float rotation;
    Color tint;
};

//a ui element
typedef struct Element {
    //position
    Vector2 pos;

    //dimentions
    Vector2 dim;

    //scales the element up or down by this factor, is treated as 1 if NULL
    float *scale;

    //the type of the element
    enum {
        //used after an element has been freed
        NOELEM,

        //a basic colored box
        BOX,

        //a colored text box
        TEXTBOX,

        //a box with an image in it
        IMAGEBOX

    } type;

    //type specific data of the element
    union {
        struct ElementBoxData *box;

        struct ElementTextboxData *textbox;

        struct ElementImageboxData *imagebox;
    } data;

} Element;

//makes a colored box
Element MakeBox(
    Color color,
    Vector2 pos,
    Vector2 dim,
    float *scale
);

//makes a textbox
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
);

//makes a grid
Element MakeGrid(
    int numRows,
    int numCols,
    float *rowEnds,
    float *colEnds,
    int maxElems,
    Vector2 pos,
    float *scale
);

//moves an element to a pos on a grid
void MoveElemToGrid(
    int rowStart,
    int rowEnd,
    int colStart,
    int colEnd,
    Grid grid,
    Element *addToGrid
);

//makes a box with an image in it
Element MakeImageBox(
    Color color,
    Vector2 pos,
    Vector2 dim,
    float *scale,
    Texture2D texture,
    Rectangle source,
    Vector2 origin,
    float rotation
);

//draws an element to the screen
void DrawElement(Element elem);

//frees data allocated to an element
void FreeElement(Element *elem);

//allows the user to drag elements (in screen space)
typedef struct Dragger {

    //the type of hitbox
    enum {ELEM_HITBOX, REC_HITBOX, NO_HITBOX} HitboxType;

    //the hitbox that can be clicked to drag
    union {
        Element *elem;
        Rectangle *rec;
    } hitbox;

    //if the tracker was clicked
    char clicked;

    //where the hitbox was when it got clicked
    Vector2 clickedPos;

    //the type of target
    enum {ELEM_TARGET, REC_TARGET, VECTOR_TARGET} TargetType;

    //the element that is being dragged
    union {
        Element *elem;
        Rectangle *rec;
        Vector2 *vec;
    } target;

    //the type of bounds
    enum {ELEM_BOUNDS, REC_BOUNDS, NO_BOUNDS} BoundsType;

    //the bounds of where the target can go, if null, is unbounded
    union {
        Element *elem;
        Rectangle *rec;
        Camera2D *cam;
    } bound;

    //how the bounding box works
    enum BoundStyle {
        //the target must be fully inside the bounds
        BOUND_BOX,

        //the target must, at some point, touch the bounds
        BOUND_TOUCH
    } boundStyle;

    //scales the amount the target gets dragged
    float scale;

    //the camera used, if null, no cam is used
    Camera2D *cam;
} Dragger;

//updates drag
void UpdateDrag(Dragger drag);

//applies the bounding box to a gragged element
void BoundDrag(Dragger drag);

//the output styles for a button
enum ButtonOutputStyle {
    //output is set to 1 for exactly one frame when the mouse goes up
    BUTTON_OUTPUT_FRAME,

    //output is on when the element is clicked until it is unclicked
    BUTTON_OUTPUT_SELECT,

    //changes state when the user clicks on it
    BUTTON_OUTPUT_TOGGLE,

    //allows for custom behavior by doing nothing on its own
    BUTTON_OUTPUT_NONE
};

//the input styles for a button
enum ButtonInputStyle {

    //when the mouse goes down while on the hitbox
    BUTTON_INPUT_ON,

    //when the mouse goes up while on the hitbox
    BUTTON_INPUT_OFF,

    //if the mouse is down while it is over the hitbox
    BUTTON_INPUT_DOWN,

    //clicked if the mouse went down and up while over the button
    BUTTON_INPUT_ON_AND_OFF,

    //same as on and off with the added condition that the mouse must have remained
    //on the button while it was down
    BUTTON_INPUT_STAY,

    //allows for custom behavior by doing nothing on its own
    BUTTON_INPUT_NONE
};

//a button
typedef struct Button {
    //the type of hitbox
    enum {ELEM_BUTTON, REC_BUTTON} hitboxType;

    //the hitbox that can be clicked
    union {
        Element *elem;
        Rectangle *rec;
    } hitbox;

    //if the mouse is on the button
    char mouseOn;

    //if the mouse went down over the button
    char mouseDownOver;

    //if the mouse went up over the button
    char mouseUpOver;

    //if the mouse leaves the hitbox while down
    char mouseLeft;

    //if the mouse goes down, then goes over the hitbox
    char mouseEntered;

    enum ButtonInputStyle inputStyle;

    //what output the button gives
    char output;

    enum ButtonOutputStyle outputStyle;

    //if there is a camea for the thing that the button is applied to
    Camera2D *cam;
} Button;

//updates the button
void UpdateButton(Button *btn);

//makes a button from an element. this doesn't allocate any mem, just a QoL
//thing since a lot of params in the Button struct will, in most cases, be 0
Button MakeElemButton(
    Camera2D *cam,
    Element *hitbox,
    enum ButtonInputStyle inStyle,
    enum ButtonOutputStyle outStyle
);

//makes a button from an rectangle. this doesn't allocate any mem, just a QoL
//thing since a lot of params in the Button struct will, in most cases, be 0
Button MakeRectButton(
    Camera2D *cam,
    Rectangle *hitbox,
    enum ButtonInputStyle inStyle,
    enum ButtonOutputStyle outStyle
);

#endif
