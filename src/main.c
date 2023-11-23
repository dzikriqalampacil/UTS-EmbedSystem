/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
 /**
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <stdio.h>

void setup_timer(void);
void print_message(void);
void emit1(void);
void emit2(void);
void listen1(void);
void listen2(void);
void increase(void);

int ovf_cnt = 0;
int distance1 = 0;
int distance2 = 0;

int pingDistance = 1;
int defaultDistance = 13;
int timeStart = 0;
int timeEnd = 0;
int timeCounter = 0;

float speed = 0;
int statusSpeed = 0;

static char buffarray1[200];
static char buffarray2[200];
static char buffarraySpeed[200];
static char buffarrayStatus[200];

//Fungsi setup timer
void setup_timer(void){
 tc_enable(&TCC0);
 tc_set_overflow_interrupt_callback(&TCC0,increase);
 tc_set_wgm(&TCC0, TC_WG_NORMAL);
 tc_write_period(&TCC0, 58);
 tc_set_overflow_interrupt_level(&TCC0, TC_INT_LVL_HI);
 tc_write_clock_source(&TCC0, TC_CLKSEL_DIV1_gc); 
}

void toggleBuzzer() {
	PORTE.DIR = 0b1111111;
	//ioport_set_pin_high(J4_PIN0);
	PORTE.OUT = 0b0000000;
	PORTE.OUT = 0b1111111;
	delay_ms(500);
	PORTE.OUT = 0b0000000;
}

void toggleLED() {
	gpio_toggle_pin(LED2);
}

void toggleWarning() {
	toggleLED();
	toggleBuzzer();
	toggleLED();
}

void emit1(void) {
   PORTB.DIR = 0b11111111; //Set output
   PORTB.OUT = 0b00000000; //Set low
   PORTB.OUT = 0b11111111; //Set high selama 5us
   delay_us(5);
   PORTB.OUT = 0b00000000; //Kembali menjadi low
   PORTB.DIR = 0b00000000; //Set menjadi input
   delay_us(750); //Delay holdoff selama 750us
}

void emit2(void) {
 PORTC.DIR = 0b11111111; //Set output
 PORTC.OUT = 0b00000000; //Set low
 PORTC.OUT = 0b11111111; //Set high selama 5us
 delay_us(5);
 PORTC.OUT = 0b00000000; //Kembali menjadi low
 PORTC.DIR = 0b00000000; //Set menjadi input
 delay_us(750); //Delay holdoff selama 750us
}

void listen1(void) {
 cpu_irq_enable(); //Mulai interrupt
 while(PORTB.IN & PIN0_bm){
  //Tidak ada apa-apa di sini. Loop ini berfungsi untuk mendeteksi pin 0 PORT B yang berubah menjadi low
 }
 cpu_irq_disable(); //Interrupt dimatikan
}

void listen2(void) {
 cpu_irq_enable(); //Mulai interrupt
 while(PORTC.IN & PIN0_bm){
  //Tidak ada apa-apa di sini. Loop ini berfungsi untuk mendeteksi pin 0 PORT B yang berubah menjadi low
 }
 cpu_irq_disable(); //Interrupt dimatikan
}

void increase(void){
 ovf_cnt++;
}

int main (void)
{
 // Insert system clock initialization code here (sysclk_init()).
 board_init();
 sysclk_init();
 pmic_init();
 gfx_mono_init();
 
 gpio_set_pin_high(NHD_C12832A1Z_BACKLIGHT);

 // Workaround for known issue: Enable RTC32 sysclk
 sysclk_enable_module(SYSCLK_PORT_GEN, SYSCLK_RTC);
 
 while (RTC32.SYNCCTRL & RTC32_SYNCBUSY_bm) {
  // Wait for RTC32 sysclk to become stable
 }
 
 delay_ms(1000);
 setup_timer();
 
 // Insert application code here, after the board has been initialized.
 while(1){
  emit1();
  
  ovf_cnt = TCC0.CNT = 0;
  
  listen1();
  
  distance1 = ovf_cnt;
  
  emit2();
  
  ovf_cnt = TCC0.CNT = 0;
  
  listen2();
  
  distance2 = ovf_cnt;
  
  snprintf(buffarray1, sizeof(buffarray1), "Panjang: %d cm  %d cm", distance1, distance2);
  gfx_mono_draw_string(buffarray1, 0, 0, &sysfont);
  
  if (distance1 != defaultDistance) {
   timeStart = timeCounter;
  }
  if (distance2 != defaultDistance) {
   timeEnd = timeCounter;
  }
  if (timeStart != 0 && timeEnd != 0) {
   speed = 0.6/(timeEnd - timeStart);
   if (speed > 0.2) {
    statusSpeed = 1;
	toggleWarning();
   } else {
    statusSpeed = 0;
   }
   timeStart = 0;
   timeEnd = 0;
  }
  
  snprintf(buffarray2, sizeof(buffarray2), "Timer: %d s  %d s", timeStart, timeEnd);
  gfx_mono_draw_string(buffarray2, 0, 8, &sysfont);
  snprintf(buffarraySpeed, sizeof(buffarraySpeed), "Speed: %.2f m/s  ", speed);
  gfx_mono_draw_string(buffarraySpeed, 0, 16, &sysfont);
  if (statusSpeed) {
	snprintf(buffarrayStatus, sizeof(buffarrayStatus), "Status: Warning ");  
  } else {
	snprintf(buffarrayStatus, sizeof(buffarrayStatus), "Status: Safe ");  
  }
  gfx_mono_draw_string(buffarrayStatus, 0, 24, &sysfont);
  
  timeCounter = timeCounter + 1;
  
 }
}

/*
*Keterangan: Kecepatan suara yang digunakan adalah 1/29 cm/us.
*Hasil sedikit tidak akurat, kemungkinan karena salah pengimplementasian rumus. Mohon dikoreksi jika menemukan kesalahan pada kode ini
*/
