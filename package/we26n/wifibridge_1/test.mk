
target := wifibridge_1

all : $(target)
.PHONY : clean all

OBJDIR := ./obj
SRCDIR := ./src


srcs :=  $(wildcard $(SRCDIR)/*.c)
objs :=  $(addprefix $(OBJDIR)/, $(notdir $(srcs:.c=.o)))
deps :=  $(objs:.o=.d)

-include $(deps)


$(objs) : $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

$(target) : $(objs)
	$(CC) -o $@  $^  $(LDFLAGS)

