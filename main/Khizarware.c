#include <stdio.h>
#include "ssd1306.h"
#include <stdbool.h>
#include "rotary_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include <string.h>
#include "iot_button.h"



#define TAG "Redford Main"

// Rotary Encoder
#define ROT_ENC_A_GPIO 33 // to rotary encoder 1
#define ROT_ENC_B_GPIO 35 // to rotary encoder 2
#define ENABLE_HALF_STEPS true // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT 128           // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION false   // Set to true to reverse the clockwise/counterclockwise sense
// pin 32 to rotary encoder switch.

// Others 
#define UV_PIN 27
#define RGB_PIN 14

// OLED SSD1306
#define I2C_MASTER_SCL_IO 26      /*!< gpio number for I2C master clock */ // Display SCL
#define I2C_MASTER_SDA_IO 25      /*!< gpio number for I2C master data  */ // DIsplay SDA
#define I2C_MASTER_NUM I2C_NUM_1  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 400000 /*!< I2C master clock frequency */



// intial selection
int selected = 1;

// main menu items(each 15 chars to ensure full pixel overwrite)
char *menu[] = {"Enable Laser  ", "Enable RGB    ", "Wifi-CaptPort ", "Wifi-EvilTwin ", "Wifi-Sniffer  ", "BT-Sniffer    "};

// max index of main menu array
int MAX = 5;

// inital values
int UV = 0;
int RGB = 0;


// init 
static ssd1306_handle_t ssd1306_dev = NULL;
enum rotary;


void list_item_by_index(int i, int *xpos, int *ypos, bool invert)
{
    ssd1306_draw_string(ssd1306_dev, *xpos, *ypos, (const uint8_t *)menu[i], 12, invert);
    ssd1306_refresh_gram(ssd1306_dev);
    //arbitrary pixel spacing between each element
    *ypos += 14;
    *xpos += 18;
}

void update_display()
{
    // ssd1306_clear_screen(ssd1306_dev, 0x00);
    char data_str[11] = {0};
    sprintf(data_str, "[Marauder]");
    ssd1306_draw_string(ssd1306_dev, 25, 0, (const uint8_t *)data_str, 16, 1);
    // initial coordinates to draw menu from
    int xpos = 5;
    int ypos = 15;
    if (selected == 0)
    {
        list_item_by_index(MAX, &xpos, &ypos, 1);
        list_item_by_index(selected, &xpos, &ypos, 0);
        list_item_by_index(1, &xpos, &ypos, 1);
    }
    else if (selected == MAX)
    {
        list_item_by_index(MAX - 1, &xpos, &ypos, 1);
        list_item_by_index(selected, &xpos, &ypos, 0);
        list_item_by_index(0, &xpos, &ypos, 1);
    }
    else
    {
        list_item_by_index(selected - 1, &xpos, &ypos, 1);
        list_item_by_index(selected, &xpos, &ypos, 0);
        list_item_by_index(selected + 1, &xpos, &ypos, 1);
    }
    ESP_LOGI(TAG, "item 0: %s ", (const uint8_t *)menu[selected]);
}

void check_state()
{

    ESP_LOGI(TAG, "My integer value: %d", selected);

    switch (selected)
    {
    case 0:
        if (UV == 0)
        {
            gpio_set_level(UV_PIN, 1);
            UV = 1;
            menu[selected] = "Disable Laser ";
            update_display();
        }
        else
        {
            gpio_set_level(UV_PIN, 0);
            UV = 0;
            menu[selected] = "Enable Laser  ";
            update_display();
        }
        ESP_LOGI(TAG, "UV Action");
        break;
    case 1:
        if (RGB == 0)
        {
            gpio_set_level(RGB_PIN, 1);
            RGB = 1;
            menu[selected] = "Disable RGB   ";
            update_display();
        }
        else
        {
            gpio_set_level(RGB_PIN, 0);
            RGB = 0;
            menu[selected] = "Enable RGB    ";
            update_display();
        }
        ESP_LOGI(TAG, "RGB Action");
        break;
    case 3:
    break;
    }
}

static void button_single_click_cb(void *arg, void *usr_data)
{
    check_state(selected);
    ESP_LOGI(TAG, "button pressed");
}
void get_rotary()
{
    // update_display();
    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    rotary_encoder_info_t info = {0};
    ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
    ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
#ifdef FLIP_DIRECTION
    ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
#endif
    QueueHandle_t event_queue = rotary_encoder_create_queue();
    ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
    while (1)
    {
        // Wait for incoming events on the event queue.
        rotary_encoder_event_t event = {0};
        if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
        {
            if (event.state.direction == 1)
            {
                // CLOCKWISE EVENT

                if (selected < MAX)
                {
                    selected++;
                    update_display();
                }
                else if (selected == MAX)
                {
                    selected = 0;
                    update_display();
                }
            }
            else if (event.state.direction == 2)
            {
                // ANTICLOCKWISE EVENT
                if (selected > 0)
                {
                    selected--;
                    update_display();
                }
                else if (selected == 0)
                {
                    selected = MAX;
                    update_display();
                }
            }
        }
    }
    ESP_LOGE(TAG, "queue receive failed");
    ESP_ERROR_CHECK(rotary_encoder_uninit(&info));
}

void app_main(void)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
    conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

    ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);

    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
        .gpio_button_config = {
            .gpio_num = 32,
            .active_level = 0,
        },
    };
    button_handle_t gpio_btn = iot_button_create(&gpio_btn_cfg);

    if (NULL == gpio_btn)
    {
        ESP_LOGE(TAG, "Button create failed");
    }
    iot_button_register_cb(gpio_btn, BUTTON_SINGLE_CLICK, button_single_click_cb, NULL);



    ssd1306_clear_screen(ssd1306_dev, 0x00);
    update_display();
    ssd1306_refresh_gram(ssd1306_dev);

    gpio_pad_select_gpio(UV_PIN);
    gpio_set_direction(UV_PIN, GPIO_MODE_OUTPUT);
    gpio_pad_select_gpio(RGB_PIN);
    gpio_set_direction(RGB_PIN, GPIO_MODE_OUTPUT);
    get_rotary();

}
