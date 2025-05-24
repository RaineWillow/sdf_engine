#include "shapes_container.hpp"

ShapesContainer::ShapesContainer(int memoryBufferId) : _memoryBuffer(64, 2000, 52, memoryBufferId),
_defaultMat("default") {

  _defaultMat.setAlbedo(sf::Glsl::Vec3(0.5, 0.5, 0.5));
  _defaultMat.setShine(1.0);
  Pixel defaultAddress = _memoryBuffer.newItem();
  _defaultMat.setAddress(defaultAddress);
  _defaultMat.updateParams(_writeBuffer);
  _memoryBuffer.writeItem(_defaultMat.getAddress().pointerIndex(), _writeBuffer);

  //_writeBuffer = new sf::Uint8[52*4];
}

ShapesContainer::~ShapesContainer() {
  //delete[] _writeBuffer;
}

void ShapesContainer::loadMaterial(Material mat) {
  if (mat.getName() == _defaultMat.getName()) {
    throw std::invalid_argument("Error, attempted to load default material!");
    return;
  }

  if (_materials.find(mat.getName()) != _materials.end()) {
    throw std::invalid_argument("Error, attempted to overwrite material " + mat.getName() + " Hint: Try using the \"overwriteMaterial\" function.");
  }
  //create new material entry
  MaterialEntry newMaterial;

  //set the material of the material entry to the material we are trying to load
  newMaterial.mat = mat;
  
  //put the new material entry in the map
  _materials[mat.getName()] = newMaterial;

  //write the material to the shader
  Pixel newAddress = _memoryBuffer.newItem();
  _materials[mat.getName()].mat.setAddress(newAddress);
  _materials[mat.getName()].mat.updateParams(_writeBuffer);
  _memoryBuffer.writeItem(_materials[mat.getName()].mat.getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::overwriteMaterial(Material mat) {
  if (mat.getName() == _defaultMat.getName()) {
    throw std::invalid_argument("Error, attempted to overwrite default material!");
    return;
  }

  if (_materials.find(mat.getName()) == _materials.end()) {
    throw std::invalid_argument("Error, attempted to overwrite material " + mat.getName() + ", but it didn't exist! Hint: Try using the \"loadMaterial\" function to add a new material.");
  }

  mat.setAddress(_materials[mat.getName()].mat.getAddress());

  _materials[mat.getName()].mat = mat;
  _materials[mat.getName()].mat.updateParams(_writeBuffer);
  _memoryBuffer.writeItem(_materials[mat.getName()].mat.getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::removeMaterial(std::string name) {
  //if the material isn't in the shader
  if (_materials.find(name) == _materials.end()) {
    throw std::invalid_argument("Error, attempted to remove material " + name + ", but it was not loaded!");
    return;
  }

  //should not overwrite the default material
  if (name == _defaultMat.getName()) {
    throw std::invalid_argument("Error, attempted to remove default material!");
    return;
  }
  
  //loop through all shapes that are recorded as having that material and give them the default material
  for (int i = 0; i < _materials[name].shapeWithMat.size(); i++) {
    _materials[name].shapeWithMat[i]->setMaterial(_defaultMat.getAddress(), _defaultMat.getName());
    _materials[name].shapeWithMat[i]->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(_materials[name].shapeWithMat[i]->getAddress().pointerIndex(), _writeBuffer);
  }

  _memoryBuffer.freeItem(_materials[name].mat.getAddress().pointerIndex());

  //remove the material
  _materials.erase(name);
}

void ShapesContainer::addShape(Shape * shape) {
  if (!shape->getAddress().isNullPointer()) {
    throw std::invalid_argument("You should not add a shape which has already been added!");
  }

  Pixel newAddress = _memoryBuffer.newItem();
  //if the shape already has a material, try to go ahead and set it
  if (_materials.find(shape->getMaterial()) != _materials.end()) {
    setMaterial(shape, shape->getMaterial());
  } else { //otherwise, just give it the default material
    shape->setMaterial(_defaultMat.getAddress(), _defaultMat.getName());
  }

  //write the shape to the memory buffer
  shape->setAddress(newAddress);
  shape->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::destroyShape(Shape * shape) {

  if (shape->getAddress().isNullPointer()) {
    throw std::invalid_argument("Attempted to remove a shape before adding it!");
  }
  //free the shape from the memory buffer
  _memoryBuffer.freeItem(shape->getAddress().pointerIndex());
  Pixel NULLPTR;
  shape->setAddress(NULLPTR);

  //if the shape has a material in the material buffer
  if (_materials.find(shape->getMaterial()) != _materials.end()) {
    //find the shape in the material buffer
    auto it = std::find(_materials[shape->getMaterial()].shapeWithMat.begin(), _materials[shape->getMaterial()].shapeWithMat.end(), shape);
    //if the shape was in the material buffer, erase it
    if (it != _materials[shape->getMaterial()].shapeWithMat.end()) {
      _materials[shape->getMaterial()].shapeWithMat.erase(it);
    }
  }
}

void ShapesContainer::updateShape(Shape * shape) {
  if (shape->getAddress().isNullPointer()) {
    throw std::invalid_argument("Attempted to update a shape before adding it!");
  }
  shape->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::setMaterial(Shape * shape, std::string name) {
  //if the shape's material is not an empty string or the default material, and it is a material in the collection, it should be erased
  if ((shape->getMaterial() != "") && (shape->getMaterial() != _defaultMat.getName()) && (_materials.find(shape->getMaterial()) != _materials.end())) {
    //find the material
    auto it = std::find(_materials[shape->getMaterial()].shapeWithMat.begin(), _materials[shape->getMaterial()].shapeWithMat.end(), shape);
    //if the shape was there, erase it
    if (it != _materials[shape->getMaterial()].shapeWithMat.end()) {
      _materials[shape->getMaterial()].shapeWithMat.erase(it);
    }
  }
  
  //if we are just setting the shape to the default material, go ahead
  if (name==_defaultMat.getName()) {
    shape->setMaterial(_defaultMat.getAddress(), _defaultMat.getName());
    shape->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);
    return;
  }

  //if the material is not in the list
  if (_materials.find(name) == _materials.end()) {
    throw std::invalid_argument("Error, attempted to give shape a material " + name + ", but it was not loaded!");
    return;
  }

  //go ahead and set the material
  shape->setMaterial(_materials[name].mat.getAddress(), name);
  shape->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(shape->getAddress().pointerIndex(), _writeBuffer);

  //find the shape in the list of shapes with that material
  auto it = std::find(_materials[shape->getMaterial()].shapeWithMat.begin(), _materials[shape->getMaterial()].shapeWithMat.end(), shape);
  //if the shape was there, erase it
  if (it != _materials[shape->getMaterial()].shapeWithMat.end()) {
    _materials[shape->getMaterial()].shapeWithMat.erase(it);
  }
  //append the shape to the list of shapes with that material
  _materials[name].shapeWithMat.push_back(shape);
}

std::vector<std::string> ShapesContainer::getMaterials() {
  std::vector<std::string> materialNames;
  materialNames.push_back(_defaultMat.getName());

  for (const auto& pair : _materials) {
    materialNames.push_back(pair.first);
  }

  return materialNames;
}

void ShapesContainer::addLight(Light * light, sf::Shader & shader) {

  if (std::find(_lights.begin(), _lights.end(), light) != _lights.end()) {
    throw std::invalid_argument("Attempted to read a light which has already been added (try making a copy of it if you need to)");
  }
  Pixel newAddress = _memoryBuffer.newItem();
  light->setAddress(newAddress);

  _lights.push_back(light);

  for (int i = 0; i < _lights.size(); i++) {
    if (i < _lights.size()-1) {
      _lights[i]->setNext(_lights[i+1]->getAddress());
    } else {
      Pixel NULLPTR;
      _lights[i]->setNext(NULLPTR);
    }

    _lights[i]->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(_lights[i]->getAddress().pointerIndex(), _writeBuffer);
  }
  
  shader.setUniform("headLightPointer", _lights[0]->getAddress().asVec4());
}

void ShapesContainer::destroyLight(Light * light, sf::Shader & shader) {
  auto it = std::find(_lights.begin(), _lights.end(), light);

  if (it == _lights.end()) {
    throw std::invalid_argument("Error! Attempted to remove a light that has not yet been added!");
    return;
  }

  _memoryBuffer.freeItem(light->getAddress().pointerIndex());

  _lights.erase(it);

  for (int i = 0; i < _lights.size(); i++) {
    if (i < _lights.size()-1) {
      _lights[i]->setNext(_lights[i+1]->getAddress());
    } else {
      Pixel NULLPTR;
      _lights[i]->setNext(NULLPTR);
    }

    _lights[i]->updateParams(_writeBuffer);
    _memoryBuffer.writeItem(_lights[i]->getAddress().pointerIndex(), _writeBuffer);
  }

  if (_lights.size() > 0) {
    shader.setUniform("headLightPointer", _lights[0]->getAddress().asVec4());
  } else {
    Pixel NULLPTR;
    shader.setUniform("headLightPointer", NULLPTR.asVec4());
  }
}

void ShapesContainer::updateLight(Light * light) {
  if (std::find(_lights.begin(), _lights.end(), light) == _lights.end()) {
    throw std::invalid_argument("Attempted to update a light which has not been added and has no pointer!");
  }
  light->updateParams(_writeBuffer);
  _memoryBuffer.writeItem(light->getAddress().pointerIndex(), _writeBuffer);
}

void ShapesContainer::bind(sf::Shader & shader, std::string bufferName) {
  shader.setUniform("defaultMatPointer", _defaultMat.getAddress().asVec4());
  Pixel NULLPTR;
  shader.setUniform("headLightPointer", NULLPTR.asVec4());
  _memoryBuffer.bind(shader, bufferName);
}

void ShapesContainer::update() {
  _memoryBuffer.update();
}

void ShapesContainer::render(sf::RenderTexture & renderTarget) {
  //_memoryBuffer.render(renderTarget);
}
