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

#define TAG "app"

// #define ROT_ENC_A_GPIO (CONFIG_ROT_ENC_A_GPIO)
// #define ROT_ENC_B_GPIO (CONFIG_ROT_ENC_B_GPIO)
#define ROT_ENC_A_GPIO 33
#define ROT_ENC_B_GPIO 35

#define ENABLE_HALF_STEPS true // Set to true to enable tracking of rotary encoder at half step resolution
#define RESET_AT 128           // Set to a positive non-zero number to reset the position if this value is exceeded
#define FLIP_DIRECTION false   // Set to true to reverse the clockwise/counterclockwise sense

#define I2C_MASTER_SCL_IO 26      /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO 25      /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM I2C_NUM_1  /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 400000 /*!< I2C master clock frequency */
int selected = 1;
char *menu[] = {"W-DeAuth", "W-BeaconSpm", "W-CaptPort", "W-EvilTwin", "W-Sniffer", "B-Sniffer"};
int MAX = 5;
static ssd1306_handle_t ssd1306_dev = NULL;
enum rotary;

// void scroll_implement(){
//     // ssd1306_clear_screen(ssd1306_dev, 0x00);
//     // ROWLENGTH = (ROWLENGTH >= 1 ? ROWLENGTH : 128 + ROWLENGTH);
//     ESP_LOGI(TAG, "RL: %d", ROWLENGTH );
//     ESP_LOGI(TAG, "RN: %d", ROWNUMBER );
//     if (ROWNUMBER > 1){
//         ssd1306_fill_rectangle(ssd1306_dev, 0, 0, 128, (ROWNUMBER-1), 1);
//     }

//     ESP_LOGI(TAG, "CPD1" );
//     ssd1306_fill_rectangle(ssd1306_dev, 0, (ROWNUMBER-1), ROWLENGTH, ROWNUMBER, 1);
//     ESP_LOGI(TAG, "CPD2" );
//     ssd1306_refresh_gram(ssd1306_dev);
//     ESP_LOGI(TAG, "Display refreshed");
// }

// void get_rotary()
// {
//     // esp32-rotary-encoder requires that the GPIO ISR service is installed before calling rotary_encoder_register()
//     ESP_ERROR_CHECK(gpio_install_isr_service(0));

//     // Initialise the rotary encoder device with the GPIOs for A and B signals
//     rotary_encoder_info_t info = { 0 };
//     ESP_ERROR_CHECK(rotary_encoder_init(&info, ROT_ENC_A_GPIO, ROT_ENC_B_GPIO));
//     ESP_ERROR_CHECK(rotary_encoder_enable_half_steps(&info, ENABLE_HALF_STEPS));
// #ifdef FLIP_DIRECTION
//     ESP_ERROR_CHECK(rotary_encoder_flip_direction(&info));
// #endif

//     // Create a queue for events from the rotary encoder driver.
//     // Tasks can read from this queue to receive up to date position information.
//     QueueHandle_t event_queue = rotary_encoder_create_queue();
//     ESP_ERROR_CHECK(rotary_encoder_set_queue(&info, event_queue));
//     ESP_LOGI(TAG, "Change detected");
//     while (1)
//     {
//         // Wait for incoming events on the event queue.
//         rotary_encoder_event_t event = { 0 };
//         if (xQueueReceive(event_queue, &event, 1000 / portTICK_PERIOD_MS) == pdTRUE)
//         {
//             ROWLENGTH = event.state.position;
//             if (RESET_AT && (ROWLENGTH >= RESET_AT ))
//             {
//                 ESP_LOGI(TAG, "Reset+");
//                 ESP_ERROR_CHECK(rotary_encoder_reset(&info));
//                 ROWNUMBER = ROWNUMBER +1;
//             }
//             ROWLENGTH = event.state.position + konstant;
//             if (RESET_AT && (ROWLENGTH < 0))
//             {
//                 konstant = konstant + 128;
//                 ROWLENGTH = event.state.position + konstant;
//                 // ROWLENGTH = ROWLENGTH;
//                 ESP_LOGI(TAG, "INCR RL129");
//                 // ESP_ERROR_CHECK(rotary_encoder_reset(&info));
//                 ROWNUMBER = ROWNUMBER - 1;
//             }

//             ESP_LOGI(TAG, "%d", ROWLENGTH);
//             scroll_implement();
//             ssd1306_clear_screen(ssd1306_dev, 0x00);

//         }

//     }
//     ESP_LOGE(TAG, "queue receive failed");

//     ESP_ERROR_CHECK(rotary_encoder_uninit(&info));
// }

void list_item_by_index(int i, int *xpos, int *ypos, bool invert)
{

    ssd1306_draw_string(ssd1306_dev, *xpos, *ypos, (const uint8_t *)menu[i], 12, invert);
    ssd1306_refresh_gram(ssd1306_dev);
    *ypos +=14;
    *xpos+=20;

}
void update_display()
{
    ssd1306_clear_screen(ssd1306_dev, 0x00);
    char data_str[12] = {0};
    sprintf(data_str, "Th1nK R@z0r");
    ssd1306_draw_string(ssd1306_dev, 0, 0, (const uint8_t *)data_str, 14, 1);
    int xpos = 5;
    int ypos = 20;
    if (selected == 0)
    {
        list_item_by_index(MAX, &xpos, &ypos, 1);
        list_item_by_index(selected, &xpos, &ypos, 0);
        list_item_by_index(1, &xpos, &ypos, 1);
        
    }
    else if (selected == MAX){
        list_item_by_index(MAX-1, &xpos, &ypos, 1);
        list_item_by_index(selected, &xpos, &ypos, 0);
        list_item_by_index(0, &xpos, &ypos, 1);
    }
    else{
    list_item_by_index(selected - 1, &xpos, &ypos, 1);
    list_item_by_index(selected, &xpos, &ypos, 0);
    list_item_by_index(selected + 1, &xpos, &ypos, 1);
    }
    // ssd1306_draw_line(ssd1306_dev, xpos, ypos, (xpos + height), (ypos + width));
    ESP_LOGI(TAG, "item 0: %s ", (const uint8_t *)menu[selected]);
    // ssd1306_draw_string(ssd1306_dev, xpos, ypos,(const uint8_t *)menu[0], height, 1 );
    // ssd1306_draw_string(ssd1306_dev, xpos, ypos+14,(const uint8_t *)menu[1], height, 1 );
    // ssd1306_draw_string(ssd1306_dev, xpos, ypos+28,(const uint8_t *)menu[2], height, 1 );
    // ssd1306_refresh_gram(ssd1306_dev);
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
                
                if (selected < MAX){
                    update_display();
                    selected++;
                }
                else if (selected == MAX){
                    update_display();
                    selected = 0;
                }
                
            }
            else if (event.state.direction == 2)
            {
                // ANTICLOCKWISE EVENT
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
    // ssd1306_refresh_gram(ssd1306_dev);
    // ssd1306_clear_screen(ssd1306_dev, 0x00);
    
    get_rotary();
    // update_display();

    // char data_str[23] = {0};
    // sprintf(data_str, "Khizar is the best");
    // ssd1306_draw_string(ssd1306_dev, 1, 1, (const uint8_t *)data_str, 12, 1);
    // for (size_t q = 0; q < 64; q++)
    // {
    //     for (size_t i = 0; i < 128; i++)
    //     {
    //         ssd1306_fill_rectangle(ssd1306_dev, 1, 1, i, q, 1);
    //         ssd1306_refresh_gram(ssd1306_dev);
    //     }
    // }

    // ssd1306_clear_screen(ssd1306_dev, 0x00);
}
