#include "HausUI.hpp"
#include <Arduino.h>

#include <TFT_eSPI.h> // Hardware-specific library
/*********************
 *   lvgl-INCLUDES
 *********************/
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <SPI.h>

/**********************
 *  lvgl PROTOTYPES
 **********************/
void handle_display(void *parameter);

/*Change to your screen resolution*/
static const uint16_t screenWidth = 320;
static const uint16_t screenHeight = 240;

#define HSPI_MISO 12
#define HSPI_MOSI 13
#define HSPI_SCLK 14
#define HSPI_CS 16

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[screenWidth * screenHeight / 10];

/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp);
}

/*Read the touchpad*/
void my_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    uint16_t touchX = 0, touchY = 0;

    bool touched = false; // tft.getTouch( &touchX, &touchY, 600 );

    if (!touched)
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print("Data x ");
        Serial.println(touchX);

        Serial.print("Data y ");
        Serial.println(touchY);
    }
}

void HausUI::init()
{
    SPIClass hspi = SPIClass(HSPI);
    hspi.begin();
    hspi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, HSPI_CS); // SCLK, MISO, MOSI, SS
    pinMode(HSPI_CS, OUTPUT);                          // HSPI SS

    /*Initialize LVGL*/
    lv_init();
    tft.begin();        /* TFT init */
    tft.setRotation(3); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * screenHeight / 10);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    ui_init();

    xTaskCreate(
        handle_display,   /* Task function. */
        "handle_display", /* String with name of task. */
        10000,            /* Stack size in bytes. */
        NULL,             /* Parameter passed as input of the task */
        1,                /* Priority of the task. */
        NULL);            /* Task handle. */
}

void handle_display(void *parameter)
{
  while (1)
  {
    lv_timer_handler(); /* let the GUI do its work */
    delay(5);
  }
}
