#include "render_tester.hpp"

RenderTester::RenderTester(Console * inConsole, State * inState) {
  console = inConsole;
  state = inState;

  _center = sf::Vector2i(state->windowWidth/2.0, state->windowHeight/2.0);

  _renderView.reset(sf::FloatRect(0, 0, 480, 270));
  float yPos = std::max((1-(((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight))/2.f, 0.f);
  float viewHeight = (yPos > 0) ? ((270.f/480.f)*(float)state->windowWidth)/(float)state->windowHeight : 1.f;
  _renderView.setViewport(sf::FloatRect(0.f, yPos, 1.f, viewHeight));

  testLight1 = new Light();
  testLight1->setPosition(sf::Glsl::Vec3(10, 2.0, 40));
  testLight1->setColor(sf::Glsl::Vec3(1.0, .5, 0.5));
  testLight1->setIntensity(1.0);
  testLight1->setAttenuation(sf::Glsl::Vec3(1.0, 0.0007, 0.00017));
  state->rayMarcher.addLight(testLight1);

  testLight2 = new Light();
  testLight2->setPosition(sf::Glsl::Vec3(0, 4, 2));
  testLight2->setColor(sf::Glsl::Vec3(1.0, 1.0, 1.0));
  testLight2->setIntensity(1.0);
  testLight2->setAttenuation(sf::Glsl::Vec3(1.0, 0.0001, 0.00017));
  state->rayMarcher.addLight(testLight2);

  //state->rayMarcher.update();
  testSphere = new Sphere();
  testSphere->setRadius(1.0);
  testSphere->transform.setOffset(Vector3(0.0, 0.0, 0.0));
  state->rayMarcher.addShape(testSphere);

  testSphere2 = new Sphere();
  testSphere2->setRadius(1.0);
  testSphere2->transform.setOffset(Vector3(1.0, 0.0, 0.0));
  state->rayMarcher.addShape(testSphere2);



  testBox = new Box();
  testBox->setSize(sf::Glsl::Vec3(1.0, 2.0, 1.0));
  testBox->transform.setOffset(Vector3(10.0, 2.0, 0.0));
  //testBox->transform.setRotationOrigin(Vector3(1.0, 2.0, 1.0));
  state->rayMarcher.addShape(testBox);

  testParent.addChild(&testBox->transform);
  testParent.addChild(&testSphere->transform);
  testParent.addChild(&testSphere2->transform);
  
  
  //testSphere2->printParams();

  int rangeX = 120;
  int rangeY = 120;
  int rangeZ = 120;
  for (int i = 0; i < 4000; i++) {
    int randX = (rand() % rangeX)-60;
    int randY = (rand() % rangeY)-60;
    int randZ = -(rand() % rangeZ);
    Sphere * newSphere = new Sphere();
    newSphere->transform.setOffset(Vector3(randX, randY, randZ));
    newSphere->setRadius(.8);
    state->rayMarcher.addShape(newSphere);
    spheres.push_back(newSphere);
    basePositions.push_back(newSphere->transform.getOffset());
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
    } else if (event.key.code==sf::Keyboard::Y) {
      lockedMouse = !lockedMouse;

      window->setMouseCursorGrabbed(lockedMouse);
      window->setMouseCursorVisible(!lockedMouse);

      sf::Mouse::setPosition(_center, *window);
      _globalCenter = sf::Mouse::getPosition();
    }
  }
}

void RenderTester::handleController() {

}

void RenderTester::update(sf::RenderWindow * window) {

  float xChange = 0.0;
  float yChange = 0.0;

  if (lockedMouse) {
    sf::Vector2i mousePosition = _globalCenter - sf::Mouse::getPosition();
    xChange = mousePosition.x;
    yChange = mousePosition.y;
    sf::Mouse::setPosition(_center, *window);
    _globalCenter = sf::Mouse::getPosition();
  }
  testSphereOffset += state->deltaTime.asSeconds();

  Camera3d * camera = state->rayMarcher.getCurrentCamera();

  camera->setPitchAndYaw(yChange*0.001, xChange*0.001);

  //std::cout << "xChange: " << xChange << " yChange: " << yChange << std::endl;

  testSphere2->transform.setOffset(Vector3((std::sin(testSphereOffset)+1.5), 0, 0));
  state->rayMarcher.updateShape(testSphere2);
  testSphere->transform.setOffset(Vector3(0.0, std::cos(testSphereOffset)+1, 0));
  state->rayMarcher.updateShape(testSphere);

  Quaternion boxOrientation = testBox->transform.getOrientation();

  Quaternion qPitch(Vector3(1.0, 0.0, 0.0), state->deltaTime.asSeconds());
  Quaternion qYaw(Vector3(0.0, 1.0, 0.0), state->deltaTime.asSeconds());
  testBox->transform.setOrientation((boxOrientation*qPitch).normalize());

  testParent.setOrientation((testParent.getOrientation()*qYaw).normalize());
  //testParent.setOffset(Vector3((std::sin(testSphereOffset)+1.5), 0, 0));

  //testBox->setSize(sf::Glsl::Vec3(std::cos(testSphereOffset*7.4)+1.6, std::sin(testSphereOffset*10.0)+1.1, 1.0));
  state->rayMarcher.updateShape(testBox);

  //testBox->debugPrint();



  for (int i = 0; i < 4000; i++) {
    Vector3 curPos = basePositions[i];
    spheres[i]->transform.setOffset(Vector3(curPos.getX()+std::sin(testSphereOffset*(0.5*((float)i / (float)400))), curPos.getY()+std::cos(testSphereOffset*(0.5*((float)i / 400.0))), curPos.getZ()));
    
    //std::cout << spheres.size() << std::endl;
    state->rayMarcher.updateShape(spheres[i]);
  }

  state->rayMarcher.update();
}

void RenderTester::render(sf::RenderWindow * window) {


  const sf::Texture & scene = state->rayMarcher.render();
  sf::Sprite drawScene(scene);
  window->setView(_renderView);
  window->draw(drawScene);
  window->setView(window->getDefaultView());
}
