#ifndef RAYUI_H
#define RAYUI_H
#include <raylib.h>

//centers the camera
void CenterCamera(Camera2D *cam);

//zooms the given float based on mouse wheel movement, speed, and bounds
void Zoom(float *fl, float min, float max, float speed);

//a grid on which elements or rectangles can be placed
typedef struct Grid {
    //the position of the grid
    Vector2 pos;

    //the number of rows in the grid
    int numRows;

    //the number of columns in the grid
    int numCols;

    //list of where the rows in the grid end, should be in increasing order
    float *rowEnds;

    //list of where the columns in the grid end, should be in increasing order
    float *colEnds;

    //if scale is NULL, then it is treated as 1.0f
    float *scale;
} Grid;

//moves a rectangle to a spot on a grid
void MoveRecToGrid(
    int rowStart,
    int rowEnd,
    int colStart,
    int colEnd,
    Grid grid,
    Rectangle *rec
);

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

//moves an element to a pos on a grid
void MoveElemToGrid(
    int rowStart,
    int rowEnd,
    int colStart,
    int colEnd,
    Grid grid,
    Element *elem
);

//sets the scaled position of an element to a vec2
void SetElemPos(Element *elem, Vector2 pos);

//sets the scaled dimensions of an element to a vec2
void SetElemDim(Element *elem, Vector2 dim);

//gets the scaled position of an element
Vector2 GetElemPos(Element elem);

//gets the scaled dimensions of an element
Vector2 GetElemDim(Element elem);

//dragger hitbox type
enum DraggerHitboxType {
    //an elem being clicked detirmines if the dragger should drag
    DRAGGER_ELEM_HITBOX,

    //a rec being clicked derirmines if the dragger should drag
    DRAGGER_REC_HITBOX,

    //the dragger does not detirmine for itself if it should drag anything
    //instead, you should manually set .clicked for each dragger with this
    //hitbox type
    DRAGGER_NO_HITBOX,

    //.clicked is set to whatever DraggerHitbox.clickedPtr is set to for
    //draggers of this type
    DRAGGER_PTR_HITBOX
};

//the hitbox of a dragger
union DraggerHitbox {
    Element *elem;
    Rectangle *rec;
    char *clickedPtr;
};

//dragger target type
enum DraggerTargetType {
    DRAGGER_ELEM_TARGET, DRAGGER_REC_TARGET, DRAGGER_VECTORS_TARGET
};

//the target of a dragger
union DraggerTarget {
    Element *elem;
    Rectangle *rec;
    //if *dim is NULL, then it is interpreted as {0.0f, 0.0f}
    struct {
        Vector2 *pos;
        Vector2 *dim;
    } vecs;
};

//Dragger bounds type
enum DraggerBoundsType {
    DRAGGER_ELEM_BOUNDS, DRAGGER_REC_BOUNDS, DRAGGER_NO_BOUNDS
};

//the bounds for a dragger
union DraggerBounds {
    Element *elem;
    Rectangle *rec;
};

//dragger bounds style
    enum DraggerBoundStyle {
    //the target must be fully inside the bounds
    DRAGGER_BOUND_BOX,

    //the target must, at some point, touch the bounds
    DRAGGER_BOUND_TOUCH,

    //no bounds exist
    DRAGGER_BOUND_FALSE
};

//allows the user to drag elements (in screen space)
typedef struct Dragger {

    //the type of hitbox
    enum DraggerHitboxType hitboxType;

    //the hitbox that can be clicked to drag
    union DraggerHitbox hitbox;

    //if the dragger was clicked
    char clicked;

    //if the dragger was clicked the last time it was updated
    char prevClicked;

    //where the hitbox was when it got clicked
    Vector2 clickedPos;

    //where the mouse was last update
    Vector2 prevMousePos;

    //the type of target
    enum DraggerTargetType targetType;

    //the element that is being dragged
    union DraggerTarget target;

    //the type of bounds
    enum DraggerBoundsType boundsType;

    //the bounds of where the target can go, if null, is unbounded
    union DraggerBounds bound;

    //how the bounding box works
    enum DraggerBoundStyle boundStyle;

    //scales the amount the target gets dragged
    float *scale;

    //the camera used, if null, no cam is used
    Camera2D *cam;
} Dragger;

//makes a dragger, although does not alloc mem
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
);

//updates drag
void UpdateDrag(Dragger *drag);

//applies the bounding box to a gragged element
void BoundDrag(
    enum DraggerTargetType targetType,
    union DraggerTarget target,
    enum DraggerBoundsType boundsType,
    union DraggerBounds bound,
    enum DraggerBoundStyle boundStyle
);

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

    //same as on and off with the added condition that the mouse must have
    //remained on the button while it was down
    BUTTON_INPUT_STAY,

    //turns on when the user clicks on it, turns off when the mouse leaves the
    //button
    BUTTON_INPUT_FOCUS,

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

    //if the mouse was on the button last update
    char mouseWasOn;

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

//gets the output of a button given inital conditions and an input and output
//style. This style may be diffrent than the style in the button itself which
//allows for one button to give outputs in many styles. Also, given that the
//output of a button is usually dependent on the previous output of the button,
//it is another param
char GetButtonOutput(
    Button *btn,
    enum ButtonInputStyle inputStyle,
    enum ButtonOutputStyle outputStyle,
    char prevOutput
);

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

struct CharRange {
    //the lowest char in the range (inclusive)
    char min;

    //the highest char allowed in the rane
    char max;
};

typedef struct TextInput {
    //where the text input goes to
    char *output;

    //the maximum len of the output
    unsigned int maxLen;

    //current length of the output
    unsigned int curLen;

    //if the text input is currently being used
    char focused;

    //focused is set to the value at this ptr unless it is NULL
    char *focusPtr;

    //the number of char ranges
    char charRangesNum;

    //ranges of chars that are allowed as inputs
    struct CharRange *allowedChars;
} TextInput;

TextInput MakeTextInput
(
    char *output,
    unsigned int maxLen,
    char *focusedPtr,
    char charRangesNum,
    struct CharRange *allowedChars  
);

void UpdateTextInput(TextInput *input);

#endif
