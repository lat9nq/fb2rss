#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	META	1
#define	DIV	2
#define P	3
#define IMG	4
#define INPUT	5
#define ABBR	6

int main(int argc, char * argv[]) {

	FILE * in = stdin;
	char * inputname = NULL;
	FILE * f = stdout;
	char * filename = NULL;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'o':
					if (argv[i][2]) {
						filename = argv[i] + 2;
					}
					else {
						i++;
						if (i < argc && argv[i][0] != '-') 
							filename = argv[i];
						else {
							fprintf(stderr, "error: -o specified without a parameter passed\n");
							return 0;
						}
					}
				break;
				default:
					fprintf(stderr, "unknown switch \'%s\'\n", argv[i]);
				break;
			}
		}
		else {
			if (!inputname)
				inputname = argv[i];
			else
				fprintf(stderr, "warning: multiple inputs\n");
		}
	}

	if (inputname) {
		in = fopen(inputname, "rb");
		if (!in) {
			fprintf(stderr, "error: file \'%s\' is inaccessible\n", inputname);
			return 0;
		}
	}
	if (filename) {
		f = fopen(filename, "wb");
		if (!f) {
			fprintf(stderr, "error: file \'%s\' is inaccessible\n", filename);
			return 0;
		}
	}

	char * separator = calloc(128,sizeof(*separator));
	separator[' '] = 1;
	separator['<'] = 1;
	separator['>'] = 1;
	separator['/'] = 1;
	separator['\n'] = 1;
	separator['\r'] = 1;
	separator['\t'] = 1;
	separator['\"'] = 1;
	separator['('] = 1;
	separator[')'] = 1;
	separator[']'] = 1;
	separator['['] = 1;
	separator['\''] = 1;
	separator['`'] = 1;
	separator[0] = 1;

	char * s = malloc(sizeof(*s)*4096);
	char c;
	int x = 0;
	//int y = 0;

	int tag = 0;
	int open_tag = 0;
	int closer = 0;
	int get_property = 0;
	int get_class = 0;
	int get_title = 0;
	int get_image = 0;
	int get_time = 0;
	int get_url = 0;
	int get_description = 0;
	int get_name = 0;
	int get_value = 0;
	int making_entries = 0;
	int quote = 0;
	int descriptions = 0;
	int desc_ptr = 0;
	char * title = malloc(sizeof(*title)*256);
	char * temp = malloc(sizeof(*temp)*512);
	char * url = malloc(sizeof(*url)*512);
	char * value = malloc(sizeof(*value)*128);
	char * image = malloc(sizeof(*image)*512);
	char * description = malloc(sizeof(*description)*4096);
	char * media = NULL;
	time_t post_time = 0;

	char * out = calloc(65536, sizeof(*out));//malloc(sizeof(*out)*16384);
	out[0] = 0;
	int ptr = 0;
	strcpy(out, "<\?xml version=\"1.0\" encoding=\"utf-8\"\?>\n");
	ptr = strlen(out);
	//strcpy(out+ptr, "<feed xmlns=\"http://www.w3.org/2005/Atom\" xmlns:media=\"http://search.yahoo.com/mrss/\">\n");
	strcpy(out+ptr, "<rss xmlns:media=\"http://search.yahoo.com/mrss/\" version=\"2.0\" xmlns:atom=\"http://www.w3.org/2005/Atom\"  xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n<channel>\n");
	ptr += strlen(out + ptr);
	//Sat, 30 Sep 2017 19:07:40 +0000
	time_t now = time(0);
	strftime(s,256,"%a, %d %b %Y %H:%M:%S +0000", localtime(&now));
	sprintf(temp,"\t<pubDate>%s</pubDate>\n", s);
	strcpy(out+ptr, temp);
	ptr += strlen(out+ptr);
	//fprintf(f, "<feed xmlns=\"http://www.w3.org/2005/Atom\" xmlns:media=\"http://search.yahoo.com/mrss/\">\n");
	//fprintf(f, "<\?xml version=\"1.0\" encoding=\"utf-8\"\?>\n");
	//puts(out);

	while ((c = fgetc(in)) != EOF) {
		if (feof(in)) {
			break;
		}
		if (c == '\"') {
			quote = !quote;
		}
		closer = open_tag & (c == '/');
		tag *= !(c == '>');
#pragma GCC diagnostic ignored "-Wchar-subscripts"
		if (!separator[c] || (quote && c != '\"')) {
#pragma GCC diagnostic warning "-Wchar-subscripts"
			s[x] = c;
			x++;
			s[x] = 0;
		}
		else {
			if (open_tag) {
				tag = DIV * (!strcmp(s, "div"));
				tag += META * (!strcmp(s, "meta"));
				tag += P * (!strcmp(s, "p"));
				tag += IMG * (!strcmp(s, "img"));
				tag += INPUT * (!strcmp(s, "input"));
				tag += ABBR * (!strcmp(s, "abbr"));
				descriptions += tag == P;
				if (tag == P) {
					tag *= (descriptions <= 1);
				}

				open_tag = 0;
			}
			open_tag = (c == '<');

			if (tag == META && s[0]) {
				if (closer) continue;
				//puts(s);
				if (get_title) {
					if (strcmp(s, "content=")) {
						strcpy(title,s);
						//fprintf(stderr, "\t<title>%s</title>\n", title);
						sprintf(temp, "\t<title>%s</title>\n", title);
						strcpy(out+ptr, temp);
						ptr += strlen(temp);
						get_title = 0;
					}
				} else if (get_image) {
					if (strcmp(s, "content=")) {
						strcpy(image,s);
						int n = strstr(image, "&amp;") - image;
						strcpy(strstr(image, "&amp;"), "&amp;");
						strcpy(image + strlen(image), s+n+5);
						sprintf(temp, "\t<image>\n");
						sprintf(temp, "%s\t\t<url>%s</url>\n", temp, image);
						sprintf(temp, "%s\t\t<title>%s</title>\n", temp, title);
						sprintf(temp, "%s\t</image>\n", temp);
						strcpy(out+ptr,temp);
						ptr += strlen(temp);
						get_image = 0;
					}
				} else if (get_description) {
					if (strcmp(s,"content=")) {
						sprintf(temp, "\t<description>%s</description>\n",s);
						strcpy(out+ptr, temp);
						ptr += strlen(temp);
						get_description = 0;
					}
				} else if (get_url) {
					if (strcmp(s,"content=")) {
						sprintf(temp, "\t<link>%s</link>\n",s);
						strcpy(url, s);
						strcpy(out+ptr, temp);
						ptr += strlen(temp);
						get_description = 0;
					}
				} else {
					get_title = (get_property && !strcmp(s, "og:title"));
					get_image = (get_property && !strcmp(s, "og:image"));
					get_description = (get_property && !strcmp(s, "og:description"));
					get_url = (get_property && !strcmp(s, "og:url"));
				}
				get_property = !strcmp(s,"property=");
			}
			else if (tag == DIV && s[0]) {
				if (closer) continue;
				if (get_class) {
					if (strstr(s, "userContent")) {
						if (making_entries) {
							//fprintf(f,"\t</entry>\n");
							sprintf(out+ptr,"\t\t<description><![CDATA[%s%s%s]]></description>\n",
									(strstr(description, "</p>") ? "<p>" : ""),
									description,
									(strstr(description, "<p>") ? "</p>" : ""));
							ptr += strlen(out+ptr);
							sprintf(out+ptr, "\t\t<media:content>%s</media:content>\n",media);
							free(media);
							media = NULL;
							ptr += strlen(out+ptr);
							sprintf(out+ptr, "\t\t<guid>%s/%s</guid>\n", url, value);
							ptr += strlen(out+ptr);
							sprintf(out+ptr, "\t\t<link>%s/%s</link>\n", url, value);
							ptr += strlen(out+ptr);
							strcpy(out+ptr,"\t</item>\n\n");
							ptr += strlen(out + ptr);
						}
						descriptions = 0;
						making_entries = 1;
						//fprintf(f,"\t<entry>\n");
						strcpy(out+ptr,"\t<item>\n");
						ptr += strlen(out + ptr);
						strftime(s,256,"%a, %d %b %Y %H:%M:%S +0000", localtime(&post_time));
						sprintf(out+ptr, "\t\t<pubDate>%s</pubDate>\n",s);
						ptr += strlen(out+ptr);
					}
					get_class = 0;
				}
				get_class = !strcmp(s, "class=");
			}
			else if (tag == ABBR && s[0]) {
				if (get_time) {
					post_time = atoi(s) + 3600;
				}
				get_time = (!strcmp(s,"data-utime="));
			}
			else if (tag == IMG && s[0]) {
				if (get_url) {
					if (desc_ptr) {
						if(!media) {
							media = malloc(sizeof(*media)*512);
							strcpy(media,s);
						}
						strcpy(description+desc_ptr,"<br/><br/><img src=\"");
						desc_ptr += strlen(description+desc_ptr);
						strcpy(description+desc_ptr,s);
						desc_ptr += strlen(description+desc_ptr);
						strcpy(description+desc_ptr,"\"/>");
						desc_ptr += strlen(description+desc_ptr);
						get_url = 0;
						tag = 0;
					}
				}
				get_url = !strcmp(s,"src=");
			}
			else if (tag == INPUT && s[0]) {
				if (get_value) {
					if (strcmp(s,"value=")) {
						strcpy(value, s);
						tag = 0;
						get_value = 0;
					}
				}
				if (get_name) {
					get_value = !strcmp(s, "ft_ent_identifier");
					get_name = 0;
					if (!get_value)
						tag = 0;
				}
				get_name = !strcmp(s,"name=");
			}
			else if (tag == P && s[0]) {
#ifndef INTAG
#define INTAG	3
#define OUTTAG	1
#define CHECKS	2
#define CHECKP	4
#define EXIT	0
#endif
				char ** map = malloc(sizeof(*map)*128);
				for (int i = 0; i < 128; i++)
					map[i] = NULL;
				map['<'] = "<";
				map['/'] = "/";
				/*map['>'] = "&gt;";
				map['<'] = "&lt;";
				map['&'] = "&amp;";
				map['!'] = "&excl;";
				map['#'] = "&num;";
				map['$'] = "&dollar;";
				map['%'] = "&percent;";
				map['('] = "&lpar;";
				map[')'] = "&rpar;";
				map['['] = "&lbrack;";
				map[']'] = "&rbrack;";
				map['/'] = "&sol;";
				map['\\'] = "&bsol;";*/
				char * res = malloc(sizeof(*res)*4096);
				int mode = OUTTAG;
				int old_ptr = ptr;
				int title_at = 0;
				ptr = 0;
				int divs = 0;
				//int spans;
				//int overwrite_ptr = 0;
				
				while ((c = fgetc(in)) != EOF) {
					//putchar(c);
					if (((c == '<') || (ptr > 23 && c == ' ')) && !title_at) {
						title_at = ptr;
					}
					if (ptr > 5 && !strcmp(res+ptr-5,"</div")) {
						if (divs <= 0) {
							for(ptr -= 6; res[ptr] != '<'; ptr--);
								//putchar(res[ptr]);
							res[ptr] = 0;
							/*for (int i = 0; i < spans; i++) {
								strcpy(res+ptr,"</span>");
								ptr += 7;
							}*/
							break;
						}
						divs --;
						ptr -= 5;
						res[ptr] = 0;
						mode = INTAG;
					}
					else if (ptr > 6 && !strcmp(res+ptr-6, "</span")) {
						ptr -= 6;
						res[ptr] = 0;
						mode = INTAG;
					}
					else if (ptr > 3 && !strcmp(res+ptr-3, "</a")) {
						ptr -= 3;
						res[ptr] = 0;
						mode = INTAG;
					}
					else if (ptr > 2 && !strcmp(res+ptr-2, "<a")) {
						ptr -= 2;
						res[ptr] = 0;
						mode = INTAG;
					}
					else if (ptr > 5 && !strcmp(res+ptr-5, "<span")) {
						ptr -= 5;
						res[ptr] = 0;
						mode = INTAG;
					}
					else if (ptr > 4 && (!strcmp(res+ptr-4, "<div"))) {
						ptr -= 4;
						res[ptr] = 0;
						divs ++;
						mode = INTAG;
					}
					if (mode == INTAG) {
						if (c == '>') {
							mode = OUTTAG;
						}
					}
					else if (mode == OUTTAG) {
						if (!map[c]) {
							res[ptr] = c;
							ptr++;
							res[ptr] = 0;
						}
						else {
							strcpy(res+ptr,map[c]);
							ptr += strlen(map[c]);
						}
					}
				}

				free(map);
				desc_ptr = ptr;
				ptr = old_ptr;
				//sprintf(out+ptr,"\t\t<description>%s%s%s</description>\n", (strstr(res, "</p>") ? "<p>" : ""), res, (strstr(res, "<p>") ? "</p>" : ""));
				strcpy(description, res);
				//ptr += strlen(out + ptr);
				if (title_at)
					res[title_at] = 0;
				sprintf(out+ptr,"\t\t<title>%s</title>\n",res);
				ptr += strlen(out + ptr);
				free(res);
			}

			s[0] = 0;
			x = 0;
		}
	}
	sprintf(out+ptr,"\t\t<description><![CDATA[%s%s%s]]></description>\n",
			(strstr(description, "</p>") ? "<p>" : ""),
			description,
			(strstr(description, "<p>") ? "</p>" : ""));
	ptr += strlen(out+ptr);
	sprintf(out+ptr, "\t\t<media:content>%s</media:content>\n",media);
	free(media);
	media = NULL;
	ptr += strlen(out+ptr);
	sprintf(out+ptr, "\t\t<link>%s/%s</link>\n", url, value);
	ptr += strlen(out+ptr);
	sprintf(out+ptr, "\t\t<guid>%s/%s</guid>\n", url, value);
	ptr += strlen(out+ptr);

	strcpy(out + ptr, "\t</item>\n</channel>\n</rss>\n");
	ptr += strlen(out + ptr);
	//fprintf(f, "\t</entry>\n");
	//fprintf(f, "</feed>\n");

	fwrite(out, sizeof(*out), ptr, f);
	//fprintf(f, "%s", out);

	if (inputname)
		fclose(in);
	if (filename)
		fclose(f);

	return 0;
}
