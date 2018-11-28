/*
 * ctl_clock.h
 *
 *  Created on: 2018-1-10
 *      Author: hongyonghuang
 */

#ifndef __libaroma_aroma_h__
	#error "Include <aroma.h> instead."
#endif

#ifndef __CTL_SCROLL_LABEL_H__
#define __CTL_SCROLL_LABEL_H__


extern LIBAROMA_CONTROLP libaroma_ctl_scroll_label_new(
                                LIBAROMA_WINDOWP win, word id,
                                int x, int y, int w, int h,
                                char *txt, word color, byte size, byte fontid);
extern void libaroma_ctl_scroll_set_text(LIBAROMA_CONTROLP ctl, char *txt);
extern void libaroma_ctl_scroll_label_set_hidden(LIBAROMA_CONTROLP ctl, byte hidden);




#endif /* INCLUDE_AROMA_CONTROLS_CTL_CLOCK_H_ */
