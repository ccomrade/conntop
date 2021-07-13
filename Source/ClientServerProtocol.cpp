/**
 * @file
 * @brief Implementation of client-server communication protocol.
 */

#include <stdexcept>
#include <new>

#include "ClientServerProtocol.hpp"
#include "App.hpp"
#include "Log.hpp"
#include "Platform.hpp"
#include "CmdLine.hpp"
#include "conntop_config.h"  // CONNTOP_VERSION_STRING

using rapidjson::Value;
using rapidjson::Document;

static std::string CreateMsgString(const rapidjson::Value & message)
{
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

	message.Accept(writer);

	std::string data;
	data.reserve(buffer.GetLength() + 1);  // newline character
	data.append(buffer.GetString(), buffer.GetLength());
	data += '\n';

	return data;
}

ClientServerProtocol::ClientServerProtocol()
: m_clientMsgMap(),
  m_serverMsgMap(),
  m_disconnectReasonMap()
{
	m_clientMsgMap["HELLO"]                   = EClientMsg::HELLO;
	m_clientMsgMap["DISCONNECT"]              = EClientMsg::DISCONNECT;
	m_clientMsgMap["REQUEST_DATA"]            = EClientMsg::REQUEST_DATA;

	m_serverMsgMap["BANNER"]                  = EServerMsg::BANNER;
	m_serverMsgMap["HELLO"]                   = EServerMsg::HELLO;
	m_serverMsgMap["DISCONNECT"]              = EServerMsg::DISCONNECT;
	m_serverMsgMap["UPDATE_TICK"]             = EServerMsg::UPDATE_TICK;
	m_serverMsgMap["DATA_STATUS"]             = EServerMsg::DATA_STATUS;
	m_serverMsgMap["DATA"]                    = EServerMsg::DATA;

	m_disconnectReasonMap["TIMEOUT"]          = EDisconnectReason::TIMEOUT;
	m_disconnectReasonMap["USER_DECISION"]    = EDisconnectReason::USER_DECISION;
	m_disconnectReasonMap["SERVER_QUIT"]      = EDisconnectReason::SERVER_QUIT;
	m_disconnectReasonMap["VERSION_MISMATCH"] = EDisconnectReason::VERSION_MISMATCH;
	m_disconnectReasonMap["CONNECTION_LOST"]  = EDisconnectReason::CONNECTION_LOST;
	m_disconnectReasonMap["PROTOCOL_ERROR"]   = EDisconnectReason::PROTOCOL_ERROR;
	m_disconnectReasonMap["SOCKET_ERROR"]     = EDisconnectReason::SOCKET_ERROR;
}

KString ClientServerProtocol::getClientMsgName(EClientMsg msg) const
{
	switch (msg)
	{
		case EClientMsg::UNKNOWN:      break;
		case EClientMsg::HELLO:        return "HELLO";
		case EClientMsg::DISCONNECT:   return "DISCONNECT";
		case EClientMsg::REQUEST_DATA: return "REQUEST_DATA";
	}
	return "?";
}

KString ClientServerProtocol::getServerMsgName(EServerMsg msg) const
{
	switch (msg)
	{
		case EServerMsg::UNKNOWN:     break;
		case EServerMsg::BANNER:      return "BANNER";
		case EServerMsg::HELLO:       return "HELLO";
		case EServerMsg::DISCONNECT:  return "DISCONNECT";
		case EServerMsg::UPDATE_TICK: return "UPDATE_TICK";
		case EServerMsg::DATA_STATUS: return "DATA_STATUS";
		case EServerMsg::DATA:        return "DATA";
	}
	return "?";
}

KString ClientServerProtocol::getDisconnectReasonName(EDisconnectReason reason) const
{
	switch (reason)
	{
		case EDisconnectReason::UNKNOWN:          break;
		case EDisconnectReason::TIMEOUT:          return "TIMEOUT";
		case EDisconnectReason::USER_DECISION:    return "USER_DECISION";
		case EDisconnectReason::SERVER_QUIT:      return "SERVER_QUIT";
		case EDisconnectReason::VERSION_MISMATCH: return "VERSION_MISMATCH";
		case EDisconnectReason::CONNECTION_LOST:  return "CONNECTION_LOST";
		case EDisconnectReason::PROTOCOL_ERROR:   return "PROTOCOL_ERROR";
		case EDisconnectReason::SOCKET_ERROR:     return "SOCKET_ERROR";
	}
	return "?";
}

EClientMsg ClientServerProtocol::getClientMsgEnum(const KString & msg) const
{
	auto it = m_clientMsgMap.find(msg);
	return (it != m_clientMsgMap.end()) ? it->second : EClientMsg::UNKNOWN;
}

EServerMsg ClientServerProtocol::getServerMsgEnum(const KString & msg) const
{
	auto it = m_serverMsgMap.find(msg);
	return (it != m_serverMsgMap.end()) ? it->second : EServerMsg::UNKNOWN;
}

EDisconnectReason ClientServerProtocol::getDisconnectReasonEnum(const KString & reason) const
{
	auto it = m_disconnectReasonMap.find(reason);
	return (it != m_disconnectReasonMap.end()) ? it->second : EDisconnectReason::UNKNOWN;
}

SerializedClientMessage ClientServerProtocol::createClientMsg_HELLO() const
{
	const EClientMsg msgType = EClientMsg::HELLO;
	const KString msgName = getClientMsgName(msgType);
	const KString platformName = gPlatform->getSystemName();

	KString name;
	std::string hostnameBuffer;

	CmdLineArg *nameArg = gCmdLine->getArg("name");
	if (nameArg)
	{
		name = nameArg->getValue();
	}
	else
	{
		hostnameBuffer = gPlatform->getCurrentHostName();
		name = hostnameBuffer;
	}

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("name", Value().SetString(name.c_str(), name.length()), allocator);
	document.AddMember("version", Value().SetString(CONNTOP_VERSION_STRING), allocator);
	document.AddMember("platform", Value().SetString(platformName.c_str(), platformName.length()), allocator);

	return SerializedClientMessage(CreateMsgString(document), msgType);
}

SerializedClientMessage ClientServerProtocol::createClientMsg_DISCONNECT(EDisconnectReason reason) const
{
	const EClientMsg msgType = EClientMsg::DISCONNECT;
	const KString msgName = getClientMsgName(msgType);
	const KString reasonName = getDisconnectReasonName(reason);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("reason", Value().SetString(reasonName.c_str(), reasonName.length()), allocator);

	return SerializedClientMessage(CreateMsgString(document), msgType);
}

SerializedClientMessage ClientServerProtocol::createClientMsg_REQUEST_DATA(int dataFlags, int dataUpdateFlags) const
{
	const EClientMsg msgType = EClientMsg::REQUEST_DATA;
	const KString msgName = getClientMsgName(msgType);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("dataFlags", Value().SetInt(dataFlags), allocator);
	document.AddMember("dataUpdateFlags", Value().SetInt(dataUpdateFlags), allocator);

	return SerializedClientMessage(CreateMsgString(document), msgType);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_BANNER() const
{
	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("conntop_server", Value().SetInt(VERSION), allocator);

	return SerializedServerMessage(CreateMsgString(document), EServerMsg::BANNER);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_HELLO() const
{
	const EServerMsg msgType = EServerMsg::HELLO;
	const KString msgName = getServerMsgName(msgType);
	const KString platformName = gPlatform->getSystemName();

	KString name;
	std::string hostnameBuffer;

	CmdLineArg *nameArg = gCmdLine->getArg("name");
	if (nameArg)
	{
		name = nameArg->getValue();
	}
	else
	{
		hostnameBuffer = gPlatform->getCurrentHostName();
		name = hostnameBuffer;
	}

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("name", Value().SetString(name.c_str(), name.length()), allocator);
	document.AddMember("version", Value().SetString(CONNTOP_VERSION_STRING), allocator);
	document.AddMember("platform", Value().SetString(platformName.c_str(), platformName.length()), allocator);

	return SerializedServerMessage(CreateMsgString(document), msgType);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_DISCONNECT(EDisconnectReason reason) const
{
	const EServerMsg msgType = EServerMsg::DISCONNECT;
	const KString msgName = getServerMsgName(msgType);
	const KString reasonName = getDisconnectReasonName(reason);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("reason", Value().SetString(reasonName.c_str(), reasonName.length()), allocator);

	return SerializedServerMessage(CreateMsgString(document), msgType);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_UPDATE_TICK(uint32_t timestamp) const
{
	const EServerMsg msgType = EServerMsg::UPDATE_TICK;
	const KString msgName = getServerMsgName(msgType);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("timestamp", Value().SetUint(timestamp), allocator);

	return SerializedServerMessage(CreateMsgString(document), msgType);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_DATA_STATUS(int dataFlags, int dataUpdateFlags) const
{
	const EServerMsg msgType = EServerMsg::DATA_STATUS;
	const KString msgName = getServerMsgName(msgType);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("dataFlags", Value().SetInt(dataFlags), allocator);
	document.AddMember("dataUpdateFlags", Value().SetInt(dataUpdateFlags), allocator);

	return SerializedServerMessage(CreateMsgString(document), msgType);
}

SerializedServerMessage ClientServerProtocol::createServerMsg_DATA(int type, bool isUpdate, Document && data) const
{
	const EServerMsg msgType = EServerMsg::DATA;
	const KString msgName = getServerMsgName(msgType);

	Document document(rapidjson::kObjectType);
	auto & allocator = document.GetAllocator();
	document.AddMember("message", Value().SetString(msgName.c_str(), msgName.length()), allocator);
	document.AddMember("type", Value().SetInt(type), allocator);
	document.AddMember("isUpdate", Value().SetBool(isUpdate), allocator);
	document.AddMember("data", data, allocator);

	return SerializedServerMessage(CreateMsgString(document), msgType);
}

KString ClientServerProtocol::SessionStateToString(ESessionState state)
{
	switch (state)
	{
		case ESessionState::DISCONNECTED: return "DISCONNECTED";
		case ESessionState::CLOSING:      return "CLOSING";
		case ESessionState::OPENING:      return "OPENING";
		case ESessionState::CONNECTED:    return "CONNECTED";
	}
	return "?";
}

KString MessageParserException::InternalErrorToString(EInternalError error)
{
	switch (error)
	{
		case MSG_TOKEN_TOO_BIG:          return "Message token is too big";
		case MSG_DESERIALIZATION_FAILED: return "Message deserialization failed";
	}
	return "?";
}

rapidjson::CrtAllocator MessageParserHandler::s_baseAllocator;

MessageParserHandler::MessageParserHandler()
: m_stack(&s_baseAllocator, 1024),  // default stack capacity
  m_allocator(RAPIDJSON_ALLOCATOR_DEFAULT_CHUNK_CAPACITY, &s_baseAllocator)
{
}

void MessageParserHandler::init()
{
	m_stack.Clear();
	m_allocator.Clear();
}

void MessageParserHandler::popLastValue()
{
	Value *pValue = m_stack.Pop<Value>(1);
	pValue->~Value();
}

rapidjson::Value *MessageParserHandler::get()
{
	return (m_stack.GetSize() == sizeof (Value)) ? m_stack.Pop<Value>(1) : nullptr;
}

bool MessageParserHandler::Null()
{
	new (m_stack.Push<Value>()) Value();
	return true;
}

bool MessageParserHandler::Bool(bool value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::Int(int value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::Uint(unsigned value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::Int64(int64_t value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::Uint64(uint64_t value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::Double(double value)
{
	new (m_stack.Push<Value>()) Value(value);
	return true;
}

bool MessageParserHandler::RawNumber(const Ch *numberString, rapidjson::SizeType length, bool copy)
{
	return this->String(numberString, length, copy);
}

bool MessageParserHandler::String(const Ch *string, rapidjson::SizeType length, bool copy)
{
	if (copy)
	{
		new (m_stack.Push<Value>()) Value(string, length, m_allocator);
	}
	else
	{
		new (m_stack.Push<Value>()) Value(string, length);
	}
	return true;
}

bool MessageParserHandler::StartObject()
{
	new (m_stack.Push<Value>()) Value(rapidjson::kObjectType);
	return true;
}

bool MessageParserHandler::Key(const Ch *name, rapidjson::SizeType length, bool copy)
{
	return this->String(name, length, copy);
}

bool MessageParserHandler::EndObject(rapidjson::SizeType memberCount)
{
	using ValueMember = typename Value::Member;

	ValueMember *members = m_stack.Pop<ValueMember>(memberCount);
	Value *pObject = m_stack.Top<Value>();
	pObject->MemberReserve(memberCount, m_allocator);
	for (size_t i = 0; i < memberCount; i++)
	{
		pObject->AddMember(members[i].name, members[i].value, m_allocator);
	}

	return true;
}

bool MessageParserHandler::StartArray()
{
	new (m_stack.Push<Value>()) Value(rapidjson::kArrayType);
	return true;
}

bool MessageParserHandler::EndArray(rapidjson::SizeType elementCount)
{
	Value *elements = m_stack.Pop<Value>(elementCount);
	Value *pArray = m_stack.Top<Value>();
	pArray->Reserve(elementCount, m_allocator);
	for (size_t i = 0; i < elementCount; i++)
	{
		pArray->PushBack(elements[i], m_allocator);
	}
	return true;
}

ClientSession::ClientSession(const ClientContext & context)
: m_context(&context),
  m_socket(),
  m_socketReader(m_socket, ServerMessageParser(this)),
  m_socketWriter(m_socket),
  m_sendQueue(),
  m_state(ESessionState::DISCONNECTED),
  m_expectedMsg(EExpectedMsg::NONE),
  m_isConnectionEstablished(false),
  m_sessionTTL(),
  m_dataFlags(0),
  m_dataUpdateFlags(0),
  m_currentTimestamp(),
  m_serverName(),
  m_serverVersion(),
  m_serverPlatformName(),
  m_serverHost(),
  m_serverEndpoint(),
  m_disconnectReason(),
  m_disconnectError()
{
}

ClientSession::~ClientSession()
{
	if (m_socket.isConnected())
	{
		gApp->getPollSystem()->remove(m_socket);
	}
}

void ClientSession::sendMessage(SerializedClientMessage && msg)
{
	if (m_sendQueue.empty())
	{
		gApp->getPollSystem()->reset(m_socket, EPollFlags::INPUT | EPollFlags::OUTPUT);
	}

	m_sendQueue.emplace_back(std::move(msg));
}

void ClientSession::quickDisconnect(EDisconnectReason reason, const char *error)
{
	if (m_socket.isConnected())
	{
		gApp->getPollSystem()->remove(m_socket);
		m_socket.close_nothrow();
	}
	m_state = ESessionState::DISCONNECTED;
	m_isConnectionEstablished = false;
	m_sendQueue.clear();
	m_disconnectReason = reason;
	m_disconnectError = (error) ? error : "";
	m_context->getSessionCallback()->onSessionDisconnect(this);
}

void ClientSession::onMessage(rapidjson::Value & message)
{
	const ClientServerProtocol & proto = m_context->getProtocol();
	IClientSessionCallback *callback = m_context->getSessionCallback();

	EServerMsg msgType = EServerMsg::UNKNOWN;

	if (!message.IsObject())
		throw std::invalid_argument("Invalid message");

	const auto msgTypeIt = message.FindMember("message");
	if (msgTypeIt != message.MemberEnd() && msgTypeIt->value.IsString())
	{
		msgType = proto.getServerMsgEnum(msgTypeIt->value.GetString());
	}
	else if (message.FindMember("conntop_server") != message.MemberEnd())
	{
		msgType = EServerMsg::BANNER;
	}
	else
	{
		throw std::invalid_argument("Invalid message");
	}

	switch (msgType)
	{
		case EServerMsg::UNKNOWN:
		{
			std::string error = "Unknown message '";
			error += msgTypeIt->value.GetString();
			error += "'";
			throw std::invalid_argument(error);
		}
		case EServerMsg::BANNER:
		{
			if (m_state != ESessionState::OPENING || m_expectedMsg != EExpectedMsg::SERVER_BANNER)
				throw std::invalid_argument("Unexpected BANNER message");

			const auto protocolVersionIt = message.FindMember("conntop_server");
			if (protocolVersionIt == message.MemberEnd())
				throw std::invalid_argument("Missing server identification");
			else if (!protocolVersionIt->value.IsInt())
				throw std::invalid_argument("Invalid server identification value type");

			int serverProtocolVersion = protocolVersionIt->value.GetInt();

			if (serverProtocolVersion != ClientServerProtocol::VERSION)
			{
				quickDisconnect(EDisconnectReason::VERSION_MISMATCH);
			}
			else
			{
				m_expectedMsg = EExpectedMsg::SERVER_HELLO;
				sendMessage(proto.createClientMsg_HELLO());
			}

			break;
		}
		case EServerMsg::HELLO:
		{
			if (m_state != ESessionState::OPENING || m_expectedMsg != EExpectedMsg::SERVER_HELLO)
				throw std::invalid_argument("Unexpected HELLO message");

			const auto nameIt = message.FindMember("name");
			if (nameIt == message.MemberEnd())
				throw std::invalid_argument("Missing server name");
			else if (!nameIt->value.IsString())
				throw std::invalid_argument("Invalid server name value type");

			const auto versionIt = message.FindMember("version");
			if (versionIt == message.MemberEnd())
				throw std::invalid_argument("Missing server version");
			else if (!versionIt->value.IsString())
				throw std::invalid_argument("Invalid server version value type");

			const auto platformNameIt = message.FindMember("platform");
			if (platformNameIt == message.MemberEnd())
				throw std::invalid_argument("Missing server platform name");
			else if (!platformNameIt->value.IsString())
				throw std::invalid_argument("Invalid server platform name value type");

			m_serverName = nameIt->value.GetString();
			m_serverVersion = versionIt->value.GetString();
			m_serverPlatformName = platformNameIt->value.GetString();

			m_expectedMsg = EExpectedMsg::NONE;
			m_state = ESessionState::CONNECTED;
			m_sessionTTL = 3;
			m_currentTimestamp = 0;
			m_dataFlags = 0;
			m_dataUpdateFlags = 0;

			callback->onSessionEstablished(this);

			break;
		}
		case EServerMsg::DISCONNECT:
		{
			const auto reasonIt = message.FindMember("reason");
			if (reasonIt == message.MemberEnd())
				throw std::invalid_argument("Missing disconnect reason");
			else if (!reasonIt->value.IsString())
				throw std::invalid_argument("Invalid disconnect reason value type");

			const char *reasonName = reasonIt->value.GetString();
			EDisconnectReason reason = proto.getDisconnectReasonEnum(reasonName);

			quickDisconnect(reason, (reason == EDisconnectReason::UNKNOWN) ? reasonName : nullptr);

			break;
		}
		case EServerMsg::UPDATE_TICK:
		{
			if (m_state == ESessionState::CLOSING)
				return;

			if (m_state != ESessionState::CONNECTED)
				throw std::invalid_argument("Unexpected UPDATE_TICK message");

			const auto timestampIt = message.FindMember("timestamp");
			if (timestampIt == message.MemberEnd())
				throw std::invalid_argument("Missing update tick timestamp");
			else if (!timestampIt->value.IsUint())
				throw std::invalid_argument("Invalid update tick timestamp value type");

			uint32_t timestamp = timestampIt->value.GetUint();

			if (timestamp != (m_currentTimestamp+1) && m_currentTimestamp != 0)
			{
				throw std::invalid_argument("Unexpected update tick timestamp");
			}
			else
			{
				m_currentTimestamp = timestamp;
			}

			m_sessionTTL++;

			callback->onSessionServerTick(this);

			break;
		}
		case EServerMsg::DATA_STATUS:
		{
			if (m_state == ESessionState::CLOSING)
				return;

			if (m_expectedMsg != EExpectedMsg::SERVER_DATA_STATUS)
				throw std::invalid_argument("Unexpected DATA_STATUS message");

			const auto dataFlagsIt = message.FindMember("dataFlags");
			if (dataFlagsIt == message.MemberEnd())
				throw std::invalid_argument("Missing data status flags");
			else if (!dataFlagsIt->value.IsInt())
				throw std::invalid_argument("Invalid data status flags value type");

			const auto dataUpdateFlagsIt = message.FindMember("dataUpdateFlags");
			if (dataUpdateFlagsIt == message.MemberEnd())
				throw std::invalid_argument("Missing data status update flags");
			else if (!dataUpdateFlagsIt->value.IsInt())
				throw std::invalid_argument("Invalid data status update flags value type");

			int dataFlags = dataFlagsIt->value.GetInt();
			int dataUpdateFlags = dataUpdateFlagsIt->value.GetInt();
			bool isDifferent = m_dataFlags != dataFlags;

			m_dataFlags = dataFlags;
			m_dataUpdateFlags = dataUpdateFlags;
			m_expectedMsg = EExpectedMsg::NONE;
			m_sessionTTL = 3;

			callback->onSessionDataStatus(this, isDifferent);

			break;
		}
		case EServerMsg::DATA:
		{
			if (m_state == ESessionState::CLOSING)
				return;

			if (m_state != ESessionState::CONNECTED)
				throw std::invalid_argument("Unexpected DATA message");

			const auto typeIt = message.FindMember("type");
			if (typeIt == message.MemberEnd())
				throw std::invalid_argument("Missing data type");
			else if (!typeIt->value.IsInt())
				throw std::invalid_argument("Invalid data type value type");

			const auto isUpdateIt = message.FindMember("isUpdate");
			if (isUpdateIt == message.MemberEnd())
				throw std::invalid_argument("Missing data update flag");
			else if (!isUpdateIt->value.IsBool())
				throw std::invalid_argument("Invalid data update flag value type");

			const auto dataIt = message.FindMember("data");
			if (dataIt == message.MemberEnd())
				throw std::invalid_argument("Missing data");

			int type = typeIt->value.GetInt();
			bool isUpdate = isUpdateIt->value.GetBool();

			if (!(type & m_dataFlags))
				throw std::invalid_argument("Unexpected data");

			if (isUpdate && !(type & m_dataUpdateFlags))
				throw std::invalid_argument("Unexpected data update");

			callback->onSessionData(this, type, isUpdate, dataIt->value);

			break;
		}
	}
}

void ClientSession::onUpdate()
{
	if (m_state != ESessionState::DISCONNECTED)
	{
		if (m_sessionTTL > 0)
		{
			m_sessionTTL--;
		}
		else
		{
			quickDisconnect(EDisconnectReason::TIMEOUT);
		}
	}
}

void ClientSession::connect(StreamSocket && socket)
{
	if (m_state == ESessionState::DISCONNECTED && socket.isConnected())
	{
		m_socket = std::move(socket);
		m_state = ESessionState::OPENING;
		m_sessionTTL = 4;
		m_expectedMsg = EExpectedMsg::SERVER_BANNER;
		// OUTPUT flag for checking socket connect status
		gApp->getPollSystem()->add(m_socket, EPollFlags::INPUT | EPollFlags::OUTPUT, SocketPollHandler, this);
	}
}

void ClientSession::disconnect()
{
	const EDisconnectReason reason = EDisconnectReason::USER_DECISION;

	if (m_state == ESessionState::CONNECTED)
	{
		m_state = ESessionState::CLOSING;
		m_sessionTTL = 2;
		m_disconnectReason = reason;
		m_disconnectError = "";
		sendMessage(m_context->getProtocol().createClientMsg_DISCONNECT(reason));
	}
	else if (m_state == ESessionState::OPENING)
	{
		quickDisconnect(reason);
	}
}

void ClientSession::requestData(int dataFlags, int dataUpdateFlags)
{
	if (m_state == ESessionState::CONNECTED && m_expectedMsg == EExpectedMsg::NONE)
	{
		m_expectedMsg = EExpectedMsg::SERVER_DATA_STATUS;
		sendMessage(m_context->getProtocol().createClientMsg_REQUEST_DATA(dataFlags, dataUpdateFlags));
	}
}

void ClientSession::SocketPollHandler(int flags, void *param)
{
	ClientSession *self = static_cast<ClientSession*>(param);

	if (flags & EPollFlags::INPUT)  // something has been received or server closed the connection
	{
		bool isConnectionLost = false;
		try
		{
			// receive and parse data, and eventually call onMessage function
			isConnectionLost = self->m_socketReader.doReceive();
		}
		catch (const SocketException & e)
		{
			std::string error = (self->m_isConnectionEstablished) ? "Unable to receive data: " : "";
			error += e.what();
			self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, error.c_str());
		}
		catch (const MessageParserException & e)
		{
			self->quickDisconnect(EDisconnectReason::PROTOCOL_ERROR, e.what());
		}
		catch (const std::invalid_argument & e)  // onMessage
		{
			self->quickDisconnect(EDisconnectReason::PROTOCOL_ERROR, e.what());
		}

		if (isConnectionLost)
		{
			if (self->m_state == ESessionState::CLOSING)
			{
				// see disconnect function
				self->quickDisconnect(self->m_disconnectReason);
			}
			else
			{
				self->quickDisconnect(EDisconnectReason::CONNECTION_LOST);
			}
		}
	}

	if (flags & EPollFlags::OUTPUT && self->m_state != ESessionState::DISCONNECTED)  // something can be sent
	{
		if (self->m_isConnectionEstablished)
		{
			try
			{
				// try to send all messages waiting in the send queue
				while (self->m_socketWriter.doSend())
				{
					if (self->m_sendQueue.empty())
					{
						break;
					}

					self->m_socketWriter.send(std::move(self->m_sendQueue.front()));
					self->m_sendQueue.pop_front();
				}
			}
			catch (const SocketException & e)
			{
				std::string error = "Unable to send data: ";
				error += e.what();
				self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, error.c_str());
			}
		}
		else
		{
			try
			{
				// check connection status
				self->m_socket.verifyConnect();
				// connection to server has been established
				self->m_isConnectionEstablished = true;
			}
			catch (const SocketException & e)
			{
				self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, e.what());
			}

			if (self->m_isConnectionEstablished)
			{
				try
				{
					self->m_serverEndpoint = self->m_socket.getRemoteEndpoint();
				}
				catch (const SocketException & e)
				{
					gLog->error("[ClientSession] Unable to get remote endpoint: %s", e.what());
				}

				self->m_context->getSessionCallback()->onSessionConnectionEstablished(self);
			}
		}
	}

	if (flags & EPollFlags::ERROR && self->m_state != ESessionState::DISCONNECTED)
	{
		self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, "Socket poll failed");
	}

	if (self->m_socket.isConnected())
	{
		int flags = EPollFlags::INPUT;

		if (!self->m_sendQueue.empty())
		{
			flags |= EPollFlags::OUTPUT;
		}

		// continue waiting for socket events
		gApp->getPollSystem()->reset(self->m_socket, flags);
	}
}

ServerSession::ServerSession(StreamSocket && socket, const ServerContext & context)
: m_context(&context),
  m_socket(std::move(socket)),
  m_socketReader(m_socket, ClientMessageParser(this)),
  m_socketWriter(m_socket),
  m_sendQueue(),
  m_state(ESessionState::DISCONNECTED),
  m_isSendingData(false),
  m_sessionTTL(),
  m_dataFlags(0),
  m_dataUpdateFlags(0),
  m_clientName(),
  m_clientVersion(),
  m_clientPlatformName(),
  m_clientEndpoint(),
  m_disconnectReason(),
  m_disconnectError()
{
	if (m_socket.isConnected())
	{
		try
		{
			m_clientEndpoint = m_socket.getRemoteEndpoint();
		}
		catch (const SocketException & e)
		{
			gLog->error("[ServerSession] Unable to get remote endpoint: %s", e.what());
		}
		m_state = ESessionState::OPENING;
		gApp->getPollSystem()->add(m_socket, EPollFlags::INPUT, SocketPollHandler, this);
		sendSharedMessage(m_context->getCachedBannerMsg());
	}
}

ServerSession::~ServerSession()
{
	if (m_socket.isConnected())
	{
		gApp->getPollSystem()->remove(m_socket);
	}
}

void ServerSession::sendMessage(SerializedServerMessage && msg)
{
	if (m_sendQueue.empty())
	{
		gApp->getPollSystem()->reset(m_socket, EPollFlags::INPUT | EPollFlags::OUTPUT);
	}

	m_sendQueue.emplace_back(new SerializedServerMessage(std::move(msg)));
}

void ServerSession::sendSharedMessage(const SerializedServerMessage & msg)
{
	if (m_sendQueue.empty())
	{
		gApp->getPollSystem()->reset(m_socket, EPollFlags::INPUT | EPollFlags::OUTPUT);
	}

	m_sendQueue.emplace_back(const_cast<SerializedServerMessage*>(&msg), SendQueueMsgDeleter(true));
}

void ServerSession::quickDisconnect(EDisconnectReason reason, const char *error)
{
	if (m_socket.isConnected())
	{
		gApp->getPollSystem()->remove(m_socket);
		m_socket.close_nothrow();
	}
	m_state = ESessionState::DISCONNECTED;
	m_isSendingData = false;
	m_sendQueue.clear();
	m_disconnectReason = reason;
	m_disconnectError = (error) ? error : "";
	m_context->getSessionCallback()->onSessionDisconnect(this);
}

void ServerSession::onMessage(rapidjson::Value & message)
{
	const ClientServerProtocol & proto = m_context->getProtocol();
	IServerSessionCallback *callback = m_context->getSessionCallback();

	if (!message.IsObject())
		throw std::invalid_argument("Invalid message");

	const auto msgTypeIt = message.FindMember("message");
	if (msgTypeIt == message.MemberEnd())
		throw std::invalid_argument("Missing message type");
	else if (!msgTypeIt->value.IsString())
		throw std::invalid_argument("Invalid message type value type");

	EClientMsg msgType = proto.getClientMsgEnum(msgTypeIt->value.GetString());

	switch (msgType)
	{
		case EClientMsg::UNKNOWN:
		{
			std::string error = "Unknown message '";
			error += msgTypeIt->value.GetString();
			error += "'";
			throw std::invalid_argument(error);
		}
		case EClientMsg::HELLO:
		{
			if (m_state != ESessionState::OPENING)
				throw std::invalid_argument("Unexpected HELLO message");

			const auto nameIt = message.FindMember("name");
			if (nameIt == message.MemberEnd())
				throw std::invalid_argument("Missing client name");
			else if (!nameIt->value.IsString())
				throw std::invalid_argument("Invalid client name value type");

			const auto versionIt = message.FindMember("version");
			if (versionIt == message.MemberEnd())
				throw std::invalid_argument("Missing client version");
			else if (!versionIt->value.IsString())
				throw std::invalid_argument("Invalid client version value type");

			const auto platformNameIt = message.FindMember("platform");
			if (platformNameIt == message.MemberEnd())
				throw std::invalid_argument("Missing client platform name");
			else if (!platformNameIt->value.IsString())
				throw std::invalid_argument("Invalid client platform name value type");

			m_clientName = nameIt->value.GetString();
			m_clientVersion = versionIt->value.GetString();
			m_clientPlatformName = platformNameIt->value.GetString();

			sendSharedMessage(m_context->getCachedHelloMsg());

			m_state = ESessionState::CONNECTED;

			callback->onSessionEstablished(this);

			break;
		}
		case EClientMsg::DISCONNECT:
		{
			const auto reasonIt = message.FindMember("reason");
			if (reasonIt == message.MemberEnd())
				throw std::invalid_argument("Missing disconnect reason");
			else if (!reasonIt->value.IsString())
				throw std::invalid_argument("Invalid disconnect reason value type");

			const char *reasonName = reasonIt->value.GetString();
			EDisconnectReason reason = proto.getDisconnectReasonEnum(reasonName);

			quickDisconnect(reason, (reason == EDisconnectReason::UNKNOWN) ? reasonName : nullptr);

			break;
		}
		case EClientMsg::REQUEST_DATA:
		{
			if (m_state == ESessionState::CLOSING)
				return;

			if (m_state != ESessionState::CONNECTED)
				throw std::invalid_argument("Unexpected REQUEST_DATA message");

			const auto dataFlagsIt = message.FindMember("dataFlags");
			if (dataFlagsIt == message.MemberEnd())
				throw std::invalid_argument("Missing data status type");
			else if (!dataFlagsIt->value.IsInt())
				throw std::invalid_argument("Invalid data status type value type");

			const auto dataUpdateFlagsIt = message.FindMember("dataUpdateFlags");
			if (dataUpdateFlagsIt == message.MemberEnd())
				throw std::invalid_argument("Missing data status update");
			else if (!dataUpdateFlagsIt->value.IsInt())
				throw std::invalid_argument("Invalid data status update value type");

			int dataFlags = dataFlagsIt->value.GetInt();
			int dataUpdateFlags = dataUpdateFlagsIt->value.GetInt();

			callback->onSessionDataRequest(this, dataFlags, dataUpdateFlags);

			break;
		}
	}
}

void ServerSession::onUpdate()
{
	m_dataFlags = 0;
	m_dataUpdateFlags = 0;

	if (m_state == ESessionState::CONNECTED)
	{
		sendSharedMessage(m_context->getCachedUpdateTickMsg());
	}
	else if (m_state == ESessionState::CLOSING)
	{
		if (m_sessionTTL > 0)
		{
			m_sessionTTL--;
		}
		else
		{
			quickDisconnect(EDisconnectReason::TIMEOUT);
		}
	}
}

void ServerSession::disconnect()
{
	const EDisconnectReason reason = EDisconnectReason::SERVER_QUIT;

	if (m_state == ESessionState::CONNECTED)
	{
		m_state = ESessionState::CLOSING;
		m_sessionTTL = 4;
		m_disconnectReason = reason;
		m_disconnectError = "";
		sendMessage(m_context->getProtocol().createServerMsg_DISCONNECT(reason));
	}
	else if (m_state == ESessionState::OPENING)
	{
		quickDisconnect(reason);
	}
}

void ServerSession::sendDataStatus(int dataFlags, int dataUpdateFlags)
{
	if (m_state == ESessionState::CONNECTED)
	{
		m_dataFlags = dataFlags;
		m_dataUpdateFlags = dataUpdateFlags;
		sendMessage(m_context->getProtocol().createServerMsg_DATA_STATUS(dataFlags, dataUpdateFlags));
	}
}

void ServerSession::sendData(const SerializedServerMessage & dataMsg)
{
	if (m_state == ESessionState::CONNECTED)
	{
		sendSharedMessage(dataMsg);
	}
}

bool ServerSession::stopSendingData()
{
	bool wasSendingData = false;
	for (auto it = m_sendQueue.begin(); it != m_sendQueue.end();)
	{
		auto & pMsg = *it;
		if (pMsg->getType() == EServerMsg::DATA)
		{
			it = m_sendQueue.erase(it);
			wasSendingData = true;
		}
		else
		{
			++it;
		}
	}

	bool status = true;
	if (m_isSendingData)
	{
		status = m_socketWriter.stop();
		m_isSendingData = false;
		wasSendingData = true;
	}

	if (wasSendingData)
	{
		m_dataFlags = 0;
		m_dataUpdateFlags = 0;
	}

	return status;
}

void ServerSession::SocketPollHandler(int flags, void *param)
{
	ServerSession *self = static_cast<ServerSession*>(param);

	if (flags & EPollFlags::INPUT)  // something has been received or client closed the connection
	{
		bool isConnectionLost = false;
		try
		{
			// receive and parse data, and eventually call onMessage function
			isConnectionLost = self->m_socketReader.doReceive();
		}
		catch (const SocketException & e)
		{
			std::string error = "Unable to receive data: ";
			error += e.what();
			self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, error.c_str());
		}
		catch (const MessageParserException & e)
		{
			self->quickDisconnect(EDisconnectReason::PROTOCOL_ERROR, e.what());
		}
		catch (const std::invalid_argument & e)  // onMessage
		{
			self->quickDisconnect(EDisconnectReason::PROTOCOL_ERROR, e.what());
		}

		if (isConnectionLost)
		{
			if (self->m_state == ESessionState::CLOSING)
			{
				// see disconnect function
				self->quickDisconnect(self->m_disconnectReason);
			}
			else
			{
				self->quickDisconnect(EDisconnectReason::CONNECTION_LOST);
			}
		}
	}

	if (flags & EPollFlags::OUTPUT && self->m_state != ESessionState::DISCONNECTED)  // something can be sent
	{
		try
		{
			// try to send all messages waiting in the send queue
			while (self->m_socketWriter.doSend())
			{
				if (self->m_sendQueue.empty())
				{
					self->m_isSendingData = false;
					break;
				}

				auto & pMsg = self->m_sendQueue.front();
				self->m_isSendingData = pMsg->getType() == EServerMsg::DATA;
				if (pMsg.get_deleter().isShared())
				{
					self->m_socketWriter.sendShared(*pMsg);
				}
				else
				{
					self->m_socketWriter.send(std::move(*pMsg));
				}
				self->m_sendQueue.pop_front();
			}
		}
		catch (const SocketException & e)
		{
			std::string error = "Unable to send data: ";
			error += e.what();
			self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, error.c_str());
		}
	}

	if (flags & EPollFlags::ERROR && self->m_state != ESessionState::DISCONNECTED)
	{
		self->quickDisconnect(EDisconnectReason::SOCKET_ERROR, "Socket poll failed");
	}

	if (self->m_socket.isConnected())
	{
		int flags = EPollFlags::INPUT;

		if (!self->m_sendQueue.empty())
		{
			flags |= EPollFlags::OUTPUT;
		}

		// continue waiting for socket events
		gApp->getPollSystem()->reset(self->m_socket, flags);
	}
}
