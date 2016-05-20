#
# VantageVue Connect - A small program to connect a vantage vue console
# Copyright (C) 2015  BRUNEAUX Jerome <jbruneaux@laposte.net>
#
# This file is part of VantageVueConnect.
#
# VantageVueConnect is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# any later version.
#
# VantageVueConnect is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with VantageVueConnect.  If not, see <http://www.gnu.org/licenses/>.
#

TOOLCHAIN_PATH=/opt/OpenWrt-Toolchain-ar71xx-generic_gcc-4.8-linaro_uClibc-0.9.33.2.Linux-i686/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin/
TOOLCHAIN_PREFIX=mips-openwrt-linux-uclibc

IDIR=.

CC=${TOOLCHAIN_PATH}/${TOOLCHAIN_PREFIX}-gcc
#CC=gcc
CFLAGS=-I$(IDIR) -Wall -DLOG_USES_SYSLOG

ODIR=obj
LDIR=../lib 
LIBS=-ldl -lpthread -lm

_OBJ = main.o \
       vantage_serial.o \
       data_updater.o \
       log.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c 
	@mkdir -p $(ODIR)
	@echo "CC $@"
	@STAGING_DIR= $(CC) -c -o $@ $< $(CFLAGS)

vantage_connect: $(OBJ)
	@echo "LD $@"
	@STAGING_DIR= ${CC} -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o#~ vantage_connect $(INCDIR)/*~ 
	rm -rf ${ODIR}

