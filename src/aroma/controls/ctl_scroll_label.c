#include <aroma_internal.h>
#include "../ui/ui_internal.h"


/* SCROLL CONTROL HANDLER */
dword _libaroma_ctl_scroll_label_message(
  LIBAROMA_CONTROLP, LIBAROMA_CTL_SCROLL_CLIENTP,LIBAROMA_MSGP,int,int);
void _libaroma_ctl_scroll_label_destroy(
  LIBAROMA_CONTROLP, LIBAROMA_CTL_SCROLL_CLIENTP);
byte _libaroma_ctl_scroll_label_thread(
  LIBAROMA_CONTROLP, LIBAROMA_CTL_SCROLL_CLIENTP);
void _libaroma_ctl_scroll_label_draw(
  LIBAROMA_CONTROLP, LIBAROMA_CTL_SCROLL_CLIENTP, LIBAROMA_CANVASP,
  int, int, int, int);
static LIBAROMA_CTL_SCROLL_CLIENT_HANDLER _libaroma_ctl_scroll_label_handler={
  .message=_libaroma_ctl_scroll_label_message,
  .draw=_libaroma_ctl_scroll_label_draw,
  .destroy=_libaroma_ctl_scroll_label_destroy,
  .thread=_libaroma_ctl_scroll_label_thread
};
#define CONST_LCD_HEIGHT 400
#define PACE_COMMON_INFO_PRINT printf
#define CTL_CLK_JUDGE_AND_FREE(x, p) if((x)!=NULL)\
      {\
          p(x);\
          x=NULL;\
      }
#define CTL_CLK_JUDGE_AND_FREE_CANVAS(x) if(x)\
{\
  libaroma_canvas_free(x);\
  x=NULL;\
}


/* list structure */
typedef struct {
  char *txt;
  
  byte size;
  byte fontid; 
  byte update;
  byte hidden;

  word color;

  int h;

  
  LIBAROMA_CANVASP image; /* 缓存字的图片 */

  LIBAROMA_MUTEX mutex;

} LIBAROMA_CTL_SCROLL_LABEL, * LIBAROMA_CTL_SCROLL_LABELP;


static void _ctl_label_init_txt_image(LIBAROMA_CONTROLP ctl, 
                                LIBAROMA_CTL_SCROLL_LABELP me, int w)
{
    static byte cnt = 0;
    LIBAROMA_TEXT txt = libaroma_text(me->txt, me->color, w, 
                                      LIBAROMA_FONT(me->fontid, me->size), 0);

    if (txt != NULL)
    {
        int th = libaroma_text_height(txt);
        int tw = libaroma_text_width(txt);

        int dh = th + 5;
        if (dh > CONST_LCD_HEIGHT * 2)
        {
            dh = CONST_LCD_HEIGHT * 2;
        }

        dh += cnt;
        cnt++;
        if (cnt > 1)
        {
            cnt = 0;
        }
        
        /* set initial height */
        (void)libaroma_ctl_scroll_set_height(ctl, dh);

        CTL_CLK_JUDGE_AND_FREE_CANVAS(me->image);
        me->image = libaroma_canvas(tw, dh);

        PACE_COMMON_INFO_PRINT( "image->w:%d h:%d", tw, th);
        
        (void)libaroma_text_draw(me->image, txt, 0, 0);        

        (void)libaroma_text_free(txt);
    }    
}


LIBAROMA_CONTROLP libaroma_ctl_scroll_label_new(
    LIBAROMA_WINDOWP win, word id,
    int x, int y, int w, int h,
    char *txt, word color, byte size, byte fontid
){
  /* allocating internal data */
  LIBAROMA_CTL_SCROLL_LABELP mi = (LIBAROMA_CTL_SCROLL_LABELP)
                                  calloc(sizeof(LIBAROMA_CTL_SCROLL_LABEL),1);
  if (!mi){
    ALOGW("libaroma_ctl_list cannot allocating memory for list control");
    return NULL;
  }
  
  mi->color = color;
  mi->size = size;
  mi->fontid = fontid;
  mi->h = h;

  if(txt)
  {
    mi->txt = strdup(txt);
    mi->update = 1;
  }

  libaroma_mutex_init(mi->mutex);
  
  /* create scroll control */
  LIBAROMA_CONTROLP ctl = libaroma_ctl_scroll(win, id, x, y, w, h, 0, 0);
  if (NULL == ctl)
  {
    libaroma_mutex_free(mi->mutex);
    CTL_CLK_JUDGE_AND_FREE(mi->txt, free);
    free(mi);
    return NULL;
  }
  
  /* set scroll client */
  (void)libaroma_ctl_scroll_set_client(ctl, (voidp) mi, &_libaroma_ctl_scroll_label_handler);

  _ctl_label_init_txt_image(ctl, mi, w);
  
  return ctl;
} /* End of libaroma_ctl_list */

void libaroma_ctl_scroll_set_text(LIBAROMA_CONTROLP ctl, char *txt)
{
    LIBAROMA_CTL_SCROLL_CLIENTP client = libaroma_ctl_scroll_get_client(ctl);
    if (client == NULL)
    {
        PACE_COMMON_INFO_PRINT( "get scroll client error");
        return;
    }

    if (client->handler != &_libaroma_ctl_scroll_label_handler){
        PACE_COMMON_INFO_PRINT( "scroll client error");
        return;
    }

    LIBAROMA_CTL_SCROLL_LABELP me = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;
    
    CTL_CLK_JUDGE_AND_FREE(me->txt, free);
    me->txt = strdup(txt);

    _ctl_label_init_txt_image(ctl, me, ctl->w);
    
    (void)libaroma_ctl_scroll_set_pos(ctl, 0);
    me->update = 1;

    return;
}

void libaroma_ctl_scroll_label_set_hidden(LIBAROMA_CONTROLP ctl, byte hidden)
{
    LIBAROMA_CTL_SCROLL_CLIENTP client = libaroma_ctl_scroll_get_client(ctl);
    if (client == NULL)
    {
        PACE_COMMON_INFO_PRINT( "get scroll client error");
        return;
    }

    if (client->handler != &_libaroma_ctl_scroll_label_handler){
        PACE_COMMON_INFO_PRINT( "scroll client error");
        return;
    }

    LIBAROMA_CTL_SCROLL_LABELP me = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;

    me->hidden = hidden;
    me->update = 1;

    return;
}


/*
 * Function    : _libaroma_ctl_scroll_label_thread
 * Return Value: byte
 * Descriptions: scroll list thread
 */
byte _libaroma_ctl_scroll_label_thread(
    LIBAROMA_CONTROLP ctl, LIBAROMA_CTL_SCROLL_CLIENTP client){
  if (client->handler!=&_libaroma_ctl_scroll_label_handler){
    return 0;
  }
  LIBAROMA_CTL_SCROLL_LABELP mi = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;
  
  (void)ctl;

  if (mi->update)
  {
    return 1;
  }
  
  return 0;
} /* End of _libaroma_ctl_scroll_label_thread */


/*
 * Function    : _libaroma_ctl_scroll_label_draw
 * Return Value: void
 * Descriptions: draw routine
 */
void _libaroma_ctl_scroll_label_draw(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_SCROLL_CLIENTP client,
    LIBAROMA_CANVASP cv,
    int x, int y, int w, int h){
  if (client->handler!=&_libaroma_ctl_scroll_label_handler){
    return;
  }
  LIBAROMA_CTL_SCROLL_LABELP mi = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;

  (void)ctl;
  (void)x;
  (void)y;
  (void)w;
  (void)h;

  (void)libaroma_control_erasebg(ctl, cv);

  if (mi->hidden)
  {
    return;
  }

  if (mi->image != NULL)
  {
    int ret = libaroma_draw(cv, mi->image, 0, 0, 0xff);
    if (ret == 0)
    {
      ALOGE("[%s]draw image error", __FUNCTION__);
    }
  }


  return;
} /* End of _libaroma_ctl_scroll_label_draw */

/*
 * Function    : _libaroma_ctl_scroll_label_scroll_message
 * Return Value: dword
 * Descriptions: handle scroll message
 */
dword _libaroma_ctl_scroll_label_scroll_message(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_SCROLL_CLIENTP client,
    LIBAROMA_CTL_SCROLL_LABELP mi,
    int msg,
    int param,
    int x,
    int y){

    //fix pc-lint
    (void)ctl;
    (void)client;
    (void)mi;
    (void)param;
    (void)x;
    (void)y;
    
  switch(msg){
    case LIBAROMA_CTL_SCROLL_MSG_ISNEED_TOUCH:{
        }
      break;
    case LIBAROMA_CTL_SCROLL_MSG_TOUCH_DOWN:{
        }
      break;
    case LIBAROMA_CTL_SCROLL_MSG_TOUCH_MOVE:{
        }
      break;
    case LIBAROMA_CTL_SCROLL_MSG_TOUCH_UP:
    case LIBAROMA_CTL_SCROLL_MSG_TOUCH_CANCEL:{
        }
      break;
    default:
      break;
  }
  return 0;
} /* End of _libaroma_ctl_scroll_label_scroll_message */

/*
 * Function    : _libaroma_ctl_scroll_label_message
 * Return Value: dword
 * Descriptions: message handler
 */
dword _libaroma_ctl_scroll_label_message(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_SCROLL_CLIENTP client,
    LIBAROMA_MSGP msg,
    int x, int y)
{
  if (client->handler!=&_libaroma_ctl_scroll_label_handler){
    return 0;
  }
  LIBAROMA_CTL_SCROLL_LABELP mi = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;
  
  dword res=0;
  
  /* handle the message */
  switch(msg->msg){
    case LIBAROMA_CTL_SCROLL_MSG:{
        res=_libaroma_ctl_scroll_label_scroll_message(
          ctl, client, mi, msg->x, msg->y, x, y);
      }
      break;
    default:
      break;
  }

  return res;
} /* End of _libaroma_ctl_scroll_label_message */

/*
 * Function    : _libaroma_ctl_scroll_label_destroy
 * Return Value: void
 * Descriptions: destroy scroll list client
 */
void _libaroma_ctl_scroll_label_destroy(
    LIBAROMA_CONTROLP ctl, LIBAROMA_CTL_SCROLL_CLIENTP client)
{
  (void)ctl;

  PACE_COMMON_INFO_PRINT( " %s :	 GUI_START	\r\n",__func__);

  if (client->handler!=&_libaroma_ctl_scroll_label_handler){
    return;
  }
  LIBAROMA_CTL_SCROLL_LABELP mi = (LIBAROMA_CTL_SCROLL_LABELP) client->internal;

  CTL_CLK_JUDGE_AND_FREE_CANVAS(mi->image);
  CTL_CLK_JUDGE_AND_FREE(mi->txt, free);
  
  libaroma_mutex_free(mi->mutex);
  free(mi);
  client->internal=NULL;
  client->handler=NULL;
  PACE_COMMON_INFO_PRINT( " %s :	GUI_END   	\r\n",__func__);

} /* End of _libaroma_ctl_scroll_label_destroy */

