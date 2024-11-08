#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "my_utils.h"
// #include "ui/ui.h"



#define LGFX_USE_V1
#define DEBUG_TOUCH 1

#define screenWidth 480
#define screenHeight 320

class LGFX : public lgfx::LGFX_Device{
  lgfx::Panel_ST7796      _panel_instance;
  lgfx::Bus_Parallel8     _bus_instance;   
  lgfx::Light_PWM         _light_instance;
  lgfx::Touch_FT5x06      _touch_instance; // FT5206, FT5306, FT5406, FT6206, FT6236, FT6336, FT6436

  public:
    LGFX(void)
    {
      { 
        auto cfg = _bus_instance.config();

        cfg.freq_write = 20000000;    
        cfg.pin_wr = 47; // pin number connecting WR
        cfg.pin_rd = -1; // pin number connecting RD
        cfg.pin_rs = 0;  // Pin number connecting RS(D/C)
        cfg.pin_d0 = 9;  // pin number connecting D0
        cfg.pin_d1 = 46; // pin number connecting D1
        cfg.pin_d2 = 3;  // pin number connecting D2
        cfg.pin_d3 = 8;  // pin number connecting D3
        cfg.pin_d4 = 18; // pin number connecting D4
        cfg.pin_d5 = 17; // pin number connecting D5
        cfg.pin_d6 = 16; // pin number connecting D6
        cfg.pin_d7 = 15; // pin number connecting D7
        //cfg.i2s_port = I2S_NUM_0; // (I2S_NUM_0 or I2S_NUM_1)

        _bus_instance.config(cfg);                    // Apply the settings to the bus.
        _panel_instance.setBus(&_bus_instance);       // Sets the bus to the panel.
      }

      { // Set display panel control.
        auto cfg = _panel_instance.config(); // Get the structure for display panel settings.

        cfg.pin_cs = -1;   // Pin number to which CS is connected (-1 = disable)
        cfg.pin_rst = 4;   // pin number where RST is connected (-1 = disable)
        cfg.pin_busy = -1; // pin number to which BUSY is connected (-1 = disable)

        // * The following setting values ​​are set to general default values ​​for each panel, and the pin number (-1 = disable) to which BUSY is connected, so please try commenting out any unknown items.

        cfg.memory_width = 320;  // Maximum width supported by driver IC
        cfg.memory_height = 480; // Maximum height supported by driver IC
        cfg.panel_width = 320;   // actual displayable width
        cfg.panel_height = 480;  // actual displayable height
        cfg.offset_x = 0;        // Panel offset in X direction
        cfg.offset_y = 0;        // Panel offset in Y direction
        cfg.offset_rotation = 0;  // was 2
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits = 1;
        cfg.readable = true;     // was false
        cfg.invert = true;
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = true; // was false something to do with SD?

        _panel_instance.config(cfg);
      }

      { // Set backlight control. (delete if not necessary)
        auto cfg = _light_instance.config(); // Get the structure for backlight configuration.

        cfg.pin_bl = 45;     // pin number to which the backlight is connected
        cfg.invert = false;  // true to invert backlight brightness
        cfg.freq = 44100;    // backlight PWM frequency
        cfg.pwm_channel = 0; // PWM channel number to use (7??)

        _light_instance.config(cfg);
        _panel_instance.setLight(&_light_instance); // Sets the backlight to the panel.
      }

  //*
      { // Configure settings for touch screen control. (delete if not necessary)
        auto cfg = _touch_instance.config();

        cfg.x_min = 0;   // Minimum X value (raw value) obtained from the touchscreen
        cfg.x_max = 319; // Maximum X value (raw value) obtained from the touchscreen
        cfg.y_min = 0;   // Minimum Y value obtained from touchscreen (raw value)
        cfg.y_max = 479; // Maximum Y value (raw value) obtained from the touchscreen
        cfg.pin_int = 7; // pin number to which INT is connected
        cfg.bus_shared = false; // set true if you are using the same bus as the screen
        cfg.offset_rotation = 0;

        // For I2C connection
        cfg.i2c_port = 0;    // Select I2C to use (0 or 1)
        cfg.i2c_addr = 0x38; // I2C device address number
        cfg.pin_sda = 6;     // pin number where SDA is connected
        cfg.pin_scl = 5;     // pin number to which SCL is connected
        cfg.freq = 400000;   // set I2C clock

        _touch_instance.config(cfg);
        _panel_instance.setTouch(&_touch_instance); // Set the touchscreen to the panel.
      }
  //*/
      setPanel(&_panel_instance); // Sets the panel to use.
    }
};

//Define tft display
LGFX tft;


// lv debugging can be set in lv_conf.h
#if LV_USE_LOG != 0
  /* Serial debugging */
  void my_print(const char * buf)
  {
      Serial.printf(buf);
      Serial.flush();
  }
#endif


/* Declare buffer for 1/10 screen size; BYTES_PER_PIXEL will be 2 for RGB565. */
// #define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
// static uint8_t buf1[screenWidth * screenHeight / 10 * BYTES_PER_PIXEL];
// static uint8_t buf2[screenWidth * screenHeight / 10 * BYTES_PER_PIXEL];

#define DRAW_BUF_SIZE (screenWidth * screenHeight / 10 * (LV_COLOR_DEPTH / 8))
static uint32_t buf1[DRAW_BUF_SIZE / 4];
// static uint32_t buf2[DRAW_BUF_SIZE / 4];

/* Set display buffer for display `display1`. */
void my_display_flush(lv_display_t * display, const lv_area_t * area, uint8_t * px_map){
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  uint16_t * buf16 = (uint16_t *)px_map;

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  // tft.writePixels((lgfx::rgb565_t *)&color_p->full, w * h);
  // tft.writePixels((lgfx::rgb565_t *)buf16, w * h);
  tft.writePixels((lgfx::rgb565_t *)px_map, w * h);
  tft.endWrite();

  /* IMPORTANT!!!
    * Inform LVGL that flushing is complete so buffer can be modified again. */
  lv_display_flush_ready(display);
}

/*Read the touchpad*/
void my_touch_read(lv_indev_t *indev_driver, lv_indev_data_t *data) {
    
    uint16_t touchX, touchY;
    bool touched = tft.getTouch(&touchX, &touchY);
    if (!touched) { data->state = LV_INDEV_STATE_REL; }
    else {
      data->state = LV_INDEV_STATE_PR;
      data->point.x = touchX;
      data->point.y = touchY;

      #if DEBUG_TOUCH !=0
      Serial.print( "Data x " ); Serial.println( touchX );
      Serial.print( "Data y " ); Serial.println( touchY );
      #endif

    }
}



static uint32_t my_tick_get_cb(void){
  return esp_timer_get_time() / 1000;
}

void setup() {

  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.begin(115200);
  delay(1100);  //Set this for other serial ready.
  
  Serial.println("Initial LCD Display");
  tft.begin();
  tft.setRotation(3);
  tft.setBrightness(255);

  //Initial LVGL
  lv_init();


  Serial.println("Starting setup process. Please wait");
  Serial.println( LVGL_Arduino );
  
  #if LV_USE_LOG != 0
  my_print("Hello Arduino");
  #endif
  

  // #if LV_USE_LOG != 0
  //   lv_log_register_print_cb( my_print ); /* register print function for debugging */
  // #endif

  //Initial tick
  lv_tick_set_cb(my_tick_get_cb);

  // #if LV_USE_LOG != 0
  // lv_log_register_print_cb( my_print );
  // #endif


  /*Initialize the display*/
  //----------- For LVGL v9.2.2 --------------  
  lv_display_t * display1 = lv_display_create(screenWidth, screenHeight);
  lv_display_set_default(display1);
  lv_display_set_buffers(display1, buf1, NULL, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  // lv_display_set_buffers(display1, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
  lv_display_set_flush_cb(display1, my_display_flush);

  //--------------- End here  -----------------


  //----------- For LVGL v8.3.11 --------------
  /*
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  */
  //----------- For LVGL v8.3.11 --------------  

  /*Initialize the input device driver*/

  //----------- For LVGL v9.2.2 --------------
  /* Create and set up at least one display before you register any input devices. */
  lv_indev_t * indev = lv_indev_create();        /* Create input device connected to Default Display. */
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);   /* Touch pad is a pointer-like device. */
  lv_indev_set_read_cb(indev, my_touch_read);    /* Set driver function. */
 //--------------- End here  -----------------

  //----------- For LVGL v8.3.11 --------------
  /*
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touch_read;
  lv_indev_drv_register(&indev_drv);
  */
  //--------------- End here  -----------------

  //******************  start programing here *******************
  lv_obj_t *textArea = lv_textarea_create(lv_screen_active());
  lv_obj_set_width(textArea,480);
  lv_obj_set_height(textArea,320);
  lv_obj_align(textArea,LV_ALIGN_CENTER,0,0);
  lv_textarea_add_text(textArea,"Hello WT 32 SC01 Plus, I'm LVGL!\n");
  lv_refr_now(display1);
  delay(100);

  lv_textarea_add_text(textArea,"Initial system. Please wait\n");
  lv_refr_now(display1);
  delay(100);

  lv_textarea_add_text(textArea,"WiFi Connecting.....\n");
  lv_refr_now(display1);
  wifimulti.addAP("myWiFi","1100110011");
  connectWiFi(wifimulti,5,true);
  if (WiFi.status() == WL_CONNECTED) {
    wifiStatus = true;
    lv_textarea_add_text(textArea,"  |-Connected.\n");
    lv_refr_now(display1);
    lv_textarea_add_text(textArea,"  |-SSID: ");
    lv_textarea_add_text(textArea,WiFi.SSID().c_str());
    lv_textarea_add_text(textArea,"\n");
    lv_refr_now(display1);
    delay(150);

    IPAddress ip = WiFi.localIP();
    char ipbuf[30];
    sprintf(ipbuf,"  |-IP: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);
    // lv_textarea_add_text(textArea,"WiFi IP: ");
    lv_textarea_add_text(textArea,(const char*)ipbuf);
 
    lv_refr_now(display1);
  }else{
    wifiStatus = false;
  }





  lv_textarea_add_text(textArea,"Initial system done.\n");
  lv_refr_now(display1);
  delay(100);
  
  Serial.println( "Setup done" );
  printf("System ready\n");
  #if LV_USE_LOG != 0
  my_print("System Ready....");
  #endif
  // delay(4000);

  // lv_obj_add_flag(textArea, LV_OBJ_FLAG_HIDDEN );

  // lv_obj_t *label = lv_label_create( lv_screen_active() );
  // lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN| LV_STATE_DEFAULT);
  // lv_label_set_text( label, "WT32 SC01 Plus, Ready" );
  // lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );

  
  //Initial UI Here
  // ui_init();
}

void loop() {
  lv_timer_handler();
  delay(5);
}


