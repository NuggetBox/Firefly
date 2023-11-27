#ifndef SCRIPT_TYPES_INCLUDE_SECTION
#define SCRIPT_TYPES_INCLUDE_SECTION

// Place your includes for registered types here.
#include <string>

#else
/********************************************************************************************************************************
 *
 * Here you should declare all types that the ScriptGraph should be aware of. I.e. all
 * types that can appear as a Pin on a Script Graph Node or as a Variable.
 *
 * Format goes:
 * BeginDataTypeHandler(...)
 * <specific code or nothing at all>
 * EndDataTypeHandler
 *
 * BeginDataTypeHandler(FriendlyName, Type, Color, InPlaceConstructible)
 * - FriendlyName is the name you will see in the UI for this type.
 * - Type is the C++ type itself, like float, std::string, GameObject* etc.
 * - Color is the color which will be used to identify this type of variable.
 * - InPlaceConstructible is if we can create this type with a ImGui widget or not
 *   during runtime. You MUST override RenderconstructWidget between Begin and End for
 *	 this to work. See examples below.
 *
 * You can override methods found in the base class:
 * void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
 * - This should draw an ImGui widget to edit the provided data pointer.
 *
 * std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
 * - This should return a string representation of the data, useful for showing default
 *   values in the UI or other things. Allows calling of ScriptGraphDataTypeRegistry::GetString
 *   to get an easily loggable representation of the contents in a data container.
 *
 * virtual void SerializeData(const void* aDataPtr, const ScriptGraphType& aTypeInfo, std::vector<uint8_t>& outData) const
 * - If, and only if, you need to handle serialization of your data type on your own. The default serializer works for all
 *   POD types, but if you want to serialize pointers and things you need to do this yourself :).
 *
 * virtual void DeserializeData(const std::vector<uint8_t>& inData, const ScriptGraphType& aTypeInfo, void* outDataPtr) const
  - If, and only if, you need to handle deserialization of your data type on your own. The default deserializer works for all
 *  POD types, but if you want to deserialize pointers and things you need to do this yourself :).
 *
 *******************************************************************************************************************************/

BeginDataTypeHandler(Float, float, ImVec4(181, 230, 29, 255), true)
void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
{
	//const float y = ImGui::GetCursorPosY();
	//ImGui::SetCursorPosY(y);
	const ImVec2 inputSize = ImGui::CalcTextSize("0.00000");
	ImGui::SetNextItemWidth(inputSize.x);
	const float* floatPtr = (float*)aDataPtr;
	ImGui::InputFloat(aContainerUUID.c_str(), (float*)aDataPtr, 0, 0, "%.4f");
}

std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	return std::to_string(*(const float*)aDataPtr);
}
EndDataTypeHandler

BeginDataTypeHandler(Bool, bool, ImVec4(210, 0, 0, 255), true)
void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
{
	//const float y = ImGui::GetCursorPosY();
	//ImGui::SetCursorPosY(y);
	ImGui::Checkbox(aContainerUUID.c_str(), (bool*)aDataPtr);
}

std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	return *(const bool*)aDataPtr ? "True" : "False";
}
EndDataTypeHandler

BeginDataTypeHandler(String, std::string, ImVec4(255, 0, 255, 255), true)
void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
{
	ImGui::PushItemWidth(100);
	ImGui::InputText(aContainerUUID.c_str(), (std::string*)aDataPtr);
	ImGui::PopItemWidth();
}

std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	return *(const std::string*)aDataPtr;
}

virtual void SerializeData(const void* aDataPtr, const ScriptGraphType& aTypeInfo, std::vector<uint8_t>& outData) const override
{
	// Strings are "fun" :P
	// We assume that aDataPtr has a null terminator at the end...
	const std::string* aString = reinterpret_cast<const std::string*>(aDataPtr);

	// We can't trust the size in typeinfo for the size of a string data block.
	// Since we're not wstring we're just string length * sizeof(char) which is string length.
	outData.resize(aString->length());
	memcpy_s(outData.data(), outData.capacity(), aString->c_str(), aString->length());
}

virtual void DeserializeData(const std::vector<uint8_t>& inData, const ScriptGraphType& aTypeInfo, void* outDataPtr) const override
{
	// And when we build a string back from a vector we need to make sure it's sized properly.
	// outDataPtr is already initialized as a pointer of the correct type.
	if(!inData.empty())
	{
		std::string* aString = reinterpret_cast<std::string*>(outDataPtr);
		const std::string dataString = std::string(inData.begin(), inData.end());
		*aString = dataString;
	}
	int a = 1;
}

EndDataTypeHandler
#endif