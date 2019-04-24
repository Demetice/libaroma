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
 * Filename    : commondraw.c
 * Description : common drawing
 *
 * + This is part of libaroma, an embedded ui toolkit.
 * + 06/04/15 - Author(s): Ahmad Amarullah
 *
 */
#ifndef __libaroma_commondraw_c__
#define __libaroma_commondraw_c__
#include <aroma_internal.h>


/*
 * Function    : libaroma_draw_limit
 * Return Value: int
 * Descriptions: get limit position
 */
int libaroma_draw_limit(
    int x, int max) {
  if (x<0) {
    return 0;
  }  
  if (x>=max) {
    return max-1;
  }
  return x;
} /* End of libaroma_draw_limit */

/*
 * Function    : libaroma_draw_limited
 * Return Value: byte
 * Descriptions: is draw position limited/overflow
 */
byte libaroma_draw_limited(
    int x, int max) {
  return ((x < 0) || (x >= max) ? 1 : 0);
} /* End of libaroma_draw_limited */

/*
 * Function    : libaroma_draw_ex2
 * Return Value: byte
 * Descriptions: canvas drawing
 */
byte libaroma_draw_ex2(
  LIBAROMA_CANVASP dst,
  LIBAROMA_CANVASP src,
  int dx, int dy,
  int sx, int sy,
  int sw, int sh,
  byte draw_flags, 
  byte opacity,
  byte ismask,
  word maskcolor
) {
  if (src == NULL) {
    ALOGW("libaroma_draw_ex1 src = NULL");
    return 0;
  }
  if (dst == NULL) {
    dst = libaroma_fb()->canvas;
  }
  if ((dx >= dst->w) || (dy >= dst->h)) {
    ALOGW("libaroma_draw_ex1 dx/dy bigger that destination size");
    return 0;
  }
  if (opacity==0) {
    return 1; /* No Need Any Process */
  }
  
  byte useAlpha = (draw_flags&LIBAROMA_DRAW_WITH_ALPHA)?1:0;
  byte noDither = (draw_flags&LIBAROMA_DRAW_NODITHER)?1:0;
  byte toBlack  = (draw_flags&LIBAROMA_DRAW_TO_BLACK)?1:0;
  
  /* fix positions */
  if (sx < 0) {
    dx += abs(sx);
    sw -= abs(sx);
    sx = 0;
  }
  if (sy < 0) {
    dy += abs(sy);
    sh -= abs(sy);
    sy = 0;
  }
  
  /* fix size */
  if (sw + sx >= src->w) {
    sw -= (sw + sx) - src->w;
  }
  if (sh + sy >= src->h) {
    sh -= (sh + sy) - src->h;
  }
  if ((sw <= 0) || (sh <= 0)) {
    ALOGW("libaroma_draw_ex1 calculated sw/sh < 1");
    return 0;
  }
  
  /* set calculated units */
  int sr_w = sw;
  int sr_h = sh;
  int sr_x = sx;
  int sr_y = sy;
  int ds_x = dx;
  int ds_y = dy;
  
  /* fix destination */
  if (dx < 0) {
    int ndx = abs(dx);
    sr_x += abs(ndx);
    sr_w -= ndx;
    ds_x = 0;
  }
  if (dy < 0) {
    int ndy = abs(dy);
    sr_y += ndy;
    sr_h -= ndy;
    ds_y = 0;
  }
  /* fix source size */
  if (sr_w + dx > dst->w) {
    sr_w -= (sr_w + dx) - dst->w;
  }
  if (sr_h + dy > dst->h) {
    sr_h -= (sr_h + dy) - dst->h;
  }
  
  /* prepare loop data */
  int y;
  int pos_sr_x = sr_x * 2;
  int pos_ds_x = ds_x * 2;
  int pos_sc_w = src->l * 2;
  int pos_dc_w = dst->l * 2;
  int copy_sz  = sr_w * 2;
  byte * src_data   = ((byte *) src->data);
  byte * dst_data   = ((byte *) dst->data);
  if (useAlpha) {
    if (src->alpha == NULL) {
      useAlpha = 0;
    }
  }
  if (!useAlpha){
    ismask=0;
  }
  
  if (opacity == 0xff) {
    if (useAlpha) {
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
      for (y = 0; y < sr_h; y++) {
        wordp dst_mem = (wordp) (dst_data+((ds_y + y)*pos_dc_w)+pos_ds_x);
        if (ismask){
          libaroma_alpha_mono(
            sr_w, dst_mem, dst_mem, maskcolor, 
            (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
          );
        }
        else{
          wordp src_mem = (wordp) (src_data+((sr_y + y)*pos_sc_w)+pos_sr_x);
          if (noDither){
            libaroma_alpha_px(
              sr_w, dst_mem, dst_mem,
              src_mem, (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
            );
          }
          else{
            libaroma_alpha_px_line(
              y, sr_w, dst_mem, dst_mem,
              src_mem, (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
            );
          }
        }
      }
    }
    else {
      /* Copy Data Directly */
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
      for (y = 0; y < sr_h; y++) {
        memcpy(
          dst_data + ((ds_y + y)*pos_dc_w) + pos_ds_x,
          src_data + ((sr_y + y)*pos_sc_w) + pos_sr_x,
          copy_sz
        );
      }
    }
  }
  else {
    if (useAlpha) {
      /* Blend Destination with Source */
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
      for (y = 0; y < sr_h; y++) {
        wordp tmp_dst = (wordp) malloc(sr_w * 2);
        wordp dst_mem = (wordp) (dst_data + ((ds_y + y) * pos_dc_w) + pos_ds_x);
        if (ismask){
          libaroma_alpha_mono(
            sr_w, tmp_dst, dst_mem, maskcolor, 
            (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
          );
          libaroma_alpha_const(
            sr_w, dst_mem, dst_mem, tmp_dst, opacity
          );
        }
        else{
          wordp src_mem = (wordp) (src_data+((sr_y + y)*pos_sc_w)+pos_sr_x);
          if (toBlack){
            libaroma_alpha_px(
              sr_w, tmp_dst, dst_mem, src_mem,
              (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
            );
            libaroma_alpha_black(sr_w, dst_mem, tmp_dst, opacity);
          }
          else if (noDither){
            libaroma_alpha_px(
              sr_w, tmp_dst, dst_mem, src_mem,
              (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
            );
            libaroma_alpha_const(
              sr_w, dst_mem, dst_mem, tmp_dst, opacity
            );
          }
          else{
            libaroma_alpha_px_line(
              y, sr_w, tmp_dst, dst_mem, src_mem,
              (bytep) (src->alpha + ((sr_y + y) * src->l) + sr_x)
            );
            libaroma_alpha_const_line(
              y, sr_w, dst_mem, dst_mem, tmp_dst, opacity
            );
          }
        }
        free(tmp_dst);
      }
    }
    else {
      /* Blend Data Directly */
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
      for (y = 0; y < sr_h; y++) {
        wordp dst_mem = (wordp) (dst_data + ((ds_y + y) * pos_dc_w) + pos_ds_x);
        wordp src_mem = (wordp) (src_data + ((sr_y + y) * pos_sc_w) + pos_sr_x);
        if (toBlack){
          libaroma_alpha_black(sr_w, dst_mem, src_mem, opacity);
        }
        else if (noDither){
          libaroma_alpha_const(
            sr_w, dst_mem, dst_mem, src_mem, opacity
          );
        }
        else{
          libaroma_alpha_const_line(
            y, sr_w, dst_mem, dst_mem, src_mem, opacity
          );
        }
      }
    }
  }
  return 1;
} /* End of libaroma_draw_ex1 */

/*
 * Function    : libaroma_draw_rect
 * Return Value: byte
 * Descriptions: draw rectangle
 */
byte libaroma_draw_rect(
    LIBAROMA_CANVASP dst,
    int x, int y, int w, int h,
    word color, byte alpha) {
  if (dst == NULL) {
    dst = libaroma_fb()->canvas;
  }
  
  /* fix position */
  int x2 = x + w;
  int y2 = y + h;
  if (x2 > dst->w) {
    x2 = dst->w;
  }
  if (y2 > dst->h) {
    y2 = dst->h;
  }
  if (x < 0) {
    x = 0;
  }
  if (y < 0) {
    y = 0;
  }
  
  /* fixed size */
  w = x2 - x;
  h = y2 - y;
  
  /* draw */
  int dy;
  
  if (alpha == 0xff) {
    wordp datapos   = dst->data + x;
#ifdef libaroma_memset16
    for (dy = y; dy < y2; dy++) {
      wordp linepos = datapos + (dy * dst->l);
      libaroma_color_set(linepos,color,w);
    }
#else
    int w2=w*2;
    wordp firstline = datapos + (y * dst->l);
    libaroma_color_set(firstline, color, w);
#ifdef LIBAROMA_CONFIG_OPENMP
    #pragma omp parallel for
#endif
    for (dy = y+1; dy < y2; dy++) {
      wordp linepos = datapos + (dy * dst->l);
      memcpy(linepos,firstline,w2);
    }
#endif
  }
  else {
#ifdef LIBAROMA_CONFIG_OPENMP
    #pragma omp parallel for
#endif
    for (dy = y; dy < y2; dy++) {
      wordp linepos = dst->data + (dy * dst->l) + x;
#ifdef __engine_have_libaroma_alpha_rgba_fill
      libaroma_alpha_rgba_fill_line(dy, w, linepos, linepos, color, alpha);
#else
      libaroma_alpha_rgba_fill(w, linepos, linepos, color, alpha);
#endif
    }
  }
  return 1;
} /* End of libaroma_draw_rect */

/*
 * Function    : libaroma_draw_pixel
 * Return Value: byte
 * Descriptions: draw pixel
 */
byte libaroma_draw_pixel(
    LIBAROMA_CANVASP dest,
    int dx, int dy,
    word color,
    byte alpha
  ){
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  if ((dx<0)||(dy<0)||(dy>=dest->h)||(dx>=dest->w)){
    return 0;
  }
  wordp d=&dest->data[dest->l * dy + dx];
  if (alpha==0xff){
    *d = color;
  }
  else if (alpha>0){
    *d = libaroma_alpha(*d,color,alpha);
  }
  return 1;
} /* End of libaroma_draw_pixel */

/*
 * Function    : libaroma_draw_alphapixel
 * Return Value: byte
 * Descriptions: set alpha pixel
 */
byte libaroma_draw_alphapixel(
    LIBAROMA_CANVASP dest,
    int dx, int dy,
    byte alpha
  ){
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  if ((dx<0)||(dy<0)||(dy>=dest->h)||(dx>=dest->w)){
    return 0;
  }
  if (dest->alpha==NULL){
    return 0;
  }
  dest->alpha[dest->l * dy + dx] = alpha;
  return 1;
} /* End of libaroma_draw_pixel */

/*
 * Function    : libaroma_draw_line
 * Return Value: byte
 * Descriptions: draw line
 */
byte libaroma_draw_line(
  LIBAROMA_CANVASP dest,
  int x0, int y0, int x1, int y1,
  float wd,
  word color,
  byte alpha,
  byte is_mask){

#define __DRAW_PIX(x,y,a) \
  if (is_mask==1){ \
    if (!libaroma_draw_alphapixel( \
      dest, x, y, \
      MIN(alpha,MAX(0, alpha * (1-(a)))) \
    )) { break; } \
  } \
  else if (is_mask==2){ \
    if (!libaroma_draw_alphapixel( \
      dest, x, y, \
      MIN(0xff,MAX(0, 255 * (a))) \
    )) { break; } \
  } \
  else{ \
    if (!libaroma_draw_pixel( \
      dest, x, y, color, \
      MIN(0xff,MAX(0, alpha * (1-(a)))) \
    )) { break; } \
  }
  
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  
  int dx = abs(x1-x0), sx = x0 < x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0 < y1 ? 1 : -1;
  int err = dx-dy, e2, x2, y2;
  float ed = dx+dy == 0 ? 1 : sqrt((float)dx*dx+(float)dy*dy);
  for (wd = (wd+1)/2; ; ) {
    if ((x0>=0)&&(y0>=0)){
      __DRAW_PIX(x0,y0,
        abs(err-dx+dy)/ed-wd+1
      );
    }
    e2 = err; x2 = x0;
    if (2*e2 >= -dx) {
      for (e2 += dy, y2 = y0; e2 < ed*wd && (y1 != y2 || dx > dy); e2 += dx){
        if ((x0>=0)&&(y2>=0)){
          __DRAW_PIX(x0, y2+=sy,
            abs(e2)/ed-wd+1
          );
        }
      }
      if (x0==x1){
        break;
      }
      e2 = err; err -= dy; x0 += sx; 
    } 
    if (2*e2 <= dy){
      for (e2 = dx-e2; e2 < ed*wd && (x1 != x2 || dx < dy); e2 += dy){
        if ((x2>=0)&&(y0>=0)){
          __DRAW_PIX(x2 += sx, y0,
            abs(e2)/ed-wd+1
          );
        }
      }
      if (y0==y1){
        break;
      }
      err += dx; y0 += sy; 
    }
  }
#undef __DRAW_PIX
  return 1;
} /* End of libaroma_draw_line */

/*
 * Function    : libaroma_draw_subpixel
 * Return Value: byte
 * Descriptions: draw subpixel
 */
byte libaroma_draw_subpixel(
    LIBAROMA_CANVASP dest,
    float dx, float dy, float tickness,
    word color,
    byte alpha){
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  if ((dx<=-1)||(dy<=-1)||(dy>=dest->h)||(dx>=dest->w)){
    return 0;
  }
  int x, y;
  float px, py;
  float ht=(tickness-1.0)/2;
  for (y=floor(dy-ht);y<=ceil(dy+ht);y++){
    if ((y>=0)&&(y<dest->h)){
      int pos = y * dest->l;
      for (x=floor(dx-ht);x<=ceil(dx+ht);x++){
        if ((x>=0)&&(x<dest->w)){
          px = abs((dx<x)?dx-x:x-dx)/ht;
          py = abs((dy<y)?dy-y:y-dy)/ht;
          int alp = MIN(0xff,MAX((1-(px+py)) * 0xff,0));
          wordp d = dest->data + pos + x;
          word cl = libaroma_alpha(*d, color, alp);
          if (alpha!=0xff){
            cl=libaroma_alpha(*d,cl,alpha);
          }
          *d=cl;
        }
      }
    }
  }
  return 1;
} /* End of libaroma_draw_subpixel */

/*
 * Function    : libaroma_draw_mask_circle
 * Return Value: byte
 * Descriptions: draw masked circle
 */
byte libaroma_draw_mask_circle(
    LIBAROMA_CANVASP dst, 
    LIBAROMA_CANVASP src,
    int dx, int dy, 
    int sx, int sy,
    int sz,
    byte alpha){
  if (dst == NULL) {
    dst = libaroma_fb()->canvas;
  }
  if (src == NULL) {
    return 0;
  }
  if (sz<2){
    return 1;
  }
  int radius = sz/2;
  int rad2   = radius * radius;
  int y;
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
  for(y=-radius; y<=radius; y++){
    int pdy = dy + y;
    int psy = sy + y;
    if ((pdy<dst->h)&&(pdy>=0)&&(psy<src->h)&&(psy>=0)){
      int pos_d = pdy * dst->l;
      int pos_s = psy * src->l;
      int x   = sqrt(rad2-y*y);
      int w   = x*2;
      if (sx-x<0){
        w-=abs(sx-x);
        x=sx;
      }
      if (dx-x<0){
        w-=abs(dx-x);
        x=dx;
      }
      int pdx = dx-x;
      int sdx = sx-x;
      if (sdx+w>src->w){
        w=src->w-sdx;
      }
      if (pdx+w>dst->w){
        w=dst->w-pdx;
      }
      if (w>0){
        wordp dd = dst->data + pos_d + pdx;
        wordp sd = src->data + pos_s + sdx;
        if (alpha==0xff){
          memcpy(dd,sd,w*2);
        }
        else{
          //libaroma_alpha_const_line(pdy,w,dd,dd,sd,alpha);
          libaroma_alpha_const(w,dd,dd,sd,alpha);
        }
      }
    }
  }
  return 1;
} /* End of libaroma_draw_mask_circle */

/*
 * Function    : libaroma_draw_circle
 * Return Value: byte
 * Descriptions: draw filled circle
 */
byte libaroma_draw_circle(
    LIBAROMA_CANVASP dst, 
    word color,
    int dx, int dy,
    int sz,
    byte alpha){
  if (dst == NULL) {
    dst = libaroma_fb()->canvas;
  }
  if (sz<2){
    return 1;
  }
  int radius = sz/2;
  int rad2   = radius * radius;
  int y;
#ifdef LIBAROMA_CONFIG_OPENMP
  #pragma omp parallel for
#endif
  for(y=-radius; y<=radius; y++){
    int pdy = dy + y;
    if ((pdy<dst->h)&&(pdy>=0)){
      int pos_d = pdy * dst->l;
      int x   = sqrt(rad2-y*y);
      int w   = x*2;
      if (dx-x<0){
        w-=abs(dx-x);
        x=dx;
      }
      int pdx = dx-x;
      if (pdx+w>dst->w){
        w=dst->w-pdx;
      }
      if (w>0){
        wordp dd = dst->data + pos_d + pdx;
        if (alpha==0xff){
          libaroma_color_set(dd,color,w);
        }
        else{
#ifdef __engine_have_libaroma_alpha_rgba_fill
          libaroma_alpha_rgba_fill_line(pdy,w,dd, dd,color,alpha);
#else
          libaroma_alpha_rgba_fill(w,dd, dd,color,alpha);
#endif
        }
      }
    }
  }
  return 1;
} /* End of libaroma_draw_circle */

/*
 * Function    : libaroma_draw_line_width
 * Return Value: byte
 * Descriptions: draw line with width
 */
byte libaroma_draw_line_width(
  LIBAROMA_CANVASP dest,
  float x1, float y1, float x2, float y2,
  float wd,
  word color,
  byte alpha,
  byte is_mask,
  float aliasing){
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  if ((is_mask)&&(dest->alpha==NULL)){
    return 0;
  }
  if ((!is_mask)&&(alpha<1)){
    return 1;
  }
  
  float angle = atan2(y2 - y1, x2 - x1);
  float t2sina1 = wd / 2 * sin(angle);
  float t2cosa1 = wd / 2 * cos(angle);
  float t2sina2 = wd / 2 * sin(angle);
  float t2cosa2 = wd / 2 * cos(angle);
  LIBAROMA_PATHP path=libaroma_path(x1 + t2sina1, y1 - t2cosa1);
  libaroma_path_add(path, x2 + t2sina2, y2 - t2cosa2);
  libaroma_path_add(path, x2 - t2sina2, y2 + t2cosa2);
  libaroma_path_add(path, x2 - t2sina2, y2 + t2cosa2);
  libaroma_path_add(path, x1 - t2sina1, y1 + t2cosa1);
  libaroma_path_add(path, x1 + t2sina1, y1 - t2cosa1);
  byte res=libaroma_path_draw(
    dest,
    path,
    color,
    alpha,
    is_mask,
    aliasing);
  libaroma_path_free(path);
  return res;
} /* End of libaroma_draw_line_width */

/*
 * Function    : _libaroma_draw_arc_findpoint
 * Return Value: byte
 * Descriptions: find arc point
 */
byte _libaroma_draw_arc_findpoint(
    LIBAROMA_PATHP path,
    float dx, float dy,
    float radius_w, float radius_h,
    float xt0, float yt0,
    float xt1, float yt1,
    double start, double end
  ){
  double radian;
  if (start==end){
    return 0;
  }
  else if (start<end){
    radian = start + ((end - start) / 2.0);
  }
  else{
    radian = end + ((start - end) / 2.0);
  }
  float xt = dx + radius_w*cos(radian);
  float yt = dy + radius_h*sin(radian);
  if ((abs(xt-xt0)>=2)||(abs(yt-yt0)>=2)) {
    _libaroma_draw_arc_findpoint(
      path, dx, dy, radius_w, radius_h,
      xt0, yt0, xt, yt,
      start, radian
    );
  }
  libaroma_path_add(path, xt, yt);
  
  if ((abs(xt-xt1)>=2)||(abs(yt-yt1)>=2)) {
    _libaroma_draw_arc_findpoint(
      path, dx, dy, radius_w, radius_h,
      xt, yt, xt1, yt1,
      radian, end
    );
  }
  libaroma_path_add(path, xt1, yt1);
  return 1;
} /* End of _libaroma_draw_arc_findpoint */

/*
 * Function    : libaroma_draw_arc
 * Return Value: byte
 * Descriptions: draw arc into canvas
 */
byte libaroma_draw_arc(
  LIBAROMA_CANVASP dest,
  float dx, float dy,
  float radius_w, float radius_h,
  float width,
  float start_angle, float end_angle,
  word color,byte alpha,byte is_mask,float aliasing
){
  if (!dest){
    dest=libaroma_fb()->canvas;
  }
  if ((is_mask)&&(dest->alpha==NULL)){
    return 0;
  }
  if ((!is_mask)&&(alpha<1)){
    return 1;
  }
  if (start_angle==end_angle){
    /* no draw needed */
    return 1;
  }
  /*
  start_angle=fmod(start_angle,360);
  end_angle=fmod(end_angle,360);
  */
  /*
  start_angle=360-start_angle;
  end_angle=360-end_angle;
  */
  if (start_angle>end_angle){
    float tmp=start_angle;
    start_angle=end_angle;
    end_angle=tmp;
  }
  
  double start_radian = start_angle* __PI / 180.0;
  double end_radian   = end_angle  * __PI / 180.0;
  float start_x = dx + radius_w*cos(start_radian);
  float start_y = dy + radius_h*sin(start_radian);
  float end_x   = dx + radius_w*cos(end_radian);
  float end_y   = dy + radius_h*sin(end_radian);
  LIBAROMA_PATHP path=libaroma_path(start_x, start_y);
  _libaroma_draw_arc_findpoint(
      path, dx, dy, radius_w, radius_h,
      start_x, start_y, end_x, end_y,
      start_radian, end_radian
    );
  libaroma_path_add(path, end_x, end_y);
  
  if ((width>0)&&(width<radius_w/2)&&(width<radius_h/2)) {
    radius_w -= width;
    radius_h -= width;
    
    /* roll */
    start_x = dx + radius_w*cos(end_radian);
    start_y = dy + radius_h*sin(end_radian);
    end_x   = dx + radius_w*cos(start_radian);
    end_y   = dy + radius_h*sin(start_radian);
    libaroma_path_add(path, start_x, start_y);
    _libaroma_draw_arc_findpoint(
      path, dx, dy, radius_w, radius_h,
      start_x, start_y, end_x, end_y,
      end_radian, start_radian
    );
  }
  
  byte res=libaroma_path_draw(
    dest,
    path,
    color,
    alpha,
    is_mask,
    aliasing);
  libaroma_path_free(path);
  return res;
} /* End of libaroma_draw_arc */


#include <math.h>

#define MYPI 3.14159265358979323846f

#define TEMP_DESC(x) (1)
#if TEMP_DESC("图片旋转算法")

/******************************************************************************
** 图片旋转算法相关
******************************************************************************/
/* 开一字节对齐 */
#pragma pack(2)
typedef struct tagColor
{
    word r:5;
    word g:6;
    word b:5;
}_LIBAROMA_COLOR_S;
/* 关闭设定的对齐方式，会到默认的对齐方式 */
#pragma pack()

/*******************************************************************************
** 函数名称: _libaroma_img_rotate_get_four_min_and_detla
** 功能描述: 求四个值的最小值, 和delta值
** 输入参数:
** 输出参数: 
** 返 回 值: RTN_OK 成功，其它值请见错误码
** 作     者:  easonhuang@pacewear.com
** 日     期:  2018年2月23日
*******************************************************************************/
static void _libaroma_img_rotate_get_four_min_and_detla(float a, float b, float c, float d, int *pmin, int *pdelta)
{
    float temp[3] = {b, c, d };
    float max = a;
    float min = a;

    for (int i = 0; i < 3; i++)
    {
        if (max < temp[i])
        {
            max = temp[i];
        }

        if (min > temp[i])
        {
            min = temp[i];
        }
    }

    *pmin = floor(min);
    *pdelta = floor(max) - floor(min);
}

/*******************************************************************************
** 函数名称: _libaroma_img_rotate_get_postion
** 功能描述: 获取旋转后图片的边界点，方便后面申请图片
** 输入参数:
** 输出参数: 
** 返 回 值: RTN_OK 成功，其它值请见错误码
** 作     者:  easonhuang@pacewear.com
** 日     期:  2018年2月23日
*******************************************************************************/
static int _libaroma_img_rotate_get_postion
(
    float sinAngle, float cosAngle, 
    int rx, int ry, int w, int h, 
    int *minx, int *miny, int *dstw, int *dsth)
{
    float x1, x2, x3, x4;
    float y1, y2, y3, y4;

    x1 = (0 - rx) * cosAngle - (0 - ry)*sinAngle + rx;
    x2 = (w - rx) * cosAngle - (0 - ry)*sinAngle + rx;
    x3 = (w - rx) * cosAngle - (h - ry)*sinAngle + rx;
    x4 = (0 - rx) * cosAngle - (h - ry)*sinAngle + rx;

    y1 = (0 - rx)* sinAngle + (0 - ry)*cosAngle + ry;
    y2 = (w - rx)* sinAngle + (0 - ry)*cosAngle + ry;
    y3 = (w - rx)* sinAngle + (h - ry)*cosAngle + ry;
    y4 = (0 - rx)* sinAngle + (h - ry)*cosAngle + ry;

    _libaroma_img_rotate_get_four_min_and_detla(x1, x2, x3, x4, minx, dstw);
    _libaroma_img_rotate_get_four_min_and_detla(y1, y2, y3, y4, miny, dsth);

    return 0;
}

//判断一个点是否在图片中
byte PixelsIsInPic(const LIBAROMA_CANVASP pic, const int x, const int y)
{
    return ((x >= 0) && (x < pic->w) && (y >= 0) && (y < pic->h));
}

/*******************************************************************************
** 函数名称: bilinear_interpolation
** 功能描述:     双线性插值算法解决图片旋转后的锯齿问题 
** 输入参数:
** 输出参数: 
** 返 回 值: RTN_OK 成功，其它值请见错误码
** 作     者:  easonhuang@pacewear.com
** 日     期:  2018年2月09日
*******************************************************************************/
static void _bilinear_interpolation(float xSrc, float ySrc, LIBAROMA_CANVASP pSrc, wordp pDst, bytep alpha)
{
    _LIBAROMA_COLOR_S SrcColor[4];
    _LIBAROMA_COLOR_S *DstColor = (_LIBAROMA_COLOR_S *)pDst;  
    float tempR;
    float tempG;
    float tempB;
    float tempA;

    int xSmall = floor(xSrc);
    int xBig = ceil(xSrc);
    int ySmall = floor(ySrc);
    int yBig = ceil(ySrc);    

    byte a1 = 0, a2 = 0, a3 = 0, a4 = 0;

    memset(&SrcColor, 0, sizeof(SrcColor));
    if (xSmall >= 0 && ySmall >= 0) 
    {
        memcpy(&SrcColor[0], &pSrc->data[ySmall*pSrc->w + xSmall], sizeof(word)); 
        a1 = pSrc->alpha[ySmall*pSrc->w + xSmall];
    }
    
    if (xBig < pSrc->w && ySmall >= 0) 
    {
        memcpy(&SrcColor[1], &pSrc->data[ySmall*pSrc->w + xBig], sizeof(word)); 
        a2 = pSrc->alpha[ySmall*pSrc->w + xBig];
    }

    if (xSmall >= 0 && yBig < pSrc->h) 
    {
        memcpy(&SrcColor[2], &pSrc->data[yBig*pSrc->w + xSmall], sizeof(word)); 
        a3 = pSrc->alpha[yBig*pSrc->w + xSmall];
    }

    if (xBig < pSrc->w && yBig < pSrc->h) 
    {
        memcpy(&SrcColor[3], &pSrc->data[yBig*pSrc->w + xBig], sizeof(word)); 
        a4 = pSrc->alpha[yBig*pSrc->w + xBig];
    }

    float ux = xSrc - xSmall;
    float uy = ySrc - ySmall;

    tempR = (1 - ux)*(1 - uy)*SrcColor[0].r + (1 - ux)*uy*SrcColor[2].r 
            + (1 - uy)*ux*SrcColor[1].r + ux * uy*SrcColor[3].r;
    DstColor->r = floor(tempR);
    
    tempG = (1 - ux)*(1 - uy)*SrcColor[0].g + (1 - ux)*uy*SrcColor[2].g 
            + (1 - uy)*ux*SrcColor[1].g + ux * uy*SrcColor[3].g;
    DstColor->g = floor(tempG);

    tempB = (1 - ux)*(1 - uy)*SrcColor[0].b + (1 - ux)*uy*SrcColor[2].b
            + (1 - uy)*ux*SrcColor[1].b + ux * uy*SrcColor[3].b;
    DstColor->b = floor(tempB);

    tempA = (1 - ux)*(1 - uy)*a1 + (1 - ux)*uy*a3
            + (1 - uy)*ux*a2 + ux * uy*a4;
    *alpha = floor(tempA);
            
    return;
}

/*******************************************************************************
** 函数名称: libaroma_image_rotate
** 功能描述: 图片旋转算法，目前这个算法适用于扰图片中某一个点旋转的场景
** 输入参数: Src 输入的图像 
             value 需要旋转的角度，单位°, 取值范围0-360
             rx0 旋转点的x坐标， 图片的左上角(0,0)
             ry0 旋转点的Y坐标
             
** 输出参数: *dx 旋转后旋转点的X坐标
             *dy 旋转后旋转点的Y坐标
** 返 回 值: 失败返回NULL
             其它值 旋转后的图像资源，
             请注意，调用者用完后释放 
** 作     者:  easonhuang@pacewear.com
** 日     期:  2018年3月28日
*******************************************************************************/
LIBAROMA_CANVASP libaroma_image_rotate(const LIBAROMA_CANVASP Src, 
                            int value, int rx0, int ry0, int *dx, int *dy)
{
    float angle = MYPI * value * 0.0055555555555555556f; /* 本来是乘以π 除以180，除法换乘法 */
    float sinAngle = sin(angle);
    float cosAngle = cos(angle);
    LIBAROMA_CANVASP Dst = NULL;
    int minx, miny, dstw, dsth;

    if ((Src == NULL) || (dx == NULL) || (dy == NULL))
    {
        return NULL;
    }

    /* 获取旋转后图片的大小，和对应的位置 */
    _libaroma_img_rotate_get_postion(sinAngle, cosAngle, rx0, ry0, Src->w, Src->h, 
                    &minx, &miny, &dstw, &dsth);
                        
    *dx = rx0 - minx;
    *dy = ry0 - miny;

    Dst = libaroma_canvas_alpha(dstw, dsth);    
    if(Dst == NULL)
    {
        ALOGW("libaroma_image_rotate alloc memory failed");
        return NULL;
    }

    memset(Dst->alpha, 0, Dst->w * Dst->h * sizeof(byte));

    for (int y = 0; y < Dst->h; y++)
    {
        for (int x = 0; x < Dst->w; x++)
        {
            float srcx = (minx+x-rx0) * cosAngle + (miny+y-ry0) * sinAngle + rx0;
            float srcy = (miny+y-ry0) * cosAngle - (minx+x-rx0) * sinAngle + ry0;
                            
            if (PixelsIsInPic(Src, (int)srcx, (int)srcy))
            {  
                _bilinear_interpolation(srcx, srcy, Src, 
                    &Dst->data[y * Dst->w + x],
                    &Dst->alpha[y * Dst->w + x]);
            }
        }
    }

    return Dst;
}

#endif

#endif /* __libaroma_commondraw_c__ */

