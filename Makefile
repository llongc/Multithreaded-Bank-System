TARGET: bankingClient bankingServer
normal: $(TARGET)
bankingClient: client.c
	gcc client.c database.c -o bankingClient -lpthread
bankingServer: server.c
	gcc server.c database.c -o bankingServer -lpthread
clean: 
	$(RM) $(TARGET)
