prefix = .
datarootdir = $(prefix)/share
mandir = $(datarootdir)/man

SRCS = $(wildcard *.md)
OBJS = $(SRCS:.md=)

secdirs = $(addprefix $(mandir)/man, $(subst .,,$(suffix $(OBJS))))
objs = $(join $(addsuffix /,$(secdirs)),$(OBJS))

$(info $(objs))

all: $(OBJS)

$(secdirs):
	mkdir -p $@

$(OBJS): %: %.md $(secdirs)
	pandoc $< -s -t man > $(mandir)/man$(subst .,,$(suffix $@))/$@

.PHONY: clean
clean:
	rm -f $(objs)
