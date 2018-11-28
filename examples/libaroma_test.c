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
      "file://examples/recovery/res/fonts/Roboto-Regular.ttf"
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


void scroll_label_test(void)
{
    LIBAROMA_WINDOWP win = libaroma_window(NULL, 0, 0, LIBAROMA_SIZE_FULL, LIBAROMA_SIZE_FULL);
    LIBAROMA_CONTROLP scr_label = libaroma_ctl_scroll_label_new(win,  1,
    20, 20, 200, 380, "test scroll", RGB(FFFFFF), 3, 0);

    LIBAROMA_CONTROLP btn5 = libaroma_ctl_button(
                                win, 2,
                                40, 480, 95, 60,
                                "Exit",
                                LIBAROMA_CTL_BUTTON_RAISED|LIBAROMA_CTL_BUTTON_COLORED,
                                RGB(ffcccc)
                                );

    libaroma_window_anishow(win, LIBAROMA_WINDOW_SHOW_ANIMATION_PAGE_LEFT, 400);
    byte onpool=1;
    byte cnt = 0, dnt = 0;
    char *ptr = NULL;
    char tmp[1024] = {0};
  do{
    LIBAROMA_MSG msg;
    dword command=libaroma_window_pool(win,&msg);
    byte cmd  = LIBAROMA_CMD(command);
    word id   = LIBAROMA_CMD_ID(command);
    /*
    byte param= LIBAROMA_CMD_PARAM(command);
    */
    printf("Window Command = (CMD: %i, ID: %i, Param: %i)\n",
        LIBAROMA_CMD(command),
        LIBAROMA_CMD_ID(command),
        LIBAROMA_CMD_PARAM(command));

    if (id==btn5->id){
        if (cmd==LIBAROMA_CMD_HOLD ){
            printf("Exit Button Pressed...\n");
            onpool = 0;
        }
        else if(cmd == LIBAROMA_CMD_CLICK)
        {
            char abc[12] = {'a', 'b','c', '\0'};
            libaroma_ctl_scroll_set_text(scr_label, abc);
        }      
    }

    libaroma_sleep(10);
    cnt++;
    if(cnt >= 2)
    {
        cnt = 0;
        dnt++;
        sprintf(tmp, " --%d--\n%s", dnt, "123\njjk\njlkjlkjl\njlkjlkjl");
        libaroma_ctl_scroll_set_text(scr_label, tmp);
    }

  }
  while(onpool);
  
  libaroma_window_free(win);

    return;
}

#endif /* __libaroma_libaroma_test_c__ */
