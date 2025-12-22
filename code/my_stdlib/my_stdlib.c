#include "my_stdlib.h"



int close(int fd){Halt();}
int open(char * name, int mode){Halt();}
int write(int fd, char *buf, size_t size){Halt();}
ssize_t read(int fd, char *buf, size_t size){Halt();}

void * malloc(unsigned int size){Halt();}
int free(void * ptr){Halt();}

void my_printf(char *buf){
    if (buf){
        PutString(buf, my_strlen(buf));
    }
}

void my_scanf(char *format, ...){}

unsigned int my_strlen(char *str){
    unsigned int result = 0;
    while(str[result] != '\0'){result++;}
    return result;
}

void print_error(const char *msg) {
    my_printf((char *) msg);
    my_printf(" (errno=");
    PutInt(GetLastError());
    my_printf(")\n");
}

void my_memcpy(void *dest, void *src, size_t size){
    unsigned int i;
    for (i = 0; i + sizeof(uint64_t) <= size; i += sizeof(uint64_t)){
        *((uint64_t *)((char*)dest + i)) = * ((uint64_t *)((char*)src + i));
    }

    // Zero remaining bytes (less than 8)
    for (; i < size; i++){
        *((uint8_t *)((char*)dest + i)) = *((uint8_t *)((char*) src+ i));
    }
}

void my_strcpy(char * dest, char *src){
    my_memcpy(dest, src, my_strlen(src));
}

IOBUF_FILE* iobuf_open(char* nom, char mode){
    IOBUF_FILE* f = malloc(sizeof(IOBUF_FILE));
    
    if (mode == 'R'){
        f->file_descriptor = open(nom, O_RDONLY);
    }
    else if (mode == 'W'){
        f->file_descriptor = open(nom, O_WRONLY);
    }
    else{
        f->file_descriptor = -1;
    }

    if (f->file_descriptor == -1){
        free(f);
        return NULL;
    }

    f->mode = mode;
    f->start_buff = &f->buffer[0];
    f->end_buff = &f->buffer[0];
    f->empty = 1;

    return f;
}

int iobuf_close(IOBUF_FILE* f) {
    if (f == NULL) { return -1; }
    if (f->mode == 'W' && f->empty == 0) { iobuf_flush(f); }
    if (close(f->file_descriptor) == -1) { return -1; }
    free(f);
    return 0;
}

int iobuf_read(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f) {
    if (p == NULL || f == NULL || f->mode != 'R') { return -1; }
    const unsigned int total_bytes = taille * nbelem;
    if (total_bytes > f->end_buff - f->start_buff) {
        ssize_t rval = iobuf_fill(f);
        if (rval < 0) { return -1; }
    }

    unsigned int bytes_to_transfer = total_bytes;
    if (bytes_to_transfer > (unsigned int)(f->end_buff - f->start_buff)) {
        bytes_to_transfer = (unsigned int)(f->end_buff - f->start_buff);
    }

    unsigned int i = 0;
    while (i < bytes_to_transfer) {
      my_memcpy((char*)p + i, f->start_buff, taille);
        i += taille;
        f->start_buff += taille;
    }
    return (int)i;
}

// Write use a circular buffer
int iobuf_write(void* p, unsigned int taille, unsigned int nbelem, IOBUF_FILE * f) {
    if (p == NULL || f == NULL || f->mode != 'W') { return -1; }
    unsigned int nbelem_written = 0;

    if (f->start_buff == f->end_buff && f->empty == 0) {
        ssize_t rval = iobuf_flush(f);
        if (rval < 0) { return -1; }
    }

    unsigned int available_space;
    if (f->start_buff <= f->end_buff) {
        available_space = BUFFER_SIZE - (unsigned int)(f->end_buff - f->start_buff);
    } else {
        available_space = (unsigned int)(f->start_buff - f->end_buff);
    }

    while (taille <= available_space && nbelem_written < nbelem) {
        if (f->end_buff + taille <= f->buffer + BUFFER_SIZE) {
          my_memcpy(f->end_buff, (char*)p + nbelem_written * taille, taille);
            f->end_buff += taille;
            if (f->end_buff >= f->buffer + BUFFER_SIZE) {
                f->end_buff = f->buffer;
            }
        } else {
            unsigned int space_to_end = (unsigned int)(f->buffer + BUFFER_SIZE - f->end_buff);
          my_memcpy(f->end_buff, (char*)p + nbelem_written * taille, space_to_end);
            f->end_buff = f->buffer;
          my_memcpy(f->end_buff, (char*)p + nbelem_written * taille + space_to_end, taille - space_to_end);
            f->end_buff += taille - space_to_end;
        }
        nbelem_written++;
        available_space -= taille;
    }

    if (nbelem_written != 0) {
        f->empty = 0;
    }

    if (f->start_buff == f->end_buff) {
        ssize_t rval = iobuf_flush(f);
        if (rval < 0) { return -1; }
    }

    return (int)nbelem_written;
}

// int iobuf_fprintf(IOBUF_FILE* fp, char* format, ...) {
//     int nbelem_written = 0;
//
//     va_list(args);
//     va_start(args, format);
//
//     char* current_pos = format;
//     while (*current_pos != '\0') {
//         if (*current_pos == '%') {
//             current_pos++;
//             char* string_to_write = "%";
//             if (*current_pos == '%') {
//                 string_to_write = "%";
//             } else if (*current_pos == 'c') {
//                 const char char_to_write = (char)va_arg(args, int);
//                 string_to_write = malloc(2*sizeof(char));
//                 string_to_write[0] = char_to_write;
//                 string_to_write[1] = '\0';
//             } else if (*current_pos == 's') {
//                 string_to_write = va_arg(args, char*);
//             } else if (*current_pos == 'd') {
//                 const int num = va_arg(args, int);
//                 string_to_write = itos(num, 10);
//             } else {
//                 current_pos--;
//             }
//
//             char* ptr = string_to_write;
//             while (*ptr != '\0') {
//                 if (iobuf_write(ptr, sizeof(char), 1, fp) != 1) {
//                     if (*current_pos == 'c') {
//                         free(string_to_write);
//                     }
//                     return -1;
//                 }
//                 nbelem_written++;
//                 ptr++;
//             }
//
//             if (*current_pos == 'c') {
//                 free(string_to_write);
//             }
//
//         } else {
//             if (iobuf_write(current_pos, sizeof(char), 1, fp) != 1) {
//                 return -1;
//             }
//             nbelem_written++;
//         }
//         current_pos++;
//     }
//     return nbelem_written;
// }


ssize_t iobuf_flush(IOBUF_FILE* f) {
    if (f == NULL || f->mode != 'W') { return -1; }
    ssize_t bytes_to_write = f->end_buff - f->start_buff;
    if (f->start_buff >= f->end_buff) {
        bytes_to_write = BUFFER_SIZE + f->buffer - f->start_buff;
    }
    ssize_t rval = write(f->file_descriptor, f->start_buff, bytes_to_write);

    if (rval < 0){
        return -1;
    }

    f->start_buff = f->start_buff + rval;
    if (f->start_buff >= f->buffer + BUFFER_SIZE) {
        f->start_buff -= BUFFER_SIZE;
    }

    if (f->start_buff == f->end_buff) {
        f->empty = 1;
    }

    return rval;
}

ssize_t iobuf_fill(IOBUF_FILE* f) {
    if (f == NULL || f->mode != 'R' || f->end_buff < f->start_buff) { return -1; }
    if (f->start_buff == f->end_buff) {
        f->start_buff = f->buffer;
        f->end_buff = f->buffer;
    }

    ssize_t bytes_to_read = BUFFER_SIZE - (f->end_buff - f->start_buff);
    ssize_t rval = read(f->file_descriptor, f->end_buff, bytes_to_read);

    if (rval < 0){
        return -1;
    }

    f->end_buff += rval;

    return rval;
}

char *itos(int value, const int base) {
    const char* digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int negative = 0;

    if (value < 0) {
        negative = 1;
        value = -value;
    }

    if (base < 1 || base > 36) {
        return NULL;
    }

    if (value < base) {
        char* result = malloc(2 * sizeof(char));
        result[0] = digits[value];
        result[1] = '\0';
        return result;
    }
    char* result = itos(value / base, base);
    size_t len = my_strlen(result);
    result[len] = digits[value % base];
    result[len + 1] = '\0';

    if (negative) {
        char* negative_result = malloc((len + 2) * sizeof(char));
        negative_result[0] = '-';
        my_strcpy(negative_result + 1, result);
        free(result);
        return negative_result;
    }

    return result;
}
