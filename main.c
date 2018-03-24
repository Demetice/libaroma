#include <stdio.h>
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

int write_canvas_to_file(LIBAROMA_CANVASP canvas, FILE *in, char *name);

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: ./png2ram xxx.png out.h\r\n");
        return -1;
    }

    LIBAROMA_STREAMP stream = NULL;
    LIBAROMA_CANVASP canvas = NULL;

    stream = libaroma_stream_file(argv[1]);
    if (stream == NULL)
    {
        printf("error in get image stream \r\n");
        return -2;
    }

    canvas = libaroma_image_ex(stream, 1, 0);
    if (canvas == NULL)
    {
        printf("get canvas error \r\n");
        return -3;
    }

    printf("load image success W:%d, h:%d, flag:%d, l:%d, s:%d\n", 
                canvas->w, canvas->h, canvas->flags, canvas->l, canvas->s);

    FILE *file_in = fopen(argv[2], "w+");
    if (file_in == NULL)
    {
        libaroma_canvas_free(canvas);
        return -4;
    }

    fprintf(file_in, "const char png_res[] = {\r\n");
    
    write_canvas_to_file(canvas, file_in, argv[1]);
    
    fprintf(file_in, "\r\n};\r\n");

    fclose(file_in);
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

int write_canvas_head_to_file(LIBAROMA_CANVASP canvas, FILE *in, char *name)
{
    LIBAROMA_CANVAS_FILE_HDR hdr = {0};
    bytep canvas_header = (bytep)&hdr;
    
    construct_canvas_file_head(canvas, &hdr);
    
    fprintf(in, "\r\n /* %s image header  */ \r\n", name);

    for (int i = 0; i < sizeof(hdr); i++)
    {
        fprintf(in, " %d,", canvas_header[i]);
    }

    return 0;
}

int write_canvas_to_file(LIBAROMA_CANVASP canvas, FILE *in, char *name)
{
    write_canvas_head_to_file(canvas, in, name);

    /* 用于记录换行的 */
    int count = 0;
    unsigned char *data_char_type = (unsigned char *)canvas->data;

    fprintf(in, "\r\n /* %s image data */ \r\n", name);
    for(int i = 0; i < canvas->h; i++)
    {
        for(int j = 0; j < canvas->w; j++)
        {
            fprintf(in, " %d, %d,", data_char_type[count], data_char_type[count + 1]);

            if (((count + 1) % 16) == 15)
            {
                fprintf(in, "\r\n");
            }
            count += 2;
        }
    }


    if (canvas->alpha == NULL)
    {
        return 0;
    }

    fprintf(in, "\r\n /* %s alpha data */ \r\n", name);
    count = 0;

    for(int i = 0; i < canvas->h; i++)
    {
        for(int j = 0; j < canvas->w; j++)
        {
            fprintf(in, " %d,", canvas->alpha[count]);

            if ((count % 16) == 15)
            {
                fprintf(in, "\r\n");
            }

            count ++;
        }
    }

    return 0;
}