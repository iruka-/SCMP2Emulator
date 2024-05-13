/************************************************************************************
 *
 ************************************************************************************
 */
#ifndef	led_h_
#define	led_h_

#define	SCREEN_X2	1

#if	SCREEN_X2
#define	SCREEN_W	1000
#define	SCREEN_H	512
//	LED配電盤の表示座標.
#define	LED_X  (512+16)
#define	LED_Y  16
#define	LED_W  (512-16-16-24)
#define	LED_H  180

//	トグルSWの表示Y座標.
#define	SW_Y   200
#else

#define	SCREEN_W	768
#define	SCREEN_H	320
//	LED配電盤の表示座標.
#define	LED_X  (256+16)
#define	LED_Y  16
#define	LED_W  (512-16-16)
#define	LED_H  180

//	トグルSWの表示Y座標.
#define	SW_Y   200
#endif

void	VRAM_output(int adrs,int data);
void	LED_output(int acc , int ea);
void	gr_putnum(int x,int y,int num);
void	led16draw(int x,int y,int w,int h,int data,int color,int cnt,char *name);
void	SW_draw(int x,int y,int w,int h,int data,int color,int cnt,char *name);
int		SW_hitchk(int i,int mx,int my);
void	SW_flip(int x,int y);
void	LED_speed_change();
void	LED_trace_change();
int		LED_draw(int pc,int draw);
void	gr_keydown_callback(int vk_key);
void	gr_mouse_callback(int x,int y);

#ifdef	_LINUX_
#define VK_UP	38
#define VK_SPACE	32
#endif

#endif
