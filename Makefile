CC = arm-linux-gcc

CPPFLAGS += -I ./inc
CPPFLAGS += -I ./inc/libxml2
LDFLAGS  += -L ./lib

LDFLAGS += -lxml2
LDFLAGS += -lz
LDFLAGS += -pthread


voicectl:voicectl.c common.c lcd.c bmp.c touch.c  main.c word.c gy39.c beep.c ts.c mq2.c camera.c camera_screen.c mplayer.c
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS)

# 伪目标
clean:
	rm voicectl -rf
