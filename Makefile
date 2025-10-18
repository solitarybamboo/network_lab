CC = arm-linux-gcc

CPPFLAGS += -I ./inc
CPPFLAGS += -I ./inc/libxml2
LDFLAGS  += -L ./lib

LDFLAGS += -lxml2
LDFLAGS += -lz


voicectl:voicectl.c common.c lcd.c bmp.c touch.c  main.c word.c
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS)

# 伪目标
clean:
	rm voicectl -rf
