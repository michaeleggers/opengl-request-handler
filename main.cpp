#include <arpa/inet.h>
//#include <curl/curl.h>
#include <math.h>
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
#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "platform.h"
#include "shader.h"
#include "utils.h"

static int                     g_fdServerSocket;
static SDL_Window*             g_pWindow;
static SDL_GLContext           g_glContext;
static SDL_Thread*             g_pRequestHandlerThread;
static std::queue<std::string> g_EventQueue;
std::string                    g_BasePath;

struct Uniforms
{
    float    runningTimeMs;
    float    runningTimeSec;
    uint32_t windowWidth;
    uint32_t windowHeight;
};

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

// TODO: Remove later.
static void PrintBytes(HKD_File file)
{
    for ( size_t i = 0; i < file.size; i++ )
    {
        if ( file.data[ i ] == '\0' )
        {
            printf("\\0"); // Print \0 for null byte
        }
        else if ( file.data[ i ] == '\n' )
        {
            printf("\\n"); // Print \n for newline character
        }
        else if ( file.data[ i ] == '\r' )
        {
            printf("\\r"); // Print \r for carriage return character
        }
        else
        {
            printf("%c", file.data[ i ]); // Print other characters normally
        }
    }

    printf("\n");
}

std::string CreateResponse(const std::string& jsonMsg)
{
    size_t      contentLength = jsonMsg.size();
    std::string result        = HTTP_RESPONSE_STRING;
    result += "Content-Length: " + std::to_string(contentLength);
    result += "\r\n\r\n";
    result += jsonMsg;

    return result;
}

static SDL_HitTestResult WindowHittestCallback(SDL_Window* win, const SDL_Point* area, void* data)
{
    return SDL_HITTEST_DRAGGABLE; /**< Region is normal. No special properties. */
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

struct WaylandState
{
    wl_compositor* compositor;
};

static void
registry_handle_global(void* data, wl_registry* wl_registry, uint32_t name, const char* interface, uint32_t version)
{
    printf("Running global registry handler\n");
    WaylandState* state = (WaylandState*)data;
    if ( strcmp(interface, "wl_compositor") == 0 )
    {
        state->compositor = (wl_compositor*)wl_registry_bind(wl_registry, name, &wl_compositor_interface, 4);
    }
}

static void registry_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
    // This space deliberately left blank
}

static const wl_registry_listener g_registry_listener = {
    .global        = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

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
    if ( !SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN, true) )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR,
                     "Failed to apply SDL_PROP_WINDOW_CREATE_ALWAYS_ON_TOP_BOOLEAN property!\nError-Msg: %s\n",
                     SDL_GetError());
    }
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_TRANSPARENT_BOOLEAN, true);

    g_pWindow = SDL_CreateWindowWithProperties(props);
    if ( !g_pWindow )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL3 Window!\nError-Msg: %s\n", SDL_GetError());
        return 66;
    }

#if 0
    if ( SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0 )
    {

        wl_surface* waylandSurface = (wl_surface*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(g_pWindow), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        wl_display* waylandDisplay = (wl_display*)SDL_GetPointerProperty(
            SDL_GetWindowProperties(g_pWindow), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);

        wl_registry* registry = wl_display_get_registry(waylandDisplay);
        printf("wl_registry pointer: %p\n", registry);

        WaylandState waylandState{};
        wl_registry_add_listener(registry, &g_registry_listener, &waylandState);
        wl_display_roundtrip(waylandDisplay);
        printf("wl_registry_add_listener done\n");
        printf("wl_compositor pointer: %p\n", waylandState.compositor);

        wl_region* waylandRegion = wl_compositor_create_region(waylandState.compositor);
        wl_region_add(waylandRegion, 0, 0, 0, 0);
        wl_surface_set_input_region(waylandSurface, waylandRegion);
        printf("Done with wayland crap\n"); 
    }
    else
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Only wayland supported. Sorry!\n");
    }
#endif

#if 0
    if ( !SDL_SetWindowFocusable(g_pWindow, false) )
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not set Window to not focusable!\nError-Msg: %s\n", SDL_GetError());
        return 66;
    }
#endif

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
    printf("Display size: %d, %d\n", displayMode->w, displayMode->h);
    SDL_SetWindowSize(g_pWindow, displayMode->w, displayMode->h);
    SDL_SetWindowPosition(g_pWindow, 0, 0);
    //SDL_SetWindowFullscreen(g_pWindow, true);

    SDL_GLContext context = SDL_GL_CreateContext(g_pWindow);
    int           version = gladLoadGL();

#if 1
    // Set window hittest callback
    if ( !SDL_SetWindowHitTest(g_pWindow, WindowHittestCallback, nullptr) )
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_ERROR, "Could not set Hittest Callback for Window!\nError-Msg: %s\n", SDL_GetError());
        return 66;
    }
#endif

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

    // Set path in which to look for assets (shaders etc.)
    if ( argc > 0 )
    {
        g_BasePath = std::string(argv[ 1 ]);
    }
    else
    {
        g_BasePath = GetExePath();
    }
    printf("BaseDir set to: %s\n", g_BasePath.c_str());

    // Load shaders
    Shader basicShader{};
    if ( !basicShader.Load("shaders/basic.vert", "shaders/basic.frag", 0) )
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create vert/frag shader-pair\n");
        exit(66);
    }

    Shader compShader{};
    if ( !compShader.LoadComputeShader("shaders/particles.comp") )
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create compute shader!\n");
        exit(66);
    }

    // Create buffers
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);

    // NOTE: Keep this in place as reference as we might want to use VBOs later.
    {
        //glEnableVertexAttribArray(VAO);
        //GLuint VBO;
        //glCreateBuffers(1, &VBO);
        //glNamedBufferData(VBO, 1000, nullptr, GL_DYNAMIC_DRAW);

        //// Bind the buffer to a binding point (binding index = 0 here)
        //glVertexArrayVertexBuffer(VAO, 0, VBO, 0, sizeof(float) * 3);

        //// Define attribute format
        //glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

        //// Link attribute index 0 to binding index 0
        //glVertexArrayAttribBinding(VAO, 0, 0);

        //// Enable the attribute
        //glEnableVertexArrayAttrib(VAO, 0);
    }

    // Create uniform buffer
    Uniforms uniforms{};
    GLuint   uniformBuffer;
    glCreateBuffers(1, &uniformBuffer);
    glNamedBufferData(uniformBuffer, sizeof(Uniforms), &uniforms, GL_DYNAMIC_DRAW);

    // Init color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup timers
    uint64_t timeStart         = 0;
    uint64_t countsPerSecond   = SDL_GetPerformanceFrequency();
    double   frameTimeSec      = 0.0f;
    double   totalFrameTimeSec = 0.0f;
    uint64_t frameTime         = 0;
    uint64_t timeEnd           = 0;

    // Run event loop
    bool done = false;
    while ( !done )
    {
        frameTime    = timeEnd - timeStart;
        frameTimeSec = double(frameTime) / double(countsPerSecond);
        totalFrameTimeSec += frameTimeSec;
        float totalFrameTimeMs = float(totalFrameTimeSec * 1000.0f);

        timeStart = SDL_GetPerformanceCounter();

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

            if ( event.type == SDL_EVENT_MOUSE_BUTTON_DOWN || event.type == SDL_EVENT_MOUSE_BUTTON_UP
                 || event.type == SDL_EVENT_MOUSE_MOTION || event.type )
            {
                //continue;
            }

            if ( event.type == SDL_EVENT_WINDOW_HIT_TEST )
            {
                //printf("SDL Window Hittest was not normal!\n");
            }
        }

        // Do game logic, present a frame, etc.
        //
        if ( !g_EventQueue.empty() )
        {
            g_EventQueue.pop();
            totalFrameTimeSec = 0.0f;
        }

        // Update uniforms
        uniforms.runningTimeMs  = totalFrameTimeMs;
        uniforms.runningTimeSec = totalFrameTimeSec;
        uniforms.windowWidth    = displayMode->w;
        uniforms.windowHeight   = displayMode->h;
        glNamedBufferSubData(uniformBuffer, 0, sizeof(Uniforms), &uniforms);

        glViewport(0, 0, displayMode->w, displayMode->h);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        basicShader.Use();
        glBindVertexArray(VAO);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        SDL_GL_SwapWindow(g_pWindow);

        timeEnd = SDL_GetPerformanceCounter();
    }

    //HandleRequests();

    // Shutdown
    SDL_GL_DestroyContext(g_glContext);
    SDL_DestroyWindow(g_pWindow);
    SDL_Quit();

    //freeaddrinfo(servinfo);

    return 0;
}
