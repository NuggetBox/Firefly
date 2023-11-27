#pragma once
#include "ScriptGraphDataObject.h"

struct ScriptGraphVariable
{
	// The data during runtime.
	ScriptGraphDataObject Data;

	// Whatever the default value is of this variable.
	// This should be read-only during runtime and
	// copied to Data when the graph is loaded.
	ScriptGraphDataObject DefaultData;
	std::string Name;

	//BENNE VARIABLE
	bool NoSet;

	FORCEINLINE std::shared_ptr<const ScriptGraphType> GetTypeData() { return Data.TypeData; }

	/**
	 * \brief Resets the value of this Variable to its Default value.
	 */
	void ResetVariable();
};

