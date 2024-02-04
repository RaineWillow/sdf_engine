#include "render_tester.hpp"

RenderTester::RenderTester(Console * inConsole, State * inState) {
  console = inConsole;
  state = inState;

  _renderView.reset(sf::FloatRect(0, 0, 480, 270));
  float yPos = std::max((1-(((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight))/2.f, 0.f);
  float viewHeight = (yPos > 0) ? ((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight : 1.f;
  _renderView.setViewport(sf::FloatRect(0.f, yPos, 1.f, viewHeight));

  state->rayMarcher.update();
  testSphere = new Sphere();
  testSphere->setRadius(1.0);
  testSphere->setPos(sf::Glsl::Vec3(0.0, 0.0, 0.0));
  testSphere->setAmbient(sf::Glsl::Vec3(1.0, 0.0, 0.0));
  state->rayMarcher.addShape(testSphere);

  testSphere2 = new Sphere();
  testSphere2->setRadius(1.0);
  testSphere2->setPos(sf::Glsl::Vec3(1.0, 0.0, 0.0));
  testSphere2->setAmbient(sf::Glsl::Vec3(0.0, 0.0, 1.0));
  state->rayMarcher.addShape(testSphere2);

  int rangeX = 120;
  int rangeY = 120;
  int rangeZ = 150;
  for (int i = 0; i < 4000; i++) {
    int randX = (rand() % rangeX)-60;
    int randY = (rand() % rangeY)-60;
    int randZ = (rand() % rangeZ);
    Sphere * newSphere = new Sphere();
    newSphere->setPos(sf::Glsl::Vec3(randX, randY, randZ));
    newSphere->setRadius(0.5);
    newSphere->setAmbient(sf::Glsl::Vec3(0.0, 0.9, 0.3));
    state->rayMarcher.addShape(newSphere);
    spheres.push_back(newSphere);
  }

  Sphere * finalSphere = new Sphere();
  finalSphere->setPos(sf::Glsl::Vec3(5, -1, -50));
  finalSphere->setRadius(0.8);
  finalSphere->setAmbient(sf::Glsl::Vec3(1.0, 1.0, 0.0));
  state->rayMarcher.addShape(finalSphere);
  spheres.push_back(finalSphere);
  

  state->rayMarcher.update();
}

RenderTester::~RenderTester() {
  console=NULL;
  state=NULL;
  delete testSphere;
  delete testSphere2;

  for (int i = 0; i < spheres.size(); i++) {
    delete spheres[i];
  }
}

void RenderTester::handleScreensizeChange() {
  float yPos = std::max((1-(((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight))/2.f, 0.f);
  float viewHeight = (yPos > 0) ? ((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight : 1.f;
  _renderView.setViewport(sf::FloatRect(0.f, yPos, 1.f, viewHeight));
}

void RenderTester::handleEvent(sf::RenderWindow * window, sf::Event & event) {

}

void RenderTester::handleController() {

}

void RenderTester::update(sf::RenderWindow * window) {
  testSphereOffset += state->deltaTime.asSeconds();
  //state->rayMarcher.update();
}

void RenderTester::render(sf::RenderWindow * window) {


  const sf::Texture & scene = state->rayMarcher.render();
  sf::Sprite drawScene(scene);
  window->setView(_renderView);
  window->draw(drawScene);
  window->setView(window->getDefaultView());
}
