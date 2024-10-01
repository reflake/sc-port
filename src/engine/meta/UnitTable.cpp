#include "UnitTable.hpp"

namespace meta {
	
	void ReadUnitTable(filesystem::Storage& storage, UnitTable& table)
	{
		storage.Read("arr/units.dat", table);
	}
}