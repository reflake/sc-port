#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../data/Common.hpp"
#include "../filesystem/Storage.hpp"

namespace script
{
	const char SCOPE_HEADER_MAGIC[] = "SCPE";

	typedef uint64_t ticks;

	struct IScriptEntry
	{
		uint16_t iscriptID;
		uint16_t offset;

		bool IsLast();
	};

	struct ScopeHeader
	{
		char     magic[4];
		uint32_t type;

		int GetStateCount();
	};

	class IScript
	{
	public:

		enum class opcode : uint8_t;
		enum class state : uint16_t;

		IScript(uint32_t type, std::shared_ptr<uint16_t[]> stateOffsets, int stateCount);

		void SetState(state state);
		void Run(ticks currentTick, data::StreamReader codeReader);

		const int GetFrameIndex() const;

	private:

		uint32_t _type;
		uint16_t _pointer = 0;
		ticks    _waitTimer = 0;
		uint16_t _currentSpriteFrame = 0;
		int      _stateCount;

		std::shared_ptr<uint16_t[]> _stateOffsets;
	};

	class IScriptEngine
	{
	public:

		void Init();
		void SetScriptData(std::shared_ptr<uint8_t[]> data, int dataSize);
		void PlayNextFrame();

		std::shared_ptr<IScript> InstantiateScript(uint32_t scriptEntryID);

	private:

		ticks _elaspedTicks;

		std::vector<IScriptEntry>        _entries;
		std::shared_ptr<uint8_t[]>       _scriptData;
		int                              _scriptDataSize;

		std::vector<std::shared_ptr<IScript>> _scriptInstances;
	};

	enum class IScript::opcode : uint8_t
	{
		PlayFrame = 0x00,

		Wait = 0x05,

		Goto = 0x07,

		Imgul = 0x09,

		Imgulnextid = 0x3D,
	};

	enum class IScript::state : uint16_t
	{
		Init = 0,
		Death,
	};

	extern void ReadIScriptFile(filesystem::Storage& storage, const char* path, IScriptEngine& engine);
}