typedef struct{
 int portNumber;
 int pinNumber;
}semaphoreSignals_t;
semaphoreSignals_t* signals;
void* semphrSignals = signals;
SemaphoreHandle_t globalButtonOneSemaphore = semphrSignals; //Declare global semaphore
QueueHandle_t  Global_Queue_Handler = 0;
//This function uses function pointers to call another function and passes the port and pin numbers.
void interrupt_enable(uint8_t port_num, uint8_t pin_num, void (*func)(int,int))
{
 u0_dbg_printf("Interrupt Enabled\n");
 func(port_num,pin_num);
}
/*
 * This function enters the port number and pin number it recieves as parameters,
 * to a struct pointed to by the semaphore. The port and pin number are then
 * differentiated by the semaphore. The semaphore then becomes set by the give
 * function.
 */
void button_one_isr(int portNumber, int pinNumber)
{
 (*(semaphoreSignals_t* )globalButtonOneSemaphore).portNumber = portNumber;
 (*(semaphoreSignals_t* )globalButtonOneSemaphore).pinNumber = pinNumber;
 xSemaphoreGiveFromISR(globalButtonOneSemaphore, NULL); //returns wake_task true if giving this semaphore allowed a task to become unblocked
}
/*This function overrides EINT3_IRQHandler in the IVT vector
 * once the interrupt is triggered this function will be called first
 * This goes through all the status registers which are four in total for this architecture
 * and finds the pins and ports that the interrupts are coming from. This function
 * then disables the interrupts through the clear register and proceeds to a call the interrupt_enable function.
 */
void EINT3_IRQHandler(void){
  static int portNumber;
  static int pinNumber;
  static int fallingRising;
     bool pinFound = false;
  u0_dbg_printf("\n**************************************\n");
  //Go through all the status register and see which pin the interrupt is on
  u0_dbg_printf("Inside Handler\n");
  for(int i = 0; !pinFound && i < 31; i++){
   portNumber = 0;
   pinNumber = i;
   fallingRising = 0;
   if(LPC_GPIOINT->IO0IntStatR & (1 << i)){
    pinFound = true;
    break;
   }
  }
  for(int i = 0; !pinFound && i < 31; i++){
   portNumber = 0;
   pinNumber = i;
   fallingRising = 1;
   if(LPC_GPIOINT->IO0IntStatF & (1 << i)){
    pinFound = true;
    break;
   }
  }
  for(int i = 0; !pinFound && i < 14; i++){
   portNumber = 2;
   pinNumber = i;
   fallingRising = 0;
   if(LPC_GPIOINT->IO2IntStatR & (1 << i)){
    pinFound = true;
    break;
   }
  }
  for(int i = 0; !pinFound && i < 14; i++){
   portNumber = 2;
   pinNumber = i;
   fallingRising = 0;
   if(LPC_GPIOINT->IO2IntStatF & (1 << i)){
    pinFound = true;
    break;
   }
  }
  if(!pinFound){
   u0_dbg_printf("Pin not found!\n");
  }
  interrupt_enable(portNumber,pinNumber,button_one_isr);
  if(portNumber == 0){
   LPC_GPIOINT->IO0IntClr |= (1 << pinNumber);
  }
  if(portNumber == 2){
   LPC_GPIOINT->IO2IntClr |= (1 << pinNumber);
  }
}
/*
 * This task receives the number of pins used, the port number
 * , pin number and rising or falling edge through a queue passed from main.
 * Then based on the port and pin this function initializes the GPIO pins.
 * This also sets the interrupt 3 "flag" through NVIC_EnableIRQ by setting
 * the 21st bit of the ISER register. Then the isr_register overrides the entry
 * in the vector table to the function EINT3_IRQHandler.
 *
 * In run, if the semaphore is released then the port and pin numbers will be
 * retreieved from the semaphore and the seven segment LED will be set to the
 * pin number and the on board LEDs will be set to the corresponding port numbers.
 * The semaphore is taken once more to ensure that any issues with switch debouncing
 * will be relieved.
 * After a delay the LED is turned off.
 */
class lab4:public scheduler_task {
public: lab4(): scheduler_task("lab4",2000,PRIORITY_LOW){};
bool init(void){
 u0_dbg_printf("Inside Init\n");
 static int numberOfPins;
 static int portNumber;
 static int pinNumber;
 static int fallingRising;
 static int i;
 xQueueReceive(Global_Queue_Handler, &numberOfPins, 1000);
 while(i < numberOfPins){
  xQueueReceive(Global_Queue_Handler, &portNumber, 1000);
  xQueueReceive(Global_Queue_Handler, &pinNumber, 1000);
  xQueueReceive(Global_Queue_Handler, &fallingRising, 1000);
  if(portNumber == 0){
   LPC_GPIO0->FIODIR &= ~(1 << pinNumber); //Off board switch input
   fallingRising ? LPC_GPIOINT->IO0IntEnF |= (1 << pinNumber) : LPC_GPIOINT->IO0IntEnR |= (1 << pinNumber);
   if(pinNumber <= 15){
    LPC_PINCON->PINSEL0 &= ~(3 << pinNumber*2);
   }
   if(pinNumber > 15){
    LPC_PINCON->PINSEL1 &= ~(3 << (pinNumber-16)*2);
   }
  }
  if(portNumber == 2){
   LPC_GPIO1->FIODIR &= ~(1 << pinNumber); //Off board switch input
   fallingRising ? LPC_GPIOINT->IO2IntEnF |= (1 << pinNumber) : LPC_GPIOINT->IO2IntEnR |= (1 << pinNumber);
   LPC_PINCON->PINSEL4 &= ~(3 << pinNumber*2);
  }
  u0_dbg_printf("\nPin Number %i", pinNumber);
  ++i;
 }
 NVIC_EnableIRQ(EINT3_IRQn);
 isr_register(EINT3_IRQn, EINT3_IRQHandler);
 return true;
}

bool run(void * p){
 static int pinNum;
 static int portNum;
 if (xSemaphoreTake(globalButtonOneSemaphore, portMAX_DELAY)) { //Wait for the semaphore
  pinNum = (*(semaphoreSignals_t* )globalButtonOneSemaphore).pinNumber;
  portNum = (*(semaphoreSignals_t* )globalButtonOneSemaphore).portNumber;
  LD.setNumber(pinNum);
  u0_dbg_printf("The pin is %i", (*(semaphoreSignals_t* )globalButtonOneSemaphore).pinNumber );
  u0_dbg_printf("\nTake semaphore\n");
  if(portNum == 0){
   LE.on(1); //turn on, on-board LED
  }
  if(portNum == 2){
   LE.on(2); //turn on, on-board LED
  }
 }
 vTaskDelay(500);
 u0_dbg_printf("**************************************\n");
 if(xSemaphoreTake(globalButtonOneSemaphore,0)){
  u0_dbg_printf("Successfully debounced");
 }
 LE.off(1);
 return true;
}
};
 
int main(void)
{
 globalButtonOneSemaphore = xSemaphoreCreateBinary();
 int numberOfPins = 0;
 int portNumber = 0;
 int pinNumber = 0;
 int fallingRising = 0;
 Global_Queue_Handler = xQueueCreate(30, sizeof(int));
 printf("Welcome to FREERTOS Interrupts\n");
 printf("How many pins are you using: ");
  scanf("%i", &numberOfPins);
 xQueueSend(Global_Queue_Handler, &numberOfPins, 1000);
 for(int i = 0; i < numberOfPins; i++){
  printf("\nEnter the number of port # %i : ", i+1);
   scanf("%i", &portNumber);
  printf("\nEnter the number of pin # %i : ", i+1);
   scanf("%i", &pinNumber);
  printf("\nEnter 0 for Rising Edge Interrupts or 1 for Falling Edge Interrupts for pin # %i: ", i+1);
   scanf("%i", &fallingRising);
  xQueueSend(Global_Queue_Handler, &portNumber, 1000);
  xQueueSend(Global_Queue_Handler, &pinNumber, 1000);
  xQueueSend(Global_Queue_Handler, &fallingRising, 1000);
 }

 scheduler_add_task(new lab4());
