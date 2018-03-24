//
// GUIslice Library Examples
// - Calvin Hass
// - https://www.impulseadventure.com/elec/guislice-gui.html
// - https://github.com/ImpulseAdventure/GUIslice
// - Example 02 (Arduino): [minimum RAM version]
//   - Accept touch input, text button
//   - Demonstrates the use of ElemCreate*_P() functions.
//     These RAM-reduced examples take advantage of the internal
//     Flash storage (via PROGMEM).
//

// ARDUINO NOTES:
// - GUIslice_config.h must be edited to match the pinout connections
//   between the Arduino CPU and the display controller (see ADAGFX_PIN_*).

#include "GUIslice.h"
#include "GUIslice_ex.h"
#include "GUIslice_drv.h"

// Defines for resources

// Enumerations for pages, elements, fonts, images
enum {E_PG_1,E_PG_2, E_PG_3};
enum {E_ELEM_BOX,E_ELEM_BTN_1, E_ELEM_BTN_2};
enum {E_FONT_BTN};

bool    m_bQuit = false;

// Instantiate the GUI
#define MAX_PAGE 3
#define MAX_FONT 1
#define MAX_ELEM_PG_2 22
#define MAX_ELEM_PG_1 14
#define MAX_ELEM_PG_3 2

// Define the maximum number of elements per page
// - To enable the same code to run on devices that support storing
//   data into Flash (PROGMEM) and those that don't, we can make the
//   number of elements in Flash dependent upon GSLC_USE_PROGMEM
// - This should allow both Arduino and ARM Cortex to use the same code
#if (GSLC_USE_PROGMEM)
  #define MAX_ELEM_PG_2_PROG   22                                         // # Elems in Flash
  #define MAX_ELEM_PG_1_PROG   14
  #define MAX_ELEM_PG_3_PROG   2
#else
  #define MAX_ELEM_PG_1_PROG   0                                         // # Elems in Flash
  #define MAX_ELEM_PG_2_PROG   0                                         // # Elems in Flash
  #define MAX_ELEM_PG_3_PROG   0                                         // # Elems in Flash
#endif
#define MAX_ELEM_PG_1_RAM      MAX_ELEM_PG_1 - MAX_ELEM_PG_1_PROG  // # Elems in RAM
#define MAX_ELEM_PG_2_RAM      MAX_ELEM_PG_2 - MAX_ELEM_PG_2_PROG  // # Elems in RAM
#define MAX_ELEM_PG_3_RAM      MAX_ELEM_PG_3 - MAX_ELEM_PG_3_PROG  // # Elems in RAM

gslc_tsGui                  m_gui;
gslc_tsDriver               m_drv;
gslc_tsFont                 m_asFont[MAX_FONT];
gslc_tsPage                 m_asPage[MAX_PAGE];

gslc_tsElem                 m_asFirstElem[MAX_ELEM_PG_1_RAM];   // Storage for all elements in RAM
gslc_tsElemRef              m_asFirstElemRef[MAX_ELEM_PG_2];

gslc_tsElem                 m_asSecondElem[MAX_ELEM_PG_2_RAM];
gslc_tsElemRef              m_asSecondElemRef[MAX_ELEM_PG_2];

gslc_tsElem                 m_asThirdElem[MAX_ELEM_PG_3_RAM];
gslc_tsElemRef              m_asThirdElemRef[MAX_ELEM_PG_2];



// Define debug message function
static int16_t DebugOut(char ch) { Serial.write(ch); return 0; }

// Button callbacks
bool First(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
//  if (eTouch == GSLC_TOUCH_UP_IN) {
//    gslc_SetPageCur(&m_gui,E_PG_2);
//  }
  gslc_SetPageCur(&m_gui,E_PG_2);
  return true;
}

// Button callbacks
bool Second(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
//  if (eTouch == GSLC_TOUCH_UP_IN) {
//    gslc_SetPageCur(&m_gui,E_PG_2);
//  }
  gslc_SetPageCur(&m_gui,E_PG_3);
  return true;
}

// Button callbacks
bool Third(void* pvGui,void *pvElemRef,gslc_teTouch eTouch,int16_t nX,int16_t nY)
{
//  if (eTouch == GSLC_TOUCH_UP_IN) {
//    gslc_SetPageCur(&m_gui,E_PG_2);
//  }
  gslc_SetPageCur(&m_gui,E_PG_1);
  return true;
}
bool InitOverlays(){
  gslc_tsElemRef* pElemRef = NULL;

  gslc_PageAdd(&m_gui,E_PG_1,m_asFirstElem,MAX_ELEM_PG_1_RAM,m_asFirstElemRef,MAX_ELEM_PG_1);
  gslc_PageAdd(&m_gui,E_PG_2,m_asSecondElem,MAX_ELEM_PG_2_RAM,m_asSecondElemRef,MAX_ELEM_PG_2);
  gslc_PageAdd(&m_gui,E_PG_3,m_asThirdElem,MAX_ELEM_PG_3_RAM,m_asThirdElemRef,MAX_ELEM_PG_3);
  
  gslc_SetBkgndColor(&m_gui,GSLC_COL_GRAY_DK2);
  
  // PAGE: 1

  // Create background box
  gslc_ElemCreateBox_P(&m_gui,100,E_PG_1,20,50,280,150,GSLC_COL_WHITE,GSLC_COL_BLACK,true,true,NULL,NULL);

//    // Create title
//  gslc_ElemCreateTxt_P(&m_gui,101,E_PG_1,10,10,310,40,"Configuration",&m_asFont[2], // E_FONT_TITLE
//          GSLC_COL_WHITE,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_MID,false,false);

  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,101,E_PG_1,120,100,80,40,"GO",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,&First,NULL); 

  // PAGE: 2
  // Background flat color
//  gslc_SetBkgndColor(&m_gui,GSLC_COL_GRAY_DK2);

  // Create background box
  gslc_ElemCreateBox_P(&m_gui,102,E_PG_2,10,50,300,150,GSLC_COL_WHITE,GSLC_COL_BLACK,true,true,NULL,NULL);


  gslc_ElemCreateTxt_P(&m_gui,201,E_PG_2,20,60,10,10,"Input 1",&m_asFont[1], // E_FONT_TXT
          GSLC_COL_YELLOW,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,222,E_PG_2,70,60,50,20,"Forward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,223,E_PG_2,130,60,50,20,"Backward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,202,E_PG_2,190,60,50,20,"Left",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,203,E_PG_2,250,60,50,20,"Right",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateTxt_P(&m_gui,204,E_PG_2,20,90,10,10,"Input 2",&m_asFont[1], // E_FONT_TXT
          GSLC_COL_YELLOW,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,205,E_PG_2,70,90,50,20,"Forward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,206,E_PG_2,130,90,50,20,"Backward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,207,E_PG_2,190,90,50,20,"Left",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,208,E_PG_2,250,90,50,20,"Right",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateTxt_P(&m_gui,209,E_PG_2,20,120,10,10,"Input 3",&m_asFont[1], // E_FONT_TXT
          GSLC_COL_YELLOW,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,210,E_PG_2,70,120,50,20,"Forward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,211,E_PG_2,130,120,50,20,"Backward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,212,E_PG_2,190,120,50,20,"Left",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,213,E_PG_2,250,120,50,20,"Right",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateTxt_P(&m_gui,214,E_PG_2,20,150,10,10,"Input 4",&m_asFont[1], // E_FONT_TXT
          GSLC_COL_YELLOW,GSLC_COL_BLACK,GSLC_COL_BLACK,GSLC_ALIGN_MID_LEFT,false,true);
  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,215,E_PG_2,70,150,50,20,"Forward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,216,E_PG_2,130,150,50,20,"Backward",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,217,E_PG_2,190,150,50,20,"Left",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,218,E_PG_2,250,150,50,20,"Right",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,NULL,NULL);

  gslc_ElemCreateBtnTxt_P(&m_gui,219,E_PG_2,250,175,100,50,"Next",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,&Second,NULL);


 //PAGE 3:

  // Create background box
  gslc_ElemCreateBox_P(&m_gui,300,E_PG_3,10,50,300,150,GSLC_COL_WHITE,GSLC_COL_BLACK,true,true,NULL,NULL);

  // Create Quit button with text label
  gslc_ElemCreateBtnTxt_P(&m_gui,301,E_PG_3,120,100,80,40,"READY",&m_asFont[0],
    GSLC_COL_WHITE,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK4,GSLC_COL_BLUE_DK2,GSLC_COL_BLUE_DK1,GSLC_ALIGN_MID_MID,true,true,&Third,NULL); 

  return true;
}

void setup()
{
  Serial.begin(9600);
  gslc_InitDebug(&DebugOut);

  if (!gslc_Init(&m_gui,&m_drv,m_asPage,MAX_PAGE,m_asFont,MAX_FONT)) { return; }

  if (!gslc_FontAdd(&m_gui,E_FONT_BTN,GSLC_FONTREF_PTR,NULL,1)) { return; }    // m_asFont[0]
  //if (!gslc_FontAdd(&m_gui,E_FONT_TXT,GSLC_FONTREF_PTR,NULL,1)) { return; }    // m_asFont[1]
  //if (!gslc_FontAdd(&m_gui,E_FONT_TITLE,GSLC_FONTREF_PTR,NULL,1)) { return; }  // m_asFont[2]

  InitOverlays();

  gslc_SetPageCur(&m_gui,E_PG_1);

  
}

void loop()
{
  // Periodically call GUIslice update function
  gslc_Update(&m_gui);

  // In a real program, we would detect the button press and take an action.
  // For this Arduino demo, we will pretend to exit by emulating it with an
  // infinite loop. Note that interrupts are not disabled so that any debug
  // messages via Serial have an opportunity to be transmitted.
  if (m_bQuit) {
    gslc_Quit(&m_gui);
    while (1) { }
  }
}



