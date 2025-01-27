﻿#include "Engine.h"
#include "SceneSerializer.h"

#include "serializer/YAMLSerializer.h"

#include "Components.h"
#include "Entity.h"

#include "scripting/ScriptEngine.h"

namespace Paper
{
	bool EntitySerialize(Entity& entity, YAML::Emitter& out)
	{
		CORE_ASSERT(entity.HasComponent<DataComponent>(), "")

			out << YAML::BeginMap; // Entity
		//out << YAML::Key << "Entity" << YAML::Value << entity.GetPaperID();
		//out << YAML::Key << "Name" << YAML::Value << entity.GetName();

		out << YAML::Key << "Components";
		out << YAML::BeginMap;

		if (entity.HasComponent<DataComponent>())
			entity.GetComponent<DataComponent>().Serialize(out);

		if (entity.HasComponent<LineComponent>())
			entity.GetComponent<LineComponent>().Serialize(out);

		if (entity.HasComponent<SpriteComponent>())
			entity.GetComponent<SpriteComponent>().Serialize(out);

		if (entity.HasComponent<TextComponent>())
			entity.GetComponent<TextComponent>().Serialize(out);

		if (entity.HasComponent<ScriptComponent>())
		{
			entity.GetComponent<ScriptComponent>().Serialize(out);

			const auto& entityFieldStorage = ScriptEngine::GetEntityFieldStorage(entity);

			out << YAML::Key << "ScriptFields";
			out << YAML::BeginMap;

			for (const auto& [classID, fieldStorage] : entityFieldStorage)
			{
				out << YAML::Key << ScriptClass(classID).GetFullClassName();
				out << YAML::BeginMap;

				for (const auto& classFieldStorage : fieldStorage)
				{
					ScriptFieldType type = classFieldStorage->GetField()->fieldType;

					out << YAML::Key << classFieldStorage->GetField()->fieldName;
					out << YAML::BeginMap;

					out << YAML::Key << "FieldType" << YAML::Value << (int)type;
					out << YAML::Key << "Value" << YAML::Value;

					switch (type)
					{
						case ScriptFieldType::Bool:
							out << classFieldStorage->GetValue<bool>(true);
							break;
						case ScriptFieldType::Char:
							out << classFieldStorage->GetValue<char>(true);
							break;
						case ScriptFieldType::UChar:
							out << classFieldStorage->GetValue<unsigned char>(true);
							break;
						case ScriptFieldType::Int16:
							out << classFieldStorage->GetValue<int16_t>(true);
							break;
						case ScriptFieldType::UInt16:
							out << classFieldStorage->GetValue<uint16_t>(true);
							break;
						case ScriptFieldType::Int32:
							out << classFieldStorage->GetValue<int32_t>(true);
							break;
						case ScriptFieldType::UInt32:
							out << classFieldStorage->GetValue<uint32_t>(true);
							break;
						case ScriptFieldType::Int64:
							out << classFieldStorage->GetValue<int64_t>(true);
							break;
						case ScriptFieldType::UInt64:
							out << classFieldStorage->GetValue<uint64_t>(true);
							break;
						case ScriptFieldType::Float:
							out << classFieldStorage->GetValue<float>(true);
							break;
						case ScriptFieldType::Double:
							out << classFieldStorage->GetValue<double>(true);
							break;
						case ScriptFieldType::String:
							out << classFieldStorage->GetValue<std::string>(true);
							break;
						case ScriptFieldType::Vec2:
							out << classFieldStorage->GetValue<glm::vec2>(true);
							break;
						case ScriptFieldType::Vec3:
							out << classFieldStorage->GetValue<glm::vec3>(true);
							break;
						case ScriptFieldType::Vec4:
							out << classFieldStorage->GetValue<glm::vec4>(true);
							break;
						case ScriptFieldType::Entity:
							out << classFieldStorage->GetValue<PaperID>(true).toString();
							break;
						default:
							out << 0;
							break;
					}

					out << YAML::EndMap;

				}

				out << YAML::EndMap;
			}

			out << YAML::EndMap;
			out << YAML::EndMap; // SpriteComponent
		}

		if (entity.HasComponent<TransformComponent>())
			entity.GetComponent<TransformComponent>().Serialize(out);

		if (entity.HasComponent<CameraComponent>())
			entity.GetComponent<CameraComponent>().Serialize(out);

		out << YAML::EndMap;
		out << YAML::EndMap; // Entity

		return true;
	}

	void SceneSerializer::Serialize(const Shr<Scene>& scene, const std::filesystem::path& filePath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << scene->GetPaperID();
		out << YAML::Key << "Name" << YAML::Value << scene->GetName();

		std::string scene_path = scene->GetPath().string();
		const std::string abs_path = std::filesystem::current_path().string() + "\\";
		const size_t pos = scene_path.find(abs_path);
		std::filesystem::path path(scene_path);
		if (pos != std::string::npos)
			path = std::filesystem::path(scene_path.erase(pos, abs_path.length()));


		out << YAML::Key << "Path" << YAML::Value << path.string();
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		scene->registry.each([&](auto PaperID)
			{
				Entity entity = { PaperID, scene.get() };
				if (!entity)
					return;

				EntitySerialize(entity, out);
			});
		out << YAML::EndSeq;
		out << YAML::EndMap;


		std::ofstream fout(filePath);
		fout << out.c_str();
		fout.close();
	}

	

	template <typename T>
	void SetFieldStorage(YAML::Node& yamlScriptField, Shr<ScriptFieldStorage>& storage)
	{
		T value = yamlScriptField["Value"].as<T>();
		storage->SetValue(value, true);
	}

	bool DeserializeClassScriptValues(const YAML::const_iterator& yamlScriptClass, const YAML::Node& yamlScriptFields, Entity deserializedEntity)
	{
		std::string scriptClassName = yamlScriptClass->first.as<std::string>();

		ManagedClass* managedClass = ScriptEngine::GetEntityInheritClass(scriptClassName);
		if (!managedClass) return false;

		auto yamlScriptClassFields = yamlScriptFields[scriptClassName];
		for (YAML::const_iterator yamlScriptClassFieldsIT = yamlScriptClassFields.begin(); yamlScriptClassFieldsIT != yamlScriptClassFields.end(); ++yamlScriptClassFieldsIT)
		{
			std::string scriptFieldName = yamlScriptClassFieldsIT->first.as<std::string>();
			ManagedField* managedField = ScriptClass(managedClass).GetManagedField(scriptFieldName);
			if (!managedField) continue;

			auto yamlScriptField = yamlScriptClassFields[scriptFieldName];
			auto& entityFieldStorage = ScriptEngine::GetEntityFieldStorage(deserializedEntity);

			auto& scriptClassFieldStorages = entityFieldStorage[managedClass->classID];

			int index = 0;
			for (; index < scriptClassFieldStorages.size(); index++)
				if (scriptClassFieldStorages[index]->GetField() == managedField) break;

			Shr<ScriptFieldStorage> fieldStorage = MakeShr<ScriptFieldStorage>(managedField);

			ScriptFieldType type = (ScriptFieldType)yamlScriptField["FieldType"].as<int>();

			switch (type)
			{
				case ScriptFieldType::Bool:
					SetFieldStorage<bool>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Char:
					SetFieldStorage<char>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::UChar:
					SetFieldStorage<unsigned char>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Int16:
					SetFieldStorage<int16_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::UInt16:
					SetFieldStorage<uint16_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Int32:
					SetFieldStorage<int32_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::UInt32:
					SetFieldStorage<uint32_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Int64:
					SetFieldStorage<int64_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::UInt64:
					SetFieldStorage<uint64_t>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Float:
					SetFieldStorage<float>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Double:
					SetFieldStorage<double>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::String:
					SetFieldStorage<std::string>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Vec2:
					SetFieldStorage<glm::vec2>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Vec3:
					SetFieldStorage<glm::vec3>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Vec4:
					SetFieldStorage<glm::vec4>(yamlScriptField, fieldStorage);
					break;
				case ScriptFieldType::Entity:
				{
					PaperID entityID = yamlScriptField["Value"].as<std::string>();
					fieldStorage->SetValue(entityID.toUInt64(), true);
					break;
				}
			}

			scriptClassFieldStorages.emplace(scriptClassFieldStorages.begin() + index, fieldStorage);
		}
		return true;
	}

	Shr<Scene> SceneSerializer::Deserialize(const std::filesystem::path& filePath)
	{
		Shr<Scene> scene = MakeShr<Scene>();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filePath.string());
		}
		catch (YAML::Exception e)
		{
			LOG_CORE_ERROR("Failed to load file '{0}'\n\t{1}", filePath, e.what());
			return nullptr;
		}

		try
		{
			if (!data["Scene"])
				return nullptr;

			std::string scene_name = data["Name"].as<std::string>();
			LOG_CORE_TRACE("Deserializing scene '{0}' from '{1}'", scene_name, filePath.string());

			scene->name = scene_name;
			if (data["Path"])
				scene->path = data["Path"].as<std::string>();

			if (auto entities = data["Entities"])
			{
				for (auto entity : entities)
				{
					CORE_ASSERT(entity["Components"], "")
						auto components = entity["Components"];

					PaperID uuid;
					std::string entity_name;
					if (auto data_component = components["DataComponent"])
					{
						uuid = data_component["PaperID"].as<PaperID>();
						entity_name = data_component["Name"].as<std::string>();
					}

					LOG_CORE_TRACE("Deserializing entity name '{0}' and uuid '{1}' from '{2}'", entity_name, uuid.toString(), filePath.string());

					Entity deserialized_entity = scene->CreateEntity(uuid, entity_name);

					if (auto transformComponent = components["TransformComponent"])
						deserialized_entity.GetComponent<TransformComponent>().Deserialize(transformComponent);

					if (auto sprite_component = components["SpriteComponent"])
						deserialized_entity.AddComponent<SpriteComponent>().Deserialize(sprite_component);

					if (auto line_component = components["LineComponent"])
						deserialized_entity.AddComponent<LineComponent>().Deserialize(line_component);

					if (auto text_component = components["TextComponent"])
						deserialized_entity.AddComponent<TextComponent>().Deserialize(text_component);

					if (auto script_component = components["ScriptComponent"])
					{
						deserialized_entity.AddComponent<ScriptComponent>().Deserialize(script_component);

						auto yamlScriptFields = script_component["ScriptFields"];

						for (YAML::const_iterator yamlScriptClass = yamlScriptFields.begin(); yamlScriptClass != yamlScriptFields.end(); ++yamlScriptClass) {
							DeserializeClassScriptValues(yamlScriptClass, yamlScriptFields, deserialized_entity);
						}

						ScriptEngine::CreateScriptEntity(deserialized_entity);
					}

					if (auto camera_component = components["CameraComponent"])
						deserialized_entity.AddComponent<CameraComponent>().Deserialize(camera_component);
				}
			}
		}
		catch (YAML::Exception& ex)
		{
			LOG_CORE_CRITICAL(ex.msg);
			return nullptr;
		}
		scene->SetClean();
		return scene;
	}
}