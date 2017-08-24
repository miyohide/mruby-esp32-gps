#include <mruby.h>
#include <mruby/value.h>
#include "driver/uart.h"
#include "freertos/queue.h"
// #include "minmea.h"

#define BUF_SIZE (512)

static QueueHandle_t uart0_queue;

static void uart_event_task(void *pvParameters) {
    uart_event_t event;
    size_t buffered_size;
    uint8_t* dtmp = (uint8_t*) malloc(BUF_SIZE);
    for(;;) {
        if (xQueueReceive(uart0_queue, (void *)&event, (portTickType)portMAX_DELAY)) {
            switch(event.type) {
            case UART_DATA:
                uart_get_buffered_data_len(UART_NUM_2, &buffered_size);
                break;
            case UART_FIFO_OVF:
            case UART_BUFFER_FULL:
                uart_flush(UART_NUM_2);
                break;
            default:
                break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

char* read_line(uart_port_t uart) {
    uint8_t* data = (uint8_t*)malloc(BUF_SIZE);
    uint8_t tmp;
    do {
        int len = uart_read_bytes(UART_NUM_2, &tmp, 1, 100 / portTICK_RATE_MS);
        if (tmp == '\0') {
            *data = 0;
            return (const char *) data;
        } else if (tmp == '\n') {
            *data = 0;
            return (const char *) data;
        } else {
            *data++ = tmp;
        }
        // if (len > 0) {
        //     data[len] = NULL;
        //     return (const char *) data;
        // }
    } while(1);
}

static mrb_value mrb_esp32_gps_init(mrb_state *mrb, mrb_value self) {
    uart_config_t uart_conf;
    uart_conf.baud_rate  = 9600;
    uart_conf.data_bits  = UART_DATA_8_BITS;
    uart_conf.parity     = UART_PARITY_DISABLE;
    uart_conf.stop_bits  = UART_STOP_BITS_1;
    uart_conf.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uart_conf.rx_flow_ctrl_thresh = 120;

    uart_param_config(UART_NUM_2, &uart_conf);
    uart_set_pin(UART_NUM_2,
         17,  // TX
         16,  // RX
         UART_PIN_NO_CHANGE,  // RTS
         UART_PIN_NO_CHANGE); // CTS

    uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 10, &uart0_queue, 0);
    //uart_enable_pattern_det_intr(UART_NUM_2, '\n', 3, 10000, 10, 10);
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
    return self;
}

static mrb_value mrb_esp32_gps_dogps(mrb_state *mrb, mrb_value self) {
    return mrb_str_new_cstr(mrb, read_line(UART_NUM_2));
}

void mrb_mruby_esp32_gps_gem_init(mrb_state* mrb) {
    struct RClass *esp32, *gps;

    esp32 = mrb_define_module(mrb, "ESP32");
    gps = mrb_define_module_under(mrb, esp32, "GPS");

    mrb_define_module_function(mrb, gps, "init", mrb_esp32_gps_init, MRB_ARGS_NONE());
    mrb_define_module_function(mrb, gps, "doGPS", mrb_esp32_gps_dogps, MRB_ARGS_NONE());
}

void mrb_mruby_esp32_gps_gem_final(mrb_state* mrb) {}
