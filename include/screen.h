#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "ugui.h"
#include "buttons.h"

/**
 * Main screen notes
 *
 * General approach:
 *
 * Make it look approximately like https://github.com/OpenSource-EBike-firmware/SW102_LCD_Bluetooth/blob/master/design/Bafang_SW102_Bluetooth_LCD-design-proposal_1.jpg
 *
 * Split UX logic from display layout with a small screen layer - to allow refactoring and sharing much of the 850C behavior/code
 * with the SW102.  Screen redraw time should go down dramatically because only change fields will need to be drawn.
 * The 850C (I think?) is calling that redraw function a lot - a full screen transfer to the display for each field update.
 *
 * initially just show power remaining in pack, SOC, time, amount of assist
 * make assist go up/down with button press. (initially copy 850C button code, but _real soon_ to allow sharing the C source for button/packet/UX handling between both
 * modern displays)
 *
 * remove call to UG_SetRefresh, it is very expensive.  Much better to just call lcd_refresh() from screenUpdate()
 *
 * define a screen as:
 *
 * an array of FieldLayouts.  each FieldLayout is a (typically constant) struct:
 * xpos, ypos, width, height, colorOpt, fieldPtr
 * colorOpt is an enum of ColorNormal, ColorInvert
 * eventually: ColorCritical, ColorBlink (for use in menu selections) -for color
 * LCDs these enums could map to the theme picked for the UX, for black and white LCDs they map to black or white
 *
 * end of screen is market by a FieldLayout with a null field member
 *
 * fieldPtr points to a particular field with ready to render data (and fields might be shared by multiple screens).
 * Each Field contains
 *   variantCode, isDirty and extra data appropriate for that variant
 *
 * isDirty is true if the field needs to be redrawn in the gui (because data has changed)
 * datavariant is a union that depends on the opcode:
 *
 * drawText: font ptr, char msg[MAXSTRLEN]
 * fillBox: nothing - just fills box based on fore/back color
 * drawBat: soc - draw a bat icon with SOC
 * drawPlot: maxval - max value seen, points - an array of 64 previous data values, to be drawn as a plot, one per column

 *
 * helper functions:
 * fieldPrintf(fieldptr, "str %d", 5) - sets the string for the specified fields, marks the field as dirty if the string changed
 * fieldSetSOC(fieldptr, 32) - sets state of charge and marks field as dirty if the soc changed
 * fieldAddPlot(fieldptr, value) - add a new data point to a plot field
 *
 * When new state is received from the controller, fieldX...() will be called to mark the various fields as dirty.  These functions
 * are cheap and can be called when each rx packet is parsed.  If any field changed in a user visible way the field will be internally
 * marked as dirty and later updateScreen() will show that new value
 *
 * screenShow(screenptr) - set the current screen
 * screenUpdate() - redraw the minimum set of dirty fields (or the whole screen if the screen has changed).
 *   if any fields are blinking the blink animation will be serviced here as well.
 *
 * NOTE: this approach could be extended to include nice support for showing vertically scrolling menus.  Initial version
 * won't do this but should make the config screen fairly easy to code up.
 *
 * add a datavariant for
 * drawMenu: curSelection, const char **menuOptions, const char **menuValues, onChange(tbd) - properly handle showing a scrolling menu
 *   that might be logically longer than our screen. onChange callback will be called when the user changes menu entries
 *
 * bool screenHandlePress(buttonEnum)
 * if a screen is showing a menu field, it might need to intercept and handle button presses. Call this function from
 * the main loop when a press occurs.  This function will return true if it has handled this press event (and therefore
 * you should not do anything else with it).
 *
 */

#define MAX_FIELD_LEN 16

typedef enum {
  ColorNormal = 0, // white on black
  ColorInvert,     // black on white
  ColorSelected    // If we should mark that there is a cursor pointing to this item (for scrollable rows)
} ColorOp;

/**
 * specifies what type of data is in this field
 *
 * Note: might change someday to instead just be a pointer to a constant vtable like thing
 */
typedef enum {
  FieldDrawText = 0,
  FieldFill, // Fill with a solid color
  FieldMesh, // Fill with a mesh color
  FieldScrollable, // Contains a menu name and points to a submenu to optionally expand its place.  If at the root of a screen, submenu will be automatically expanded to fill remaining screen space
  FieldEditable, // An editable property with a human visible label and metadata for min/max/type of data and ptr to raw variable to render
  FieldEnd // Marker record for the last entry in a scrollable submenu - never shown to user
} FieldVariant;

/**
 * What sorts of types are supported for FieldEditable.  Currently just uint8, uint16, bool and choosing a string from a list and storing its
 * index
 */
typedef enum {
  EditUInt = 0, // This is the default type if not specified
  EditEnum // Choose a string from a list
} EditableType;

/**
 * Ready to render data (normally populated by comms code) which might be used on multiple different screens
 */
typedef struct Field {
  FieldVariant variant; //
  bool dirty; // true if this data has changed and needs to be rerendered

  union {
    struct {
      const UG_FONT *font;
      char msg[MAX_FIELD_LEN];
    } drawText;

    struct {
      struct Field *entries; // the menu entries for this submenu.
      const char *label; // the title shown in the GUI for this menu
      uint8_t first; // The first entry we are showing on the screen (ie for scrolling through a series of entries)
      uint8_t selected; // the currently highlighted entry
    } scrollable;

    struct {
      const char *label; // the label shown in the GUI for this item
      void *target; // the data we are showing/manipulating
      const EditableType typ;

      // the following parameters are particular to the editable type
      union {

        struct {
          const char *units;
          const uint8_t size; // sizeof for the specified variable - we support 1 or 2, 4 would be easy
          const uint8_t div_digits; // how many digits to divide by for fractions (i.e. 0 for integers, 1 for /10x, 2 for /100x, 3 /1000x
          const uint32_t max_value, min_value; // min/max
          const uint32_t inc_step;
        } number;

        struct {
          // we assume *target is a uint8_t
          const char **options; // An array of strings, with a NULL entry at the end to mark end of choices
        } editEnum;
      };
    } editable;
  };
} Field;

//
// Helper macros to declare fields more easily
//

#define FIELD_SCROLLABLE(lbl, arry) { .variant = FieldScrollable, .scrollable = { .label = lbl, .entries = arry } }

#define FIELD_EDITABLE_UINT(lbl, targ, unt, minv, maxv, ...) { .variant = FieldEditable, \
  .editable = { .typ = EditUInt, .label = lbl, .target = targ, \
      .number = { .size = sizeof(*targ), .units = unt, .max_value = maxv, .min_value = minv, ##__VA_ARGS__ } } }

// C99 allows anonymous constant arrays - take advantage of that here to make declaring the various options easy
#define FIELD_EDITABLE_ENUM(lbl, targ, ...) { .variant = FieldEditable, \
  .editable = { .typ = EditEnum, .label = lbl, .target = targ, \
      .editEnum = { .options = (const char *[]){ __VA_ARGS__ } } } }

#define FIELD_DRAWTEXT(fnt) { .variant = FieldDrawText, .drawText = { .font = fnt } }

#define FIELD_END { .variant = FieldEnd }


typedef int16_t Coord; // Change to int16_t for screens wider/longer than 128, screens shorter than 128 can use uint8_t

/**
 * Defines the layout of a field on a particular screen
 */
typedef struct {
  Coord x, y;

  // for text fields if negative width is in # of characters. or 0 to determine length based on remaining screen width
  // For all other cases, width is in pixels
  Coord width;

  // for text fields use height = -1 to determine height based on font size.  for all fields 0 means 'rest of screen'
  // for other cases height is in pixels
  Coord height;

  ColorOp color;
  Field *field;
} FieldLayout;


/** Called when a press has occured, return true if this function has handled the event (and therefore it should be cleared)
or false to let others handle it.

The order of handlers is:
* any active editable controls
* any scrollable controls on this screen
* the current screen handler
* the top level application handler
The top three handlers in the list above are handled by screen
*/
typedef bool (*ButtonEventHandler)(buttons_events_t events);

typedef struct {
  ButtonEventHandler onPress; // or NULL for no handler
  FieldLayout fields[];
} Screen;

// Standard vertical spacing for fonts
#define FONT12_Y 14 // we want a little bit of extra space


void screenShow(Screen *screen);
void screenUpdate();

/// Returns true if the current screen handled the press
bool screenOnPress(buttons_events_t events);

void fieldPrintf(Field *field, const char *fmt, ...);
