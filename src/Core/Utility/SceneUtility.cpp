#include "SceneUtility.h"


using namespace Vortex;

void SceneUtility::SaveScene(std::vector<Entity*>& entities, const std::string& filePath) {
	ryml::Tree sceneTree;
	ryml::NodeRef root = sceneTree.rootref();
	root |= ryml::SEQ;
	for (Entity* entity : entities) {
		ryml::NodeRef entityNode = root.append_child();
		entityNode |= ryml::MAP;
		entityNode["name"] << entity->name;
		entityNode["position"] << std::vector<float>{entity->bounds.position.x, entity->bounds.position.y};
		entityNode["size"] << std::vector<float>{entity->bounds.w, entity->bounds.h};
		entityNode["rotation"] << entity->rotation;
		std::vector<Component*> components = entity->GetAllComponents();
		if (!components.empty()) {
			ryml::NodeRef componentsNode = entityNode["components"];
			componentsNode |= ryml::SEQ;
			for (Component* component : components) {
				if (SpriteRenderer2D* sprite = dynamic_cast<SpriteRenderer2D*>(component)) {
					ryml::NodeRef spriteNode = componentsNode.append_child();
					spriteNode |= ryml::MAP;
					spriteNode["type"] << "SpriteRenderer2D";
					spriteNode["spriteLocation"] << sprite->spriteLocation;
				}
				else if (Physics2D* physics = dynamic_cast<Physics2D*>(component)) {
					ryml::NodeRef physicsNode = componentsNode.append_child();
					physicsNode |= ryml::MAP;
					physicsNode["type"] << "Physics2D";
					physicsNode["mass"] << physics->Mass();
					physicsNode["velocity"] << std::vector<float>{physics->Velocity().x, physics->Velocity().y};
					physicsNode["acceleration"] << std::vector<float>{physics->Acceleration().x, physics->Acceleration().y};
				}
			}
		}
	}

	std::ofstream outFile(filePath);
	outFile << ryml::emitrs_yaml<std::string>(sceneTree);

};

void SceneUtility::LoadScene(std::vector<Entity*>& entities, const std::string& filePath) {
	entities.clear();
	std::ifstream inFile(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = inFile.tellg();
	inFile.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	if (inFile.read(buffer.data(), size)) {
		ryml::Tree sceneTree = ryml::parse_in_place(ryml::substr(buffer.data(), buffer.size()));

		auto root = sceneTree.rootref();
		for (ryml::NodeRef entityNode : root.children()) {
			Entity* entity = new Entity();
			entityNode["name"] >> entity->name;

			std::vector<float> position;
			entityNode["position"] >> position;
			entity->bounds.position = Vec2(position[0], position[1]);

			std::vector<float> size;
			entityNode["size"] >> size;
			entity->bounds.w = size[0];
			entity->bounds.h = size[1];

			entityNode["rotation"] >> entity->rotation;

			if (entityNode.has_child("components")) {
				for (ryml::NodeRef componentNode : entityNode["components"].children()) {
					std::string type;
					componentNode["type"] >> type;
					if (type == "SpriteRenderer2D") {
						SpriteRenderer2D* sprite = new SpriteRenderer2D(nullptr);
						componentNode["spriteLocation"] >> sprite->spriteLocation;
						sprite->spriteAssigned = true;
						std::cout << "Scene Sprite Location: " << sprite->spriteLocation.c_str() << std::endl;
						sprite->LoadSprite(sprite->spriteLocation.c_str(), true);
						entity->AddComponent(sprite);
					}
					else if (type == "Physics2D") {
						Physics2D* physics = new Physics2D();
						componentNode["mass"] >> physics->Mass();
						std::vector<float> velocity;
						componentNode["velocity"] >> velocity;
						physics->Velocity() = Vec2(velocity[0], velocity[1]);
						std::vector<float> acceleration;
						componentNode["acceleration"] >> acceleration;
						physics->Acceleration() = Vec2(acceleration[0], acceleration[1]);
						entity->AddComponent(physics);
					}
				}
			}
			entities.push_back(entity);
		}
	}
	
};