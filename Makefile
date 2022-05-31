objects := xt.o editor.o buffer.o terminal.o
all: xt
xt: $(objects)
clean:
	$(RM) $(objects) $(objects:.o=.d) xt

