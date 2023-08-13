#pragma once
#include "Engine.h"
#include "ScriptUtils.h"
#include "ScriptField.h"

#include "generic/Buffer.h"

#include "generic/Entity.h"

namespace Paper
{
	using EntityFieldStorage = std::vector<Shr<ScriptFieldStorage>>;

	class ScriptClass
		: public std::enable_shared_from_this<ScriptClass>
	{
	public:
		ScriptClass(const std::string& classNameSpace, const std::string& className, MonoImage* monoImage = nullptr);

		MonoObject* Instantiate() const;
		MonoMethod* GetMethod(const std::string& methodName, uint32_t paramCount) const;
		void InvokeMethod(MonoObject* monoObject, MonoMethod* monoMethod, void** params = nullptr) const;

		std::string GetFullClassName() const;
		MonoClass* GetMonoClass() const;
		const std::vector<ScriptField>& GetFields() const { return fields; }
		ScriptField* GetField(const std::string& fieldName) const;

		bool IsSubclassOf(const Shr<ScriptClass>& scriptClass) const;

	private:
		std::string classNameSpace;
		std::string className;

		std::vector<ScriptField> fields;

		MonoClass* monoClass = nullptr;

		void InitFieldMap();

		friend class ScriptEngine;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(const Shr<ScriptClass>& scriptClass);

		void GetFieldValue(const ScriptField& scriptField, Buffer& outBuffer) const;
		void SetFieldValue(const ScriptField& scriptField, const Buffer& value) const;

		Shr<ScriptClass> GetScriptClass() const { return scriptClass; }
		MonoObject* GetMonoInstance() const { return monoInstance; }
	protected:
		Shr <ScriptClass> scriptClass = nullptr;
		MonoObject* monoInstance = nullptr;

	private:
		ScriptInstance(MonoObject* instance);

		friend class ScriptUtils;
		friend class ScriptClass;
	};

	class EntityInstance : public ScriptInstance
	{
	public:
		EntityInstance(const Shr<ScriptClass>& scriptClass, Entity entity);

		void InvokeOnCreate() const;
		void InvokeOnDestroy() const;
		void InvokeOnUpdate(float dt) const;

	private:
		MonoMethod* constructor = nullptr;
		MonoMethod* onCreateMethod = nullptr;
		MonoMethod* onDestroyMethod = nullptr;
		MonoMethod* onUpdateMethod = nullptr;
	};

	class ScriptEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);

		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static void CreateScriptEntity(Entity entity);
		static void DestroyScriptEntity(Entity entity);

		static void OnCreateEntity(Entity entity);
		static void OnDestroyEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, float dt);

		static bool EntityClassExists(const std::string& fullClassName);
		static Shr<ScriptClass> GetEntityClass(const std::string& fullClassName);

		static Scene* GetSceneContext();
		static Shr<EntityInstance> GetEntityScriptInstance(UUID entityUUID);
		static const EntityFieldStorage& GetActiveEntityFieldStorage(Entity entity);
		static std::unordered_map<Shr<ScriptClass>, EntityFieldStorage>& GetEntityFieldStorage(Entity entity);
		static MonoDomain* GetDomain();
		static MonoImage* GetAppAssemblyImage();
		static MonoImage* GetCoreAssemblyImage();
		static std::unordered_map<std::string, Shr<ScriptClass>>& GetEntityClasses();
		static std::unordered_map<UUID, Shr<EntityInstance>>& GetEntityInstances();
	private:
		static void InitMono();
		static void ShutdownMono();

		static EntityFieldStorage& GetActiveEntityFieldStorageInternal(Entity entity);

		static void LoadAssemblyClasses(MonoAssembly* monoAssembly);
	};

    
}

