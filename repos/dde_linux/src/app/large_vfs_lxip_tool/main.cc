

#include <base/log.h>
#include <util/string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

/* openssl includes */
#include <openssl/conf.h>
#include <openssl/engine.h>
#include <openssl/err.h>
#include <openssl/ui.h>
#include <openssl/ssl.h>
//#include <tls_mosq.h>

enum {
	PORT         = 8888,
	TX_SIZE      = 128 * 1024,
	BUFFER_SIZE  = TX_SIZE * 2,
};


void init_tls(void)
{
	OPENSSL_init_crypto(OPENSSL_INIT_ADD_ALL_CIPHERS | OPENSSL_INIT_ADD_ALL_DIGESTS | OPENSSL_INIT_LOAD_CONFIG, NULL);
	auto x { SSL_get_ex_new_index(0, (void *)"client context", NULL, NULL, NULL) };
}


void handle_client(int client_socket)
{
	using Genode::error;
	using Genode::log;

	Genode::log(" >>>>>> ECHO SERVER  client with socket-fd ", client_socket);

	char    *buffer   { (char *)malloc(BUFFER_SIZE) };
	ssize_t  read_cnt { 0 };
	while (read_cnt < TX_SIZE) {
		ssize_t cnt { read(client_socket, buffer, TX_SIZE) };
		log("read ", cnt, " bytes");
		read_cnt += cnt;
	}

	log("total read ", read_cnt, " bytes");

	// TODO: enc echo back
	ssize_t  write_cnt { write(client_socket, buffer, read_cnt) };
	while (write_cnt < TX_SIZE) {
		ssize_t cnt { write(client_socket, buffer, read_cnt) };
		log("written ", write_cnt, " bytes");
		write_cnt += cnt;
	}
	
	log("total written ", write_cnt, " bytes");

	free(buffer);

	// closing the connected socket
	close(client_socket);
}


int echo_server_impl()
{
	using Genode::error;
	using Genode::log;

	Genode::log(" >>>>>> ECHO SERVER");

	int server_fd { socket(AF_INET, SOCK_STREAM, 0) };
	if (server_fd < 0) {
		error("socket failed");
		exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port 8080
	int opt       { 1 };
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		error("setsockopt");
	    exit(EXIT_FAILURE);
	}

	// Forcefully attaching socket to the port
	sockaddr_in const  addr  { 0, AF_INET, htons(PORT), { INADDR_ANY } };
	sockaddr    const *paddr { reinterpret_cast<sockaddr const *>(&addr) };
	if (bind(server_fd, paddr, sizeof(addr)) < 0) {
	    error("bind failed");
	    exit(EXIT_FAILURE);
	}

	if (listen(server_fd, 3) < 0) {
	    error("listen");
	    exit(EXIT_FAILURE);
	}

	while (true) {

		socklen_t addrlen { sizeof(addr) };
		int new_socket    { accept(server_fd, (struct sockaddr*)&addr, &addrlen) };
		if (new_socket < 0) {
		    error("accept");
		    exit(EXIT_FAILURE);
		}

		handle_client(new_socket);
	}

	// closing the listening socket
	close(server_fd);

	return 0;
}


int sender_receiver_impl()
{
	using Genode::error;
	using Genode::log;

	log(" >>>>>> SENDER RECEIVER ");

	int client_fd { socket(AF_INET, SOCK_STREAM, 0) };
	if (client_fd < 0) {
		error("socket failed");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in serv_addr { .sin_family = AF_INET, .sin_port = htons(PORT) };
	if (inet_pton(AF_INET, "10.0.3.5", &serv_addr.sin_addr) <= 0) {
		error("Settin IP address failed");
		exit(EXIT_FAILURE);
	}

	int status { connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) };
	if (status < 0) {
		error("socket failed");
		exit(EXIT_FAILURE);
	}

	char    *buffer   { (char *)malloc(BUFFER_SIZE) };
	ssize_t  send_cnt { 0 };

	while (send_cnt < TX_SIZE) {
		ssize_t cnt { send(client_fd, buffer, TX_SIZE, 0) };
		log("sent ", cnt, " bytes");
		send_cnt += cnt;
	}

	log("total sent ", send_cnt, " bytes");

	ssize_t read_cnt { 0 };
	while (read_cnt < TX_SIZE) {
		ssize_t cnt { read(client_fd, buffer, TX_SIZE) };
		log("read ", cnt, " bytes");
		read_cnt += cnt;
	}

	log("total read ", read_cnt, " bytes");

	return 0;
}


int main(int argc, char* argv[])
{
	using Genode::warning;
	using Genode::log;

	if (argc < 3) {
		warning("usage: <large_vfs_lxip_tool> --mode [sender_reciver|echo]");
		log("");
		log("     sender_reciver  : send the message and wait for its response");
		log("     echo_server     : wait for the message and echo it back");
		return -1;
	}

	char *m { argv[2] };
	Genode::String<30> mode { Genode::Cstring { m } };

	if (mode == "echo_server") {
		return echo_server_impl();
	}

	if (mode == "sender_receiver") {
		return sender_receiver_impl();
	}

	Genode::error("UNKNOWN MODE : ",mode);
	return -1;
}
