#include <mruby.h>
#include <mruby/value.h>
#include <stdio.h>
#include "driver/uart.h"
// #include "minmea.h"

static QueueHandle_t uart0_queue;

char* read_line(uart_port_t uart) {
    uint8_t* data = (uint8_t*)malloc(1024);
    memset(data, 0x0, 1024);
    do {
        int len = uart_read_bytes(UART_NUM_2, data, 1024, 100 / portTICK_RATE_MS);
        if (len > 0) {
            return (const char *) data;
        }
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

    uart_driver_install(UART_NUM_2, 2048, 2048, 10, &uart0_queue, 0);

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
