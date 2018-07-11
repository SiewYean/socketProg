#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // structures for networking
#include <pthread.h> 
#define BUFSIZE 128

void* Receiver(void *arg){
	int sd =*(int*)arg; //client socket
	int numbytes;
	char buf[BUFSIZE];
	do {
		//empty the buffer
		memset(buf,0,BUFSIZE);
		numbytes = recv(sd,buf,BUFSIZE,0);
		printf("%s", buf);
		//ensure you can quit the connection if type 'quit'
		//strstr = search for a string in a string
		if(strstr(buf,"quit") != NULL){
			printf("..quitting..\n"); //this is just a simulation, does not really quit
			exit(0); //premature death
		}
	}while(numbytes>0);

	return 0;
}

void* Sender(void *arg) {
	int sd =*(int*)arg;
	char buf[BUFSIZE];
	while(1) {
		memset(buf,0,BUFSIZE);
		fgets(buf,BUFSIZE,stdin);
		unsigned len = strlen(buf);
		buf[len]='\r';
		send(sd,buf,BUFSIZE,0);
	}
}


int main(){
	pthread_t tidReceiver, tidSender;
	
	char server_message[256] = "You have reached server\n";
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM, 0); // 0 default TCP

	int one = 1;
	// SOL_SOCKET = socket options, one = 1 means enable 
	// ensure that there is no waiting time if you reconnect again in 10-15 sec
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));

	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(9002);
	//INADDR_ANY = it accepts any address and bind to it
	server_address.sin_addr.s_addr = INADDR_ANY;

	//struct sockaddr *... is used to silence 'error msg'
	bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address));
	
	//start listeing and accepts a max of 5 simultaneous connection
	listen(server_socket,5);
	
	int client_socket;
	client_socket = accept(server_socket,NULL,NULL);

	//asynchronous connection, both receiver child and sender child do not need to wait for each other
	if (pthread_create(&tidReceiver, NULL, &Receiver, &client_socket)) {
		perror("Fail create Receiver thread");
	}
	if (pthread_create(&tidSender, NULL, &Sender, &client_socket)) {
		perror("Fail create Sender thread");
	}

	//wait for children thread to die before exiting, if not the parent will not exit
	pthread_join(tidReceiver, NULL);
	pthread_join(tidSender, NULL);


	close(client_socket);
	close(server_socket);
	return 0;
}
