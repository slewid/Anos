int v_mem_index = 0;
static void print(const char chars[], const int len);

extern int main (){
    char arr[] = "Hello, World!";
    print(arr, (sizeof(arr) / sizeof(arr[0])) - 1);

    return 0;
}

void print(const char chars[], int len) {
    char * v_mem = (char*) 0xB8000;

    for (int i = 0; i < len; i++) {
        int offset = i * 2;
        v_mem[offset] = chars[i];
        v_mem[offset + 1] = 0x02;

    }
}