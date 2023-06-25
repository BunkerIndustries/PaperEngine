#include "Editor.h"

#define INCLUDE_MAIN
#include "EntryPoint.h"
#include "editor/PELayer.h"

#include "scene/EditorScene/EditorScene.h"
#include "project/ProjectManager.h"

#include "editor/PELayer.h"

class PaperEditor : public Application {
public:
	PaperEditor()
		: Application(WindowProps("PaperEditor"))
	{
		
	}
	
	~PaperEditor() override {
		
	}

	void Init() override {
		ProjectManager::Init();
		//Application::ChangeScene(new EditorScene);
		
		AddLayer(new PELayer());

		
		YAML::Emitter out;
		Shr<EditorScene> scene = MakeShr<EditorScene>();
		const UUID uuid = scene->CreateEntity("lol").GetUUID();

		scene->GetEntity(uuid).AddComponent<SpriteComponent>();
		scene->GetEntity(uuid).AddComponent<CircleComponent>();
		scene->GetEntity(uuid).AddComponent<LineComponent>();
		scene->GetEntity(uuid).AddComponent<TextComponent>();
		scene->GetEntity(uuid).AddTag({ "A", "B", "C", "AA", "aB", "Ac" });
		const YAMLSerializer serializer;
		serializer.SceneSerialize("bunker.yaml", scene);

		LOG_CORE_WARN("DESERIALIZE");

		LOG_CORE_WARN(serializer.SceneDeserialize("bunker.yaml")->GetEntity(uuid).GetName());

	}

};

Application* ppr::CreateApplication() {
	return new PaperEditor();
}