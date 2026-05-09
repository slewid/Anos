//==============================================================
#include <stdint.h>
//==============================================================
static const char normal_map[128];
void print(const char chars[], int len);
static inline uint8_t inb(uint16_t port);
static inline void outb(uint16_t port, uint8_t val);
static inline void io_wait(void);
char scancode_to_ascii(uint8_t sc);
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);
void scroll();
void clear_line(int y);
void clear_screen();
void printl(const char* chars);
void new_line();
void boot_splash();
int cursor_row = 0;
int cursor_col = 0;
//==============================================================

extern int main () {
    enable_cursor(13, 15);
    boot_splash();
    return 0;
}

static const char normal_map[128] = {
    [0x01] = 27,    // ESC
    [0x02] = '1', [0x03] = '2', [0x04] = '3', [0x05] = '4',
    [0x06] = '5', [0x07] = '6', [0x08] = '7', [0x09] = '8',
    [0x0A] = '9', [0x0B] = '0',
    [0x0C] = '-', [0x0D] = '=',
    [0x0E] = '\b', // Backspace
    [0x0F] = '\t', // Tab
    [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
    [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
    [0x18] = 'o', [0x19] = 'p',
    [0x1A] = '[', [0x1B] = ']',
    [0x1C] = '\n', // Enter
    [0x1E] = 'a', [0x1F] = 's', [0x20] = 'd', [0x21] = 'f',
    [0x22] = 'g', [0x23] = 'h', [0x24] = 'j', [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';', [0x28] = '\'',
    [0x29] = '`',
    [0x2B] = '\\',
    [0x2C] = 'z', [0x2D] = 'x', [0x2E] = 'c', [0x2F] = 'v',
    [0x30] = 'b', [0x31] = 'n', [0x32] = 'm',
    [0x33] = ',', [0x34] = '.', [0x35] = '/',
    [0x39] = ' ' // Space
};

void print(const char chars[], int len) {
    char * v_mem = (char*) 0xB8000;

    for (int i = 0; i < len; i++) {
        int offset = ((cursor_row * 80)+(cursor_col + i)) * 2;
        v_mem[offset] = chars[i];
        v_mem[offset + 1] = 0x02;
    }
    cursor_col += len;
    // if (cursor_col >= 80){
    //     cursor_col = 0;
    //     cursor_row++;
    // }
    // if (cursor_row >= 25) {
    //     return
    //     scroll();
    // }
    update_cursor(cursor_col, cursor_row);
}

void printl(const char* chars) {
    int len = 0;
    while (chars[len] != '\0') len++;

    print(chars, len);
    new_line();
}

void new_line() {
    cursor_row++;
    cursor_col = 0;
    if (cursor_row >= 25) {
        scroll();
    }
    update_cursor(cursor_col, cursor_row);
}

void scroll(){
    uint16_t* v_mem = (uint16_t*) 0xB8000;

    for (int i = 1; i < 25; i++) {
        for (int x = 0; x < 80; x++){
            v_mem[(i-1) * 80 + x] = v_mem[i * 80 + x];
        }
    }
    clear_line(24);
    cursor_row = 24;
    cursor_col = 0;
}

void clear_line(int y){
    uint16_t* v_mem = (uint16_t*) 0xB8000;
    for (int x = 0; x < 80; x++){
        v_mem[(y * 80) + x] = 0x0720;
    }
}

void clear_screen(){
    cursor_row = 0;
    cursor_col = 0;

    for (int y = 0; y < 25; y++){
        clear_line(y);
    }

    update_cursor(0, 0);
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0"
                   : "=a"(ret)
                   : "Nd"(port)
                   : "memory");
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");

}  

static inline void io_wait(void) {
    outb(0x80, 0);
}

char scancode_to_ascii(uint8_t sc) {
    // ignore key releases
    if (sc & 0x80) return 0;

    if (sc >= 128) return 0;

    return normal_map[sc];
}

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outb(0x3D4, 0x0A);
	outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

	outb(0x3D4, 0x0B);
	outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void disable_cursor() {
	outb(0x3D4, 0x0A);
	outb(0x3D5, 0x20);
}

void update_cursor(int x, int y) {
    const uint16_t VGA_WIDTH = 80;
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void boot_splash() {
    printl("  ___        _____      ");
    printl(" / _ \\      |  _  |     ");
    printl("/ /_\\ \\_ __ | | | | ___ ");
    printl("|  _  | '_ \\| | | |/ __|");
    printl("| | | | | | \\ \\_/ /\\__ \\");
    printl("\\_| |_/_| |_|\\___/ |___/");
}