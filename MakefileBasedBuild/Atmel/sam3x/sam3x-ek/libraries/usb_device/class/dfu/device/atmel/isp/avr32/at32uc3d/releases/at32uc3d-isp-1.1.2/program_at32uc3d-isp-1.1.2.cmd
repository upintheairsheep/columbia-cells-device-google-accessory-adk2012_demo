@echo off
setlocal enableextensions enabledelayedexpansion

: This command script programs the ISP (flash array), the ISP
: configuration word (User page) and the general-purpose fuse bits.

: Copyright (C) 2006-2008, Atmel Corporation All rights reserved.
:
: Redistribution and use in source and binary forms, with or without
: modification, are permitted provided that the following conditions are met:
:
: 1. Redistributions of source code must retain the above copyright notice, this
: list of conditions and the following disclaimer.
:
: 2. Redistributions in binary form must reproduce the above copyright notice,
: this list of conditions and the following disclaimer in the documentation and/
: or other materials provided with the distribution.
:
: 3. The name of ATMEL may not be used to endorse or promote products derived
: from this software without specific prior written permission.
:
: THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
: WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
: MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
: SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
: INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
: BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
: DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
: OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
: NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
: EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


echo.
echo Performing a JTAG Chip Erase command.
avr32program -p AVRONE chiperase -F
if ERRORLEVEL 1 (
	color 47
	pause
	exit
)

echo.
echo Programming MCU memory from `at32uc3d-isp-1.1.2.hex'.
avr32-objcopy -I ihex -O binary at32uc3d-isp-1.1.2.hex at32uc3d-isp-1.1.2.bin
avr32program -p AVRONE program -finternal@0x80000000 -cint -e -v -O0x80000000 -Fbin at32uc3d-isp-1.1.2.bin
if ERRORLEVEL 1 (
	color 47
	pause
	exit
)
del at32uc3d-isp-1.1.2.bin

echo.
echo Programming ISP configuration word \(default for UC3D_EK i.e. Word2 == 0x929E0D6B and Word1 == 0xE11EFFD7\)
avr32program -p AVRONE program -finternal@0x80000000 -cint -e -v -O0x808000F8 -Fbin at32uc3d-isp_cfg-1.1.2.dat
if ERRORLEVEL 1 (
	color 47
	pause
	exit
)

echo.
echo Programming general-purpose fuse bits.
avr32program -p AVRONE writefuses -finternal@0x80000000 gp=0xFFF5FFFF
if ERRORLEVEL 1 (
	color 47
	pause
	exit
)

echo.
echo Resetting MCU.
avr32program -p AVRONE run -R
if ERRORLEVEL 1 (
	color 47
	pause
	exit
)

color 27
pause
