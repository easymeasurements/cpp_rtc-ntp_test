# (1)コンパイラ
# gdb debug
#CC  = g++ -g3
CC  = g++
# (2)コンパイルオプション
CFLAGS    = 
# (3)実行ファイル名
TARGET  = rtc_test
# (4)コンパイル対象のソースコード
SRCS    = rtc_test.cpp func_rtc3231.cpp func_ntp.cpp
#SRCS    = rtc_test.cpp RTC.cpp
# (5)オブジェクトファイル名
OBJS    = $(SRCS:.cpp=.o)
 
# (6)インクルードファイルのあるディレクトリパス
INCDIR  = -I../inc
 
# (7)ライブラリファイルのあるディレクトリパス
LIBDIR  = 
 
# (8)追加するライブラリファイル
LIBS    = -lbcm2835

# (9)ターゲットファイル生成
$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)
	
# (10)オブジェクトファイル生成
$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(INCDIR) -c $(SRCS)

# (11)"make all"で make cleanとmakeを同時に実施。
all: clean $(OBJS) $(TARGET)
# (12).oファイル、実行ファイル、.dファイルを削除
clean:
	-rm -f $(OBJS) $(TARGET) *.d