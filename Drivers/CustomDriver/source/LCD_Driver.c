#include "LCD_Driver.h"
#include "Delay.h"
#include "lcd_font.h"

#define LCD_WR_PIN			GPIO_Pin_14
#define LCD_WR_PORT			GPIOD
#define LCD_WR_APB2Periph	RCC_APB2Periph_GPIOD
#define LCD_CS_PIN      	GPIO_Pin_7
#define LCD_CS_PORT     	GPIOC
#define LCD_CS_APB2Periph	RCC_APB2Periph_GPIOC
#define LCD_RS_PIN      	GPIO_Pin_15
#define LCD_RS_PORT     	GPIOD
#define LCD_RS_APB2Periph	RCC_APB2Periph_GPIOD
#define LCD_RD_PIN      	GPIO_Pin_13
#define LCD_RD_PORT     	GPIOD
#define LCD_RD_APB2Periph	RCC_APB2Periph_GPIOD
#define LCD_DATA_PORT   	GPIOE
#define LCD_DATA_APB2Periph	RCC_APB2Periph_GPIOE

/* Control pin set */
#define lcd_set_cs()	GPIO_SetBits(LCD_CS_PORT, LCD_CS_PIN);
#define lcd_set_wr()	GPIO_SetBits(LCD_WR_PORT, LCD_WR_PIN);
#define lcd_set_rs()	GPIO_SetBits(LCD_RS_PORT, LCD_RS_PIN);
#define lcd_set_rd()	GPIO_SetBits(LCD_RD_PORT, LCD_RD_PIN);
#define lcd_reset_cs()	GPIO_ResetBits(LCD_CS_PORT, LCD_CS_PIN);
#define lcd_reset_wr()	GPIO_ResetBits(LCD_WR_PORT, LCD_WR_PIN);
#define lcd_reset_rs()	GPIO_ResetBits(LCD_RS_PORT, LCD_RS_PIN);
#define lcd_reset_rd()	GPIO_ResetBits(LCD_RD_PORT, LCD_RD_PIN);


static void      lcd_write_reg(uint16_t reg,uint16_t value);
static void      lcd_ram_prepare(void);
static void      lcd_write_ram(uint16_t value);
static uint16_t  lcd_bgr2rgb(uint16_t c);
static uint16_t  lcd_read_ram(void);
static void      lcd_data_as_input(void);
static void      lcd_data_as_output(void);
static uint16_t  lcd_read_sta(void);


/**
 *  LCD test
 */
void lcd_test(void)
{
	/* Output text */
	uint16_t i;
	uint8_t ch[] = "ABCDEFG";	
	uint8_t *str = ch;	
	for (i = 0; *(str + i); i++)
	{
		lcd_putchar_16x24((16*i), 24, *(str + i), RED, YELLOW);
	}
}

void LCD_Configure(void)
{
	lcd_set_cs();	
	lcd_set_wr();
	lcd_set_rd();
	lcd_set_rs();

	/* Enable internal OSC */
 	lcd_write_reg(0x0000,0x0001);		
 	delay_ms(50);						/* Wait osc is stable */

	/* 执行上电流程 */
	lcd_write_reg(0x0003,0xA8A4);		
	lcd_write_reg(0x000C,0x0000);        
	lcd_write_reg(0x000D,0x080C);        
	lcd_write_reg(0x000E,0x2B00);        
	lcd_write_reg(0x001E,0x00B0);  
	delay_ms(50);	  
	
    /* .13  0：RGB的值越小越亮；1：RGB的值越大越亮
     * .11  0：RGB；1: BGR
     * .9   0：319-->0；1: 0-->319
     * .14  0：719-->0；1: 0-->719
     * .0 ~.8:  设置最大行号 (0x13f = 319)
     */
	lcd_write_reg(0x0001,0x293F);		

	lcd_write_reg(0x0002,0x0600);     	/**< LCD Driving Waveform control */
	lcd_write_reg(0x0010,0x0000);     	/**< .0  0: 关闭睡眠模式; 1: 打开睡眠模式 */
	lcd_write_reg(0x0011,0x6070);				/**< .13-.14  11：16位RGB模式；10：18位RGB模式 */	
	lcd_write_reg(0x0016,0xEF1C);       /**< .15-.8   设置每行的像素数，0xef: 设为240 */
	lcd_write_reg(0x0017,0x0003);       /**< Vertical Porch */
	lcd_write_reg(0x0007,0x0233);		    /**< Display Control */  
	lcd_write_reg(0x000B,0x0000);       /**< Frame Cycle Control */  
	lcd_write_reg(0x000F,0x0000);				/**< Gate Scan Position */  
	lcd_write_reg(0x0041,0x0000);     	/**< Vertical Scroll Control */  
	lcd_write_reg(0x0042,0x0000);     	/**< Vertical Scroll Control */  
	lcd_write_reg(0x0048,0x0000);     	/**< Screen driving position */  
	lcd_write_reg(0x0049,0x013F);     	/**< Screen driving position */  
	lcd_write_reg(0x004A,0x0000);     	/**< Screen driving position */  
	lcd_write_reg(0x004B,0x0000);     	/**< Screen driving position */  
	lcd_write_reg(0x0044,0xEF00);     	/**< Horizontal RAM address position */  
	lcd_write_reg(0x0045,0x0000);    		/**< Horizontal RAM address position */   
	lcd_write_reg(0x0046,0x013F);     	/**< Horizontal RAM address position */  
	lcd_write_reg(0x0030,0x0707);     	/**< Gamma Control */  
	lcd_write_reg(0x0031,0x0204);     	/**< Gamma Control */  
	lcd_write_reg(0x0032,0x0204);     	/**< Gamma Control */  
	lcd_write_reg(0x0033,0x0502);     	/**< Gamma Control */  
	lcd_write_reg(0x0034,0x0507);     	/**< Gamma Control */  
	lcd_write_reg(0x0035,0x0204);     	/**< Gamma Control */  
	lcd_write_reg(0x0036,0x0204);    		/**< Gamma Control */   
	lcd_write_reg(0x0037,0x0502);   		/**< Gamma Control */    
	lcd_write_reg(0x003A,0x0302);   		/**< Gamma Control */    
	lcd_write_reg(0x003B,0x0302);   		/**< Gamma Control */    
	lcd_write_reg(0x0023,0x0000);    		/**< RAM write data mask */   
	lcd_write_reg(0x0024,0x0000);     	/**< RAM write data mask */  
	lcd_write_reg(0x0025,0x8000);     	/**< Frame Frequency Control; 0X8000: 对应屏幕响应频率为65Hz */  
	lcd_write_reg(0x004e,0);        		/**< 列(X)首地址设置 */  
	lcd_write_reg(0x004f,0);        		/**< 行(Y)首地址设置 */	
 	       	
}

void lcd_clr(uint16_t color)
{
	uint32_t index=0;
	lcd_set_cursor(0,0); 
	lcd_ram_prepare(); 	

	lcd_reset_cs();
	lcd_set_rs();

	for(index=0;index<76800;index++)			/** 320*240 = 76800 */
	{
		lcd_reset_wr();
		GPIO_Write(LCD_DATA_PORT, color);
		lcd_set_wr();		
	}
	lcd_set_cs();   
}	

/**
 *  @param [in]x 列
 *  @param [in]y 行
 */
void lcd_set_cursor(uint16_t x,uint16_t y)
{
 	lcd_write_reg(0x004e,y);		
	lcd_write_reg(0x004f,x);		
}

/**
 *  设置窗口
 *  @param [in]窗口起始点和结束点位置
 *  @return None
 */
void lcd_set_windows(uint16_t start_x,uint16_t start_y,uint16_t end_x,uint16_t end_y)
{
	lcd_set_cursor(start_x, start_y);
	lcd_write_reg(0x0050, start_x);
	lcd_write_reg(0x0052, start_y);
	lcd_write_reg(0x0051, end_x);
	lcd_write_reg(0x0053, end_y);
}

/**
 *  获取指定像素点的值
 *  @param [in]x 列
 *  @param [in]y 行
 *  @return 像素点处的值
 */
uint16_t lcd_get_point(uint16_t x,uint16_t y)
{
	lcd_set_cursor(x, y);
	return (lcd_bgr2rgb(lcd_read_ram()));
}

/**
 *  指定像素点赋值
 *  @param [in]x 列
 *  @param [in]y 行
 *  @param [in]value 像素点处的值
 *  @return None
 */
void lcd_set_point(uint16_t x, uint16_t y, uint16_t value)
{
	lcd_set_cursor(x, y);
	lcd_ram_prepare();
	lcd_write_ram(value);	
}

/**
 *  固定窗口显示图片
 *  源图为RGB888模式: R8 G8 B8 R8 G8 B8 R8 G8 B8...
 *  需转换为:         BGR565 BGR565 BGR565 ...	 
 *  @param [in]窗口起始点和结束点位置
 *  @param [in]*picture 图像指针
 *  @return None
 */
void lcd_draw_picture(uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint16_t *picture)
{
	uint16_t g_r, r_b, b_g, bgr565;	
	uint32_t length_div2;
	uint32_t i;
	uint16_t x, y;
	uint16_t *p_pic = picture;

	x = start_x;
	y = start_y;

	length_div2 = (end_x - start_x + 1) * (end_y - start_y + 1) / 2;  
	for (i = 0; i < length_div2; i++)
	{
		g_r = *p_pic++;
		r_b = *p_pic++;
		b_g = *p_pic++;

		bgr565 = ( ((g_r >> 3) & 0x001f) | 			       /**< 取r的高5位 */
		           ((g_r >> 5) & 0x07E0) | 			       /**< 取g的高6位 */
							 ((r_b << 8) & 0xF800)  );	   /**< 取b的高5位 */
		lcd_set_point(x, y, bgr565);

		y++;
		if (y > end_y)
		{
			x++;
			y = start_x;
		}
	
		bgr565 = ( ((r_b >> 11) & 0x001f) | 		   /**< 取r的高5位 */
		           ((b_g <<  3) & 0x07E0) | 		   /**< 取g的高6位 */
							 ( b_g        & 0xF800)  );			 /**< 取b的高5位 */
		lcd_set_point(x, y, bgr565);
	    
		y++;
		if (y > end_y)
		{
			x++;
			y = start_x;
		}
	}
}

/*
 *  Display 8x16 character
 */
void lcd_putchar_8x16(uint16_t x, uint16_t y, uint8_t ch, uint16_t ch_color, uint16_t bk_color)
{
	uint16_t i,j;	
	uint8_t temp = 0;		
	
	for(i = 0; i < 16; i++)
	{
		temp = ASCII_8x16[((ch - 0x20) * 16) + i];  /**< 取字形代码的1个字节,字库中省略了ASCII表中的前32个字符 */
		for(j = 0; j < 8; j++)
		{
			if((temp >> (7 - j)) & 0x01)							/**< 字符颜色 */
			{
				lcd_set_point(x + j, y + i, ch_color); 
			}
			else  if(bk_color != HYALINE)							/**< 背景颜色 */
			{				
				lcd_set_point(x + j, y + i, bk_color);  								
			}
		}
	}		
}

/*
 *  Display 16x24 character
 */
void lcd_putchar_16x24(uint16_t x, uint16_t y, uint8_t ch, uint16_t ch_color, uint16_t bk_color)
{
	uint16_t i,j;	
	uint16_t temp = 0;		
	
	for(i = 0; i < 24; i++)
	{
		temp = ASCII_16x24[((ch - 0x20) * 24) + i]; 
		for(j = 0; j < 16; j++)
		{
			if((temp >> j) & 0x01)
			{
				lcd_set_point(x + j, y + i, ch_color); 
			}
			else if(bk_color != HYALINE)			
			{				
				lcd_set_point(x + j, y + i, bk_color);  								
			}
		}
	}		
}

/*
 *  Write LCD register
 */
static void lcd_write_reg(uint16_t reg,uint16_t value)
{
	lcd_reset_cs();
	lcd_reset_rs();
	lcd_reset_wr();
	GPIO_Write(LCD_DATA_PORT, reg);
	lcd_set_wr();

	lcd_set_rs();
	lcd_reset_wr(); 
	GPIO_Write(LCD_DATA_PORT, value);
	lcd_set_wr();
	lcd_set_cs();
}

/*
 *  Prepare for RAM read and write
 */
static void lcd_ram_prepare(void)
{
	lcd_reset_cs();
	lcd_reset_rs();
	lcd_reset_wr();	 
	GPIO_Write(LCD_DATA_PORT, 0x22);
	lcd_set_wr();
	lcd_set_cs();
}

/*
 *  Write RAM
 */
static void lcd_write_ram(uint16_t value)
{
	lcd_reset_cs();
	lcd_set_rs();
	lcd_reset_wr();	 
	GPIO_Write(LCD_DATA_PORT, value);
	lcd_set_wr();
	lcd_set_cs();
}

/*
 *  Read RAM
 */
static uint16_t lcd_read_ram(void)
{
	uint16_t dummy;
	uint16_t value;
	lcd_ram_prepare();
	lcd_data_as_input();
	dummy = lcd_read_sta();
	dummy ++;
	value = lcd_read_sta();
	lcd_data_as_output();
	
	return value;
}

/**
 *  BBBBBGGGGGGRRRRR -> RRRRRGGGGGGBBBBB
 *  @param  BGR
 *  @return RGB
 */
static uint16_t lcd_bgr2rgb(uint16_t c)
{
	uint16_t  b, g, r, rgb;
	
	r = (c>>0)  & 0x1f;
	g = (c>>5)  & 0x3f;
	b = (c>>11) & 0x1f;
	
	rgb =  (r << 11) + (g << 5) + (b << 0);
	
	return( rgb );
}

static void lcd_data_as_input(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;  
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);
}

static void lcd_data_as_output(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;  
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(LCD_DATA_PORT, &GPIO_InitStructure);
}

/*
 *  读状态字
 *  @param  None
 *  @return 读出的状态字
 */
static uint16_t lcd_read_sta(void)
{
	uint16_t value;
	
	lcd_reset_cs();
	lcd_set_rs();
	lcd_reset_rd(); 
	lcd_set_rd();
	value = GPIO_ReadInputData(LCD_DATA_PORT); 	
	lcd_set_cs();

	return value;
}

/*
 ********************************************************************
 *       Driver for STemWin
 ********************************************************************
 */

/*
 *  Write LCD register
 */
void lcd_write_addr(uint16_t dat)
{
	lcd_reset_cs();
	lcd_reset_rs();
	lcd_reset_wr();
	GPIO_Write(LCD_DATA_PORT, dat);
	lcd_set_wr();
    lcd_set_cs();
}

/*
 *  Write LCD data
 */
void lcd_write_data(uint16_t dat)
{
    lcd_reset_cs();
	lcd_set_rs();
	lcd_reset_wr(); 
	GPIO_Write(LCD_DATA_PORT, dat);
	lcd_set_wr();
	lcd_set_cs();
}

void lcd_read_data_multiple(uint16_t *p_dat, uint16_t num_items)
{
    lcd_data_as_input();
    lcd_reset_cs();
	lcd_set_rs();
	lcd_reset_rd(); 
	lcd_set_rd();
    while(num_items--)
    {
        *p_dat++ = GPIO_ReadInputData(LCD_DATA_PORT); 
    }
	lcd_set_cs();
	lcd_data_as_output();
}
