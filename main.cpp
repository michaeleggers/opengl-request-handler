#include <arpa/inet.h>
//#include <curl/curl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <queue>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#include <glad/glad.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

static int                     g_fdServerSocket;
static SDL_Window*             g_pWindow;
static SDL_GLContext           g_glContext;
static SDL_Thread*             g_pRequestHandlerThread;
static std::queue<std::string> g_EventQueue;

static float RandBetween(float min, float max)
{
    float range = max - min;
    float rand  = float(random()) / float(RAND_MAX);

    return min + range * rand;
}

static const std::string HTTP_RESPONSE_STRING
    = "HTTP/1.1 200 OK\r\n"
      "Access-Control-Allow-Origin:*\r\n"
      "Access-Control-Allow-Credentials:true\r\n"
      "Access-Control-Allow-Headers: origin, content-type, accept\r\n"
      "Strict-Transport-Security: max-age=0\r\n"
      "insecureHTTPParser:true\r\n"
      "Content-Type: application/json; charset=utf-8\r\n"
      "Connection:close\r\n";

std::string CreateResponse(const std::string& jsonMsg)
{
    size_t      contentLength = jsonMsg.size();
    std::string result        = HTTP_RESPONSE_STRING;
    result += "Content-Length: " + std::to_string(contentLength);
    result += "\r\n\r\n";
    result += jsonMsg;

    return result;
}

static int HandleRequests(void* ptr)
{
    while ( 1 )
    {
        // New client data
        sockaddr_in client_addr;
        socklen_t   client_len = sizeof(client_addr);

        // listen and accept
        listen(g_fdServerSocket, 1);
        int newsockfd = accept(g_fdServerSocket, (struct sockaddr*)&client_addr, &client_len);

        // Handle client data.
        char clientAddrStr[ INET6_ADDRSTRLEN ];
        inet_ntop(client_addr.sin_family, &client_addr.sin_addr, clientAddrStr, sizeof(clientAddrStr));
        printf("Request received from IP: %s\n", clientAddrStr);

        // Send response to client.
        //const char* responseMsg = "Thanks for calling the OpenGL-Request-Player. Have a nice day!\n";
        std::string res          = CreateResponse("{\"message\":42}\n");
        size_t      responseSize = res.size();
        printf("responseSize: %lu\n", responseSize);
        size_t sentBytes = send(newsockfd, res.data(), responseSize, 0);
        printf("sentBytes: %lu\n", sentBytes);

        // Add even to queue
        g_EventQueue.push({ "event-message" });
    }
}

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_PropertiesID props = SDL_CreateProperties();
    if ( props == 0 )
    {
        SDL_Log("Unable to create properties: %s", SDL_GetError());
        return 0;
    }

    // Assume the following calls succeed
    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "My COOL OPENGL WINDOW!!!");
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 640);
    SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 480);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, true);
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_TRANSPARENT_BOOLEAN, true);

    g_pWindow = SDL_CreateWindowWithProperties(props);
    if ( !g_pWindow )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL3 Window!\nError-Msg: %s\n", SDL_GetError());
        return 66;
    }

    // Query available displays.
    int            numAvailDisplays;
    SDL_DisplayID* displayIDs = SDL_GetDisplays(&numAvailDisplays);
    printf("Displays found:\n");
    SDL_DisplayID* currentDisplay = displayIDs;
    for ( int i = 0; i < numAvailDisplays; i++ )
    {
        const SDL_DisplayMode* displayMode = SDL_GetCurrentDisplayMode(*currentDisplay);
        printf("%d: %s\n\tRefresh Rate: %f\n\tSize: %d, %d\n",
               i,
               SDL_GetDisplayName(*currentDisplay),
               displayMode->refresh_rate,
               displayMode->w,
               displayMode->h);
        currentDisplay++;
    }

    // Set window size to size of Desktop.
    const SDL_DisplayMode* displayMode
        = SDL_GetCurrentDisplayMode(*displayIDs); // TODO: might not be the display we want.
    SDL_SetWindowSize(g_pWindow, displayMode->w, displayMode->h);

    SDL_GLContext context = SDL_GL_CreateContext(g_pWindow);
    int           version = gladLoadGL();

    g_glContext = SDL_GL_CreateContext(g_pWindow);
    if ( !g_glContext )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL3 OpenGL Context\nError-Msg: %s\n", SDL_GetError());
        return 66;
    }

    // Print GL Version
    const unsigned char* sGLVersion = glGetString(GL_VERSION);
    printf("Got OpenGL Version: %s\n", sGLVersion);
    const unsigned char* sGLRenderer = glGetString(GL_RENDERER);
    printf("Got OpenGL Renderer: %s\n", sGLRenderer);

    // Setup this server.
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

    // Create as socket.
    g_fdServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( g_fdServerSocket < 0 )
    {
        printf("Failed to create socket!\n");
        exit(66);
    }

    // lose the pesky "Address already in use" error message
    int yes = 1;
    setsockopt(g_fdServerSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);

    // bind it to the port we passed in to getaddrinfo():
    if ( bind(g_fdServerSocket, (sockaddr*)&addr, sizeof(addr)) < 0 )
    {
        printf("Failed to bind socket to port %d\n", ntohs(addr.sin_port));
        exit(66);
    }

    char ipstr[ INET6_ADDRSTRLEN ];
    inet_ntop(addr.sin_family, &addr.sin_addr, ipstr, sizeof(ipstr));
    printf("🦭 Socket created. IP: %s\n", ipstr);

    // Setup the request handler thread and start running
    g_pRequestHandlerThread = SDL_CreateThread(HandleRequests, "HandleRequests", (void*)NULL);
    int threadReturnValue;
    if ( NULL == g_pRequestHandlerThread )
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s", SDL_GetError());
    }
    else
    {
        SDL_DetachThread(g_pRequestHandlerThread);
        //SDL_Log("Thread returned value: %d", threadReturnValue);
    }

    // Run event loop
    bool done = false;
    while ( !done )
    {
        SDL_Event event;
        while ( SDL_PollEvent(&event) )
        {
            if ( event.type == SDL_EVENT_QUIT )
            {
                done = true;
            }

            if ( event.type == SDL_EVENT_KEY_DOWN )
            {
                if ( event.key.key == SDLK_ESCAPE )
                {
                    done = true;
                }
            }
        }

        // Do game logic, present a frame, etc.

        if ( !g_EventQueue.empty() )
        {
            g_EventQueue.pop();
            glClearColor(RandBetween(0.0f, 1.0f), RandBetween(0.0f, 1.0f), RandBetween(0.0f, 1.0f), 0.5f);
        }

        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(g_pWindow);
    }

    //HandleRequests();

    // Shutdown
    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();

    //freeaddrinfo(servinfo);

    return 0;
}
