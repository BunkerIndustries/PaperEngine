#include "Engine.h"

#include "Scene.h"
#include "Entity.h"

#include "camera/EntityCamera.h"
#include "generic/Application.h"
#include "renderer/Renderer2D.h"
#include "renderer/Renderer3D.h"
#include "scripting/ScriptEngine.h"

#include "Components.h"

namespace Paper {

	Scene::Scene()
		: uuid(PaperID()), name("[Scene]"), is_dirty(true) { }

	Scene::Scene(const PaperID& uuid)
		: uuid(uuid), name("[Scene]"), is_dirty(true) { }

	Scene::Scene(const std::string& name)
		: uuid(PaperID()), name(name), is_dirty(true) { }

	Scene::Scene(const PaperID& uuid, const std::string& name)
		: uuid(uuid), name(name), is_dirty(true) { }

	Scene::~Scene()
	{
		registry.clear();
	}

	template <typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<PaperID, entt::entity>& dstEntityMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto [e, component] : view.each())
			{
				PaperID srcUUID = src.get<DataComponent>(e).uuid;
				entt::entity dstEntity = dstEntityMap.at(srcUUID);

				dst.emplace_or_replace<Component>(dstEntity, component);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<PaperID, entt::entity>& dstEntityMap)
	{
		CopyComponent<Component...>(dst, src, dstEntityMap);
	}

	template <typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Shr<Scene> Scene::Copy()
	{
		Shr<Scene> newScene = MakeShr<Scene>();

		auto& dstSceneRegistry = newScene->registry;

		auto dataView = registry.view<DataComponent>();
		for (auto e : dataView)
		{
			PaperID uuid = registry.get<DataComponent>(e).uuid;
			std::string name = registry.get<DataComponent>(e).name;
			auto tags = registry.get<DataComponent>(e).tags;

			Entity entity = newScene->CreateEntity(uuid, name);
			entity.GetComponent<DataComponent>().tags = tags;
		}

		CopyComponent(AllComponents{},dstSceneRegistry, registry, newScene->entity_map);

		newScene->name = name;
		newScene->path = path;
		newScene->is_dirty = false;
		newScene->uuid = uuid;

		return newScene;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		auto view = registry.view<CameraComponent>();
		for (auto [entity, camera] : view.each()) {
			if (!camera.fixedAspectRatio)
				camera.camera.SetViewportSize(width, height);
		}
	}

	void Scene::OnRuntimeStart()
	{
		//Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			auto view = registry.view<ScriptComponent>();
			for (auto [e, script] : view.each()) {
				Entity entity(e, this);
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		{
			auto view = registry.view<ScriptComponent>();
			for (auto [e, script] : view.each()) {
				Entity entity(e, this);
				ScriptEngine::OnDestroyEntity(entity);
			}
		}
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnRuntimeUpdate()
	{
		//Update rotation in transform component
		{
			auto view = registry.view<TransformComponent>();
			for (auto [entity, transform] : view.each()) {
				transform.UpdateRotation();
			}
		}

		if (!isPaused || framesToStep-- > 0)
		{

			//Scripting
			float dt = Application::GetDT();
			{
				auto view = registry.view<ScriptComponent>();
				for (auto [e, script] : view.each()) {
					Entity entity(e, this);
					ScriptEngine::OnUpdateEntity(entity, dt);
				}
			}

			//TODO: update physics

		}
		//get primary camera
		EntityCamera* entityCamera = nullptr;
		glm::mat4 cameraTransform;

		{
			auto view = registry.view<CameraComponent, TransformComponent>();
			for (auto [entity, camera, transform] : view.each()) {
				if (camera.primary)
				{
					entityCamera = &camera.camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		//Render
		if (entityCamera)
		{
			Renderer2D::BeginRender(*entityCamera, cameraTransform);
			Render();
			Renderer2D::EndRender();
		}
	}

	void Scene::OnSimulationStart()
	{
		
	}

	void Scene::OnSimulationStop()
	{
	}

	void Scene::OnSimulationUpdate(const Shr<EditorCamera>& camera)
	{
		//Update rotation in transform component
		{
			auto view = registry.view<TransformComponent>();
			for (auto [entity, transform] : view.each()) {
				transform.UpdateRotation();
			}
		}

		if (!isPaused || framesToStep-- > 0)
		{
			//TODO: update physics

		}

		//render
		Renderer2D::BeginRender(camera);
		Render();
		Renderer2D::EndRender();
	}

	void Scene::OnEditorUpdate(const Shr<EditorCamera>& camera)
	{
		//render
		EditorRender(camera);
	}

	void Scene::EditorRender(const Shr<EditorCamera>& camera)
	{
		Renderer2D::BeginRender(camera);
		Render();

		//camera icons
		{
			auto view = registry.view<TransformComponent, CameraComponent>();
			for (auto [entity, transform, line] : view.each())
			{
				EdgeRenderData data;
				data.texture = DataPool::GetTexture("resources/editor/world/camera_symbol.png", true);
				data.color = glm::vec4(1.0f);
				data.transform = transform.GetTransform() * glm::toMat4(glm::quat(glm::radians(glm::vec3(0.0f, -90.0f, 0.0f))));
				data.enity_id = (entity_id)entity;

				Renderer2D::DrawRectangle(data);
			}
		}

		Renderer2D::EndRender();
	}

	void Scene::Render()
	{
		//Sprites
		{
			auto view = registry.view<TransformComponent, SpriteComponent>();
			for (auto [entity, transform, sprite] : view.each()) {
				if (sprite.geometry == Geometry::CIRCLE)
				{
					CircleRenderData data;
					data.transform = transform.GetTransform();
					data.color = sprite.color;
					data.texture = sprite.texture;
					data.tilingFactor = sprite.tiling_factor;
					data.texCoords = sprite.tex_coords;
					data.coreIDToAlphaPixels = sprite.register_alpha_pixels_to_event;
					data.enity_id = (entity_id)entity;

					data.thickness = sprite.thickness;
					data.fade = sprite.fade;

					Renderer2D::DrawCircle(data);
				}
				else
				{
					EdgeRenderData data;
					data.transform = transform.GetTransform();
					data.color = sprite.color;
					data.texture = sprite.texture;
					data.tilingFactor = sprite.tiling_factor;
					data.texCoords = sprite.tex_coords;
					data.coreIDToAlphaPixels = sprite.register_alpha_pixels_to_event;
					data.enity_id = (entity_id)entity;

					if (sprite.geometry == Geometry::RECTANGLE)
						Renderer2D::DrawRectangle(data);
					else if (sprite.geometry == Geometry::TRIANGLE)
						Renderer2D::DrawTriangle(data);
				}
			}
		}

		//lines
		{
			auto view = registry.view<TransformComponent, LineComponent>();
			for (auto [entity, transform, line] : view.each()) 
			{
				LineRenderData data;
				data.transform = transform.GetTransform();
				data.color = line.color;
				data.thickness = line.thickness;
				data.enity_id = (entity_id)entity;
				
				Renderer2D::DrawLine(data);
			}
		}

		//text
		{
			auto view = registry.view<TransformComponent, TextComponent>();
			for (auto [entity, transform, text] : view.each()) {

				TextRenderData data;
				data.transform = transform.GetTransform();
				data.color = text.color;
				data.text = text.text;
				data.font = text.font;
				data.coreIDToAlphaPixels = text.register_alpha_pixels_to_event;
				data.enity_id = (entity_id)entity;
				
				Renderer2D::DrawString(data);
			}
		}
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		is_dirty = true;
		return CreateEntity(PaperID(), name);
	}

	Entity Scene::CreateEntity(const PaperID& id, const std::string& name)
	{
		is_dirty = true;
		Entity entity(registry.create(), id, name, this);
		entity_map[id] = entity;

		return entity;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		Entity dst = CreateEntity(entity.GetName());
		CopyComponentIfExists(AllComponents{}, dst, entity);
		return dst;
	}

	bool Scene::DestroyEntity(Entity entity)
	{
		if (!entity_map.contains(entity.GetPaperID())) return false;

		if (entity.HasComponent<ScriptComponent>())
			ScriptEngine::OnDestroyEntity(entity);

		is_dirty = true;

		entity_map.erase(entity.GetPaperID());
		registry.destroy(entity);
		return true;
	}

	Entity Scene::GetEntity(const PaperID& id)
	{
		//CORE_ASSERT(entity_map.contains(id), "Entity does not exists");
		if (!entity_map.contains(id))
			return {};

		return {entity_map.at(id), this};
	}

	Entity Scene::GetEntityByName(const std::string& name)
	{
		for (const auto& entt : entity_map | std::views::values)
		{
			Entity entity{ entt, this };
			if (entity.GetName() == name)
				return entity;
		}
		return Entity();
	}
}
