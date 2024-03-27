LIBS= -lpthread -lm #Libraries used if needed

SRCS= lab1.c

BIN=app1
CFLAGS+= -g -O0
OBJS=$(subst .c,.o,$(SRCS))
all : $(BIN)
$(BIN): $(OBJS)
	@echo [link] $@
	$(CXX) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
%.o: %.cpp
	@echo [Compile] $<
	$(CXX) -c $(CFLAGS) $< -o $@
	
clean:
	@rm -f $(OBJS) $(BIN)
