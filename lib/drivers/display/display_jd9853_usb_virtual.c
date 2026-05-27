#include "display_jd9853_usb_virtual.h"
#include "core/kernel.h"
#include "core/record.h"
#include "display_jd9853_reg.h"

#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>
#include <furi_hal_i2c_config.h>

#include <pico/types.h>
#include <stdio.h>


struct DisplayJd9853QSPI {
    FuriSemaphore* busy;
    uint8_t backlight;

    uint8_t* last_frame_buffer;
};

static DisplayJd9853QSPI* display_instance = NULL;

DisplayJd9853QSPI* display_jd9853_qspi_init(void) {
    furi_check(display_instance == NULL);
    DisplayJd9853QSPI* display = malloc(sizeof(DisplayJd9853QSPI));
    display_instance = display;

    display->busy = furi_semaphore_alloc(1, 1);
    display->backlight = 100;

    display->last_frame_buffer = malloc(JD9853_WIDTH * JD9853_HEIGHT);
    memset(display->last_frame_buffer, 0, JD9853_WIDTH * JD9853_HEIGHT);

    if(!furi_record_exists("display")) {
        furi_record_create("display", display);
    }

    return display;
}

void display_jd9853_qspi_write_buffer(DisplayJd9853QSPI *display, const uint8_t *buffer, size_t size) {
    furi_assert(display);
    furi_check(size == JD9853_WIDTH * JD9853_HEIGHT); //size must be equal to full buffer size

    furi_check(furi_semaphore_acquire(display->busy, FuriWaitForever) == FuriStatusOk);
    
    memcpy(display->last_frame_buffer, buffer, size);

    furi_semaphore_release(display->busy);
}

const uint8_t* display_jd9853_qspi_get_frame_buffer(DisplayJd9853QSPI* display) {
    return display->last_frame_buffer;
}

void display_jd9853_qspi_deinit(DisplayJd9853QSPI *display) {
    furi_check(display);

    furi_delay_ms(10);

    furi_semaphore_free(display->busy);
    free(display->last_frame_buffer);
    free(display);
    display_instance = NULL;
}

void display_jd9853_load_config(DisplayJd9853QSPI *display, const uint8_t *config) {
    UNUSED(display);
    UNUSED(config);
}

void display_jd9853_qspi_on_sleep_enter(void) {
}

void display_jd9853_qspi_on_sleep_exit(void) {
}

void display_jd9853_qspi_set_brightness(DisplayJd9853QSPI *display, int8_t brightness) {
    furi_check(display);
    if(brightness > 100) brightness = 100;
    if(brightness < 0) brightness = 0;
    display->backlight = (uint8_t)brightness;
}

int8_t display_jd9853_qspi_get_brightness(DisplayJd9853QSPI* display) {
    furi_check(display);
    return (int8_t)display->backlight;
}

void display_jd9853_qspi_fill(DisplayJd9853QSPI* display, uint8_t color) {
    furi_assert(display);
    const size_t width = JD9853_WIDTH; // 1 byte per pixel
    const size_t height = JD9853_HEIGHT;

    uint8_t* data = (uint8_t*)malloc(width * height);
    for (size_t i = 0; i < width * height; i += 1) {
        data[i] = color;
    }

    display_jd9853_qspi_write_buffer(display, data, width * height);
    free(data);
}

void display_jd9853_qspi_eco_mode(DisplayJd9853QSPI *display, bool enable) {
    UNUSED(display);
    UNUSED(enable);
}

void display_jd9853_qspi_set_vci(DisplayJd9853QSPI *display, float_t voltage) {
    UNUSED(display);
    UNUSED(voltage);
}

float_t display_jd9853_qspi_get_vci(DisplayJd9853QSPI *display) {
    furi_check(display);
    return 3.3f;
}

bool display_jd9853_qspi_is_init(void) {
    return display_instance != NULL;
}

void display_jd9853_irq_qspi_write_buffer(const uint8_t* buffer, size_t size) {
    UNUSED(buffer);
    UNUSED(size);
}
