this is how i run the code using the linler:
gcc server.c main.c  sqlite3.c DBFunctions/DB.c helpers/escape_JSON.c -o backend -lws2_32 -I.

i built this project to understand how c works. im fairly new to c and building stuff like this so i have to admit i refered to ai to help me at times when documentation really wanst helping.

so far i can actually return responses in the terminal and on the web. curl also works really well. my next step is to build a php page doing curl requests. god bless php.