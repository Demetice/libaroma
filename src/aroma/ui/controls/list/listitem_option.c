/********************************************************************[libaroma]*
 * Copyright (C) 2011-2015 Ahmad Amarullah (http://amarullz.com/)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *______________________________________________________________________________
 *
 * Filename    : listitem_option.c
 * Description : list item option
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 07/03/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_aroma_c__
  #error "Should be inside aroma.c."
#endif
#ifndef __libaroma_listitem_option_c__
#define __libaroma_listitem_option_c__

/* LIST ITEM HANDLER */
byte _libaroma_listitem_option_message(
  LIBAROMA_CONTROLP, LIBAROMA_CTL_LIST_ITEMP,byte,dword,int,int);
void _libaroma_listitem_option_draw(
  LIBAROMA_CONTROLP,LIBAROMA_CTL_LIST_ITEMP,LIBAROMA_CANVASP,word,byte);
void _libaroma_listitem_option_destroy(
  LIBAROMA_CONTROLP,LIBAROMA_CTL_LIST_ITEMP);
static LIBAROMA_CTL_LIST_ITEM_HANDLER _libaroma_listitem_option_handler =
{
  message:_libaroma_listitem_option_message,
  draw:_libaroma_listitem_option_draw,
  destroy:_libaroma_listitem_option_destroy
};

/* LIST ITEM INTERNAL DATA */
typedef struct{
  byte selected;
  char * main_text;
  char * extra_text;
  LIBAROMA_CANVASP icon;
  int h;
} _LIBAROMA_LISTITEM_OPTION, * _LIBAROMA_LISTITEM_OPTIONP;

/*
 * Function    : _libaroma_listitem_option_message
 * Return Value: byte
 * Descriptions: message handler
 */
byte _libaroma_listitem_option_message(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_LIST_ITEMP item,
    byte msg,
    dword param,
    int x,
    int y){
  if (item->handler!=&_libaroma_listitem_option_handler){
    return 0;
  }
  /*
  _LIBAROMA_LISTITEM_OPTIONP mi = 
    (_LIBAROMA_LISTITEM_OPTIONP) item->internal;
  */
  switch (msg){
    case LIBAROMA_CTL_LIST_ITEM_MSG_THREAD:
      {
        return 0;
      }
      break;
    case LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_DOWN:
      {
        printf("OPT MSG = DOWN\n");
      }
      break;
    case LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_HOLDED:
      {
        printf("OPT MSG = HOLDED\n");
      }
      break;
    case LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_UP:
      {
        if (param!=LIBAROMA_CTL_LIST_ITEM_MSGPARAM_HOLDED){
          printf("OPT MSG = TOUCH UP - NOT HOLDED\n");
        }
      }
    case LIBAROMA_CTL_LIST_ITEM_MSG_TOUCH_CANCEL:
      {
        return 0;
      }
      break;
  }
  return 0;
} /* End of _libaroma_listitem_option_message */

/*
 * Function    : _libaroma_listitem_option_draw
 * Return Value: void
 * Descriptions: item draw routine
 */
void _libaroma_listitem_option_draw(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_LIST_ITEMP item,
    LIBAROMA_CANVASP cv,
    word bgcolor,
    byte state){
  if (item->handler!=&_libaroma_listitem_option_handler){
    return;
  }
  _LIBAROMA_LISTITEM_OPTIONP mi = (_LIBAROMA_LISTITEM_OPTIONP) item->internal;
  
  byte is_dark=libaroma_color_isdark(bgcolor);
  word textcolor, graycolor;
  
  
  byte flags=item->flags;
  int vpad = (flags&LIBAROMA_LISTITEM_LARGE_PADDING)?16:8;
  int seph = (flags&LIBAROMA_LISTITEM_WITH_SEPARATOR)?1:0;
  byte small_icon = (flags&LIBAROMA_LISTITEM_OPTION_SMALL_ICON)?1:0;
  
  if (state){
    word selcolor = is_dark?RGB(335577):RGB(999999);
    libaroma_draw_rect(
      cv,
      0,0,
      cv->w,
      cv->h-libaroma_dp(1),
      selcolor,0xff
    );
    textcolor= is_dark?RGB(ffffff):RGB(000000);
    graycolor= is_dark?RGB(888888):RGB(777777);
  }
  else{
    textcolor= is_dark?RGB(ffffff):RGB(000000);
    graycolor= is_dark?RGB(888888):RGB(777777);
  }
  
  if ((item->next)&&(flags&LIBAROMA_LISTITEM_WITH_SEPARATOR)){
    libaroma_draw_rect(
      cv,
      0,
      cv->h-libaroma_dp(1),
      cv->w,
      libaroma_dp(1),
      is_dark?RGB(555555):RGB(cccccc),
      0xff
    );
  }
  
  int icoh=libaroma_dp(vpad*2+seph);
  int tw = cv->w - libaroma_dp(32);
  int tx = libaroma_dp(16);
  if (mi->icon){
    int icon_x=libaroma_dp(small_icon?23:16);
    if ((flags&LIBAROMA_LISTITEM_OPTION_FREE_ICON)||
      (!(flags&LIBAROMA_LISTITEM_OPTION_SHARED_ICON))){
      libaroma_draw(
        cv,
        mi->icon,
        icon_x, libaroma_dp(vpad),
        1
      );
      icoh+=mi->icon->h;
    }
    else{
      int dpsz=libaroma_dp(small_icon?24:38);
      libaroma_draw_scale_smooth(
        cv, mi->icon,
        icon_x, libaroma_dp(vpad), dpsz, dpsz,
        0, 0, mi->icon->w, mi->icon->h
      );
      icoh+=dpsz;
    }
  }
  if ((mi->icon)||(item->flags&LIBAROMA_LISTITEM_OPTION_INDENT_NOICON)){
    tw-=libaroma_dp(56);
    tx+=libaroma_dp(56);
  }
  
  int ty = libaroma_dp(vpad);
  LIBAROMA_TEXT mtextp=NULL;
  LIBAROMA_TEXT etextp=NULL;
  
  /* prepare main text */
  int m_y=ty;
  int m_h=0;
  if (mi->main_text){
    mtextp = libaroma_text(
      mi->main_text,
      textcolor,
      tw,
      LIBAROMA_FONT(0,4)|
      LIBAROMA_TEXT_FIXED_INDENT|
      LIBAROMA_TEXT_FIXED_COLOR|
      LIBAROMA_TEXT_NOHR,
      120
    );
    m_h=libaroma_text_height(mtextp);
    ty+=m_h;
  }
  
  /* prepare extra text */
  int e_y = ty;
  int e_h = 0;
  if (mi->extra_text){
    etextp = libaroma_text(
      mi->extra_text,
      graycolor,
      tw,
      LIBAROMA_FONT(0,3)|
      LIBAROMA_TEXT_FIXED_INDENT|
      LIBAROMA_TEXT_FIXED_COLOR|
      LIBAROMA_TEXT_NOHR,
      120
    );
    e_h=libaroma_text_height(etextp);
    ty+=e_h;
  }
  
  /* calculate whole height */
  ty+=libaroma_dp(vpad+seph);
  int my_h = MAX(icoh,ty);
  
  /* draw main text */
  if (mtextp){
    if (etextp){
      libaroma_text_draw(cv,mtextp,tx,m_y);
    }
    else{
      libaroma_text_draw(cv,mtextp,tx,(my_h>>1) - (m_h>>1));
    }
    
    libaroma_text_free(mtextp);
  }
  
  /* draw extra text */
  if (etextp){
    if (m_h){
      libaroma_text_draw(cv,etextp,tx,e_y);
    }
    else{
      libaroma_text_draw(cv,etextp,tx,(my_h>>1) - (e_h>>1));
    }
    libaroma_text_free(etextp);
  }
  
  
  if (my_h!=mi->h){
    mi->h=my_h;
    libaroma_ctl_list_item_setheight(
      ctl, item, my_h
    );
  }
} /* End of _libaroma_listitem_option_draw */

/*
 * Function    : _libaroma_listitem_option_release_internal
 * Return Value: void
 * Descriptions: release internal data
 */
void _libaroma_listitem_option_release_internal(_LIBAROMA_LISTITEM_OPTIONP mi,
  byte flags){
  if (mi->main_text){
    free(mi->main_text);
  }
  if (mi->extra_text){
    free(mi->extra_text);
  }
  if (mi->icon){
    if ((flags&LIBAROMA_LISTITEM_OPTION_FREE_ICON)||
      (!(flags&LIBAROMA_LISTITEM_OPTION_SHARED_ICON))){
      libaroma_canvas_free(mi->icon);
    }
  }
  free(mi);
} /* End of _libaroma_listitem_option_release_internal */

/*
 * Function    : _libaroma_listitem_option_destroy
 * Return Value: void
 * Descriptions: destroy option item
 */
void _libaroma_listitem_option_destroy(
    LIBAROMA_CONTROLP ctl,
    LIBAROMA_CTL_LIST_ITEMP item){
  if (item->handler!=&_libaroma_listitem_option_handler){
    return;
  }
  _libaroma_listitem_option_release_internal(
    (_LIBAROMA_LISTITEM_OPTIONP) item->internal, item->flags
  );
} /* End of _libaroma_listitem_option_destroy */

/*
 * Function    : libaroma_listitem_option
 * Return Value: LIBAROMA_CTL_LIST_ITEMP
 * Descriptions: create option item
 */
LIBAROMA_CTL_LIST_ITEMP libaroma_listitem_option(
    LIBAROMA_CONTROLP ctl,
    int id,
    byte selected,
    const char * main_text,
    const char * extra_text,
    LIBAROMA_CANVASP icon,
    byte flags,
    int at_index){
  /* check valid list control */
  if (!libaroma_ctl_list_is_valid(ctl)){
    ALOGW("listitem_option control is not valid list control");
    return 0;
  }
  _LIBAROMA_LISTITEM_OPTIONP mi = (_LIBAROMA_LISTITEM_OPTIONP)
    malloc(sizeof(_LIBAROMA_LISTITEM_OPTION));
  if (!mi){
    ALOGW("listitem_option cannot allocate internal data");
    return NULL;
  }
  memset(mi,0,sizeof(_LIBAROMA_LISTITEM_OPTION));
  mi->selected=0;
  mi->icon=NULL;
  
  int vpad = (flags&LIBAROMA_LISTITEM_LARGE_PADDING)?16:8;
  int seph = (flags&LIBAROMA_LISTITEM_WITH_SEPARATOR)?1:0;

  /* init icon */
  int h = 0;
  if (icon){
    int dpsz=libaroma_dp((flags&LIBAROMA_LISTITEM_OPTION_SMALL_ICON)?24:38);
    if ((flags&LIBAROMA_LISTITEM_OPTION_FREE_ICON)||
      (!(flags&LIBAROMA_LISTITEM_OPTION_SHARED_ICON))){
      mi->icon=libaroma_canvas_ex(dpsz,dpsz,1);
      if (mi->icon){
        flags|=LIBAROMA_LISTITEM_OPTION_INDENT_NOICON;
        memset(mi->icon->alpha,0,mi->icon->s);
        libaroma_draw_scale_smooth(
          mi->icon, icon,
          0, 0, dpsz, dpsz,
          0, 0, icon->w, icon->h
        );
      }
      if (flags&LIBAROMA_LISTITEM_OPTION_FREE_ICON){
        libaroma_canvas_free(icon);
      }
    }
    else{
      mi->icon = icon;
    }
    h=dpsz;
  }
  mi->main_text=(main_text?strdup(main_text):NULL);
  mi->extra_text=(extra_text?strdup(extra_text):NULL);
  
  /* calculate height */
  int th = (mi->main_text?libaroma_font_size_px(4):0)*1.2;
  th += (mi->extra_text?libaroma_font_size_px(3):0)*1.2;
  th += libaroma_dp(4);
  h = MAX(h,th)+libaroma_dp(vpad*2+seph);
  mi->h=h;
  
  LIBAROMA_CTL_LIST_ITEMP item = libaroma_ctl_list_add_item_internal(
    ctl,
    id,
    h,
    LIBAROMA_CTL_LIST_ITEM_RECEIVE_TOUCH|flags,
    (voidp) mi,
    &_libaroma_listitem_option_handler,
    at_index
  );
  if (!item){
    ALOGW("listitem_option add_item_internal failed");
    _libaroma_listitem_option_release_internal(mi,flags);
  }
  return item;
} /* End of libaroma_listitem_option */


#endif /* __libaroma_listitem_option_c__ */