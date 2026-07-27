#include <Ogre.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <iostream>

int main(int argc, char *argv[]) {
  // 1. Initialize SDL2
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
  SDL_Window *sdlWindow =
      SDL_CreateWindow("Reaktory", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_SHOWN);

  // 2. Extract Native Window Handle for Windows (Win32)
  SDL_SysWMinfo wmInfo;
  SDL_VERSION(&wmInfo.version);
  SDL_GetWindowWMInformation(sdlWindow, &wmInfo);
  std::string winHandle = std::to_string((size_with_t)wmInfo.info.win.window);

  // 3. Setup Ogre3D
  Ogre::Root *root = new Ogre::Root("", "", "Ogre.log");
  // (Load plugins dynamically or via StaticPluginLoader here)

  Ogre::RenderSystem *renderSystem = root->getAvailableRenderers()[0];
  root->setRenderSystem(renderSystem);
  root->initialise(false); // Do not create a default window

  // 4. Create Ogre Window tied to the SDL Window
  Ogre::NameValuePairList params;
  params["externalWindowHandle"] =
      winHandle; // Embeds Ogre inside the SDL window
  Ogre::RenderWindow *window =
      root->createRenderWindow("Reaktory", 1024, 768, false, &params);

  // 5. Setup your Scene, Camera, Viewport, and RTSS here...
  Ogre::SceneManager *scnMgr = root->createSceneManager();

  // register our scene with the RTSS
  Ogre::RTShader::ShaderGenerator *shadergen =
      Ogre::RTShader::ShaderGenerator::getSingletonPtr();
  shadergen->addSceneManager(scnMgr);

  // without light we would just get a black screen
  Ogre::Light *light = scnMgr->createLight("MainLight");
  Ogre::SceneNode *lightNode =
      scnMgr->getRootSceneNode()->createChildSceneNode();
  lightNode->setPosition(0, 10, 15);
  lightNode->attachObject(light);

  // also need to tell where we are
  Ogre::SceneNode *camNode = scnMgr->getRootSceneNode()->createChildSceneNode();
  camNode->setPosition(0, 0, 15);
  camNode->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

  // create the camera
  Ogre::Camera *cam = scnMgr->createCamera("myCam");
  cam->setNearClipDistance(5); // specific to this sample
  cam->setAutoAspectRatio(true);
  camNode->attachObject(cam);

  // and tell it to render into the main window
  ctx.getRenderWindow()->addViewport(cam);
  // 6. Complete Context-Free Main Loop
  bool running = true;
  SDL_Event event;

  while (running) {
    // Pump and process OS/Input events completely via SDL
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
          running = false;
        }
      }
    }

    // Render the frame
    if (!root->renderOneFrame()) {
      running = false;
    }
  }

  // Clean up
  delete root;
  SDL_DestroyWindow(sdlWindow);
  SDL_Quit();
  return 0;
}
