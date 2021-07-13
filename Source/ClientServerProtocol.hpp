/**
 * @file
 * @brief Client-server communication protocol.
 */

#pragma once

#include <string>
#include <map>
#include <deque>
#include <exception>
#include <memory>

#include "Types.hpp"
#include "KString.hpp"
#include "Sockets.hpp"
#include "SocketReaderWriter.hpp"

#include "rapidjson.hpp"
#include "rapidjson/document.h"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/stream.h"
#include "rapidjson/error/en.h"

enum struct EClientMsg
{
	UNKNOWN,

	HELLO,
	DISCONNECT,
	REQUEST_DATA
};

enum struct EServerMsg
{
	UNKNOWN,

	BANNER,
	HELLO,
	DISCONNECT,
	UPDATE_TICK,
	DATA_STATUS,
	DATA
};

enum struct EDisconnectReason
{
	UNKNOWN,

	TIMEOUT,
	USER_DECISION,
	SERVER_QUIT,
	VERSION_MISMATCH,  //!< Incompatible protocol version.
	CONNECTION_LOST,
	PROTOCOL_ERROR,
	SOCKET_ERROR
};

enum struct ESessionState
{
	DISCONNECTED,
	CLOSING,
	OPENING,
	CONNECTED
};

template<class E>
struct SerializedMessage
{
	using MsgEnum = E;

private:
	std::string m_msg;
	MsgEnum m_type;

	SerializedMessage(std::string && msg, MsgEnum type)
	: m_msg(std::move(msg)),
	  m_type(type)
	{
	}

	friend struct ClientServerProtocol;

public:
	SerializedMessage()
	: m_msg(),
	  m_type(MsgEnum::UNKNOWN)
	{
	}

	const std::string & getString() const
	{
		return m_msg;
	}

	MsgEnum getType() const
	{
		return m_type;
	}

	bool isEmpty() const
	{
		return m_type == MsgEnum::UNKNOWN;
	}

	// StreamSocketWriter

	const char *c_str() const
	{
		return m_msg.c_str();
	}

	size_t length() const
	{
		return m_msg.length();
	}

	void clear()
	{
		m_type = MsgEnum::UNKNOWN;
		m_msg.clear();
	}
};

using SerializedClientMessage = SerializedMessage<EClientMsg>;
using SerializedServerMessage = SerializedMessage<EServerMsg>;

struct ClientServerProtocol
{
	static constexpr int VERSION = 1;

private:
	std::map<KString, EClientMsg> m_clientMsgMap;
	std::map<KString, EServerMsg> m_serverMsgMap;
	std::map<KString, EDisconnectReason> m_disconnectReasonMap;

public:
	ClientServerProtocol();

	KString getClientMsgName(EClientMsg msg) const;
	KString getServerMsgName(EServerMsg msg) const;
	KString getDisconnectReasonName(EDisconnectReason reason) const;

	EClientMsg getClientMsgEnum(const KString & msg) const;
	EServerMsg getServerMsgEnum(const KString & msg) const;
	EDisconnectReason getDisconnectReasonEnum(const KString & reason) const;

	SerializedClientMessage createClientMsg_HELLO() const;
	SerializedClientMessage createClientMsg_DISCONNECT(EDisconnectReason reason) const;
	SerializedClientMessage createClientMsg_REQUEST_DATA(int dataFlags, int dataUpdateFlags) const;

	SerializedServerMessage createServerMsg_BANNER() const;
	SerializedServerMessage createServerMsg_HELLO() const;
	SerializedServerMessage createServerMsg_DISCONNECT(EDisconnectReason reason) const;
	SerializedServerMessage createServerMsg_UPDATE_TICK(uint32_t timestamp) const;
	SerializedServerMessage createServerMsg_DATA_STATUS(int dataFlags, int dataUpdateFlags) const;
	SerializedServerMessage createServerMsg_DATA(int type, bool isUpdate, rapidjson::Document && data) const;

	static KString SessionStateToString(ESessionState state);
};

class MessageParserException : public std::exception
{
public:
	enum EType
	{
		JSON_PARSE_ERROR,
		INTERNAL_ERROR
	};

	enum EInternalError
	{
		MSG_TOKEN_TOO_BIG,
		MSG_DESERIALIZATION_FAILED
	};

private:
	EType m_type;
	int m_errorCode;
	std::string m_what;

	static KString InternalErrorToString(EInternalError error);

public:
	MessageParserException(rapidjson::ParseErrorCode errorCode)
	: m_type(JSON_PARSE_ERROR),
	  m_errorCode(errorCode)
	{
		m_what = "JSON parse error: ";
		m_what += rapidjson::GetParseError_En(errorCode);
	}

	MessageParserException(EInternalError error)
	: m_type(INTERNAL_ERROR),
	  m_errorCode(error)
	{
		m_what = InternalErrorToString(error);
	}

	const char *what() const noexcept override
	{
		return m_what.c_str();
	}

	EType getType() const
	{
		return m_type;
	}

	int getErrorCode() const
	{
		return m_errorCode;
	}

	const std::string & getString() const
	{
		return m_what;
	}
};

class MessageParserHandler
{
public:
	using Ch = char;

private:
	rapidjson::internal::Stack<rapidjson::CrtAllocator> m_stack;
	rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> m_allocator;

	static rapidjson::CrtAllocator s_baseAllocator;

public:
	MessageParserHandler();

	void init();
	void popLastValue();
	rapidjson::Value *get();

	bool Null();
	bool Bool(bool value);
	bool Int(int value);
	bool Uint(unsigned value);
	bool Int64(int64_t value);
	bool Uint64(uint64_t value);
	bool Double(double value);
	bool RawNumber(const Ch *numberString, rapidjson::SizeType length, bool copy);
	bool String(const Ch *string, rapidjson::SizeType length, bool copy);
	bool StartObject();
	bool Key(const Ch *name, rapidjson::SizeType length, bool copy);
	bool EndObject(rapidjson::SizeType memberCount);
	bool StartArray();
	bool EndArray(rapidjson::SizeType elementCount);
};

template<class Callback>
class MessageParser
{
	MessageParserHandler m_handler;
	rapidjson::Reader m_reader;
	Callback *m_callback;

	size_t doParse(rapidjson::StringStream & stream)
	{
		constexpr int PARSER_FLAGS = rapidjson::kParseStopWhenDoneFlag | rapidjson::kParseChunkMode;

		size_t parsedLength = stream.Tell();
		while (!m_reader.IterativeParseComplete())
		{
			if (m_reader.IterativeParseNext<PARSER_FLAGS>(stream, m_handler))
			{
				parsedLength = stream.Tell();
			}
			else
			{
				if (m_reader.HasParseError())
				{
					rapidjson::ParseErrorCode errorCode = m_reader.GetParseErrorCode();
					init();
					throw MessageParserException(errorCode);
				}
				else
				{
					if (m_reader.IsLastValueInvalid())
					{
						m_handler.popLastValue();
					}
					return parsedLength;
				}
			}
		}

		rapidjson::Value *pMessage = m_handler.get();
		if (pMessage)
		{
			m_callback->onMessage(*pMessage);
			init();
		}
		else
		{
			init();
			throw MessageParserException(MessageParserException::MSG_DESERIALIZATION_FAILED);
		}

		return parsedLength;
	}

public:
	MessageParser(Callback *callback)
	: m_handler(),
	  m_reader(),
	  m_callback(callback)
	{
	}

	void init()
	{
		m_handler.init();
		m_reader.IterativeParseInit();
	}

	size_t parse(const char *data, size_t length, size_t bufferSize)
	{
		rapidjson::StringStream stream(data);

		size_t parsedLength = 0;
		while (stream.Tell() < length)
		{
			if (stream.Peek() == '\0')
			{
				stream.Take();
				parsedLength = stream.Tell();
				continue;
			}

			parsedLength = doParse(stream);
		}

		if (parsedLength == 0 && length >= (bufferSize-1))
		{
			init();
			throw MessageParserException(MessageParserException::MSG_TOKEN_TOO_BIG);
		}

		return parsedLength;
	}
};

class ClientSession;
class ServerSession;

using ClientMessageParser = MessageParser<ServerSession>;
using ServerMessageParser = MessageParser<ClientSession>;

struct IClientSessionCallback
{
	virtual void onSessionConnectionEstablished(ClientSession *session) = 0;
	virtual void onSessionEstablished(ClientSession *session) = 0;
	virtual void onSessionDisconnect(ClientSession *session) = 0;
	virtual void onSessionServerTick(ClientSession *session) = 0;
	virtual void onSessionDataStatus(ClientSession *session, bool isDifferent) = 0;
	virtual void onSessionData(ClientSession *session, int type, bool isUpdate, rapidjson::Value & data) = 0;
};

struct IServerSessionCallback
{
	virtual void onSessionEstablished(ServerSession *session) = 0;
	virtual void onSessionDisconnect(ServerSession *session) = 0;
	virtual void onSessionDataRequest(ServerSession *session, int dataFlags, int dataUpdateFlags) = 0;
};

/**
 * @brief Client-side context shared by all client sessions.
 */
class ClientContext
{
	ClientServerProtocol m_proto;
	IClientSessionCallback *m_sessionCallback;

public:
	ClientContext(IClientSessionCallback *sessionCallback)
	: m_proto(),
	  m_sessionCallback(sessionCallback)
	{
	}

	const ClientServerProtocol & getProtocol() const
	{
		return m_proto;
	}

	IClientSessionCallback *getSessionCallback() const
	{
		return m_sessionCallback;
	}
};

/**
 * @brief Server-side context shared by all server sessions.
 */
class ServerContext
{
	ClientServerProtocol m_proto;
	IServerSessionCallback *m_sessionCallback;
	uint32_t m_timestamp;
	SerializedServerMessage m_cachedMsgBanner;
	SerializedServerMessage m_cachedMsgHello;
	SerializedServerMessage m_cachedMsgUpdateTick;

public:
	ServerContext(IServerSessionCallback *sessionCallback)
	: m_proto(),
	  m_sessionCallback(sessionCallback),
	  m_timestamp(0)
	{
		m_cachedMsgBanner = m_proto.createServerMsg_BANNER();
		m_cachedMsgHello = m_proto.createServerMsg_HELLO();
		m_cachedMsgUpdateTick = m_proto.createServerMsg_UPDATE_TICK(m_timestamp);
	}

	void onUpdate()
	{
		m_timestamp++;
		m_cachedMsgUpdateTick = m_proto.createServerMsg_UPDATE_TICK(m_timestamp);
	}

	const ClientServerProtocol & getProtocol() const
	{
		return m_proto;
	}

	IServerSessionCallback *getSessionCallback() const
	{
		return m_sessionCallback;
	}

	uint32_t getTimestamp() const
	{
		return m_timestamp;
	}

	const SerializedServerMessage & getCachedBannerMsg() const
	{
		return m_cachedMsgBanner;
	}

	const SerializedServerMessage & getCachedHelloMsg() const
	{
		return m_cachedMsgHello;
	}

	const SerializedServerMessage & getCachedUpdateTickMsg() const
	{
		return m_cachedMsgUpdateTick;
	}
};

class ClientSession
{
	enum struct EExpectedMsg
	{
		NONE,
		SERVER_BANNER,
		SERVER_HELLO,
		SERVER_DATA_STATUS
	};

	const ClientContext *m_context;
	StreamSocket m_socket;
	StreamSocketReader<ServerMessageParser> m_socketReader;
	StreamSocketWriter<SerializedClientMessage> m_socketWriter;
	std::deque<SerializedClientMessage> m_sendQueue;
	ESessionState m_state;
	EExpectedMsg m_expectedMsg;
	bool m_isConnectionEstablished;
	int m_sessionTTL;
	int m_dataFlags;
	int m_dataUpdateFlags;
	uint32_t m_currentTimestamp;
	std::string m_serverName;
	std::string m_serverVersion;
	std::string m_serverPlatformName;
	std::string m_serverHost;
	SocketEndpoint m_serverEndpoint;
	EDisconnectReason m_disconnectReason;
	std::string m_disconnectError;

	void sendMessage(SerializedClientMessage && msg);
	void quickDisconnect(EDisconnectReason reason, const char *error = nullptr);
	void onMessage(rapidjson::Value & message);  // ServerMessageParser callback

	static void SocketPollHandler(int flags, void *param);

	friend ServerMessageParser;  // onMessage function is private

public:
	ClientSession(const ClientContext & context);

	// no copy
	ClientSession(const ClientSession &) = delete;
	ClientSession & operator=(const ClientSession &) = delete;

	// move allowed
	ClientSession(ClientSession &&) = default;
	ClientSession & operator=(ClientSession &&) = default;

	~ClientSession();

	ESessionState getState() const
	{
		return m_state;
	}

	const StreamSocket & getSocket() const
	{
		return m_socket;
	}

	const std::string & getServerName() const
	{
		return m_serverName;
	}

	const std::string & getServerVersionString() const
	{
		return m_serverVersion;
	}

	const std::string & getServerPlatformName() const
	{
		return m_serverPlatformName;
	}

	const std::string & getServerHostString() const
	{
		return m_serverHost;
	}

	const SocketEndpoint & getServerEndpoint() const
	{
		return m_serverEndpoint;
	}

	EDisconnectReason getDisconnectReason() const
	{
		return m_disconnectReason;
	}

	KString getDisconnectReasonName() const
	{
		return m_context->getProtocol().getDisconnectReasonName(m_disconnectReason);
	}

	bool hasDisconnectError() const
	{
		return !m_disconnectError.empty();
	}

	const std::string & getDisconnectErrorString() const
	{
		return m_disconnectError;
	}

	int getSessionTTL() const
	{
		return m_sessionTTL;
	}

	int getDataFlags() const
	{
		return m_dataFlags;
	}

	int getDataUpdateFlags() const
	{
		return m_dataUpdateFlags;
	}

	uint32_t getCurrentTimestamp() const
	{
		return m_currentTimestamp;
	}

	bool isConnectionEstablished() const
	{
		return m_isConnectionEstablished;
	}

	bool isDataRequestInProgress() const
	{
		return m_expectedMsg == EExpectedMsg::SERVER_DATA_STATUS;
	}

	void setServerHostString(const KString & host)
	{
		m_serverHost = host;
	}

	void onUpdate();

	void connect(StreamSocket && socket);
	void disconnect();
	void requestData(int dataFlags, int dataUpdateFlags);
};

class ServerSession
{
	class SendQueueMsgDeleter
	{
		bool m_isShared;

	public:
		SendQueueMsgDeleter(bool isShared = false)
		: m_isShared(isShared)
		{
		}

		bool isShared() const
		{
			return m_isShared;
		}

		void operator()(SerializedServerMessage *pMsg) const
		{
			if (!m_isShared)
			{
				delete pMsg;
			}
		}
	};

	const ServerContext *m_context;
	StreamSocket m_socket;
	StreamSocketReader<ClientMessageParser> m_socketReader;
	StreamSocketWriter<SerializedServerMessage> m_socketWriter;
	std::deque<std::unique_ptr<SerializedServerMessage, SendQueueMsgDeleter>> m_sendQueue;
	ESessionState m_state;
	bool m_isSendingData;
	int m_sessionTTL;
	int m_dataFlags;
	int m_dataUpdateFlags;
	std::string m_clientName;
	std::string m_clientVersion;
	std::string m_clientPlatformName;
	SocketEndpoint m_clientEndpoint;
	EDisconnectReason m_disconnectReason;
	std::string m_disconnectError;

	void sendMessage(SerializedServerMessage && msg);
	void sendSharedMessage(const SerializedServerMessage & msg);
	void quickDisconnect(EDisconnectReason reason, const char *error = nullptr);
	void onMessage(rapidjson::Value & message);  // ClientMessageParser callback

	static void SocketPollHandler(int flags, void *param);

	friend ClientMessageParser;  // onMessage function is private

public:
	ServerSession(StreamSocket && socket, const ServerContext & context);

	// no copy
	ServerSession(const ServerSession &) = delete;
	ServerSession & operator=(const ServerSession &) = delete;

	// move allowed
	ServerSession(ServerSession &&) = default;
	ServerSession & operator=(ServerSession &&) = default;

	~ServerSession();

	ESessionState getState() const
	{
		return m_state;
	}

	const StreamSocket & getSocket() const
	{
		return m_socket;
	}

	const std::string & getClientName() const
	{
		return m_clientName;
	}

	const std::string & getClientVersionString() const
	{
		return m_clientVersion;
	}

	const std::string & getClientPlatformName() const
	{
		return m_clientPlatformName;
	}

	const SocketEndpoint & getClientEndpoint() const
	{
		return m_clientEndpoint;
	}

	EDisconnectReason getDisconnectReason() const
	{
		return m_disconnectReason;
	}

	KString getDisconnectReasonName() const
	{
		return m_context->getProtocol().getDisconnectReasonName(m_disconnectReason);
	}

	bool hasDisconnectError() const
	{
		return !m_disconnectError.empty();
	}

	const std::string & getDisconnectErrorString() const
	{
		return m_disconnectError;
	}

	int getSessionTTL() const
	{
		return m_sessionTTL;
	}

	int getDataFlags() const
	{
		return m_dataFlags;
	}

	int getDataUpdateFlags() const
	{
		return m_dataUpdateFlags;
	}

	bool isSendingData() const
	{
		return m_isSendingData;
	}

	void onUpdate();

	void disconnect();
	void sendDataStatus(int dataFlags, int dataUpdateFlags);
	void sendData(const SerializedServerMessage & dataMsg);
	bool stopSendingData();
};

template<class T>
class ServerDataSerializer
{
	rapidjson::Document m_itemArrayDocument;

protected:
	ServerDataSerializer()
	: m_itemArrayDocument(rapidjson::kArrayType)
	{
	}

	template<class... Args>
	void addItem(const T & item, Args &&... serializeArgs)
	{
		auto & allocator = m_itemArrayDocument.GetAllocator();
		rapidjson::Document itemDocument(rapidjson::kObjectType, &allocator);

		item.serialize(itemDocument, std::forward<Args>(serializeArgs)...);

		m_itemArrayDocument.PushBack(itemDocument, allocator);
	}

	SerializedServerMessage buildMessage(const ClientServerProtocol & protocol, int dataType, bool isUpdate)
	{
		auto message = protocol.createServerMsg_DATA(dataType, isUpdate, std::move(m_itemArrayDocument));
		m_itemArrayDocument.SetArray();
		m_itemArrayDocument.GetAllocator().Clear();
		return message;
	}

public:
	bool isEmpty() const
	{
		return m_itemArrayDocument.Empty();
	}

	void clear()
	{
		m_itemArrayDocument.SetArray();
	}
};
