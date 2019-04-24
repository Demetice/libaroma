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
 * Filename    : libaroma_test.c
 * Description : libaroma test file
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 04/02/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_libaroma_test_c__
#define __libaroma_libaroma_test_c__

/* libaroma header */
#include <aroma.h>
#include <aroma/graph/canvas.h>

/* include test modules */
#include "mods/bar_test.c"
#include "mods/tab_test.c"
#include "mods/common_test.c"
void scroll_label_test(void);

/*
 * Function    : init_libaroma
 * Return Value: void
 * Descriptions: init libaroma
 */
void init_libaroma(){
  /* set libaroma runtime configuration
    snprintf(libaroma_config()->fb_shm_name,64,"recovery-mainfb");
    libaroma_config()->runtime_monitor = LIBAROMA_START_UNSAFE;
  */
  
  /*snprintf(libaroma_config()->fb_shm_name,64,"");*/
  libaroma_config()->fb_shm_name[0]=0;
  libaroma_start();
  
  /* clean display */
  libaroma_canvas_blank(libaroma_fb()->canvas);
  libaroma_sync();
  
  /* load font - id=0 */
  libaroma_font(0,
    libaroma_stream(
      "file:///home/xiaolin/Documents/libaroma-master/examples/recovery/res/fonts/Roboto-Regular.ttf"
    )
  );
} /* End of init_libaroma */

/*
 * Function    : main
 * Return Value: int
 * Descriptions: main executable function
 */
int main(int argc, char **argv){
  /* For recovery Apps:
    pid_t pp = getppid();
    kill(pp, 19);
  */
  /*libaroma_config()->runtime_monitor = LIBAROMA_START_MUTEPARENT;*/
  
  init_libaroma();
  scroll_label_test();
  //tab_test();
  
  //bar_test();
  
  /* start common test */
  //common_test();
  
  /* end libaroma process */
  libaroma_end();
  /* For recovery apps:
    kill(pp, 18);
  */
  return 0;
} /* End of main */

extern LIBAROMA_CONTROLP libaroma_ctl_clock(LIBAROMA_WINDOWP win, 
        word id, int x, int y, int w, int h, LIBAROMA_CANVASP bg,
        LIBAROMA_CANVASP img_h, LIBAROMA_CANVASP img_m, LIBAROMA_CANVASP img_s,
        int hx, int hy,
        int mx, int my,
        int sx, int sy);

extern byte libaroma_ctl_clock_set_second(LIBAROMA_CONTROLP ctl,byte value);
extern byte libaroma_ctl_clock_set_minute(LIBAROMA_CONTROLP ctl,byte value);
extern byte libaroma_ctl_clock_set_hour(LIBAROMA_CONTROLP ctl,byte value);

void scroll_label_test(void)
{
    LIBAROMA_WINDOWP win = libaroma_window(NULL, 0, 0, LIBAROMA_SIZE_FULL, LIBAROMA_SIZE_FULL);

    LIBAROMA_CONTROLP btn5 = libaroma_ctl_button(
                                win, 2,
                                40, 480, 95, 60,
                                "Exit",
                                LIBAROMA_CTL_BUTTON_RAISED|LIBAROMA_CTL_BUTTON_COLORED,
                                RGB(ffcccc)
                                );

    LIBAROMA_CANVASP clock_bg = libaroma_image_uri("file:///home/xiaolin/Documents/libaroma-master/examples/recovery/res/pngs/th4_bg.png");
    LIBAROMA_CANVASP hour = libaroma_image_uri("file:///home/xiaolin/Documents/libaroma-master/examples/recovery/res/pngs/th4_hour.png");
    LIBAROMA_CANVASP minute = libaroma_image_uri("file:///home/xiaolin/Documents/libaroma-master/examples/recovery/res/pngs/th4_min.png");
    LIBAROMA_CANVASP second = libaroma_image_uri("file:///home/xiaolin/Documents/libaroma-master/examples/recovery/res/pngs/th4_sec.png");

    LIBAROMA_CONTROLP clock = libaroma_ctl_clock(win, 10, 0, 0, 360, 360, clock_bg, hour, minute, second,
                                                175, 80, 175, 42, 176, 34);

    libaroma_canvas_free(clock_bg);

    libaroma_window_anishow(win, LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_LEFT, 400);
    byte onpool=1;
    byte cnt = 0, dnt = 0;
    char *ptr = NULL;
    char tmp[1024] = {0};
    int time_cnt = 0;
    int time_sec = 0;
    int time_min = 0;
    int time_hour = 0;
  do{

    libaroma_sleep(10);
    time_cnt++;
    if (time_cnt >= 30)
    {
      time_cnt = 0;
      time_sec++;
      if (time_sec >= 60)
      {
        time_sec = 0;
        time_min++;
        if (time_min>=60)
        {
          time_min = 0;
          time_hour++;
          if (time_hour>=12)time_hour = 0;
          libaroma_ctl_clock_set_hour(clock, time_hour);
        }
        libaroma_ctl_clock_set_minute(clock, time_min);
      }
      libaroma_ctl_clock_set_second(clock, time_sec);
    }

    //if (time_hour == 10)onpool = 0;
  }
  while(onpool);
  
  libaroma_window_free(win);

    return;
}

#endif /* __libaroma_libaroma_test_c__ */
