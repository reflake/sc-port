#pragma once

#include "../data/Common.hpp"

#include "../script/IScriptEngine.hpp"

namespace entity
{
	class ScriptedDoodad : public script::A_IScriptable
	{
	public:

		ScriptedDoodad(uint32_t scriptID, uint32_t grpID, data::position pos);

		void PlayFrame(int frame) override;

		const int GetCurrentFrame() const;
		
	public:

		const uint32_t       grpID;
		const data::position pos;

	private:

		int _currentFrame = 0;
	};

}