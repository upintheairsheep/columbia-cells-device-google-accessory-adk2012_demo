/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/**
 *  \file
 *  Implement simple system tick usage.
 */

/*----------------------------------------------------------------------------
 *         Headers
 *----------------------------------------------------------------------------*/

#include "board.h"

#define CLOCKPERSECOND (1000u)

/*----------------------------------------------------------------------------
 *         Local variables
 *----------------------------------------------------------------------------*/

/** Tick Counter united by ms */
static volatile uint32_t _dwTickCount=0 ;
static volatile uint32_t _dwCurrentSeconds = 0;
static uint32_t _dwSecondCountdown = CLOCKPERSECOND;

/*----------------------------------------------------------------------------
 *         Exported Functions
 *----------------------------------------------------------------------------*/

/**
 *  \brief Handler for Sytem Tick interrupt.
 */
extern void TimeTick_Increment( void )
{
    _dwTickCount++ ;
    
    if (--_dwSecondCountdown == 0) 
    {
      _dwCurrentSeconds++;
      _dwSecondCountdown = CLOCKPERSECOND;
    }
}

/**
 *  \brief Configures the SAM3 SysTick & reset tickCount.
 *  Systick interrupt handler will generates 1ms interrupt and increase a
 *  tickCount.
 *  \param new_mck  Current master clock.
 */
extern uint32_t TimeTick_Configure( uint32_t new_mck )
{
    _dwTickCount = 0 ;
    return SysTick_Config( new_mck/1000 ) ;
}

/**
 *  \brief Get current Tick Count, in ms.
 */
extern uint32_t GetTickCount( void )
{
    return _dwTickCount ;
}

/**
 *  \brief Get current Second Count, in s.
 */
extern uint32_t GetSecondCount( void )
{
    return _dwCurrentSeconds ;
}


/**
 *  \brief Sync Wait for several ms
 */
extern void Wait( volatile uint32_t dwMs )
{
    uint32_t dwStart ;
    uint32_t dwCurrent ;

    dwStart = _dwTickCount ;
    do
    {
        dwCurrent = _dwTickCount ;
    } while ( dwCurrent - dwStart < dwMs ) ;
}

/**
 *  \brief Sync Sleep for several ms
 */
extern void Sleep( volatile uint32_t dwMs )
{
    uint32_t dwStart ;
    uint32_t dwCurrent ;

    dwStart = _dwTickCount ;

    do
    {
        dwCurrent = _dwTickCount ;

        if ( dwCurrent - dwStart > dwMs )
        {
            break ;
        }

        __WFI() ;
    } while( 1 ) ;
}

