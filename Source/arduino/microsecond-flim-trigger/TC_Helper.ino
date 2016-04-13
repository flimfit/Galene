

IRQn_Type GetInterruptIdx(Tc *pTc, uint32_t dwChannel)
{ 
   IRQn_Type interrupt;
   if (pTc == TC0)
   {
      switch(dwChannel)
      {
      case 0:
         return TC0_IRQn;
      case 1:
         return TC1_IRQn;
      case 2: 
         return TC2_IRQn;
      }  
   }
   else if (pTc == TC1)
      switch(dwChannel)
      {
      case 0:
         return TC3_IRQn;
      case 1:
         return TC4_IRQn;
      case 2: 
         return TC5_IRQn;
      }  
   else // pTc == TC2
      switch(dwChannel)
      {
      case 0:
         return TC6_IRQn;
      case 1:
         return TC7_IRQn;
      case 2: 
         return TC8_IRQn;
      }  
   return TC0_IRQn;
}

void TC_EnableInterrupt(Tc *pTc, uint32_t dwChannel, uint32_t dwMode)
{
   pTc->TC_CHANNEL[dwChannel].TC_IER = dwMode;
   NVIC_EnableIRQ(GetInterruptIdx(pTc, dwChannel));
}

void TC_DisableInterrupt(Tc *pTc, uint32_t dwChannel, uint32_t dwMode)
{
   pTc->TC_CHANNEL[dwChannel].TC_IDR = dwMode;
   NVIC_DisableIRQ(GetInterruptIdx(pTc, dwChannel));
}


void TC_EnablePin(Tc *tc, uint32_t ch, uint32_t pin)
{
   if (tc == TC0)
   {
      if (ch == 0)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOB, PIO_PERIPH_B, PIO_PB26);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOB, PIO_PERIPH_B, PIO_PB25);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOB, PIO_PERIPH_B, PIO_PB27);         
      }
      if (ch == 1)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PA4);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PA2);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PA3);         
      }
      if (ch == 2)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PA7);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PB5);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PB6);         
      }  
   }
   else if (tc == TC1)
   {
      // MOST OF THESE PINS AREN'T CONNECTED!
      if (ch == 0)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PB22);
      }
      if (ch == 1)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOA, PIO_PERIPH_A, PIO_PA23);
      }
   }
   else if (tc == TC2)
   {
      if (ch == 0)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC27);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC25);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC26);         
      }
      if (ch == 1)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC30);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC28);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOC, PIO_PERIPH_B, PIO_PC29);         
      }
      if (ch == 2)
      {
         if (pin & TCLK)
            PIO_SetPeripheral(PIOD, PIO_PERIPH_B, PIO_PD9);
         if (pin & TIOA)
            PIO_SetPeripheral(PIOD, PIO_PERIPH_B, PIO_PD7);
         if (pin & TIOB)
            PIO_SetPeripheral(PIOD, PIO_PERIPH_B, PIO_PD8);         
      }       
   }
}
