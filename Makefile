all:client server

client:et_client.c
	@gcc -g -o $@ $<

server:et_server.c
	@gcc -g -o $@ $<
clean:
	@rm -f client server

