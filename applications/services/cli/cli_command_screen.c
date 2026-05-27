#include "cli_command_screen.h"
#include "core/kernel.h"
#include "core/record.h"

#include <drivers/display/display_jd9853_usb_virtual.h>
#include <drivers/display/display_jd9853_reg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void cli_command_screen(Cli* cli, FuriString* args, void* context) {
    UNUSED(args);
    UNUSED(context);

    const size_t width = JD9853_WIDTH; // 1 byte per pixel
    const size_t height = JD9853_HEIGHT;

    DisplayJd9853QSPI* display = furi_record_open("display");
    if (!display) {
        printf("Error: Display driver not found\r\n");
        return;
    }

    printf("Press CTR+C to stop... \r\n");
    furi_delay_ms(1000);

    char line_buffer[(width / 2) * 3 + 10];

    printf("\033[2J\033[H\033[?25l");

    while(!cli_cmd_interrupt_received(cli)) {
        const uint8_t* buffer = display_jd9853_qspi_get_frame_buffer(display);

        if (buffer) {
            printf("\033[H");

            for (size_t y = 0; y < height; y+=4) {
                size_t line_pos = 0;
                line_buffer[line_pos++] = '|';

                const uint8_t* r0 = &buffer[(y + 0) * width];
                const uint8_t* r1 = &buffer[(y + 1) * width];
                const uint8_t* r2 = &buffer[(y + 2) * width];
                const uint8_t* r3 = &buffer[(y + 3) * width];

                for (size_t x = 0; x < width; x+=2) {
                    bool p1 = r0[x + 0] > 127;
                    bool p2 = r1[x + 0] > 127;
                    bool p3 = r2[x + 0] > 127;
                    bool p4 = r0[x + 1] > 127;
                    bool p5 = r1[x + 1] > 127;
                    bool p6 = r2[x + 1] > 127;
                    bool p7 = r3[x + 0] > 127;
                    bool p8 = r3[x + 1] > 127;

                    // U+2800 - U+28FF
                    uint8_t brail_byte = (p1 << 0) | (p2 << 1) | (p3 << 2) |
                                         (p4 << 3) | (p5 << 4) | (p6 << 5) |
                                         (p7 << 6) | (p8 << 7);

                    line_buffer[line_pos++] = 0xE2;
                    line_buffer[line_pos++] = 0xA0 + (brail_byte >> 6);
                    line_buffer[line_pos++] = 0x80 + (brail_byte & 0x3F);
                }

                line_buffer[line_pos++] = '|';
                line_buffer[line_pos++] = '\r';
                line_buffer[line_pos++] = '\n';
                line_buffer[line_pos] =   '\0';

                cli_write(cli, (uint8_t*)line_buffer, line_pos);
            }
        }
        furi_delay_ms(100);
    }

    printf("\033[2J\033[H\033[?25h");
    printf("Exiting screen\r\n");

    furi_record_close("display");
}
