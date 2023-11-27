#pragma once
#include <string>
#include <typeinfo>
#include <regex>

namespace Munin
{
	class UID
	{
		static inline size_t nextBaseUID = 1;
		size_t myUID;
	public:

		// ctors get a new UID.
		UID()
			: myUID(nextBaseUID++)
		{  }

		// copies get their own UID.
		UID(const UID& other)
			: myUID(nextBaseUID++)
		{ }

		// Moved objects retain their UID.
		UID(UID&& other) noexcept
			: myUID(other.myUID)
		{ }

		UID& operator=(const UID& other)
		{
			myUID = other.myUID;
			return *this;
		}

		UID& operator=(UID&& other) noexcept
		{
			std::swap(myUID, other.myUID);
			return *this;
		}

		__forceinline size_t GetUID() const { return myUID; }
	};

	/**
	 * \brief Base container for the actual UUID so that it's retrievable without
	 * knowing the owning type.
	 */
	class UUID : public UID
	{
		std::string myUUID;
		const std::type_info& myRTTInfo;
		const std::string myOwnerTypeName;

		[[nodiscard]] std::string PopulateTypeName() const
		{
			const std::string MSVCTypeName = myRTTInfo.name();
			const size_t fromPos = MSVCTypeName.find_first_of(' ');

			size_t toPos = MSVCTypeName.find_first_of(' ', fromPos);
			if(toPos == std::string::npos)
			{
				toPos = MSVCTypeName.size() - fromPos;
			}
			else
			{
				toPos = MSVCTypeName.size() - toPos;
			}

			std::string result = MSVCTypeName.substr(fromPos + 1, toPos);

			result = std::regex_replace(result, std::regex(R"(((\bclass\b)|(\bstruct\b))\s*)"), "");

			return result;
		}

	protected:		

		UUID(const std::type_info& aType)
			: myRTTInfo(aType), myOwnerTypeName(PopulateTypeName())
		{ }

		void SetUUID(const std::string& aNewUUID)
		{
			myUUID = aNewUUID;
		}

	public:

		UUID()
			: myUUID("Invalid"), myRTTInfo(typeid(std::nullptr_t)), myOwnerTypeName("Invalid")
		{  }

		UUID(const UUID& other)
			: myUUID(other.myUUID), myRTTInfo(other.myRTTInfo), myOwnerTypeName(other.myOwnerTypeName)
		{	}

		UUID(UUID&& other) noexcept
			: UID(std::move(other)), myUUID(std::move(other.myUUID)), myRTTInfo(other.myRTTInfo), myOwnerTypeName(other.myOwnerTypeName)
		{	}

		__forceinline std::string GetUUID() const { return myUUID; }
		__forceinline std::string GetTypeName() const { return myOwnerTypeName; }
		__forceinline const std::type_info& GetRTTId() const { return myRTTInfo; }
	};

	// Quick macro to fetch the provided object as a Munin::UUID pointer.
	#define AsUUIDAwarePtr(O) dynamic_cast<const Munin::UUID*>(O)
	#define AsUUIDAwareSharedPtr(O) std::dynamic_pointer_cast<const Munin::UUID>(O)

	/**
	 * \brief Represents a Universally Unique IDentifier based on the class name and the instance used.
	 * Note: If this causes problems with UUID in rpc.h you need to #define WIN32_LEAN_AND_MEAN before
	 * you include windows.h!
	 * \tparam Class The class we should base the UUID on.
	 */
	template<typename Class>
	struct ObjectUUID : public UUID
	{
	private:
		static inline size_t nextInstanceId = 0;
		//const std::type_info& myRTTInfo = typeid(Class);
		//const std::string myOwnerTypeName = PopulateTypeName();

	public:

		// Create a new UUID based on our owning type.
		// The ## is to allow the name to be used in ImGui without showing up in text fields.
		ObjectUUID()
			: UUID(typeid(Class))
		{
			SetUUID("##" + GetTypeName() + "_" + std::to_string(nextInstanceId++));
		}

		// Create a new UUID based on our owning type.
		// The ## is to allow the name to be used in ImGui without showing up in text fields.
		ObjectUUID(const ObjectUUID& other)
			: UUID(other)
		{
			SetUUID("##" + GetTypeName() + "_" + std::to_string(nextInstanceId++));
		}

		ObjectUUID(ObjectUUID&& other) noexcept
			: UUID(std::move(other))
		{  }
	};
}

template<>
struct std::hash<Munin::UID>
{
	auto operator()(const Munin::UID& aUID) const noexcept -> size_t
	{
		return std::hash<size_t>{}(aUID.GetUID());
	}
};