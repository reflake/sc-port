#include "IScriptEngine.hpp"

#include <boost/format/format_fwd.hpp>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <vector>

#include "../filesystem/Storage.hpp"

using filesystem::Storage;
using filesystem::StorageFile;

using std::shared_ptr;

namespace script
{
	bool IScriptEntry::IsLast()
	{
		return iscriptID == 0xFFFF && offset == 0x0000;
	}

	int ScopeHeader::GetStateCount() // TODO: check if valid. some of these cases might be incorrect
	{
		switch(type)
		{
			case 0:
			case 1:
				return 2;
			case 2:
				return 4;
			case 12:
			case 13:
				return 14;
			case 14:
			case 15:
				return 15;
			case 20:
			case 21:
				return 21;
			case 23:
				return 23;
			case 24:
				return 25;
			case 26:
			case 27:
			case 28:
			case 29:
				return 27;
			default:
				throw new std::runtime_error("Unknown score header type");
		}
	}

	void IScriptEngine::Init()
	{
		data::StreamReader reader(_scriptData, _scriptDataSize);

		reader.Skip(2);

		int signature;
		int entryOffset = 0;

		reader.Read(signature);

		switch (signature) {
			case 0: // old signature
				reader.SetPointer(0);
				reader.Read(entryOffset);
				break;

			default: // scr signature?
				entryOffset = 0;
				break;
		}

		reader.SetPointer(entryOffset);

		IScriptEntry entry;

		reader.Read(entry);

		while(!entry.IsLast())
		{
			_entries.push_back(entry);
			reader.Read(entry);
		}

		_elaspedTicks = 0;
	}

	void IScriptEngine::SetScriptData(shared_ptr<uint8_t[]> data, int dataSize)
	{
		_scriptData = data;
		_scriptDataSize = dataSize;
	}

	void IScriptEngine::RunScriptableObject(std::shared_ptr<A_IScriptable> object)
	{
		auto scriptID = object->GetScriptID();

		auto it = std::find_if(_entries.begin(), _entries.end(), [scriptID] (auto& entry) {

			return entry.iscriptID == scriptID;
		});

		if (it == _entries.end())
		{
			auto error = boost::format("Script entry with ID %1% is not found") % scriptID;
			throw new std::runtime_error(error.str());
		}

		data::StreamReader reader(_scriptData, _scriptDataSize);

		IScriptEntry& entry = *it;
		ScopeHeader   scope;

		reader.SetPointer(entry.offset);
		reader.Read(scope);

		auto states = std::make_shared<uint16_t[]>(scope.GetStateCount());
		reader.Read(states.get(), scope.GetStateCount());

		object->Setup(scope.type, states, scope.GetStateCount());
		object->SetState(A_IScriptable::state::Init);

		_scriptInstances.push_back(object);
	}

	A_IScriptable::A_IScriptable(uint32_t scriptID) : _scriptID(scriptID) 
		{};
	
	void A_IScriptable::Setup(uint32_t type, shared_ptr<uint16_t[]> states, int stateCount)
	{	
		_type         = type;
		_stateOffsets = states;
		_stateCount   = stateCount;
	}

	void A_IScriptable::SetState(state state)
	{
		int iState = static_cast<int>(state);

		assert(0 <= iState && iState < _stateCount);

		_pointer = _stateOffsets[iState];
	}

	void A_IScriptable::Run(ticks currentTick, data::StreamReader reader)
	{
		if (currentTick < _waitTimer)
			return;

		reader.SetPointer(_pointer);

		uint8_t  waitTickCount;
		uint16_t jumpPos;
		opcode   opcode;

		bool yield = false;

		uint16_t frame;

		while(!yield)
		{
			reader.Read(opcode);

			switch (opcode) {
				case opcode::Imgul: // TODO: implement properly
					reader.Skip(2 + 2);
					break;

				case opcode::Imgulnextid: // TODO: implement properly
					reader.Skip(2);
					break;

				case opcode::PlayFrame:
					reader.Read(frame);

					PlayFrame(frame);

					break;

				case opcode::Goto:
					reader.Read(jumpPos);
					reader.SetPointer(jumpPos);
					break;

				case opcode::Wait:
					reader.Read(waitTickCount);
					_waitTimer = currentTick + waitTickCount + 1;
					yield = true;
					break;

				default:
				{
					auto error = boost::format("OpCode %1% is not implemented") % static_cast<uint8_t>(opcode);
					throw new std::runtime_error(error.str());
				}
			}

			_pointer = reader.GetPointer();
		}
	}
	
	const uint32_t A_IScriptable::GetScriptID() const
	{
		return _scriptID;
	}

	void IScriptEngine::PlayNextFrame()
	{
		data::StreamReader codeReader(_scriptData, _scriptDataSize);

		for(auto& script : _scriptInstances)
		{
			script->Run(_elaspedTicks, codeReader);
		}

		_elaspedTicks++;
	}

	void ReadIScriptFile(Storage& storage, const char* path, IScriptEngine& engine)
	{
		StorageFile file;
		storage.Open(path, file);

		std::shared_ptr<uint8_t[]> data = std::make_shared<uint8_t[]>(file.GetFileSize());

		file.ReadBinary(data.get(), file.GetFileSize());
		engine.SetScriptData(data, file.GetFileSize());
	}
}