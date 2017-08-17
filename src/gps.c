#include <mruby.h>
#include <mruby/value.h>
#include "driver/uart.h"
// #include "minmea.h"

#define GPS_TXD_PIN (34)

char* read_line(uart_port_u uart) {
    static char line[256];
    int size;
    char *ptr = line;
    while(1) {
        size = uart_read_bytes(UART_NUM_2, (unsigned char *)ptr, 1, portMAX_DELAY);
        if (size == 1) {
            if (*ptr == '\n') {
                ptr++:
                *ptr = 0;
                return line;
            }
            ptr++;
        }
    }
}

static mrb_value mrb_esp32_gps_dogps(mrb_state *mrb, mrb_value self) {
    uart_config_t uart_conf;
    uart_conf.baud_rate  = 9600;
    uart_conf.data_bits  = UART_DATA_8_BITS;
    uart_conf.parity     = UART_PARITY_DISABLE;
    uart_conf.stop_bits  = UART_STOP_BITS_1;
    uart_conf.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uart_conf.rx_flow_ctrl_thresh = 120;

    uart_param_config(UART_NUM_2, &uart_conf);
    uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, GPS_TXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uart_driver_install(UART_NUM_2, 2048, 2048, 10, 17, NULL);

    char *line = read_line(UART_NUM_2);
    return mrb_str_new_cstr(mrb, line);
        // switch(minmea_sentence_id(line, false)) {
        //     case MINMEA_STENTENCE_RMC:
        //     struct minmea_sentence_rmc frame;
        //     if (minmea_parse_rmc(&frame, line)) {
        //         /*
        //         frame.latitude.value, frame.latitude.scale,
        //         frame.longitude.value, frame.longitude.scale
        //         */
        //     }
        // }
}

void mrb_mruby_esp32_gps_init(mrb_state* mrb) {
    struct RClass *esp32, *gps;

    esp32 = mrb_define_module(mrb, "ESP32");
    gps = mrb_define_module_under(mrb, esp32, "GPS");

    mrb_define_module_function(mrb, gps, "doGPS", mrb_esp32_gps_dogps, MRB_ARGS_NONE());
}

void mrb_mruby_esp32_gps_final(mrb_state* mrb) {}
