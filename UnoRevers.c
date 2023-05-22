#ifdef _WIN32
	#define _WIN32_WINNT _WIN32_WINNT_WIN7
	#include <winsock2.h> //for all socket programming
	#include <ws2tcpip.h> //for getaddrinfo, inet_pton, inet_ntop
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	#include <time.h> // for rand function
	#include <pthread.h> // for multiple threads
	
	// :D me happy
	void OSInit( void )
	{
		WSADATA wsaData;
		int WSAError = WSAStartup( MAKEWORD( 2, 0 ), &wsaData ); 
		if( WSAError != 0 )
		{
			fprintf( stderr, "WSAStartup errno = %d\n", WSAError );
			exit( -1 );
		}
	}
	void OSCleanup( void )
	{
		WSACleanup();
	}
	#define perror(string) fprintf( stderr, string ": WSA errno = %d\n", WSAGetLastError() )
#else
	#include <sys/socket.h> //for sockaddr, socket, socket
	#include <sys/types.h> //for size_t
	#include <netdb.h> //for getaddrinfo
	#include <netinet/in.h> //for sockaddr_in
	#include <arpa/inet.h> //for htons, htonl, inet_pton, inet_ntop
	#include <errno.h> //for errno
	#include <stdio.h> //for fprintf, perror
	#include <unistd.h> //for close
	#include <stdlib.h> //for exit
	#include <string.h> //for memset
	void OSInit( void ) {}
	void OSCleanup( void ) {}
#endif

int initialization();
int connection( int internet_socket,char* IP );
void execution( int internet_socket,char* IP  );
void cleanup( int internet_socket, int client_internet_socket );
void global_maker(const char* client_ip, char* IP);
char* get_geo_location(const char* IP);

int main( int argc, char * argv[] )
{
	//////////////////
	//Initialization//
	//////////////////
while(1){

	OSInit();
char *IP; 
IP = (char*)calloc(INET6_ADDRSTRLEN, sizeof(char));
if (IP == NULL) {
    fprintf(stderr, "Error allocating memory for client_ip\n");
    exit(1);
}    
	int internet_socket = initialization();

	//////////////
	//Connection//
	//////////////

	int client_internet_socket = connection( internet_socket, IP );

	/////////////
	//Execution//
	/////////////

	execution( client_internet_socket, IP );


	////////////
	//Clean up//
	////////////

	cleanup( internet_socket, client_internet_socket );

	OSCleanup();

	
	}
	
}

int initialization()
{
	//Step 1.1
	struct addrinfo internet_address_setup;
	struct addrinfo * internet_address_result;
	memset( &internet_address_setup, 0, sizeof internet_address_setup );
	internet_address_setup.ai_family = AF_UNSPEC;
	internet_address_setup.ai_socktype = SOCK_STREAM;
	internet_address_setup.ai_flags = AI_PASSIVE;
	int getaddrinfo_return = getaddrinfo( NULL, "22", &internet_address_setup, &internet_address_result );
	if( getaddrinfo_return != 0 )
	{
		fprintf( stderr, "getaddrinfo: %s\n", gai_strerror( getaddrinfo_return ) );
		exit( 1 );
	}
  int no = 0;
	int internet_socket = -1;
	struct addrinfo * internet_address_result_iterator = internet_address_result;

	while( internet_address_result_iterator != NULL )
	{
		//Step 1.2
		internet_socket = socket( internet_address_result_iterator->ai_family, internet_address_result_iterator->ai_socktype, internet_address_result_iterator->ai_protocol );
		if( internet_socket == -1 )
		{
			perror( "socket" );
		}
		else
		{
			//Step 1.3
				setsockopt(internet_socket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*) &no, sizeof(no));
			int bind_return = bind( internet_socket, internet_address_result_iterator->ai_addr, internet_address_result_iterator->ai_addrlen );
			if( bind_return == -1 )
			{
				perror( "bind" );
				close( internet_socket );
			}
			else
			{
				//Step 1.4
				int listen_return = listen( internet_socket, 1 );
				if( listen_return == -1 )
				{
					close( internet_socket );
					perror( "listen" );
				}
				else
				{
					break;
				}
			}
		}
		internet_address_result_iterator = internet_address_result_iterator->ai_next;
	}

	freeaddrinfo( internet_address_result );

	if( internet_socket == -1 )
	{
		fprintf( stderr, "socket: no valid socket address found\n" );
		exit( 2 );
	}

	return internet_socket;
}

int connection( int internet_socket,char* IP )
{
	//Step 2.1
	struct sockaddr_storage client_internet_address;
	socklen_t client_internet_address_length = sizeof client_internet_address;
	int client_socket = accept( internet_socket, (struct sockaddr *) &client_internet_address, &client_internet_address_length );
	if( client_socket == -1 )
	{
		perror( "accept" );
		close( internet_socket );
		exit( 3 );
	}

        char client_ip[INET6_ADDRSTRLEN]; // This can accommodate both IPv4 and IPv6 addresses
        char tmp[INET6_ADDRSTRLEN] = {0};
      if (client_internet_address.ss_family == AF_INET)
      {
      struct sockaddr_in *s = (struct sockaddr_in *)&client_internet_address;
      inet_ntop(AF_INET, &(s->sin_addr), client_ip, sizeof client_ip);
      

      printf("%s\n",tmp);
      }
      else if (client_internet_address.ss_family == AF_INET6)
      {
      struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client_internet_address;
      inet_ntop(AF_INET6, &(s->sin6_addr), client_ip, sizeof client_ip);
      }
      else
      {
      fprintf(stderr, "Unknown address family\n");
      close(client_socket);
      close(internet_socket);
      exit(4);
      }
      
      if(client_ip[6] == ':' ){
            for(int i = 0; i < 15; i++){
      tmp[i] = client_ip[i+7];
      }
      strcpy(client_ip,tmp);}
      
      global_maker(client_ip, IP);
      printf("IP: %s\n",client_ip);
	return client_socket;
}
void global_maker(const char* client_ip, char* IP){
strcpy(IP, client_ip);
}

// this function does the get request to the api with the IP adress. 
char* get_geo_location(const char* IP) {
//208.95.112.1 dns van de api site

    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server;
    char* request = "GET /json/%s?fields=9241 HTTP/1.0\r\nHost: ip-api.com\r\n\r\n";
    char* response = NULL;
    int response_length = 0;

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initialize Winsock.\n");
        return NULL;
    }

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Failed to create socket.\n");
        WSACleanup();
        return NULL;
    }
    
    // Set up server details
    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = inet_addr("208.95.112.1");

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Connection failed.\n");
        closesocket(sock);
        WSACleanup();
        return NULL;
    }
    printf("we connected to server\n");
    // Send the GET request
    char request_buffer[1024];
    snprintf(request_buffer, sizeof(request_buffer), request, IP);
    send(sock, request_buffer, strlen(request_buffer), 0);
    printf("we sended get to server\n");
    // Receive the response
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        response = realloc(response, response_length + bytes_received + 1);
        memcpy(response + response_length, buffer, bytes_received);
        response_length += bytes_received;
    }
   

    // Null-terminate the response
    response[response_length] = '\0';

    // Cleanup and return the response
    closesocket(sock);
    WSACleanup();
    return response;
}


void execution( int internet_socket, char* IP  )
{
printf("%s\n",IP);
srand(time(NULL));
time_t current_time;
    struct tm* time_info;
    char filename[40];
   
    // Get current time
    time(&current_time);
    time_info = localtime(&current_time);

    // Format filename with current time
    strftime(filename, sizeof(filename), "log[%Y-%m-%d %H_%M_%S].txt", time_info);
    
    // Open file
    FILE* file = fopen(filename, "w");

    if (file == NULL) {
      fprintf( stderr, "Error opening file '%s' : errno = %d\n", filename, GetLastError() );
        exit(69);
    }
    printf("File opened: %s\n", filename);
    
   
   fwrite(IP, sizeof(char), strlen(IP), file);
   //IP = "208.95.112.1"; //this is for testing 
   char* response = get_geo_location(IP);
   fwrite(response, sizeof(char), strlen(response), file);
   free(response); 
    
   //Step 3.1
	int number_of_bytes_received = 0;
	char buffer[1000];
	number_of_bytes_received = recv( internet_socket, buffer, ( sizeof buffer ) - 1, 0 );
	if( number_of_bytes_received == -1 )
	{
		perror( "recv" );
	}
	else
	{
		buffer[number_of_bytes_received] = '\0';
		printf( "Received : %s\n", buffer );
	}
	int number_of_bytes_send = 0;
	char bam[65000];
	for(int i = 0; i < 65000; i++){
	bam[i] = rand()%255;
	}
	long long int count = 0;
for(int i = 0; i < 5; i++){
//	Step 3.2
  i--;
	count++;
	putchar('.'); 
	number_of_bytes_send = 0;
	number_of_bytes_send = send( internet_socket, (const char*)&bam, 65000, 0 );
	if( number_of_bytes_send == -1 )
	{
		break;
	}
	
	}
	printf("sended %lld bytes\n",count * 65000); 
	count = count * 65000;
	char sended[50] = {0};

	
	sprintf(sended, "sended bytes = %lld\n",count);
	fwrite(sended, sizeof(char),strlen(sended), file);
	fclose(file);
}



void cleanup( int internet_socket, int client_internet_socket )
{
	//Step 4.2
	int shutdown_return = shutdown( client_internet_socket, SD_RECEIVE );
	if( shutdown_return == -1 )
	{
		perror( "shutdown" );
	}

	//Step 4.1
	close( client_internet_socket );
	close( internet_socket );
}