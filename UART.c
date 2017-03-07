class lab3:public scheduler_task {
public: lab3(): scheduler_task("lab3",2000,PRIORITY_LOW){};
bool init(void){
 LPC_SC->PCONP |= (1 << 24); //Power on PCONP register
 LPC_SC->PCLKSEL1 &= ~(3 << 16); //Clear
 LPC_SC->PCLKSEL1 |= (1 << 16); //Set to CPU clock PCLKSEL1 is CPUCLK/1
 LPC_PINCON->PINSEL4 &= ~(15 << 16); //Clear RX2 and TX2
 LPC_PINCON->PINSEL4 |= (10 << 16); //Enable RX2 and TX2
 LPC_UART2->LCR = (1 << 7); //DLAB must be set in order to change the baud rate by setting this equal to 7 instead of ORing so that the rest of the bits are intentionally cleared.
 LPC_UART2->FCR = (1 << 1); //Enable the UART RX and TX FIFO
 int baud = sys_get_cpu_clock() / ((16 * 38400)); //sys_get_cpu_clock();
 LPC_UART2->DLL = (baud) & 0xFF; //LSb 8 bits of the baud rate 
 LPC_UART2->DLM = (baud >> 8) & 0xFF; //MSb 8 bits of the baud rate
 LPC_UART2->LCR = 3; // number of bits that we are handling to 8 this also disables the DLAB by clearing the rest of the bits.
 return true;
}
char uart2_putchar(char out)
{
 LPC_UART2->THR = out; //Transmit Holding Register
 while(! (LPC_UART2->LSR & (1<<5))); //See Notes
 return 1;
}
char uart2_getchar()
{
 char in;
 while(!(LPC_UART2->LSR &  (1 << 0))); //See Notes part 4
 in = LPC_UART2->RBR; // RBR - Receive buffer register
 return in;
}
bool run(void * p){
 static int i = 0; //to iterate through the string
 static int j = 1; //Trying to send a string doesn't work as expected :(
 char receiving = 0;
 static char letter[] = "Z";
 if (j == 0){
  printf("Enter the string you would like to send: ");
  scanf("%s\n",letter);
  j++;
 }
 //if(i < strlen(letter)){ //Only send as many as you have.
  uart2_putchar(letter[0]);
 //}

 receiving = uart2_getchar();
 //if(receiving != '.'){ //Stop at period
  printf("%c", receiving);
 //}
 vTaskDelay(500);
 return true;
}
};
 
int main(void)
{
scheduler_add_task(new lab3());
}
