//==============================================================
#include <stdint.h>
//==============================================================
void print(const char chars[], int len);
void printl(const char* chars);
void boot_splash();
uint8_t vga_colour = 2;
//--------------------------------------------------------------
char get_input();
char keyboard_getchar(uint8_t sc);
char keymap[128];
#define INPUT_SIZE 256
int strcmp(const char* a, const char* b);
void split_string(char* input, char** cmd, char** args);
int string_to_int(const char* str);
void execute_command(const char* cmd, const char* args);
//--------------------------------------------------------------
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void update_cursor(int x, int y);
//--------------------------------------------------------------
void scroll();
void update_screen();
void clear_screen();
void clear_line(int y);
void new_line();
int cursor_row = 0;
int cursor_col = 0;
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
//--------------------------------------------------------------
static inline uint8_t inb(uint16_t port);
static inline void outb(uint16_t port, uint8_t val);
static inline void outw(uint16_t port, uint16_t val);
static inline void io_wait(void);
//==============================================================

int main () {
    enable_cursor(13, 15);
    boot_splash();

    char input[INPUT_SIZE];
    int input_pos = 0;

    print("> ", 2);

    while (1) {
        char c = get_input();

        if (!c) {
            continue;
        }
        switch (c) {
            case '\n':
                input[input_pos] = '\0';

                new_line();
                char* cmd;
                char* args;
                split_string(input, &cmd, &args);
                execute_command(cmd, args);

                input_pos = 0;

                print("> ", 2);
                break;

            case '\b':
                if (input_pos > 0 && cursor_col > 0) {
                    input_pos--;
                    cursor_col--;

                    uint16_t* v_mem = (uint16_t*)0xB8000;
                    int offset = cursor_row * VGA_WIDTH + cursor_col;

                    v_mem[offset] = ((uint16_t)vga_colour << 8) | ' ';

                    update_cursor(cursor_col, cursor_row);
                }
                break;

            default:
                if (input_pos < INPUT_SIZE - 1) {
                    input[input_pos++] = c;
                    print(&c, 1);
                }
                break;
        }
    }
}

void print(const char chars[], int len) {
    uint16_t* v_mem = (uint16_t*)0xB8000;

    for (int i = 0; i < len; i++) {
        int offset = (cursor_row * VGA_WIDTH) + (cursor_col);
        v_mem[offset] = (vga_colour << 8) | chars[i];
    
        cursor_col ++;
        if (cursor_col >= VGA_WIDTH){
            cursor_col = 0;
            cursor_row++;
        }
        if (cursor_row >= VGA_HEIGHT) {
            scroll();
        }
    }
    update_cursor(cursor_col, cursor_row);
}

void printl(const char* chars) {
    int len = 0;
    while (chars[len] != '\0') len++;

    print(chars, len);
    new_line();
}

void boot_splash() {
    printl("  ___        _____      ");
    printl(" / _ \\      |  _  |     ");
    printl("/ /_\\ \\_ __ | | | | ___ ");
    printl("|  _  | '_ \\| | | |/ __|");
    printl("| | | | | | \\ \\_/ /\\__ \\");
    printl("\\_| |_/_| |_|\\___/ |___/");
}

char get_input() {
    if (inb(0x64) & 1) {
        uint8_t scancode = inb(0x60);

        if (!(scancode & 0x80)) {
            char c = keyboard_getchar(scancode);

            if (c) {
                return c;
            }
            else {
                return 0;
            }
        }
    }
    return 0;
}

char keyboard_getchar(uint8_t sc) {
    if (sc > 0x39) return 0;

    return keymap[sc];
}

char keymap[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,     'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\',  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
    0, 0, 0, ' '
};

int strcmp(const char* a, const char* b) {
    int i = 0;

    while (a[i] && b[i]) {
        if (a[i] != b[i]) {
            return 0;
        }
        i++;
    }

    return a[i] == b[i];
}

int strlen(const char* str) {
    int len = 0;

    while (str[len]) {
        len++;
    }
    return len;
}

void split_string(char* input, char** cmd, char** args) {
    *cmd = input;

    while (*input && *input != ' ') {
        input++;
    }

    if (*input) {
        *input = '\0';
        input++;
        *args = input;
    } else {
        *args = "";
    }
}

int string_to_int(const char* str) {
    int num = 0;

    while (*str) {
        if (*str < '0' || *str > '9') {
            return 0;
        }
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}

void execute_command(const char* cmd, const char* args) {
    if (strcmp(cmd, "help")) {
        printl("help clear refresh echo colour about reboot shutdown");
    }

    else if (strcmp(cmd, "clear")) {
        clear_screen();
    }

    else if (strcmp(cmd, "refresh")) {
        printl("Refreshing screen");
        update_screen();
    }

    else if (strcmp(cmd, "about")) {
        printl("MyOS kernel");
    }

    else if (strcmp(cmd, "reboot")) {
        printl("Rebooting...");
        outb(0x64, 0xFE);
    }

    else if (strcmp(cmd, "shutdown")) {
        printl("Power off... ");
        outw(0x604, 0x2000);
    }

    else if (strcmp(cmd, "echo")) {
        printl(args);
    }

    else if (strcmp(cmd, "colour")) {
        int num = string_to_int(args);
        if (num) {
            vga_colour = num;
            printl("Changed colour");
            update_screen();
        } else {
            printl("INVALID NUMBER");
        }

    }

    else if (strlen(cmd) == 0) {}

    else {
        print("Unknown command: ", 17);
        printl(cmd);
    }
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
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void scroll() {
    uint16_t* v_mem = (uint16_t*) 0xB8000;

    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++){
            v_mem[(y - 1) * VGA_WIDTH + x] = v_mem[y * VGA_WIDTH + x];
        }
    }
    clear_line(VGA_HEIGHT - 1);
    cursor_row = VGA_HEIGHT - 1;
    cursor_col = 0;
    update_cursor(cursor_col, cursor_row);
}

void update_screen() {
    uint16_t* v_mem = (uint16_t*) 0xB8000;

    for (int i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
        v_mem[i] = ((uint16_t)vga_colour << 8) | (v_mem[i] & 0x00FF);    }
}

void clear_screen() {
    cursor_row = 0;
    cursor_col = 0;

    for (int y = 0; y < VGA_HEIGHT; y++){
        clear_line(y);
    }

    update_cursor(0, 0);
}

void clear_line(int y){
    uint16_t* v_mem = (uint16_t*) 0xB8000;
    
    for (int x = 0; x < VGA_WIDTH; x++){
        v_mem[(y * VGA_WIDTH) + x] = (vga_colour << 8) | ' ';
    }
}

void new_line() {
    cursor_row++;
    cursor_col = 0;
    if (cursor_row >= VGA_HEIGHT) {
        scroll();
    }
    update_cursor(cursor_col, cursor_row);
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

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) : "memory");

} 

static inline void io_wait(void) {
    outb(0x80, 0);
}
