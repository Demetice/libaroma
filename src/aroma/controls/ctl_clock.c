#ifndef __libaroma_ctl_clock_c__
#define __libaroma_ctl_clock_c__
#include <aroma_internal.h>
#include "../ui/ui_internal.h"
#include "aroma/controls/ctl_clock.h"

/* HANDLER */
dword _libaroma_ctl_clock_msg(LIBAROMA_CONTROLP, LIBAROMA_MSGP);
void _libaroma_ctl_clock_draw (LIBAROMA_CONTROLP, LIBAROMA_CANVASP);
void _libaroma_ctl_clock_destroy(LIBAROMA_CONTROLP);
byte _libaroma_ctl_clock_thread(LIBAROMA_CONTROLP ctl);
static LIBAROMA_CONTROL_HANDLER _libaroma_ctl_clock_handler={
  message:_libaroma_ctl_clock_msg,
  draw:_libaroma_ctl_clock_draw,
  focus:NULL,
  destroy:_libaroma_ctl_clock_destroy,
  thread:_libaroma_ctl_clock_thread
};
  
/*
 * Structure   : __LIBAROMA_CTL_CLOCK
 * Typedef     : _LIBAROMA_CTL_CLOCK, * _LIBAROMA_CTL_CLOCKP
 * Descriptions: internal label data
 */
typedef struct __LIBAROMA_CTL_CLOCK _LIBAROMA_CTL_CLOCK;
typedef struct __LIBAROMA_CTL_CLOCK * _LIBAROMA_CTL_CLOCKP;
struct __LIBAROMA_CTL_CLOCK{
  LIBAROMA_CANVASP img_s;
  LIBAROMA_CANVASP img_m;
  LIBAROMA_CANVASP img_h;

  LIBAROMA_CANVASP img_ds;
  LIBAROMA_CANVASP img_dm;
  LIBAROMA_CANVASP img_dh;

  LIBAROMA_CANVASP img_bg;

  int sx;
  int sy;
  int mx;
  int my;
  int hx;
  int hy;

  int dsx;
  int dsy;
  int dmx;
  int dmy;
  int dhx;
  int dhy;

  int time_h;
  int time_m;
  int time_s;
 
  byte update;
  byte hidden;
  LIBAROMA_MUTEX mutex;
};

byte _libaroma_ctl_clock_thread(LIBAROMA_CONTROLP ctl) {
  _LIBAROMA_CTL_CHECK(
	  _libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
	);
  if (me->update){
    libaroma_mutex_lock(me->mutex);
  	me->update=0;
  	libaroma_mutex_unlock(me->mutex);
    return 1;
  }
  return 0;
}

void _libaroma_ctl_clock_destroy(LIBAROMA_CONTROLP ctl){
  _LIBAROMA_CTL_CHECK(
	  _libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 
	);
	libaroma_mutex_lock(me->mutex);
  
  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_h);
  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_m);
  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_s);

  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dh);
  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dm);
  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_ds);

  CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_bg);

  libaroma_mutex_unlock(me->mutex);
  libaroma_mutex_free(me->mutex);
  free(me);
}

static void _draw_hour(LIBAROMA_CANVASP c, _LIBAROMA_CTL_CLOCKP me)
{
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dh);
    me->img_dh = libaroma_image_rotate(me->img_h, me->time_h*30 + me->time_m * 0.5, 180 - me->hx, 180 - me->hy, &me->dhx, &me->dhy);
    libaroma_draw(c, me->img_dh, 180 - me->dhx, 180 - me->dhy, 0xff);
}

static void _draw_minute(LIBAROMA_CANVASP c, _LIBAROMA_CTL_CLOCKP me)
{
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dm);
    me->img_dm = libaroma_image_rotate(me->img_m, me->time_m*6, 180 - me->mx, 180 - me->my, &me->dmx, &me->dmy);
    libaroma_draw(c, me->img_dm, 180 - me->dmx, 180 - me->dmy, 0xff);
}

static void _draw_second(LIBAROMA_CANVASP c, _LIBAROMA_CTL_CLOCKP me)
{
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_ds);
    me->img_ds = libaroma_image_rotate(me->img_s, me->time_s*6, 180 - me->sx, 180 - me->sy, &me->dsx, &me->dsy);
    libaroma_draw(c, me->img_ds, 180 - me->dsx, 180 - me->dsy, 0xff);
}


void _libaroma_ctl_clock_draw(
    LIBAROMA_CONTROLP ctl, LIBAROMA_CANVASP c){
  _LIBAROMA_CTL_CHECK(
	  _libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 
	);
	
  if (me->hidden){
      return;
  }
  
  libaroma_mutex_lock(me->mutex);

  libaroma_draw(c, me->img_bg, 0, 0, 0xff);
  _draw_hour(c, me);
  _draw_minute(c, me);
  _draw_second(c, me);

  libaroma_mutex_unlock(me->mutex);
  
}

dword _libaroma_ctl_clock_msg(LIBAROMA_CONTROLP ctl, LIBAROMA_MSGP msg){
  /* internal check */
  _LIBAROMA_CTL_CHECK(
    _libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
  );
  
  switch(msg->msg){
    case LIBAROMA_MSG_WIN_ACTIVE:
    case LIBAROMA_MSG_WIN_INACTIVE:
    case LIBAROMA_MSG_WIN_RESIZE:
      {
        libaroma_mutex_lock(me->mutex);
        me->update=1;
        libaroma_mutex_unlock(me->mutex);
      }
      break;
  }
  return 0;
}

byte libaroma_ctl_clock_set_bg(LIBAROMA_CONTROLP ctl, LIBAROMA_CANVASP bg)
{
    _LIBAROMA_CTL_CHECK(_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0);

    if (bg == NULL)
    {
        return 0;
    }

    libaroma_mutex_lock(me->mutex);
    
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_bg);
    me->img_bg = libaroma_canvas_dup(bg);
    me->update=1;
    
    libaroma_mutex_unlock(me->mutex);
    
    return 1;
}

byte libaroma_ctl_clock_hidden(
	LIBAROMA_CONTROLP ctl,byte hidden){
  _LIBAROMA_CTL_CHECK(
		_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
	);
	libaroma_mutex_lock(me->mutex);
	me->hidden=hidden;
  me->update=1;
  libaroma_mutex_unlock(me->mutex);
	return 1;
}


LIBAROMA_CONTROLP libaroma_ctl_clock(LIBAROMA_WINDOWP win, 
        word id, int x, int y, int w, int h, LIBAROMA_CANVASP bg,
        LIBAROMA_CANVASP img_h, LIBAROMA_CANVASP img_m, LIBAROMA_CANVASP img_s,
        int hx, int hy,
        int mx, int my,
        int sx, int sy)
{
  _LIBAROMA_CTL_CLOCKP me = (_LIBAROMA_CTL_CLOCKP)calloc(sizeof(_LIBAROMA_CTL_CLOCK),1);
  if (!me){
    ALOGW("libaroma_ctl_clock alloc memory failed");
    return NULL;
  }

  me->img_bg = libaroma_canvas_dup(bg);
  me->img_h = libaroma_canvas_dup(img_h);
  me->img_m = libaroma_canvas_dup(img_m);
  me->img_s = libaroma_canvas_dup(img_s);

  me->hx = hx;
  me->mx = mx;
  me->sx = sx;
  me->hy = hy;
  me->my = my;
  me->sy = sy;

  me->update=1;
  libaroma_mutex_init(me->mutex);
  LIBAROMA_CONTROLP ctl =
    libaroma_control_new(
      id,
      x, y, w, h,
      libaroma_dp(10),libaroma_dp(10),
      me,
      &_libaroma_ctl_clock_handler,
      win
    );
  
  if (!ctl){
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_h);
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_m);
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_s);

    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dh);
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_dm);
    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_ds);

    CTL_CLK_JUDGE_AND_FREE_CANVAS(me->img_bg);

    libaroma_mutex_free(me->mutex);
    free(me);
  }
  return ctl;
}


byte libaroma_ctl_clock_set_second(
	LIBAROMA_CONTROLP ctl,byte value){
  _LIBAROMA_CTL_CHECK(
		_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
	);

	libaroma_mutex_lock(me->mutex);
	me->time_s=value;
    me->update=1;
    libaroma_mutex_unlock(me->mutex);

	return 1;
}

byte libaroma_ctl_clock_set_minute(
	LIBAROMA_CONTROLP ctl,byte value){
  _LIBAROMA_CTL_CHECK(
		_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
	);

	libaroma_mutex_lock(me->mutex);
	me->time_m=value;
    me->update=1;
    libaroma_mutex_unlock(me->mutex);
    
	return 1;
}

byte libaroma_ctl_clock_set_hour(
	LIBAROMA_CONTROLP ctl,byte value){
  _LIBAROMA_CTL_CHECK(
		_libaroma_ctl_clock_handler, _LIBAROMA_CTL_CLOCKP, 0
	);

	libaroma_mutex_lock(me->mutex);
	me->time_h=value;
    me->update=1;
    libaroma_mutex_unlock(me->mutex);
    
	return 1;
}

#endif /* __libaroma_CTL_CLOCK_c__ */
