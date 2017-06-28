.SUFFIXES : .c .o

OBJECTS =  bmp_ops.o\
	   global_api.o	\
		zButton.o

SRCS = 		bmp_ops.c\
	   global_api.c	\
		zButton.c

DEPENDENCY =   global_def.h \
			global_api.h	\
			bmp_ops.h
					
				
				


CROSS_COMPILE = arm-xilinx-linux-gnueabi-
CC = $(CROSS_COMPILE)gcc -lm
AR = $(CROSS_COMPILE)ar

#KERNEL_PATH = /home/fusq/idea6410/urbetter-linux2.6.28
CFLAGS = -g #-Os -Wall
##INC += -I$(KERNEL_PATH)/include     ## peter added


TARGET = zButton

$(TARGET) : $(SRCS)
		$(CC) -o $(TARGET) $(SRCS) #$(OBJECTS)
#		cp -rf ./app_tv /home/fusq/idea6410/app/UT_MultiMedia_test

#.c.o :
#		$(CC) $(INC) $(CFLAGS) $<

clean :
		rm -rf $(OBJECTS) $(TARGET) core
