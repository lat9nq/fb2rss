CC=gcc
CCFLAGS=-Wall -g

.c.o:
	$(CC) $(CCFLAGS) -c $<

fb2rss:fb2rss.o
	$(CC) $(CCFLAGS) $< -o$@

clean:
	rm -vf *.o fb2rss format-html

format-html:format-html.o
