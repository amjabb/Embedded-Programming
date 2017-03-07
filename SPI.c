class lab2:public scheduler_task {
    public: lab2(): scheduler_task("lab2",2000,PRIORITY_LOW)
    {
    };
    bool init(void){
        LPC_SC->PCONP |= (1<<10);
        LPC_SC->PCLKSEL0 &= ~(3 << 20);
        LPC_SC->PCLKSEL0 |= (1<<20);
        //Why are we selecting the peripheral clock = clock here and
        //dividing it in the CPSR register when we could just divide it here
        LPC_PINCON->PINSEL0 &= ~((3 << 14) | (3 << 16) | (3 << 18));
        LPC_PINCON->PINSEL0 |= ((2 << 14) | (2 << 16) | (2 << 18));
        LPC_SSP1->CR0 = 7; // 8 bit data transfer
        LPC_SSP1->CR1 = 2; //This isnt really selecting master?
        LPC_SSP1->CPSR = 8; //PCLKSEL0/8 must be even value
        LPC_PINCON->PINSEL0 &= ~(3 << 12);
        LPC_GPIO0->FIODIR |= (1 << 6); //Direction of Pin 0.6 as output
        return true;
    }
    void enableChipSelect(){
        LPC_GPIO0->FIOPIN &= ~(1 << 6);
        //Why are we not selecting the SSEL1 which is wired to the FLASH CS
    }
    void disableChipSelect(){
        LPC_GPIO0->FIOPIN |= (1 << 6);
    }
    char ssp1_ExchangeByte (char out){
        LPC_SSP1->DR = out;
        while(LPC_SSP1->SR & (1 << 4));
        return LPC_SSP1->DR;
    }
    bool run(void * p){
        printf("Transmitting:\n");
        char* info[] = {"Manufacturer ID","Device ID Byte 1", "Device ID Byte 2",
                        "EDI String Length", "EDI Data Byte 1", "Status Register Byte 1", "Status Register Byte 2"};
        char* status_info[]={"Page Size","Sector Protection Status","Density Code","Density Code","Density Code", "Density Code",
                            "Compare Result","Ready Busy Status","Erase Suspend","Program Suspend Status(Buffer 1)",
                            "Program Suspend Status(Buffer 2)","Sector Lockdown Enabled","Reserved for Future Use",
                            "Erase/Program Error","Reserved for Future Use","Ready/Busy Status"};
        char op_code = 0x9F;
        char received[2] = {0};
        enableChipSelect();
        ssp1_ExchangeByte(op_code);
        for(int i = 0; i < 5 ; i++){
            received[0] = ssp1_ExchangeByte(0xFF);
            printf("%s: %x\n",info[i],received[0]);
        }
        disableChipSelect();
        enableChipSelect();
        ssp1_ExchangeByte(0xD7);
            received[0] = ssp1_ExchangeByte(0xFF);
            received[1] = ssp1_ExchangeByte(0xFF);
            printf("%s: %x\n",info[5],received[0]);
            printf("%s: %x\n",info[6],received[1]);
        for(int i = 0; i < 8; i++)
        {
            int binary = (received[0] & (1 << i)) ? 1 : 0;
            printf("%s: %i\n", status_info[i],binary);
        }
        for(int i = 0; i < 8; i++)
        {
            int binary = (received[1] & (1 << i)) ? 1 : 0;
            printf("%s: %i\n", status_info[i+8],binary);
        }
        disableChipSelect();
        printf("Recieved!\n\n");
        vTaskDelay(4000);
        return true;
    }
};
int main(void){
scheduler_add_task(new lab2());
return 1;
}
