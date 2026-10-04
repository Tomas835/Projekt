#include "Application.h"
#include <cstring>

int main(int argc, char** argv)
{
    const bool broken = (argc > 1 && std::strcmp(argv[1], "--chyba") == 0);

    Application app(broken);
    app.initialization();
    app.createShaders();
    app.createModels();
    app.createScenes();
    app.run();
    return 0;
}
