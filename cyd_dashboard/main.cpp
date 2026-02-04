
/**
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
// Example CYD  ESP32-2432S028R LVGL KEYBOARD WIDGET                       //
// Design UI on Squareline Studio. LVGL V9.1                               //
// Youtube:https://www.youtube.com/@pangcrd                                //
// Github: https://github.com/pangcrd                                      //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
**/

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "lvgl.h"
#include "ui.h"
#include <Arduino.h>
#include <WiFi.h>
#include "Settimino.h"

// WiFi Credentials
const char* ssid = "roku";
const char* password = "Linux.456";

// UI Screens
lv_obj_t * ui_Menu;
lv_obj_t * ui_Gauges;
lv_obj_t * ui_Editor;
lv_obj_t * ui_Others;
bool is_dark_mode = true; 

// UI Elements (Gauges)
lv_obj_t * ui_Arc1;
lv_obj_t * ui_Arc2;
lv_obj_t * ui_ArcLabel1;
lv_obj_t * ui_ArcLabel2;

// UI Elements (Editor)
lv_obj_t * ui_TA[4]; // VW3, VW4, VW5, VW6
uint16_t ta_addrs[4] = {4, 6, 8, 10}; 
lv_obj_t * ui_SharedKB;

// UI Elements (Markers)
lv_obj_t * ui_MBtn[9];
bool marker_states[9] = {false};

// PLC Configuration
IPAddress plc_ip(192, 168, 50, 69);
S7Client Client;
byte buffer[1024]; 

// Connection Management
unsigned long lastConnectAttempt = 0;
const unsigned long connectInterval = 10000; 
bool connectionCooldown = false;



/** Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/
/** Change to your screen resolution **/
static const uint16_t screenWidth  = 320;
static const uint16_t screenHeight = 240;

enum { SCREENBUFFER_SIZE_PIXELS = screenWidth * screenHeight / 10 };
static lv_color_t* buf; // Will be allocated on heap

TFT_eSPI tft = TFT_eSPI( screenWidth, screenHeight ); /** TFT instance **/
/** Touch screen config **/
#define XPT2046_IRQ 36 
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

/*Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI)*/
#define LV_COLOR_16_SWAP 0

SPIClass tsSpi = SPIClass(VSPI);
XPT2046_Touchscreen ts (XPT2046_CS, XPT2046_IRQ);

/** Run calib_touch files to get value  **/
uint16_t touchScreenMinimumX = 200, touchScreenMaximumX = 3700, touchScreenMinimumY = 240,touchScreenMaximumY = 3800; 

/** Display flushing **/
void my_disp_flush (lv_display_t *disp, const lv_area_t *area, uint8_t *pixelmap)
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    if (LV_COLOR_16_SWAP) {
        size_t len = lv_area_get_size( area );
        lv_draw_sw_rgb565_swap( pixelmap, len );
    }

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( (uint16_t*) pixelmap, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/** ========== Read Touch ==========**/
void my_touch_read (lv_indev_t *indev_drv, lv_indev_data_t * data)
{
    if(ts.touched())
    {
        TS_Point p = ts.getPoint();
        /** Some very basic auto calibration so it doesn't go out of range **/
        if(p.x < touchScreenMinimumX) touchScreenMinimumX = p.x;
        if(p.x > touchScreenMaximumX) touchScreenMaximumX = p.x;
        if(p.y < touchScreenMinimumY) touchScreenMinimumY = p.y;
        if(p.y > touchScreenMaximumY) touchScreenMaximumY = p.y;
        /** Map this to the pixel position **/
        data->point.x = map(p.x,touchScreenMinimumX,touchScreenMaximumX,1,screenWidth); /** Touchscreen X calibration **/
        data->point.y = map(p.y,touchScreenMinimumY,touchScreenMaximumY,1,screenHeight); /** Touchscreen Y calibration **/
        data->state = LV_INDEV_STATE_PR;

        // Serial.print( "Touch x " );
        // Serial.print( data->point.x );
        // Serial.print( " y " );
        // Serial.println( data->point.y );
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/** Function to read data and update the active screen (One-shot) **/
void sync_plc_data() {
    if (!Client.Connected) {
        if (Client.Connect() != 0) return;
    }

    lv_obj_t * active_scr = lv_screen_active();
    
    if (active_scr == ui_Gauges) {
        int res = Client.ReadArea(S7AreaDB, 1, 0, 4, buffer);
        if (res == 0) {
            int16_t v1 = S7.IntegerAt(buffer, 0); 
            int16_t v2 = S7.IntegerAt(buffer, 2); 
            lv_arc_set_value(ui_Arc1, v1);
            lv_label_set_text_fmt(ui_ArcLabel1, "%d", v1);
            lv_arc_set_value(ui_Arc2, v2);
            lv_label_set_text_fmt(ui_ArcLabel2, "%d", v2);
        }
    } 
    else if (active_scr == ui_Editor) {
        int res = Client.ReadArea(S7AreaDB, 1, 4, 8, buffer); // Read VW3 to VW6
        if (res == 0) {
            for(int i=0; i<4; i++) {
                // Address 4, 6, 8, 10 mapped in code
                int16_t v = S7.IntegerAt(buffer, i * 2);
                lv_textarea_set_text(ui_TA[i], String(v).c_str());
            }
        }
    }
    else if (active_scr == ui_Others) {
        byte m_buf[2];
        if (Client.ReadArea(S7AreaMK, 0, 0, 2, m_buf) == 0) {
            for(int i=0; i<9; i++) {
                bool s = S7.BitAt(m_buf, i / 8, i % 8);
                marker_states[i] = s;
                lv_obj_set_style_bg_color(ui_MBtn[i], 
                    s ? lv_palette_main(LV_PALETTE_LIME) : lv_palette_main(LV_PALETTE_RED), 0);
            }
        }
    }
}

/** Screen Load Event Handler **/
static void screen_load_event_cb(lv_event_t * e) {
    sync_plc_data();
}

/** Keyboard Event for Editor **/
static void editor_kb_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * kb = (lv_obj_t *)lv_event_get_target(e);
    if(code == LV_EVENT_READY) {
        lv_obj_t * ta = lv_keyboard_get_textarea(kb);
        if(ta) {
            int addr = (int)(uintptr_t)lv_obj_get_user_data(ta);
            int val = atoi(lv_textarea_get_text(ta));
            byte out_buf[2];
            S7.SetIntAt(out_buf, 0, (int16_t)val);
            if(Client.WriteArea(S7AreaDB, 1, addr, 2, out_buf) == 0) {
                Serial.printf("Wrote %d to Addr %d\n", val, addr);
                sync_plc_data(); // Sync immediately after write
            }
        }
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    } else if(code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        sync_plc_data(); // Revert to current PLC value
    }
}

/** Function to Apply Global Theme **/
void apply_global_theme() {
    lv_display_t * disp = lv_display_get_default();
    lv_theme_t * th = lv_theme_default_init(disp, 
                                           lv_palette_main(LV_PALETTE_BLUE), 
                                           lv_palette_main(LV_PALETTE_CYAN), 
                                           is_dark_mode, 
                                           LV_FONT_DEFAULT);
    lv_display_set_theme(disp, th);
}

/** Theme Switch Callback **/
static void theme_switch_event_cb(lv_event_t * e) {
    lv_obj_t * sw = (lv_obj_t *)lv_event_get_target(e);
    is_dark_mode = lv_obj_has_state(sw, LV_STATE_CHECKED);
    apply_global_theme();
}

/** Setup Main Menu Screen **/
void setup_ui_menu() {
    ui_Menu = lv_obj_create(NULL);
    
    lv_obj_t * title = lv_label_create(ui_Menu);
    lv_label_set_text(title, "TecoTrack PLC Dashboard");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    lv_obj_t * btn1 = lv_button_create(ui_Menu);
    lv_obj_set_size(btn1, 140, 45);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -50);
    lv_obj_add_event_cb(btn1, [](lv_event_t * e){ lv_screen_load(ui_Gauges); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "MONITORING");
    lv_obj_center(lbl1);

    lv_obj_t * btn2 = lv_button_create(ui_Menu);
    lv_obj_set_size(btn2, 140, 45);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 5);
    lv_obj_add_event_cb(btn2, [](lv_event_t * e){ lv_screen_load(ui_Editor); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "EDITOR");
    lv_obj_center(lbl2);

    lv_obj_t * btn3 = lv_button_create(ui_Menu);
    lv_obj_set_size(btn3, 140, 45);
    lv_obj_align(btn3, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(btn3, [](lv_event_t * e){ lv_screen_load(ui_Others); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * lbl3 = lv_label_create(btn3);
    lv_label_set_text(lbl3, "MARKERS");
    lv_obj_center(lbl3);

    // Theme Switch
    lv_obj_t * sw = lv_switch_create(ui_Menu);
    lv_obj_align(sw, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    if(is_dark_mode) lv_obj_add_state(sw, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw, theme_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_t * sw_lbl = lv_label_create(ui_Menu);
    lv_label_set_text(sw_lbl, "Dark Mode");
    lv_obj_align_to(sw_lbl, sw, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_set_style_text_font(sw_lbl, &lv_font_montserrat_14, 0);
}

/** Setup Gauges Screen **/
void setup_ui_gauges() {
    ui_Gauges = lv_obj_create(NULL);
    
    ui_Arc1 = lv_arc_create(ui_Gauges);
    lv_obj_set_size(ui_Arc1, 100, 100);
    lv_arc_set_range(ui_Arc1, 0, 1000);
    lv_obj_align(ui_Arc1, LV_ALIGN_CENTER, -80, -20);
    ui_ArcLabel1 = lv_label_create(ui_Arc1);
    lv_obj_center(ui_ArcLabel1);
    lv_obj_t * t1 = lv_label_create(ui_Gauges);
    lv_label_set_text(t1, "VW1 (Addr 0)");
    lv_obj_align_to(t1, ui_Arc1, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    ui_Arc2 = lv_arc_create(ui_Gauges);
    lv_obj_set_size(ui_Arc2, 100, 100);
    lv_arc_set_range(ui_Arc2, 0, 1000);
    lv_obj_align(ui_Arc2, LV_ALIGN_CENTER, 80, -20);
    ui_ArcLabel2 = lv_label_create(ui_Arc2);
    lv_obj_center(ui_ArcLabel2);
    lv_obj_t * t2 = lv_label_create(ui_Gauges);
    lv_label_set_text(t2, "VW2 (Addr 2)");
    lv_obj_align_to(t2, ui_Arc2, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    lv_obj_t * back = lv_button_create(ui_Gauges);
    lv_obj_set_size(back, 80, 40);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(back, [](lv_event_t * e){ lv_screen_load(ui_Menu); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * backlbl = lv_label_create(back);
    lv_label_set_text(backlbl, "BACK");
    lv_obj_center(backlbl);
}

/** Setup Editor Screen **/
void setup_ui_editor() {
    ui_Editor = lv_obj_create(NULL);

    ui_SharedKB = lv_keyboard_create(ui_Editor);
    lv_keyboard_set_mode(ui_SharedKB, LV_KEYBOARD_MODE_NUMBER);
    lv_obj_set_size(ui_SharedKB, 180, 90); // 30% smaller
    lv_obj_align(ui_SharedKB, LV_ALIGN_BOTTOM_MID, 0, 0); // Override bottom area
    lv_obj_add_event_cb(ui_SharedKB, editor_kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(ui_SharedKB, LV_OBJ_FLAG_HIDDEN);

    for(int i=0; i<4; i++) {
        ui_TA[i] = lv_textarea_create(ui_Editor);
        lv_textarea_set_one_line(ui_TA[i], true);
        lv_obj_set_size(ui_TA[i], 100, 40);
        lv_obj_set_pos(ui_TA[i], (i%2==0) ? 40 : 180, (i<2) ? 40 : 100);
        lv_obj_set_user_data(ui_TA[i], (void*)(uintptr_t)ta_addrs[i]);
        
        lv_obj_t * l = lv_label_create(ui_Editor);
        lv_label_set_text_fmt(l, "Addr %d", ta_addrs[i]);
        lv_obj_align_to(l, ui_TA[i], LV_ALIGN_OUT_TOP_MID, 0, -5);

        lv_obj_add_event_cb(ui_TA[i], [](lv_event_t * e){
            lv_obj_remove_flag(ui_SharedKB, LV_OBJ_FLAG_HIDDEN);
            lv_keyboard_set_textarea(ui_SharedKB, (lv_obj_t *)lv_event_get_target(e));
        }, LV_EVENT_CLICKED, NULL);
    }

    lv_obj_t * back = lv_button_create(ui_Editor);
    lv_obj_set_size(back, 70, 35);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 5, -5);
    lv_obj_add_event_cb(back, [](lv_event_t * e){ lv_screen_load(ui_Menu); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * backlbl = lv_label_create(back);
    lv_label_set_text(backlbl, "BACK");
    lv_obj_center(backlbl);

    lv_obj_add_event_cb(ui_Editor, screen_load_event_cb, LV_EVENT_SCREEN_LOADED, NULL);
}

/** Marker Button Event Callback **/
static void marker_btn_event_cb(lv_event_t * e) {
    if (!Client.Connected) return;
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    int idx = (int)(uintptr_t)lv_obj_get_user_data(btn);
    bool new_state = !marker_states[idx];
    
    // LOGO Markers: M1-M8 = Byte 0, Bits 0-7. M9 = Byte 1, Bit 0.
    int res = Client.WriteBit(S7AreaMK, 0, idx, new_state);
    if(res == 0) {
        sync_plc_data(); // Sync immediately after change
    }
}

/** Setup Others (Markers) Screen **/
void setup_ui_others() {
    ui_Others = lv_obj_create(NULL);
    
    lv_obj_t * cont = lv_obj_create(ui_Others);
    lv_obj_set_size(cont, 300, 180);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    for(int i=0; i<9; i++) {
        ui_MBtn[i] = lv_button_create(cont);
        lv_obj_set_size(ui_MBtn[i], 80, 45);
        lv_obj_set_user_data(ui_MBtn[i], (void*)(uintptr_t)i);
        lv_obj_add_event_cb(ui_MBtn[i], marker_btn_event_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(ui_MBtn[i], lv_palette_main(LV_PALETTE_RED), 0);
        
        lv_obj_t * l = lv_label_create(ui_MBtn[i]);
        lv_label_set_text_fmt(l, "M%d", i+1);
        lv_obj_center(l);
    }

    lv_obj_t * back = lv_button_create(ui_Others);
    lv_obj_set_size(back, 80, 40);
    lv_obj_align(back, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_add_event_cb(back, [](lv_event_t * e){ lv_screen_load(ui_Menu); }, LV_EVENT_CLICKED, NULL);
    lv_obj_t * backlbl = lv_label_create(back);
    lv_label_set_text(backlbl, "BACK");
    lv_obj_center(backlbl);

    lv_obj_add_event_cb(ui_Others, screen_load_event_cb, LV_EVENT_SCREEN_LOADED, NULL);
}

/** Set tick routine needed for LVGL internal timings **/
static uint32_t my_tick_get_cb (void) { return millis(); }

/** Create timer for control function **/
// 
// }

/** Timer callback to read PLC data and update UI **/
void plc_read_timer_cb(lv_timer_t * timer) {
    if (WiFi.status() != WL_CONNECTED) return;
    unsigned long now = millis();

    if (!Client.Connected) {
        if (now - lastConnectAttempt > connectInterval) {
            lastConnectAttempt = now;
            Client.Disconnect(); 
            Client.Connect();
        }
        return;
    }

    // ONLY poll if we are on the Monitoring (Gauges) screen
    if(lv_screen_active() == ui_Gauges) {
        int res = Client.ReadArea(S7AreaDB, 1, 0, 4, buffer);
        if (res == 0) {
            int16_t v1 = S7.IntegerAt(buffer, 0); 
            int16_t v2 = S7.IntegerAt(buffer, 2); 
            lv_arc_set_value(ui_Arc1, v1);
            lv_label_set_text_fmt(ui_ArcLabel1, "%d", v1);
            lv_arc_set_value(ui_Arc2, v2);
            lv_label_set_text_fmt(ui_ArcLabel2, "%d", v2);
        } else if (res < 0x0100) {
            Client.Disconnect();
            lastConnectAttempt = now;
        }
    }
}



void setup (){

    Serial.begin( 115200 );

    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    int retry = 0;
    while (WiFi.status() != WL_CONNECTED && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Connected!");
    } else {
        Serial.println("\nWiFi Connection failed.");
    }

    // Initialize S7 Client parameters once
    Client.SetConnectionParams(plc_ip, 0x1000, 0x0200); 
    Client.RecvTimeout = 2000; // Increase timeout for stability

    lv_init();

    // Allocate buffer on heap to save DRAM
    buf = (lv_color_t*)malloc(SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t));
    if (buf == NULL) {
        Serial.println("Failed to allocate LVGL buffer!");
        while(1) delay(100);
    }

    //Initialise the touchscreen
    tsSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS); /* Start second SPI bus for touchscreen */
    ts.begin(tsSpi);      /* Touchscreen init */
    ts.setRotation(3);   /* Inverted landscape orientation to match screen */

    tft.begin();         /* TFT init */
    tft.setRotation(3); /* Landscape orientation, flipped */
                                             
    

    static lv_disp_t* disp;
    disp = lv_display_create( screenWidth, screenHeight );
    lv_display_set_buffers( disp, buf, NULL, SCREENBUFFER_SIZE_PIXELS * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL );
    lv_display_set_flush_cb( disp, my_disp_flush );

    //Initialize the Rotary Encoder input device. For LVGL version 9+ only
    lv_indev_t *touch_indev = lv_indev_create();
    lv_indev_set_type(touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(touch_indev, my_touch_read);


    lv_tick_set_cb( my_tick_get_cb );

    // Initialize Global Theme
    apply_global_theme();

    // Setup the multi-screen system
    setup_ui_menu();
    setup_ui_gauges();
    setup_ui_editor();
    setup_ui_others();

    // Start on the Menu
    lv_screen_load(ui_Menu);

 

    /** lv timer for run task */
    //lv_timer_create(led_update_cb, 5, NULL);
    
    // Create a timer to read from PLC every 1 second
    lv_timer_create(plc_read_timer_cb, 1000, NULL);

    Serial.println( "Setup done" );
}



void loop ()
{   
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
}
