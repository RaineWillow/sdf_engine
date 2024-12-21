#include "render_tester.hpp"

RenderTester::RenderTester(Console * inConsole, State * inState) {
  console = inConsole;
  state = inState;

  _renderView.reset(sf::FloatRect(0, 0, 480, 270));
  float yPos = std::max((1-(((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight))/2.f, 0.f);
  float viewHeight = (yPos > 0) ? ((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight : 1.f;
  _renderView.setViewport(sf::FloatRect(0.f, yPos, 1.f, viewHeight));

  testLight1 = new Light();
  testLight1->setPosition(sf::Glsl::Vec3(10, 2.0, -40));
  testLight1->setColor(sf::Glsl::Vec3(1.0, .5, 0.5));
  testLight1->setIntensity(1.0);
  testLight1->setAttenuation(sf::Glsl::Vec3(1.0, 0.07, 0.017));
  state->rayMarcher.addLight(testLight1);

  testLight2 = new Light();
  testLight2->setPosition(sf::Glsl::Vec3(0, 4, -2));
  testLight2->setColor(sf::Glsl::Vec3(1.0, 1.0, 1.0));
  testLight2->setIntensity(1.0);
  testLight2->setAttenuation(sf::Glsl::Vec3(1.0, 0.07, 0.017));
  state->rayMarcher.addLight(testLight2);

  //state->rayMarcher.update();
  testSphere = new Sphere();
  testSphere->setRadius(1.0);
  testSphere->setOffset(sf::Glsl::Vec3(0.0, 0.0, 0.0));
  state->rayMarcher.addShape(testSphere);

  testSphere2 = new Sphere();
  testSphere2->setRadius(1.0);
  testSphere2->setOffset(sf::Glsl::Vec3(1.0, 0.0, 0.0));
  state->rayMarcher.addShape(testSphere2);



  testBox = new Box();
  testBox->setSize(sf::Glsl::Vec3(1.0, 2.0, 1.0));
  testBox->setOffset(sf::Glsl::Vec3(10.0, 2.0, 0.0));
  state->rayMarcher.addShape(testBox);
  
  //testSphere2->printParams();

  int rangeX = 120;
  int rangeY = 120;
  int rangeZ = 80;
  for (int i = 0; i < 400; i++) {
    int randX = (rand() % rangeX)-60;
    int randY = (rand() % rangeY)-60;
    int randZ = (rand() % rangeZ);
    Sphere * newSphere = new Sphere();
    newSphere->setOffset(sf::Glsl::Vec3(randX, randY, randZ));
    newSphere->setRadius(.8);
    state->rayMarcher.addShape(newSphere);
    spheres.push_back(newSphere);
  }
/*
  Sphere * finalSphere = new Sphere();
  finalSphere->setOffset(sf::Glsl::Vec3(5, -1, -1));
  finalSphere->setRadius(0.8);
  finalSphere->setAmbient(sf::Glsl::Vec3(1.0, 1.0, 0.0));
  state->rayMarcher.addShape(finalSphere);
  spheres.push_back(finalSphere);
*/
  
  state->rayMarcher.update();
}

RenderTester::~RenderTester() {
  console=NULL;
  state=NULL;
  
  delete testSphere;
  delete testSphere2;
  delete testBox;

  delete testLight1;
  delete testLight2;

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
  if (event.type==sf::Event::KeyReleased) {
    if (event.key.code==sf::Keyboard::X) {
      std::cout << state->rayMarcher.outTree() << std::endl;
    }
  }
}

void RenderTester::handleController() {

}

void RenderTester::update(sf::RenderWindow * window) {
  testSphereOffset += state->deltaTime.asSeconds();

  

  testSphere2->setOffset(sf::Glsl::Vec3((std::sin(testSphereOffset)+1.5), 0, 0));
  state->rayMarcher.updateShape(testSphere2);
  testSphere->setOffset(sf::Glsl::Vec3(0.0, std::cos(testSphereOffset)+1, 0));
  state->rayMarcher.updateShape(testSphere);

/*
  for (int i = 0; i < 10; i++) {
    sf::Glsl::Vec3 curPos = spheres[i]->getPos();
    spheres[i]->setOffset(sf::Glsl::Vec3(curPos.x+std::sin(testSphereOffset), curPos.y+std::cos(testSphereOffset), curPos.z));
    //std::cout << spheres.size() << std::endl;
    state->rayMarcher.updateShape(spheres[i]);
  }
*/
  state->rayMarcher.update();
}

void RenderTester::render(sf::RenderWindow * window) {


  const sf::Texture & scene = state->rayMarcher.render();
  sf::Sprite drawScene(scene);
  window->setView(_renderView);
  window->draw(drawScene);
  window->setView(window->getDefaultView());
}
