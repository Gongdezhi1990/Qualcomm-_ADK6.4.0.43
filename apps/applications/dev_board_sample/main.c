/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016 - 2018

@file

    TEMPLATE for a sample application.

    The application is set up for use on the development
    board, using the LCD and buttons.

    Presses on any button update the LCD.
*/

#include <stdio.h>
#include <vmtypes.h>
#include <os.h>
#include <panic.h>
#include <operator.h>

#include <dev_board_lcd.h>
#include <dev_board_buttons.h>

/* Local function to handle button presses */
static void button_handler(uint8 button_id);


/*!
 * \brief The application entry point.
 *
 * All applications should follow the basics of initialising the
 * operating system and finishing by calling MessageLoop(), which
 * never returns.
 *
 * As the initialisation of this demo is straightforward, it is
 * embedded in the function rather than being a separate, init,
 * function.
 */
void main(void)
{
    OsInit();

    /* permit audio to stream */
    OperatorFrameworkEnable(1);

    dev_board_lcd_initialise();

    /* Write an initial message to LCD */
    dev_board_lcd_write(0, "QTIL DevBoard");
    dev_board_lcd_write(1, "Initalising....");

    /* TEMPLATE:
     * Insert application initialisation here
     */

    /* Now display the application name */
    dev_board_lcd_write(1, "DevBrd Sample");

    /* start handling buttons */
    dev_board_initialise_buttons(button_handler);

    MessageLoop();
}

/*!
 * \brief handler called when a button/switch on the
 * development button has been pressed.
 *
 * This can be extended to change the handling of buttons.
 *
 * \param button_id The button pressed (starts at 1)
 */
static void button_handler(uint8 button_id)
{
    char buf[30];

    sprintf(buf,"Pressed SW%d      ",button_id);
    dev_board_lcd_write(1, buf);

}

/* Define pmalloc pools. This definition will be merged at run time with the
   base definitions from Hydra OS - see 'pmalloc_config_P1.h'. */

#include <pmalloc.h>

_Pragma ("unitsuppress Unused")

_Pragma ("datasection apppool")

static const pmalloc_pool_config app_pools[] =
{
    {   4, 14 },
    {   8, 25 },
    {  12, 17 },
    {  16,  7 },
    {  20, 22 },
    {  24, 15 },
    {  28, 55 },
    {  32, 21 },
    {  36, 19 },
    {  40, 10 },
    {  56,  9 },
    {  64,  7 },
    {  80,  9 },
    { 120, 16 },
    { 140,  4 },
    { 180,  6 },
    { 220,  3 },
    { 288,  1 }, 
    { 512,  1 },
    { 692,  3 }    
};
