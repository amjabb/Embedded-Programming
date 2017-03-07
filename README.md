
# Embedded-Programming
*******NOTE: THIS CODE IS NOT COMPLETE********

This code is developed on top of existing FREERTOS shell and Eclipse variant provided at http://www.socialledge.com/sjsu/index.php?title=Main_Page

Only the relevant code that I developed is listed here.

Device used to build: LPC1758 which uses ARM CORTEX M3.

Check out the external interrupt driver, I used semaphore's to get around having multiple callback functions in the ISR.
