# Snap7 to SQL in Ubuntu & C++
In this project I am connecting a S71200 to a SQL server using snap7 and mySQL in Ubuntu. Most important of all, coding in C++.

My program is capable of store cycle data from a dryer controlled by a S71200 PLC, and upload it to a SQL server hosted in a raspberry. If for some reason the server is not responding as in the image above, and the dryer is trying to send the data, the program will store the cycles in a queue, when the connection is recovered, all the missing cycles will be uploaded in the server with no problem.

<img width="807" height="481" alt="image" src="https://github.com/user-attachments/assets/2db42b42-cf2c-4635-b767-06b4699feb09" />

For compile the program the command is: 
```
g++ main.cpp snap7.cpp -o main -lsnap7 -lmysqlcppconn
```

make sure you have compiled the libsnap.so


