#******************************************************************************
#
# Makefile - Rules for building the driver library.
#
# Copyright (c) 2005-2017 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
# 
#   Redistribution and use in source and binary forms, with or without
#   modification, are permitted provided that the following conditions
#   are met:
# 
#   Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
# 
#   Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the  
#   distribution.
# 
#   Neither the name of Texas Instruments Incorporated nor the names of
#   its contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# This is part of revision 2.1.4.178 of the Tiva Peripheral Driver Library.
#
#******************************************************************************

#
# The base directory for TivaWare.
#

#
# Include the common make definitions.
#
include makedefs

CFLAGS += -DTARGET_IS_TM4C123_RB2 -DPART_TM4C123GH6PM
LDSCRIPT = tm4c.ld
#
# Where to find header files that do not live in the source directory.
#

.PHONY: all clean clean_incl
#
# The default rule, which causes the driver library to be built.
#
all: blinky

blinky: blinky.o tm4c_uart.o tm4c_startup.o tm4c_miscs.o tm4c_dma.o \
		tm4c_qei.o tm4c_gpio.o ssi_display.o tm4c_ssi.o \
		blinking.o qei_position.o timer_task.o
	$(LD) $(LDFLAGS) $^ -o $@
#
# The rule to clean out all the build products.
#
clean:
	rm -rf blinky blinky.bin *.o
clean_incl: clean
	rm -f *.d
#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include $(wildcard *.d) __dummy__
endif
