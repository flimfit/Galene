
const uint32_t TCLK = 1;
const uint32_t TIOA = 2;
const uint32_t TIOB = 4;

const int n_px = 512;
const int divider = 16;

int cycle_length, pulse_length;

// EOM output from Leica connected to TC2_0_TIOA : Digital Pin 5 : LINE TRIGGER  
// Connect TC2_1 to EOM to TC2_1: Digital Pin 3 : MUX OUT

void TC6_Handler()
{
  int state = REG_TC2_SR0;  
  int count = REG_TC2_RA0;

  cycle_length = count / n_px;
  pulse_length = count / n_px / divider;

  REG_TC2_RC1 = cycle_length;
  REG_TC2_RA1 = pulse_length;  
}


void setup() 
{
  pinMode(1, INPUT);  // Coupled to pin 5
  
  pmc_enable_periph_clk(ID_TC6);    
  pmc_enable_periph_clk(ID_TC7); 
  
  TC_Configure(TC2, 1,
    TC_CMR_TCCLKS_TIMER_CLOCK1 |
    TC_CMR_WAVE |
    TC_CMR_WAVSEL_UP_RC |
    TC_CMR_EEVTEDG_RISING |
    TC_CMR_EEVT_TIOB |
    TC_CMR_ACPA_CLEAR |
    TC_CMR_AEEVT_SET |
    TC_CMR_ACPC_SET |
    TC_CMR_ENETRG
  );
  TC_EnablePin(TC2, 1, TIOA | TIOB);
  TC_Start(TC2, 1);  

  REG_TC2_RC1 = 10000; //cycle_length;
  REG_TC2_RA1 = 1000; //pulse_length;  

  TC_Configure(TC2, 0,
    TC_CMR_TCCLKS_TIMER_CLOCK1 |
    TC_CMR_ETRGEDG_RISING | 
    TC_CMR_ABETRG |
    TC_CMR_LDRA_FALLING
  );

  TC_EnablePin(TC2, 0, TIOA);
  TC_EnableInterrupt(TC2, 0, TC_IER_LDRAS); // Enable interrupt on RA loading
  TC_Start(TC2, 0);  


}

void loop() {
}
