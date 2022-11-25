#USING VARIABLES IN MAKEFILES
# TO RUN the application : 
# ./server 
# ./client 

main :
	gcc client*.c -o client -lpthread
	gcc server.c pipe.c -o server -lpthread

PHONY : clean 
clean :
	rm $(OBJS) prog