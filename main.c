#include <stdio.h>
#include <stdlib.h>  
#include <string.h>  
#include <dirent.h>  
#include <unistd.h> 
#include "aroma.h"

typedef struct tagCanvasRamDataHdr
{
    byte chk_hdr[4];    /* 私有头目前定义为 0x99, 0x50, 0x4E, 0x47 */
    dword len;          /* 后面的长度不包括自身 */
    byte alpha_flag;    /* 带不带alpha */
    byte rsv[1];        /* 保留位 */
    word w;            /* 图片的宽度 */
    word h;            /* 图片的高度 */
    word l;            /* 图片一行的大小 */
}LIBAROMA_CANVAS_FILE_HDR, *LIBAROMA_CANVAS_FILE_HDRP;

int write_canvas_to_file(LIBAROMA_CANVASP canvas, FILE *fbin, char *name);
int read_dir_pngs_convert_ram_data(char *basePath, FILE *fh);

void get_mode_name_from_dir(const char *src, char *dst)
{
    int len = strlen(src);
    int start = 0;
    int fix_flag = 0;

    dst[0] = '\0';

    for (start = len - 2; start >= 0; start--)
    {
        if(src[start] == '/')
        {
            fix_flag = 1;
            break;
        }
    }

    start++;

    if (fix_flag == 1)
    {
        int i = 0;
        for (; start < len - 1; start++)
        {
            dst[i++] = src[start];
        }
        dst[i] = '\0';
    }

    return;
}


int main(int argc, char *argv[])
{
    char *dir = NULL;

    if (argc > 2)
    {
        printf("Usage: ./png2ram [dir] \n");
        printf("\t if dir NULL use ./ \n");
        return -1;
    }
    else if (argc == 2)
    {
        dir = argv[1];
    }
    else 
    {
        dir = "./";
    }

    char ahfile[256];
    char mode_name[256];

    get_mode_name_from_dir(dir, mode_name);

    strcpy(ahfile, dir);
    strcat(ahfile, mode_name);
    strcat(ahfile, "_png_res.h");

    printf("dir:%s  mode:%s\n", dir, mode_name);
    
    FILE *fh = fopen(ahfile, "w+");
    if (fh == NULL)
    {
        fprintf(stderr, "open png_res.h file failed\n");
        return -1;
    }


    read_dir_pngs_convert_ram_data(dir, fh);

    fclose(fh);


    return 0;
}

int write_single_image_to_file(char *abs_name, char *image_name, FILE *fbin, int *all_size)
{
    LIBAROMA_STREAMP stream = NULL;
    LIBAROMA_CANVASP canvas = NULL;

    stream = libaroma_stream_file(abs_name);
    if (stream == NULL)
    {
        printf("error in get image stream \r\n");
        return -2;
    }

    canvas = libaroma_image_ex(stream, 1, 1);
    if (canvas == NULL)
    {
        printf("get canvas error \r\n");
        return -3;
    }

    printf("load image success W:%d, h:%d, flag:%d, l:%d, s:%d\n", 
                canvas->w, canvas->h, canvas->flags, canvas->l, canvas->s);

    write_canvas_to_file(canvas, fbin, image_name);
    
    int alpha_flag = (canvas->alpha == NULL) ? 0 : 1;

    *all_size = sizeof(LIBAROMA_CANVAS_FILE_HDR) + (sizeof(word) + sizeof(byte) * alpha_flag)*canvas->w*canvas->h;

    libaroma_canvas_free(canvas);
    
    return 0;
}

void construct_canvas_file_head(LIBAROMA_CANVASP canvas, LIBAROMA_CANVAS_FILE_HDRP hdr)
{
    /*  0x99, 0x50, 0x4E, 0x47 */
    hdr->chk_hdr[0] = 0x99;
    hdr->chk_hdr[1] = 0x50;
    hdr->chk_hdr[2] = 0x4e;
    hdr->chk_hdr[3] = 0x47;
    
    hdr->alpha_flag = (canvas->alpha == NULL) ? 0 : 1;
    hdr->len = sizeof(LIBAROMA_CANVAS_FILE_HDR) - sizeof(byte) * 4 - sizeof(dword);
    hdr->len += (sizeof(word) + sizeof(byte) * hdr->alpha_flag) * canvas->w * canvas->h;
    hdr->w = (word)canvas->w;
    hdr->h = (word)canvas->h;
    hdr->l = (word)canvas->l;

    return;
}

int write_canvas_head_to_file(LIBAROMA_CANVASP canvas, FILE *fbin, char *name)
{
    LIBAROMA_CANVAS_FILE_HDR hdr = {0};
    bytep canvas_header = (bytep)&hdr;
    
    construct_canvas_file_head(canvas, &hdr);

    if (fwrite(&hdr, sizeof(hdr), 1, fbin) != 1)
    {
        fprintf(stderr, "error in write fbin head\n");
    }

    return 0;
}

int write_canvas_to_file(LIBAROMA_CANVASP canvas, FILE *fbin, char *name)
{
    write_canvas_head_to_file(canvas, fbin, name);
    unsigned char *data_char_type = (unsigned char *)canvas->data;

    fwrite(canvas->data, sizeof(word), canvas->s, fbin);

    if (canvas->alpha == NULL)
    {
        return 0;
    }

    fwrite(canvas->alpha, sizeof(byte), canvas->s, fbin);

    return 0;
}

byte is_png_file(char *name)
{
    int len = strlen(name);
    //printf("%s : %c%c%c%c\n", name, name[len-4], name[len-3],name[len-2],name[len-1]);

    if ((name[len - 4] == '.')
    && ( (name[len-3] == 'p') || (name[len-3] == 'P'))
    && ( (name[len-2] == 'n') || (name[len-2] == 'N'))
    && ( (name[len-1] == 'g') || (name[len-1] == 'G')))
    {
        return 1;
    }

    return 0;
}

void fix_file_name_for_macro(const char *src, char *dst)
{
    int len = strlen(src);

    snprintf(dst, 256, "%s", src);

    if (len > 255) len = 255;
    dst[len - 4] = '\0';

    for(int i = 0; i < len; i++)
    {
        if (dst[i] >= 'a' && dst[i] <= 'z')
        {
            dst[i] = dst[i] + 'A' - 'a';
        }
    }

    return;
}

int read_dir_pngs_convert_ram_data(char *basePath, FILE *fh)
{   
    int ret = 0;
    DIR *dir;
    struct dirent *ptr;
    dword offset = 0, total_size = 0, all_size = 0;
    char buffer[256];
    char abs_dir[1024];
    dword array_offset[2048] = {0};

    if ((dir=opendir(basePath)) == NULL)
    {
        perror("Open dir error...");
        return -1;
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
        {
            continue;
        }   
        else if(ptr->d_type == 8 && is_png_file(ptr->d_name))    ///file
        {
            char aFileName[1024];

            strncpy(aFileName, basePath, 1024);
            strcat(aFileName, ptr->d_name);

            int len = strlen(aFileName);
            aFileName[len - 4] = '.';
            aFileName[len - 3] = 'r';
            aFileName[len - 2] = 'a';
            aFileName[len - 1] = 'w';
            aFileName[len] = '\0';

            printf("\r\n %s \r\n",aFileName);

            FILE *fbin = fopen(aFileName, "w+");
            if (fh == NULL)
            {
                fprintf(stderr, "open png_res.h file failed\n");
                continue;
            }

            strncpy(abs_dir, basePath, 1024);
            strcat(abs_dir, ptr->d_name);

	        printf("%s start convert \n", ptr->d_name);
            ret = write_single_image_to_file(abs_dir, ptr->d_name, fbin, &all_size);
            if (ret != 0)
            {
                fclose(fbin);
                continue;
            }

            fix_file_name_for_macro(ptr->d_name, buffer);

            fprintf(fh, "#define RES_INDEX_%s %d\n", buffer, offset);
            //            fprintf(fh, "#define RES_OFFSET_%s %d\n", buffer, total_size);

            array_offset[offset] = total_size;

            total_size += all_size;
            offset++;

            fclose(fbin);
	    }
        else 
        {
	        continue;
        }
    }

    fprintf(fh, "\r\nconst unsigned long g_res_offset[%d] = {\r\n", offset);
    for (int i = 0; i < offset; i++)
    {
        fprintf(fh, " %d,", array_offset[i]);
        if (i % 16 == 15)
        {
            fprintf(fh, "\r\n");
        } 
    }
    fprintf(fh, "\r\n};\r\n");

    closedir(dir);
    return 0;
}