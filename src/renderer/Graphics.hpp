#include <cstdint>
#include <glm/vec2.hpp>

#include <data/Tileset.hpp>

namespace renderer
{
	typedef uint32_t grpID;
	typedef uint16_t tileID;

	class A_Graphics
	{
	public:

		virtual void LoadGrp(grpID) = 0;
		virtual void DrawGrpFrame(grpID, uint32_t frame, glm::vec2 position) = 0;
		virtual void FreeGrp(grpID) = 0;

		virtual void LoadTileset(data::Tileset) = 0;
		virtual void DrawTile(data::Tileset, tileID, glm::vec2 position) = 0;
		virtual void FreeTileset(data::Tileset) = 0;

		virtual void CycleWaterPalette();

		virtual const char* GetName() const;
	};
}