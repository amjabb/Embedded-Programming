//LAB1 GPIO
 class lab1:public scheduler_task {
  public: lab1(): scheduler_task("lab1",2000,PRIORITY_LOW){};
  bool init(void){
   LPC_PINCON->PINSEL3 &= ~(3<<6); //GPIO port 1.19
   LPC_PINCON->PINSEL3 &= ~(3<<8); //GPIO port 1.20
   LPC_PINCON->PINSEL2 &= ~(3<<0); //GPIO port 1.0 mapped to LED0
   LPC_PINCON->PINSEL2 &= ~(3<<18); //GPIO port 1.9 mapped to switch0
   LPC_GPIO1->FIODIR |= (1<<19); //Off board LED output
   LPC_GPIO1->FIODIR &= ~(1<<20); //Off board switch input
   LPC_GPIO1->FIODIR |= (1<<0); //On board LED output
   LPC_GPIO1->FIODIR &= ~(1<<9); //On board switch input
   return true;
  }
  bool run(void * p){
   if(LPC_GPIO1->FIOPIN & (1<<20))
   {
    LPC_GPIO1->FIOPIN |= (1<<19);
   }
   else
   {
    LPC_GPIO1->FIOPIN &= ~(1<<19);
   }
   if(LPC_GPIO1->FIOPIN & (1<<9))
   {
    LPC_GPIO1->FIOPIN &= ~(1<<0); //LED is active low so this is will turn the LED on
   }
   else
   {
    LPC_GPIO1->FIOPIN |= (1<<0);
   }
   return true;
  }
 };
 
int main(void)
{
 scheduler_add_task(new lab1());
return 1;
}
