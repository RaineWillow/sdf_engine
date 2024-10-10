#include "window.hpp"
Window::Window(int w, int h, std::string title) {
  sf::ContextSettings settings(0, 0, 0, 4, 2);

  //sf::err().rdbuf(std::cout.rdbuf());
  _window = new sf::RenderWindow(sf::VideoMode(w, h), title, sf::Style::Default, settings);

  _state = new State(_window, w, h);
  _state->setTargetFPS(60);

  console = new Console(_state);

  ImGui::SFML::Init(*_window);

  Scene * render_tester = new RenderTester(console, _state);

  scenes["render_tester"] = render_tester;
  _state->currentScene = "render_tester";

/*
  console->write("Initializing complete!12312\n3123123123123123123123123123123\n123123123123123\n123123123123123123\n123123123123123123123123123123123123123123123123");
  for (int i = 0; i < 1000; i++) {
    console->write("testing scroll...\n" + std::to_string(i));
  }
*/
}

Window::~Window() {
  for (auto const& x : scenes) {
    delete x.second;
  }
  delete _window;
  _window = NULL;
  delete _state;
  _state = NULL;
  delete console;
  console = NULL;
}

void Window::run() {
  //console->write("Starting...");
  //console->write("\n");

  _state->elapsedClock.restart();

  _state->currentSecond = 0;
  while (_state->running) {

    _state->deltaTime = _state->deltaClock.restart();
    sf::Time updateStart = _state->deltaClock.getElapsedTime();
    //poll and handle all events, send all non-window handled events to the
    //current scene
    sf::Event event;
    while (_window->pollEvent(event)) {
      switch (event.type) {
        case sf::Event::Closed: {
          _state->running = false;
          break;
        }
        case sf::Event::Resized: {
          _state->windowWidth = event.size.width;
          _state->windowHeight = event.size.height;

          for (auto const& x : scenes) {
            x.second->handleScreensizeChange();
          }
          break;
        }
        default: {
          //send the event to the currently active scene
          break;
        }
      }
      ImGui::SFML::ProcessEvent(*_window, event);

      scenes[_state->currentScene]->handleEvent(_window, event);
    }
    ImGui::SFML::Update(*_window, _state->deltaTime);
    scenes[_state->currentScene]->update(_window);

    sf::Time updateEnd = _state->deltaClock.getElapsedTime();
    _state->updateTime = updateEnd-updateStart;
    
    ImGui::SetNextWindowPos(ImVec2(-1, 0));
    ImGui::SetNextWindowSize(ImVec2(_state->windowWidth+5, 20));
    bool active = true;
    std::string information = std::string("Delta time: " + std::to_string(_state->deltaTime.asSeconds()) +
      " FPS: " + std::to_string(1.f/_state->deltaTime.asSeconds()) +
      " Frame Time: " + std::to_string(_state->frameTime.asSeconds()) +
      " Update Time: " + std::to_string(_state->updateTime.asSeconds()) +
      " Render Time: " + std::to_string(_state->renderTime.asSeconds())
    );
    ImGui::Begin("Information", &active, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::Text("%s", information.c_str());
    ImGui::End();

    console->display();

    //handle rendering
    _window->clear(sf::Color(0, 0, 0));
    sf::Time renderStart = _state->deltaClock.getElapsedTime();
    scenes[_state->currentScene]->render(_window);
    ImGui::SFML::Render(*_window);
    _window->display();
    sf::Time renderEnd = _state->deltaClock.getElapsedTime();


    _state->renderTime = renderEnd - renderStart;

    //loop until you reach the target time

    sf::Time targetTime = _state->getTargetTime();
    sf::Time checkTime = _state->deltaClock.getElapsedTime();
    _state->frameTime = checkTime;

    
    FrameCounter outTime;
    outTime.fps = 1.f/_state->deltaTime.asSeconds();
    outTime.deltaTime = _state->deltaTime.asMicroseconds();


    //_state->collectedTimes.push_back(outTime);

    while (checkTime < targetTime) {
      checkTime = _state->deltaClock.getElapsedTime();
    }
  }

  _state->outFile.appendLine(std::to_string(_state->elapsedClock.restart().asSeconds()));

  for (int i = 0; i < _state->collectedTimes.size(); i++) {
    FrameCounter writeTime;
    writeTime.fps = _state->collectedTimes[i].fps;
    writeTime.deltaTime = _state->collectedTimes[i].deltaTime;
    _state->outFile.appendLine(std::to_string(writeTime.deltaTime) + " " + std::to_string(writeTime.fps));
  }

  _state->outFile.saveFile("output.txt");
  std::cout << "Program Closed!" << std::endl;
  _window->close();
}
