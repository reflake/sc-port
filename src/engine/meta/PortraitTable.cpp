#include "PortraitTable.hpp"

namespace meta
{
	void ReadPortraitTable(data::Assets* assets, PortraitTable& out)
	{
		assets->Read("arr/portdata.dat", out);
	}
}