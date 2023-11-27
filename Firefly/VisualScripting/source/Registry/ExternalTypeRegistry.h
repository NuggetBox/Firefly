#pragma once
// Place your includes for registered types here.
#include <cinttypes>

#include "imgui/imgui_internal.h"

#include "ScriptGraph/ScriptGraphTypes.h"
#include "Firefly/ComponentSystem/ComponentSystemUtils.h"

#include "FmodWrapper/AudioManager.h"

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

BeginDataTypeHandler(Integer, int, ScriptGraphColor(0, 255, 255, 255), true)
void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
{
	//const float y = ImGui::GetCursorPosY();
	//ImGui::SetCursorPosY(y);
	const ImVec2 inputSize = ImGui::CalcTextSize("10000000");
	ImGui::SetNextItemWidth(inputSize.x);
	ImGui::InputInt(aContainerUUID.c_str(), (int*)aDataPtr, 0, 0);
}

std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	return std::to_string(*(const int*)aDataPtr);
}
EndDataTypeHandler

BeginDataTypeHandler(Entity, uint64_t, ScriptGraphColor(0, 0, 175, 255), false)
//std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
//{
//	return Firefly::GetEntityWithID(*(const uint64_t*)aDataPtr)->GetName();
//}
EndDataTypeHandler

BeginDataTypeHandler(Vector3, Utils::Vector3f, ScriptGraphColor(255, 255, 0, 255), true)
void RenderConstructWidget(const std::string& aContainerUUID, void* aDataPtr, const ScriptGraphType& aTypeInfo) override
{
	Utils::Vector3f* vector = reinterpret_cast<Utils::Vector3f*>(aDataPtr);
	//ImGui::SameLine();
	ImGui::Dummy({0, 0});
	ImGui::SetNextItemWidth(50);
	ImGui::InputFloat((aContainerUUID + "x").c_str(), &vector->x, 0, 0);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::InputFloat((aContainerUUID + "y").c_str(), &vector->y, 0, 0);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(50);
	ImGui::InputFloat((aContainerUUID + "z").c_str(), &vector->z, 0, 0);
}

std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	const Utils::Vector3f* vector = reinterpret_cast<const Utils::Vector3f*>(aDataPtr);
	return "X: " + std::to_string(vector->x) + " Y: " + std::to_string(vector->y) + " Z: " + std::to_string(vector->z);
}
EndDataTypeHandler

BeginDataTypeHandler(Transform, Utils::Transform, ScriptGraphColor(255, 128, 0, 255), false)
std::string ToString(const void* aDataPtr, const ScriptGraphType& aTypeInfo) const override
{
	const Utils::Transform transform = *reinterpret_cast<const Utils::Transform*>(aDataPtr);
	const Utils::Vector3f position = transform.GetPosition();
	const Utils::Vector3f rotation = transform.GetRotation();
	const Utils::Vector3f scale = transform.GetScale();

	return 
		"Position X: " + std::to_string(position.x) + " Y: " + std::to_string(position.y) + " Z: " + std::to_string(position.z) +
		"\nRotation: X: " + std::to_string(rotation.x) + " Y: " + std::to_string(rotation.y) + " Z: " + std::to_string(rotation.z) +
		"\nScale: X: " + std::to_string(scale.x) + " Y: " + std::to_string(scale.y) + " Z: " + std::to_string(scale.z);
}
EndDataTypeHandler

BeginDataTypeHandler(Quaternion, Utils::Quaternion, ScriptGraphColor(128, 128, 128, 255), false)
EndDataTypeHandler

BeginDataTypeHandler(SoundEvent, SoundEventInstanceHandle, ScriptGraphColor(0, 0, 255, 255), false)
//void SerializeData(const void* aDataPtr, const ScriptGraphType& aTypeInfo, std::vector<uint8_t>& outData) const override
//{
//	const SoundEventInstanceHandle* aSound = static_cast<const SoundEventInstanceHandle*>(aDataPtr);
//
//	const std::string event = aSound->GetEvent();
//	const int instance = aSound->GetId();
//
//	outData.resize(event.size() + 1);
//	outData[0] = static_cast<unsigned char>(event.size());
//	memcpy_s(outData.data() + 1, outData.capacity() - 1, event.c_str(), event.length());
//
//	outData.resize(outData.capacity() + sizeof(int));
//	memcpy_s(outData.data() + 1 + event.size(), sizeof(int), &instance, sizeof(int));
//}
//void DeserializeData(const std::vector<uint8_t>& inData, const ScriptGraphType& aTypeInfo, void* outDataPtr) const override
//{
//	if (!inData.empty())
//	{
//		char size = inData[0];
//
//		SoundEventInstanceHandle* outData = static_cast<SoundEventInstanceHandle*>(outDataPtr);
//		const std::string dataString = std::string(inData.begin() + 1, inData.begin() + 1 + size);
//		(*outData).SetEvent(dataString);
//
//		int value;
//		memcpy_s(&value, sizeof(int), inData.data() + 1 + size, sizeof(int));
//		(*outData).SetId(value);
//	}
//}
EndDataTypeHandler