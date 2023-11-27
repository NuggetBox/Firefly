#pragma once
#include <functional>
#include <string>

namespace FMOD
{
	namespace Studio
	{
		class EventInstance;
	}
}

typedef enum SERESULT
{
    OK,
    ERR_BADCOMMAND,
    ERR_CHANNEL_ALLOC,
    ERR_CHANNEL_STOLEN,
    ERR_DMA,
    ERR_DSP_CONNECTION,
    ERR_DSP_DONTPROCESS,
    ERR_DSP_FORMAT,
    ERR_DSP_INUSE,
    ERR_DSP_NOTFOUND,
    ERR_DSP_RESERVED,
    ERR_DSP_SILENCE,
    ERR_DSP_TYPE,
    ERR_FILE_BAD,
    ERR_FILE_COULDNOTSEEK,
    ERR_FILE_DISKEJECTED,
    ERR_FILE_EOF,
    ERR_FILE_ENDOFDATA,
    ERR_FILE_NOTFOUND,
    ERR_FORMAT,
    ERR_HEADER_MISMATCH,
    ERR_HTTP,
    ERR_HTTP_ACCESS,
    ERR_HTTP_PROXY_AUTH,
    ERR_HTTP_SERVER_ERROR,
    ERR_HTTP_TIMEOUT,
    ERR_INITIALIZATION,
    ERR_INITIALIZED,
    ERR_INTERNAL,
    ERR_INVALID_FLOAT,
    ERR_INVALID_HANDLE,
    ERR_INVALID_PARAM,
    ERR_INVALID_POSITION,
    ERR_INVALID_SPEAKER,
    ERR_INVALID_SYNCPOINT,
    ERR_INVALID_THREAD,
    ERR_INVALID_VECTOR,
    ERR_MAXAUDIBLE,
    ERR_MEMORY,
    ERR_MEMORY_CANTPOINT,
    ERR_NEEDS3D,
    ERR_NEEDSHARDWARE,
    ERR_NET_CONNECT,
    ERR_NET_SOCKET_ERROR,
    ERR_NET_URL,
    ERR_NET_WOULD_BLOCK,
    ERR_NOTREADY,
    ERR_OUTPUT_ALLOCATED,
    ERR_OUTPUT_CREATEBUFFER,
    ERR_OUTPUT_DRIVERCALL,
    ERR_OUTPUT_FORMAT,
    ERR_OUTPUT_INIT,
    ERR_OUTPUT_NODRIVERS,
    ERR_PLUGIN,
    ERR_PLUGIN_MISSING,
    ERR_PLUGIN_RESOURCE,
    ERR_PLUGIN_VERSION,
    ERR_RECORD,
    ERR_REVERB_CHANNELGROUP,
    ERR_REVERB_INSTANCE,
    ERR_SUBSOUNDS,
    ERR_SUBSOUND_ALLOCATED,
    ERR_SUBSOUND_CANTMOVE,
    ERR_TAGNOTFOUND,
    ERR_TOOMANYCHANNELS,
    ERR_TRUNCATED,
    ERR_UNIMPLEMENTED,
    ERR_UNINITIALIZED,
    ERR_UNSUPPORTED,
    ERR_VERSION,
    ERR_EVENT_ALREADY_LOADED,
    ERR_EVENT_LIVEUPDATE_BUSY,
    ERR_EVENT_LIVEUPDATE_MISMATCH,
    ERR_EVENT_LIVEUPDATE_TIMEOUT,
    ERR_EVENT_NOTFOUND,
    ERR_STUDIO_UNINITIALIZED,
    ERR_STUDIO_NOT_LOADED,
    ERR_INVALID_STRING,
    ERR_ALREADY_LOCKED,
    ERR_NOT_LOCKED,
    ERR_RECORD_DISCONNECTED,
    ERR_TOOMANYSAMPLES,

    SERESULT_FORCEINT = 65536
} SERESULT;

typedef enum CALLBACKTYPE
{
    CALLBACKTYPE_CREATED = 0x00000001,
    CALLBACKTYPE_DESTROYED = 0x00000002,
    CALLBACKTYPE_STARTING = 0x00000004,
    CALLBACKTYPE_STARTED = 0x00000008,
    CALLBACKTYPE_RESTARTED = 0x00000010,
    CALLBACKTYPE_STOPPED = 0x00000020,
    CALLBACKTYPE_START_FAILED = 0x00000040,
    CALLBACKTYPE_CREATE_PROGRAMMER_SOUND = 0x00000080,
    CALLBACKTYPE_DESTROY_PROGRAMMER_SOUND = 0x00000100,
    CALLBACKTYPE_PLUGIN_CREATED = 0x00000200,
    CALLBACKTYPE_PLUGIN_DESTROYED = 0x00000400,
    CALLBACKTYPE_TIMELINE_MARKER = 0x00000800,
    CALLBACKTYPE_TIMELINE_BEAT = 0x00001000,
    CALLBACKTYPE_SOUND_PLAYED = 0x00002000,
    CALLBACKTYPE_SOUND_STOPPED = 0x00004000,
    CALLBACKTYPE_REAL_TO_VIRTUAL = 0x00008000,
    CALLBACKTYPE_VIRTUAL_TO_REAL = 0x00010000,
    CALLBACKTYPE_START_EVENT_COMMAND = 0x00020000,
    CALLBACKTYPE_NESTED_TIMELINE_BEAT = 0x00040000,
    CALLBACKTYPE_ANY = 0xFFFFFFFF
} CALLBACKTYPE;

template<class Class>
class FNginCallback
{
public:
    // This declares the type Func with void args.
    typedef void (Class::* Raw)(CALLBACKTYPE aCallbackType, const std::string& anEventName);
};

struct EventCallbackBase
{
    int CallbackTypes;
    EventCallbackBase(int aType) : CallbackTypes(aType) {}
    virtual ~EventCallbackBase() = default;
    virtual void Broadcast(CALLBACKTYPE aCallbackType, const std::string& anEventName) {}
    virtual bool IsValid() { return false; }
};

//template<class Class> // Brokey
//struct EventCallbackRaw : public EventCallbackBase
//{
//    std::weak_ptr<Class> CallbackOwner;
//    Class* CallbackOwner;
//    typename SoundEngineCallback<Class>::Raw Callback;
//
//    EventCallbackRaw(std::shared_ptr<Class> ownerPtr, int aType, typename SoundEngineCallback<Class>::Raw func)
//        : EventCallbackBase(aType), CallbackOwner(ownerPtr), Callback(func)
//    {
//    }
//
//    void Broadcast(CALLBACKTYPE aCallbackType, const std::string& anEventName) override
//    {
//        if (!CallbackOwner.expired())
//        {
//            std::shared_ptr<Class> ptr = CallbackOwner.lock();
//            (*(ptr.get()).*Callback)(aCallbackType, anEventName);
//        }
//    }
//
//    bool IsValid() override
//    {
//        return CallbackOwner.expired();
//    }
//};

struct EventCallbackLambda : public EventCallbackBase
{
    typedef std::function<void(CALLBACKTYPE, const std::string&)> Lambda;

    Lambda aFunction;

    EventCallbackLambda(int aType, Lambda aLambda)
        : EventCallbackBase(aType), aFunction(std::move(aLambda))
    {
    }

    void Broadcast(CALLBACKTYPE aCallbackType, const std::string& anEventName) override
    {
        aFunction(aCallbackType, anEventName);
    }

    bool IsValid() override
    {
        return true;
    }
};

struct SoundEventInstanceHandle
{
    friend class FNgin;

private:
    std::string myEventName;
    int myInstance = -1;
    FMOD::Studio::EventInstance* myEventInstance = nullptr;


    SoundEventInstanceHandle(const std::string& anEvent, int aHandleId);

public:
    SoundEventInstanceHandle() = default;
    static SoundEventInstanceHandle InvalidHandle;

    void SetId(int id) { myInstance = id; }
    void SetEvent(const std::string& aEvent) { myEventName = aEvent; }

    void SetEventInstance(FMOD::Studio::EventInstance* aEvent) { myEventInstance = aEvent; }

	const int& GetId() const;
    const std::string& GetEvent() const;
    FMOD::Studio::EventInstance* GetEventInstance() const { return myEventInstance; }
    bool IsValid() const;
};