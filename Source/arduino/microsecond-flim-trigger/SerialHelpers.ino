
float ReadFloatFromSerialUSB(float* v, float min, float max)
{
   float input;
   if (SerialUSB.readBytes((char*) &input, 4) && input >= min && input <= max)
   {
      *v = input;
      return true;
   }
   return false;         
}

/*
   Send a message in the form 
   |  0  |  1  |  2  |  3  |  4  |
   | cmd |       parameter       |
*/
void SendMessage1(char message, uint32_t data)
{
   SerialUSB.write(message);
   SerialUSB.write((char*)&data, 4);
}

/*
   Send a data packet in the form 
   |  0  |  1  |  2  |  3  |  4  |  ......  |
   | cmd |       data size       |   data   |  
*/
void SendMessage(char message, char* data, uint32_t data_size)
{
   // Add bit to indicate data packet follows
   message = message | 0x80;

   SerialUSB.write(message);
   SerialUSB.write((char*)(&data_size), 4);
   SerialUSB.write(data, data_size);
}
