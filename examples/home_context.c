/*
 * clock_test.c
 *
 *  Created on: 2016年8月7日
 *      Author: fine
 */
#include <aroma.h>
#include "resource.h"
#include "aroma_internal.h"
#include "pace_protocol_manager.h"
#include "pace_hrm_provider.h"

#include "ff.h"
#include "timers.h"

static void home_on_create(LIBAROMA_CONTEXTP context);
static void home_on_resume(LIBAROMA_CONTEXTP context);
static void home_on_pause(LIBAROMA_CONTEXTP context);
static void home_on_destroy(LIBAROMA_CONTEXTP context);

LIBAROMA_CANVASP libaroma_get_stream_file_in_flash_raw(char * path, int index);

#define RES_PATH "0:/res/image/png_res_clock.raw"

extern const char home_strings[869];
extern const char home_colors[97];
static LIBAROMA_CONTROLP g_home_image;
static LIBAROMA_CONTROLP g_home_label;
static TimerHandle_t g_home_timer = NULL;


extern const char png_res_clock[];

static void home_on_finger(LIBAROMA_CONTEXTP context, byte direction)
{
	LIBAROMA_INTENTP intent = NULL;

	switch (direction)
	{
	case LIBAROMA_HID_EV_FINGER_LEFT:

		break;

	case LIBAROMA_HID_EV_FINGER_RIGHT:
		libaroma_context_finish(context);
		break;

	default:
		ALOGW("%s error direction", __FUNCTION__)
		break;
	}
}
static byte hrm_event_handler(LIBAROMA_CONTEXTP context, LIBAROMA_GUI_EVENTP event)
{
	switch (event->id)
	{

	case MSG_HRM_PRO_AVG_VALUE:

		char g_home_images[16];
		sprintf(g_home_images, "%d", ((hrm_avg_data *)event->data)->hrm_avg_value);
		libaroma_ctl_label_set_text(g_home_image, g_home_images, 1);
		//printf("*********************g_home_image : %s  **********************************" ,g_home_images);
	}
}

LIBAROMA_CONTEXT libaroma_home_context =
	{
		.on_create = home_on_create,
		.on_resume = home_on_resume,
		.on_pause = home_on_pause,
		.on_destroy = home_on_destroy,
		.window = NULL,
		.event_handler = hrm_event_handler,
		.images = (char *)png_res_clock,
		.on_finger = home_on_finger,
		.strings = (char *)home_strings,
		.colors = (char *)home_colors

};

static void pace_sports_request(uint32_t function, int gmtTime)
{
	MsgParamForSports msg = {0};
	msg.needGps = 1;
	msg.type = PACE_SPORTS_RUNNING;
	msg.timestamp = gmtTime;

	if (function == 1)
	{
		MessageSend(MSG_GUI_SPORTS_PRESS_BEGIN, &msg, sizeof(msg), MESSAGE_IS_POINTER);
		ALOGI("================>send msg begin %d<====================", MSG_GUI_SPORTS_PRESS_BEGIN);
	}
	else if (function == 2)
	{
		MessageSend(MSG_GUI_SPORTS_PRESS_STOP, &msg, sizeof(msg), MESSAGE_IS_POINTER);
		ALOGI("================>send msg stop %d<====================", MSG_GUI_SPORTS_PRESS_STOP);
	}
	else if (function == 3)
	{
		MessageSend(MSG_GUI_SPORTS_PRESS_RESUME, &msg, sizeof(msg), MESSAGE_IS_POINTER);
		ALOGI("================>send msg resume %d<====================", MSG_GUI_SPORTS_PRESS_STOP);
	}
	else if (function == 4)
	{
		MessageSend(MSG_GUI_SPORTS_PRESS_PAUSE, &msg, sizeof(msg), MESSAGE_IS_POINTER);
		ALOGI("================>send msg pause %d<====================", MSG_GUI_SPORTS_PRESS_STOP);
	}
}

static void home_test_image(void)
{
    static byte index = 0;
      char text[16] = {0};
    
      LIBAROMA_CANVASP canvas = NULL;
    
      canvas = libaroma_get_stream_file_in_flash_raw(RES_PATH, index);
      if (canvas != NULL)
      {    
          libaroma_ctl_image_set_canvas(g_home_image,  canvas, 1);
          libaroma_canvas_free(canvas);
      }
    
      snprintf(text, 16, "%d", index);    
      libaroma_ctl_label_set_text(g_home_label, text, 1); 

      index++;
      if (index > 45)
      {
          index = 0;
      }    

}

static byte home_click_lisnter(LIBAROMA_CONTROLP ctl)
{
	int para = 1;
    
    home_test_image();
    
	switch (ctl->id)
	{
        case 1:
        

		break;
	case 3:
		ALOGI("================>stop click<====================");

		//pace_sports_request(2, 20000);
		MessageSend(MSG_GUI_HRM_TEST_STOP, &para, sizeof(int), MESSAGE_IS_POINTER);
		break;
	}
	return 0;
}

static LIBAROMA_WINDOWP create_home_context_window(voidp event_data, LIBAROMA_CONTEXTP context)
{

	ALOGI(" create_home_context_window--------------");
	LIBAROMA_RESOURCEP resource = libaroma_context_get_resource(context);
	LIBAROMA_WINDOWP win = libaroma_window(NULL, 0, 0, LIBAROMA_SIZE_FULL, LIBAROMA_SIZE_FULL); /* fullscreen */

    LOG_I(common, "image load from flash start tick : %d ", libaroma_tick());    
    LIBAROMA_CANVASP canvas =  libaroma_get_stream_file_in_flash_raw(RES_PATH, 0);
    LOG_I(common, "image load from flash stop tick : %d ", libaroma_tick());    
    g_home_image = libaroma_ctl_image_canvas(win, 1, canvas, 0, 0, 360, 360);
    libaroma_canvas_free(canvas);

    g_home_label = libaroma_ctl_label(win, 2, "0", 20, 165, 60, 30, 
                                    RGB(FFFFFF), LIBAROMA_CHINESE_TEXT_SIZE_18PX, 
                                           42, 10);

    libaroma_control_set_click_listener(g_home_image, home_click_lisnter);

    return win;
}

static void home_on_create(LIBAROMA_CONTEXTP context)
{
}
static void home_on_resume(LIBAROMA_CONTEXTP context)
{
	libaroma_context_set_window(context, create_home_context_window(NULL, context));
}
static void home_on_pause(LIBAROMA_CONTEXTP context)
{    
}
static void home_on_destroy(LIBAROMA_CONTEXTP context)
{
}

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


/* raw data 私有头 0x99, 0x50, 0x4E, 0x47 */

#define RAW_DATA_HDR 0x474e5099

static byte check_rawdata_hdr(const char *addr)
{
    dwordp pionter = (dwordp)addr;

    if (*pionter == RAW_DATA_HDR)
    {
        return 1;
    }
        
    return 0;
}

LIBAROMA_CANVASP libaroma_get_stream_file_in_flash_raw(char * path, int index)
{
    FRESULT ret = FR_OK;

    if (!path) 
    {
        LOG_I(common, "path is invalid");
        return NULL;
    }
        
    int filesize = libaroma_filesize(path);
    if (filesize < sizeof(LIBAROMA_CANVAS_FILE_HDR)) {
        LOG_I(common, "file (%s) not found", path);
        return NULL;
    }

    FIL fdst;   

    LIBAROMA_CANVAS_FILE_HDR hdr;
    ret = f_open(&fdst, _T(path), FA_OPEN_EXISTING | FA_READ);
    if (ret != FR_OK) 
    {
        LOG_I(common, "fopen error (%s)", path);
        return NULL;
    }

    UINT length_read ;
    FSIZE_t file_offset = 0;

    for (int i = 0; i < index; i++)
    {
        ret = f_lseek(&fdst, file_offset);
        if (ret != FR_OK)
        {
            LOG_I(common, "flseek error (%s) offset : %d", path, file_offset);
            f_close(&fdst);
            return NULL;
        }
    
        /* 先读私有头 */
        ret = f_read(&fdst, &hdr, sizeof(hdr), &length_read);
        if (FR_OK != ret && length_read != sizeof(hdr)) 
        {
            LOG_I(common, "fread error (%s)", path);
            f_close(&fdst);
            return NULL;
        }

        /* 根据私有头，确定下面的数据块的大小 */
        if (check_rawdata_hdr((const char *)&hdr) == 0)
        {            
            LOG_I(common, "decode error (%#x) correct is %#x, index:%d", *((dwordp)&hdr), RAW_DATA_HDR, i);
            f_close(&fdst);
            return NULL;
        }

        file_offset += hdr.len + sizeof(dword) + sizeof(byte) * 4;
    }

    ret = f_lseek(&fdst, file_offset);
    if (ret != FR_OK)
    {
        LOG_I(common, "flseek error (%s) offset : %d", path, file_offset);
        f_close(&fdst);
        return NULL;
    }

    /* 先读私有头 */
    ret = f_read(&fdst, &hdr, sizeof(hdr), &length_read);
    if (FR_OK != ret && length_read != sizeof(hdr)) 
    {
        LOG_I(common, "fread error (%s)", path);
        f_close(&fdst);
        return NULL;
    }
    
    /* 根据私有头，确定下面的数据块的大小 */
    if (check_rawdata_hdr((const char *)&hdr) == 0)
    {            
        LOG_I(common, "decode error (%#x) correct is %#x, index:%d", *((dwordp)&hdr), RAW_DATA_HDR, index);
        f_close(&fdst);
        return NULL;
    }

    LIBAROMA_CANVASP canvas = libaroma_canvas_alpha(hdr.w, hdr.h);
    if (canvas == NULL)
    {
        LOG_I(common, "memcpy alloc error");
        f_close(&fdst);        
        return NULL;      
    }

    LOG_I(common, "w:%d h:%d s:%d alpha", canvas->w, canvas->h, canvas->s, hdr.alpha_flag);

    ret = f_read(&fdst, canvas->data, sizeof(word) * canvas->s, &length_read);
    if (FR_OK != ret && length_read != sizeof(word) * canvas->s)
    {
        LOG_I(common, "fread error (%s)", path);
        f_close(&fdst);
        libaroma_canvas_free(canvas);
        return NULL;
    }
    
    if (hdr.alpha_flag != 0)
    {
        ret = f_read(&fdst, canvas->alpha, sizeof(byte) * canvas->s, &length_read);
        if (FR_OK != ret && length_read != sizeof(byte) * canvas->s)
        {
            LOG_I(common, "fread error (%s)", path);
            f_close(&fdst);
            libaroma_canvas_free(canvas);
            return NULL;
        }
    }
    
    f_close(&fdst); 
    return canvas;
}
