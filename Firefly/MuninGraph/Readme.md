MuninGraph
==========
A generic system for building graph-like structures. Does very little on its own but provides a boiler plate for putting together other graph structures. ScriptGraph is provided as a useful example of something that can be done using the base classes.

At the very least you need to create your own classes based on NodeGraph, NodeGraphNode and NodeGraphSchema.
 * NodeGraph provides a baes for the graph itself. Classes deriving from this should provide only functionality that is required for their graph type. For ScriptGraph it only provides methods for starting execution at some node in the graph. Should not handle any assembly logic at all!
 * NodeGraphNode is a base for the nodes that populate the NodeGraph classes. These are your bread and butter for your own graph and should be derived from accordingly. ScriptGraph uses ScriptGraphNode as its base class which in turn derives from NodeGraphNode.
 * NodeGraphSchema is an interface that provides signtures responsible for interacting with a graph. All code that involves changing, updating or otherwise modifying a NodeGraph should go here since it's code that isn't needed during normal runtime or execution of a graph. See ScriptGraphSchema for example. Each Schema instance only handles a single NodeGraph, and you can request a schema to manipulate a graph object using NodeGraph::GetGraphSchema(). This will return a uniqueptr to your specific Schema class.

 All graph objects require their own UIDs and UUIDs to work. This is provided by the UUID class. Both the UID and UUID types are _only unique per execution of the program_ but provides enough information to assist with saving and loading of objects between executions provided you keep this in mind. This is by design. It's recommended to not derive from ObjectUUID in a base class but instead do it form derived classes to provide further uniqueness to these derived classes. See ScriptGraphNode and its derivatives for example. If an object derives from ObjectUUID you can use the AsUUIDAwarePtr and AsUUIDAwareSharedPtr macros to cast to a base where you can access UUID information, even if the base class you cast from doesn't inherit from ObjectUUID.

 ScriptGraph
 ===========
 This is a system for visual scripting in a blueprint-like fashion. It provides an easy interface for creating new nodes and using any type for pins, both native C++ type and user-created. Basic functionality is provided including variables, serialization and deserialization of the graph. Each graph also supports multiple entry points and can serve as a base class for making more specialized graphs (I.e. ObjectScriptGraph, LevelScriptGraph, etc). Nodes created with the macro BeginScriptGraphNode() are automatically registered. See SGNode_MathOps.h for example.

 ScriptGraphTypeRegistry.inl
 ----------------------------
 Here you should register types that you want to use in your ScriptGraph. Types registered here can be used in both Pins and Variables. Place any includes you need for your Types in the Include Section near the top of the file and use the provided documentation in the file to register types. The most basic example is:
 
 BeginDataTypeHandler(Float, float, ImVec4(181, 230, 29, 255), true)
 EndDataTypeHandler

 More information is in the comments of this file.
