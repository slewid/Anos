//==============================================================
#include <stdint.h>
//==============================================================
static const char normal_map[128];
void print(const char chars[], int len);
static inline uint8_t inb(uint16_t port);
static inline void outb(uint16_t port, uint8_t val);
static inline void io_wait(void);
char keyboard_getchar(uint8_t sc);
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
    
    uint8_t scancode = 0;
    uint8_t last = 0;

    while (1) {
        scancode = inb(0x60);

        if (scancode != last) {
            char c = keyboard_getchar(scancode);
            switch (c) {
                case 0:
                    break;
                case '\n':
                    new_line();
                    break;
                default:
                    char buf[] = {c, '0'};
                    print(buf, 1);
                    break;
            }
        }
        last = scancode;
    }
}

char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\',  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, 0, 0, ' '
};

void print(const char chars[], int len) {
    uint16_t* v_mem = (uint16_t*)0xB8000;

    for (int i = 0; i < len; i++) {
        int offset = (cursor_row * 80) + (cursor_col + i);
        v_mem[offset] = (0x02 << 8) | chars[i];
    }
    cursor_col += len;
    if (cursor_col >= 80){
        cursor_col = 0;
        cursor_row++;
    }
    if (cursor_row >= 25) {
        scroll();
    }
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

    for (int y = 0; y < 24; y++) {
        for (int x = 0; x < 80; x++){
            v_mem[(y+1) * 80 + x] = v_mem[y * 80 + x];
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

char keyboard_getchar(uint8_t sc) {
    // ignore key releases
    if (sc & 0x80) return 0;

    if (sc >= 128) return 0;

    return keymap[sc];
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
