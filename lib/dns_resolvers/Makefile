tests: tests.o naptr.o regex_extensions.o dns_resolvers.o
	gcc -fstack-protector-all tests.o naptr.o regex_extensions.o dns_resolvers.o -lresolv -o tests

tests.o: tests.c naptr.h
	gcc -fstack-protector-all -c tests.c -o tests.o

dns_resolvers.o: dns_resolvers.c dns_resolvers.h
	gcc -fstack-protector-all -c dns_resolvers.c

naptr.o: naptr.c naptr.h
	gcc -fstack-protector-all -c naptr.c

regex_extensions.o: regex_extensions.c regex_extensions.h
	gcc -fstack-protector-all -c regex_extensions.c

clean:
	rm *.o tests
