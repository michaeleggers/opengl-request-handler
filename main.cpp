#include <arpa/inet.h>
#include <curl/curl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

int main(int argc, char** argv)
{
    /*
    printf("Init Curl...\n");
    CURL* hCurl = curl_easy_init();
    if ( !hCurl )
    {
        printf("Failed to init Curl! Bye.\n");
        exit(66);
    }

    curl_easy_cleanup(hCurl);
*/

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
    const char* myIP   = "192.168.178.150";
    const char* myPort = "50000";

    /*
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family   = AF_INET;     // Use IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags    = 0;           //AI_PASSIVE;  // fill in my IP for me
    if ( (status = getaddrinfo(myIP, myPort, &hints, &servinfo)) != 0 )
    {
        fprintf(stderr, "gai error: %s\n", gai_strerror(status));
        exit(1);
    }

    // Print info.
    char ipstr[ INET6_ADDRSTRLEN ];
    for ( addrinfo* ai = servinfo; ai != nullptr; ai = ai->ai_next )
    {
        void*         addr;
        const char*   ipver;
        sockaddr_in*  ipv4;
        sockaddr_in6* ipv6;

        // get the pointer to the address itself,
        // different fields in IPv4 and IPv6:
        if ( ai->ai_family == AF_INET )
        { // IPv4
            ipv4  = (struct sockaddr_in*)ai->ai_addr;
            addr  = &(ipv4->sin_addr);
            ipver = "IPv4";
        }
        else
        { // IPv6
            ipv6  = (struct sockaddr_in6*)ai->ai_addr;
            addr  = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it:
        inet_ntop(ai->ai_family, addr, ipstr, sizeof ipstr);
        printf("  %s: %s\n", ipver, ipstr);
    }
    */

    int         status;
    addrinfo    hints;
    addrinfo*   servinfo; // will point to the results
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;

    //addr.sin_addr.s_addr = inet_addr("0.0.0.0"); // TODO: register IP at network interface
    addr.sin_port = htons(8080);

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
        sockaddr_in client_addr;
        socklen_t   client_len = sizeof(client_addr);
        listen(sockfd, 1);
        int  newsockfd = accept(sockfd, (struct sockaddr*)&client_addr, &client_len);
        char clientAddrStr[ INET6_ADDRSTRLEN ];
        inet_ntop(client_addr.sin_family, &client_addr.sin_addr, clientAddrStr, sizeof(clientAddrStr));
        printf("Request received from IP: %s\n", clientAddrStr);
    }

    // Shutdown
    //freeaddrinfo(servinfo);

    return 0;
}
