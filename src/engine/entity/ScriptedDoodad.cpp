#include "ScriptedDoodad.hpp"

namespace entity
{
	ScriptedDoodad::ScriptedDoodad(uint32_t scriptID, uint32_t grpID, data::position pos) 
			: grpID(grpID), pos(pos), A_IScriptable(scriptID)
	{
	};

		void ScriptedDoodad::PlayFrame(int frame) { _currentFrame = frame; }

		const int ScriptedDoodad::GetCurrentFrame() const { return _currentFrame; }
}