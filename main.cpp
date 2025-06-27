#include <arpa/inet.h>
#include <curl/curl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

static const char* HTTP_RESPONSE_STRING
    = "HTTP/1.1 200 OK\r\n"
      "Access-Control-Allow-Origin:*\r\n"
      "Access-Control-Allow-Credentials:true\r\n"
      "Access-Control-Allow-Headers: origin, content-type, accept\r\n"
      "Strict-Transport-Security: max-age=0\r\n"
      "insecureHTTPParser:true\r\n"
      "Content-Type: application/json; charset=utf-8\r\n"
      "Content-Length:17\r\n"
      "Connection:close\r\n\r\n"
      "{\"message\":\"42\"}\n";

int main(int argc, char** argv)
{
    //struct addrinfo
    //{
    //  int ai_flags;			/* Input flags.  */
    //  int ai_family;		/* Protocol family for socket.  */
    //  int ai_socktype;		/* Socket type.  */
    //  int ai_protocol;		/* Protocol for socket.  */
    //  socklen_t ai_addrlen;		/* Length of socket address.  */
    //  struct sockaddr *ai_addr;	/* Socket address for socket.  */
    //  char *ai_canonname;		/* Canonical name for service location.  */
    //  struct addrinfo *ai_next;	/* Pointer to next in list.  */
    //};
    //
    const char* IP   = "192.168.178.150"; // TODO: Unused atm.
    const int   PORT = 8081;

    int         status;
    addrinfo    hints;
    addrinfo*   servinfo; // will point to the results
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    //addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // TODO: register IP at network interface
    addr.sin_port = htons(PORT);

    // Create AS socket.
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( sockfd < 0 )
    {
        printf("Failed to create socket!\n");
        exit(66);
    }

    // lose the pesky "Address already in use" error message
    int yes = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    // bind it to the port we passed in to getaddrinfo():
    if ( bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0 )
    {
        printf("Failed to bind socket to port %d\n", ntohs(addr.sin_port));
        exit(66);
    }

    char ipstr[ INET6_ADDRSTRLEN ];
    inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, sizeof(ipstr));
    printf("Socket created. IP: %s\n", ipstr);

    while ( 1 )
    {
        // Wait and accept for incoming connection.
        sockaddr_in client_addr;
        socklen_t   client_len = sizeof(client_addr);
        listen(sockfd, 1);
        int newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);

        // Handle client data.
        char clientAddrStr[ INET6_ADDRSTRLEN ];
        inet_ntop(client_addr.sin_family, &client_addr.sin_addr, clientAddrStr, sizeof(clientAddrStr));
        printf("Request received from IP: %s\n", clientAddrStr);

        // Send response to client.
        //const char* responseMsg = "Thanks for calling the OpenGL-Request-Player. Have a nice day!\n";
        size_t responseSize = strlen(HTTP_RESPONSE_STRING);
        printf("responseSize: %lu\n", responseSize);
        size_t sentBytes = send(newsockfd, HTTP_RESPONSE_STRING, strlen(HTTP_RESPONSE_STRING), 0);
        printf("sentBytes: %lu\n", sentBytes);
    }

    // Shutdown
    //freeaddrinfo(servinfo);

    return 0;
}
