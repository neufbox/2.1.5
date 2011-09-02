
lzma-objs := compress/7zapi.o \
	     compress/7zlzma.o \
	     compress/AriBitCoder.o \
	     compress/IInOutStreams.o \
	     compress/LenCoder.o \
	     compress/LiteralCoder.o \
	     compress/LZMA.o \
	     compress/LZMAEncoder.o \
	     compress/OutByte.o \
	     compress/WindowIn.o

objs := cmplzma.o

CFLAGS := -m32 -Icompress/ -DGNU
CXXFLAGS := -m32 -Icompress/ -DGNU

lzma: $(objs) $(lzma-objs)
	$(CXX) -m32 $^ -o $@ -lm

mrproper: clean

clean:
	rm -f lzma *.o
