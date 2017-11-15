/*
 * -------------------------------------------------------------
 * FLIM trigger multiplexing unit code
 * 
 * Sean Warren
 * -------------------------------------------------------------
 * 
 * Multiplex three LV-TTL trigger signals onto one output trigger where the 
 * trigger ID is encoded onto the width of the output signal
 * 
 * Supports a maximum trigger rate of ~2.6MHz (390ns minimum spacing)
 * 
 * Pin mapping:
 * 
 *   - Trigger 1 : Pixel : Pin 0 : PIO controller A
 *   - Trigger 2 : Line  : Pin 5 : PIO controller C
 *   - Trigger 3 : Frame : Pin 2 : PIO controller B
 *   - Output    : Mux   : Pin 3 :
 *   
 * Output pulse widths:  
 *   - Pixel => 25.0 ns
 *   - Line  => 36.5 ns
 *   - Frame => 48.0 ns
 * 
 * Requires an Arduino Due
 * To use this file, you need to comment out: 
 *   - PIOA_Handler
 *   - PIOB_Handler
 *   - PIOC_Handler
 * in file:
 *   - ~/Library/Arduino15/packages/arduino/hardware/sam/1.6.5/cores/arduino/WInterrupts.c 
 *   
 * Pin mappings from : https://www.arduino.cc/en/Hacking/PinMappingSAM3X  
 *   - Pin 0 : PIOA8 
 *   - Pin 5 : PIOC25  
 *   - Pin 2 : PIOB25  
 *   - Pin 3 : PIOC28  
 *   
 * The pins are set up so each trigger is on a different PIO controller so   
 * we can use a different handler for each and avoid the extra cycles checking
 * the interrupt register.   
 * 
 */
 
#define SET_OUTPUT_PIN_HIGH REG_PIOC_SODR = 1<<28
#define SET_OUTPUT_PIN_LOW  REG_PIOC_CODR = 1<<28



volatile unsigned long isr1;  

// Pixel
void PIOA_Handler()
{
   // 25 ns pulse on pin 3
   isr1 = REG_PIOA_ISR; 
   SET_OUTPUT_PIN_HIGH;
   __asm__("nop\n\t"); 
   SET_OUTPUT_PIN_LOW;
}

// Line
void PIOC_Handler()
{
   isr1 = REG_PIOC_ISR; 
   // 168 or 192 ns pulse on pin 3      // old - 36 ns pulse on pin 3
   SET_OUTPUT_PIN_HIGH;
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   SET_OUTPUT_PIN_LOW;
}

// Frame
void PIOB_Handler()
{
   // 48 ns pulse on pin 3
   isr1 = REG_PIOB_ISR; 
   
   SET_OUTPUT_PIN_HIGH;
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   __asm__("nop\n\t"); 
   SET_OUTPUT_PIN_LOW;
  
}


void dummy() {}


uint32_t pwmPin = 8;
uint32_t maxDutyCount = 4;
uint32_t clkAFreq = 42000000ul;
uint32_t pwmFreq = 10500000ul; 


void setup() 
{
  // Setup pin I/O
  pinMode(0, INPUT);  // Pixel
  pinMode(1, INPUT);  // Coupled to pin 5
  pinMode(10, INPUT); // Coupled to pin 5
  pinMode(5, INPUT);  // Line
  pinMode(2, INPUT);  // Frame
  pinMode(3, OUTPUT); // Output
  

  // Set interrupts
  attachInterrupt(0, dummy, FALLING); // Line end 
  attachInterrupt(5, dummy, RISING); // Line begin
  attachInterrupt(2, dummy, RISING); // Frame

 /*
  // Setup a 5MHz signal on pin 8 for testing
  pmc_enable_periph_clk(PWM_INTERFACE_ID);
  PWMC_ConfigureClocks(clkAFreq, 0, VARIANT_MCK);
 
  PIO_Configure(
    g_APinDescription[pwmPin].pPort,
    g_APinDescription[pwmPin].ulPinType,
    g_APinDescription[pwmPin].ulPin,
    g_APinDescription[pwmPin].ulPinConfiguration);
 
  uint32_t channel = g_APinDescription[pwmPin].ulPWMChannel;
  PWMC_ConfigureChannel(PWM_INTERFACE, channel , pwmFreq, 0, 0);
  PWMC_SetPeriod(PWM_INTERFACE, channel, maxDutyCount);
  PWMC_EnableChannel(PWM_INTERFACE, channel);
  PWMC_SetDutyCycle(PWM_INTERFACE, channel, 1);
 */
//  pmc_mck_set_prescaler(1);
}


void loop() 
{
  // We don't need to do anything here
}
