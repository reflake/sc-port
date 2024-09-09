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

	class A_IScriptable
	{
	public:

		enum class opcode : uint8_t;
		enum class state : uint16_t;

	public:

		A_IScriptable(uint32_t scriptID);

		virtual void PlayFrame(int frame) = 0;


		const uint32_t GetScriptID() const;

		void Setup(uint32_t type, std::shared_ptr<uint16_t[]> stateOffsets, int stateCount);
		void SetState(state state);
		void Run(ticks currentTick, data::StreamReader codeReader);

	private:

		const uint32_t _scriptID;
		
		uint32_t _type;
		uint16_t _pointer = 0;
		ticks    _waitTimer = 0;
		int      _stateCount;

		std::shared_ptr<uint16_t[]> _stateOffsets;
	};

	class IScriptEngine
	{
	public:

		void Clear();
		void Init();
		void SetScriptData(std::shared_ptr<uint8_t[]> data, int dataSize);
		void Process();
		void PlayNextFrame();
		void RunScriptableObject(std::shared_ptr<A_IScriptable> object);

	private:

		ticks _elaspedTicks;

		std::vector<IScriptEntry>  _entries;
		std::shared_ptr<uint8_t[]> _scriptData;
		int                        _scriptDataSize;

		std::vector<std::shared_ptr<A_IScriptable>> _scriptInstances;
	};

	enum class A_IScriptable::opcode : uint8_t
	{
		PlayFrame = 0x00,

		Wait = 0x05,

		Goto = 0x07,

		Imgul = 0x09,

		Imgulnextid = 0x3D,
	};

	enum class A_IScriptable::state : uint16_t
	{
		Init = 0,
		Death,
	};

	extern void ReadIScriptFile(filesystem::Storage& storage, const char* path, IScriptEngine& engine);
}