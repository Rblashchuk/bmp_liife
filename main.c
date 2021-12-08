#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <direct.h>



int neighbors(int i, int j, unsigned char **colors, int h, int w) {

    int neighbors = 0;

    int x = (i - 1 + h) % h;
    int y = (j - 1 + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i + 1 + h) % h;
    y = (j + 1 + w) % h;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i - 1 + h) % h;
    y = (j + 1 + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i + 1 + h) % h;
    y = (j - 1 + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i + h) % h;
    y = (j - 1 + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i - 1 + h) % h;
    y = (j + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i + 1 + h) % h;
    y = (j + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }

    x = (i + h) % h;
    y = (j + 1 + w) % w;
    if (colors[x][y] == '1'){
        neighbors++;
    }
    return neighbors;
}

void next_epoch(int h, int w32, int w, unsigned char **arr) {
    unsigned char new_epoch[h][w32];
    for (int i = h - 1; i >= 0; i--){
        for (int j = 0; j < w; j++) {
            int neighbors_count = neighbors(i, j, arr, h, w);
            if (arr[i][j] == '1' && !(neighbors_count == 2 || neighbors_count == 3)) {
                new_epoch[i][j] = '0';
            }
            else if (arr[i][j] == '0' && neighbors_count == 3) {
                new_epoch[i][j] = '1';
            }
            else {
                new_epoch[i][j] = arr[i][j];
            }
        }
    }

    for (int i = h - 1; i >= 0; i--) {
        for (int j = 0; j < w32; j++) {
            arr[i][j] = new_epoch[i][j];
        }
    }
}

struct palette {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char A;
};

void image(unsigned char **colors, struct palette P[],
           int h, int w, char *out, unsigned char header[54], int p_size) {
    FILE *fout;
    fout = fopen(out, "w");
    for (int i = 0; i < 54; i++) fprintf(fout, "%c", header[i]);
    for (int i = 0; i < p_size; i++) fprintf(fout, "%c%c%c%c", P[i].red, P[i].green, P[i].blue, 0);

    for (int i = h - 1; i >= 0; i--) {
        for (int j = 0; j < w; j += 8) {
            int sum = 0;
            int bit;

            for (int t = 0; t < 8; t++) {
                if (colors[i][j + t] == '1'){
                    bit = 1;
                }
                else{
                    bit = 0;
                }

                sum += bit * (int) pow(2, 7 - t);
            }
            fprintf(fout, "%c", sum);
        }
    }
    fclose(fout);
}

int main(int argc, char *argv[]) {

    char *dir;
    int freq = 1;
    int epochs = 0;
    FILE *start_pic;

    for (int i = 1; i < argc; i += 2) {
        if (!strcmp(argv[i], "--input")) {
            start_pic = fopen(argv[i + 1], "r");
        }
        else if (!strcmp(argv[i], "--max_iter")) {
            char *end;
            epochs = (int) strtol(argv[i + 1], &end, 10);
        }
        else if (!strcmp(argv[i], "--dump_freq")) {
            char *end;
            freq = (int) strtol(argv[i + 1], &end, 10);
        }
        else if (!strcmp(argv[i], "--output")) {
            dir = argv[i + 1];
            mkdir(dir);
        }
    }

    unsigned char header[54];
    int w32;
    int h;

    fread(header, 1, 54, start_pic);


    h = (int) (header[25] * pow(256, 3) +
                    header[24] * pow(256, 2) +
                    header[23] * pow(256, 1) +
                    header[22] * pow(256, 0));

    w32 = (int) (header[21] * pow(256, 3) +
                   header[20] * pow(256, 2) +
                   header[19] * pow(256, 1) +
                   header[18] * pow(256, 0));

    int offset;
    offset = (int) (header[13] * pow(256, 3) +
                        header[12] * pow(256, 2) +
                        header[11] * pow(256, 1) +
                        header[10] * pow(256, 0));
    
    struct palette p[(offset - 54) / 4];
    for (int i = 0; i < (offset - 54) / 4; i++) {
        unsigned char bytes[4];

        fread(bytes, 1, 4, start_pic);
        p[i].red = bytes[0];
        p[i].green = bytes[1];
        p[i].blue = bytes[2];
        p[i].A = 0;
    }

    unsigned char **colors;
    colors = malloc(h * sizeof(unsigned char *));
    for (int i = 0; i < h; i++) {
        colors[i] = malloc(w32 * sizeof(unsigned char *));
    }
    int w = w32;
    w32 += 32 - w32 % 32;
    for (int i = h - 1; i >= 0; i--) {
        for (int j = 0; j < w32; j += 8) {
            unsigned char byte[1];
            fread(byte, 1, 1, start_pic);
            int d = 7;

            while (d >= 0) {
                if(byte[0] % 2){
                    colors[i][j + d] = '1';
                }
                else{
                    colors[i][j + d] = '0';
                }
                byte[0] /= 2;
                d--;
            }
        }
    }

    for (int i = 1; i <= epochs; i++) {
        next_epoch(h, w32, w, colors);
        if (! (i % freq)) {
            char name[50];
            strcpy(name, dir);
            char num[5];
            strcat(name, "//");
            sprintf(num, "%d", i);
            strcat(name, num);
            strcat(name, ".bmp");
            image(colors, p, h, w32, name, header, (offset - 54) / 4);
        }
    }
    fclose(start_pic);
}