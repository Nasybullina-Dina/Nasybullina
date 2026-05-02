#include <stdio.h>
#include <stdlib.h>
#include "lodepng.h"
#define min_component_size 4

#define dfs_use_adj 1
#define dfs_count_size 2
#define dfs_mark_big 4
#define dfs_check 8
#define dfs_value_is_one 16
#define dfs_value_not_255 32

typedef struct point {
    int x, y;
} point;

int dfs(int start_x, int start_y, int width, int height, unsigned char* bw_pic, int* adj, int* visited, int* big, int* ok, int mode)
{
    int* stack = malloc(width * height * sizeof(int));
    if (stack==NULL)
        return 0;
    int top = 0;
    stack[top++] = start_y*width + start_x;
    visited[start_y*width + start_x] = 1;

    int count = 0;
    int di[4] = {-1, 1, 0, 0};
    int dj[4] = {0, 0, -1, 1};

    int use_adj = mode & dfs_use_adj;
    int count_size = mode & dfs_count_size;
    int mark_big = mode & dfs_mark_big;
    int check_boundary = mode & dfs_check;
    int val_not_255 = mode & dfs_value_not_255;
    int idx, x, y, nx, ny, nidx, d, can_visit;
    while (top > 0) {
        idx = stack[--top];
        if (count_size)
            count++;
        x = idx%width;
        y = idx/width;
        if (mark_big && big)
            big[idx] = 1;
        for (d = 0; d < 4; d++) {
            nx = x+dj[d];
            ny = y+di[d];
            if ((nx < 0 || nx >= width || ny < 0 || ny >= height)==0)
            {
                nidx = ny*width + nx;
                if (!visited[nidx])
                {
                    can_visit = 0;
                    if (use_adj && adj) {
                        can_visit = adj[nidx] == 1;
                    } else if (bw_pic) {
                        if (val_not_255)
                            can_visit = bw_pic[nidx] != 255;
                        else can_visit = bw_pic[nidx] == 0;
                    }
                    if (can_visit) {
                        visited[nidx] = 1;
                        stack[top++] = nidx;
                    } else if (check_boundary && ok && bw_pic && big) {
                        if (bw_pic[nidx] == 0 && big[nidx] == 1) {
                            *ok = 0;
                        }
                    }
                }
            }
        }
    }
    free(stack);
    return count;
}

unsigned char* load_png(const char* filename, unsigned int* width, unsigned int* height)
{
    unsigned char* image = NULL;
    int error = lodepng_decode32_file(&image, width, height, filename);
    if(error != 0) {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    return image;
}

void write_png(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
    unsigned char* png;
    size_t pngsize;
    int error = lodepng_encode32(&png, &pngsize, image, width, height);
    if(error == 0) {
        lodepng_save_file(png, pngsize, filename);
    } else {
        printf("error %u: %s\n", error, lodepng_error_text(error));
    }
    free(png);
}

void convert(unsigned char* bw_pic, unsigned char* pic, int size)
{
    int i; unsigned char r, g, b;
    for (i = 0; i < size; i += 4) {
        r = pic[i];
        g = pic[i+1];
        b= pic[i+2];
        bw_pic[i/4] = (r+g+b)/3;
    }
}

void convert_to_bw(unsigned char* bw_pic, unsigned char* picture, int bw_size, unsigned int width, unsigned int height)
{
    convert(bw_pic, picture, width * height * 4);
    int* used = calloc(bw_size, sizeof(int));
    if (used==NULL)
        return;
    unsigned int x, y; int idx, size;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            idx = y*width + x;
            if (bw_pic[idx] != 255 && !used[idx]) {
                size = dfs(x, y, width, height, bw_pic, NULL, used, NULL, NULL,dfs_count_size|dfs_value_not_255);
                if (size <= min_component_size) {
                    bw_pic[idx] = 0;
                }
            }
        }
    }
    free(used);
}

void contrast(unsigned char *col, int bw_size)
{
    int i;
    for (i = 0; i < bw_size; i++) {
        if (col[i] <= 75)
            col[i] = 0;
        else if (col[i] > 190)
            col[i] = 255;
    }
}

void Gauss_blur(unsigned char *col, unsigned char *blr_pic, unsigned int width, unsigned int height)
{
    int i, j;
    for(i = 0; i < height; i++) {
        for(j = 0; j < width; j++) {
            blr_pic[i*width + j] = col[i*width + j];
        }
    }
    for(i=1; i < height-1; i++) {
        for(j=1; j < width-1; j++)
        {
            blr_pic[width*i+j] = 0.084*col[width*i+j] + 0.084*col[width*(i+1)+j] + 0.084*col[width*(i-1)+j];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.084*col[width*i+(j+1)] + 0.084*col[width*i+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i+1)+(j+1)] + 0.063*col[width*(i+1)+(j-1)];
            blr_pic[width*i+j] = blr_pic[width*i+j] + 0.063*col[width*(i-1)+(j+1)] + 0.063*col[width*(i-1)+(j-1)];
        }
    }
}

void create_adj(int* adj, unsigned char* pic, unsigned int width, unsigned int height)
{
    int i, j, idx, pic_idx;
    float brightness;
    for (i = 0; i < height; i++) {
        for (j = 0; j < width; j++) {
            idx = i*width + j;
            pic_idx = idx*4;
            brightness = 0.299*pic[pic_idx] + 0.587*pic[pic_idx+1] + 0.114*pic[pic_idx+2];
            if (brightness > 17) {
                adj[idx] = 1;
            } else {
                adj[idx] = 0;
            }
        }
    }
}

int count_tankers(int* adj, unsigned int width, unsigned int height, int* big)
{
    int* visited = calloc(width * height, sizeof(int));
    if (visited==NULL)
        return 0;
    int max_sz = 0, best_x = 0, best_y = 0, sz; unsigned int x, y, i;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int idx = y*width + x;
            if (!visited[idx] && adj[idx]) {
                sz = dfs(x, y, width, height, NULL, adj, visited, NULL, NULL, dfs_count_size|dfs_use_adj|dfs_value_is_one);
                if (sz > max_sz) {
                    max_sz = sz;
                    best_x = x;
                    best_y = y;
                }
            }
        }
    }
    for (i = 0; i < width*height; i++)
        visited[i] = 0;
    if (max_sz > 0)
        dfs(best_x, best_y, width, height, NULL, adj, visited, big, NULL, dfs_mark_big|dfs_use_adj|dfs_value_is_one);
    for (i = 0; i < width*height; i++)
        visited[i] = 0;
    int count = 0, idx;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            idx = y*width + x;
            if (!visited[idx] && adj[idx]) {
                int ok = 1;
                dfs(x, y, width, height, NULL, adj, visited, big, &ok, dfs_use_adj|dfs_value_is_one|dfs_check|dfs_count_size);
                if (ok)
                    count++;
            }
        }
    }
    free(visited);
    return count;
}

int main(void)
{
    const char* filename = "skull.png";
    unsigned int width, height, size, i, bw_size;

    unsigned char* picture = load_png(filename, &width, &height);
    if (picture == NULL)
    {
        printf("Problem reading picture from the file %s. Error.\n", filename);
        return -1;
    }
    size = width * height * 4;
    bw_size = width * height;

    unsigned char* bw_pic = malloc(bw_size*sizeof(unsigned char));
    unsigned char* blr_pic = malloc(bw_size*sizeof(unsigned char));
    unsigned char* result = malloc(size*sizeof(unsigned char));
    convert_to_bw(bw_pic, picture, bw_size, width, height);
    contrast(bw_pic, bw_size);
    for (i = 0; i < bw_size; i++) {
        result[4*i] = result[4*i+1] = result[4*i+2] = bw_pic[i];
        result[4*i+3] = 255;
    }
    write_png("contrast.png", result, width, height);

    Gauss_blur(bw_pic, blr_pic, width, height);
    for (i = 0; i < bw_size; i++) {
        result[4*i] = result[4*i+1] = result[4*i+2] = blr_pic[i];
        result[4*i+3] = 255;
    }
    write_png("gauss.png", result, width, height);

    int* big = calloc(bw_size, sizeof(int));
    int* adj = malloc(bw_size * sizeof(int));
    create_adj(adj, result, width, height);
    printf("number of tankers: %d\n", count_tankers(adj, width, height, big));
    write_png("result.png", result, width, height);

    free(picture);
    free(bw_pic);
    free(blr_pic);
    free(result);
    free(big);
    free(adj);
    return 0;
}