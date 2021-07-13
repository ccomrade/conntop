/**
 * @file
 * @brief Classes for reading and writing to sockets.
 */

#pragma once

#include <cstring>  // std::memmove
#include <string>
#include <memory>

#include "Types.hpp"
#include "Sockets.hpp"

namespace ctp
{
	template<class Parser>
	class StreamSocketReader
	{
		StreamSocket *m_socket;
		std::unique_ptr<char[]> m_buffer;
		size_t m_bufferPos;
		size_t m_bufferSize;
		Parser m_parser;

	public:
		StreamSocketReader(StreamSocket & socket, Parser && parser, size_t bufferSize = 4096)
		: m_socket(&socket),
		  m_buffer(std::make_unique<char[]>(bufferSize)),
		  m_bufferPos(0),
		  m_bufferSize(bufferSize),
		  m_parser(std::move(parser))
		{
		}

		Parser & getParser()
		{
			return m_parser;
		}

		const Parser & getParser() const
		{
			return m_parser;
		}

		size_t getBufferSize() const
		{
			return m_bufferSize;
		}

		size_t getBufferedDataSize() const
		{
			return m_bufferPos;
		}

		bool doReceive()
		{
			char *buffer = m_buffer.get();

			size_t dataLength = m_socket->receive(buffer+m_bufferPos, (m_bufferSize-m_bufferPos)-1);
			if (dataLength == 0)
			{
				return true;
			}
			dataLength += m_bufferPos;
			buffer[dataLength] = '\0';  // make received data null terminated

			size_t parsedLength = m_parser.parse(buffer, dataLength, m_bufferSize);
			if (parsedLength < dataLength)
			{
				// move the remaining data to the beginning of the buffer
				size_t chunkLength = dataLength - parsedLength;
				std::memmove(buffer, buffer+parsedLength, chunkLength);
				m_bufferPos = chunkLength;
			}
			else
			{
				m_bufferPos = 0;
			}

			return false;
		}
	};

	template<class T>
	class StreamSocketWriter
	{
		enum struct EMessageType
		{
			NONE, OWN, SHARED, BUFFERED
		};

		StreamSocket *m_socket;
		EMessageType m_msgType;
		std::string m_buffer;
		const char *m_data;
		size_t m_dataLength;
		size_t m_dataPos;
		T m_msg;

	public:
		StreamSocketWriter(StreamSocket & socket)
		: m_socket(&socket),
		  m_msgType(EMessageType::NONE),
		  m_buffer(),
		  m_data(),
		  m_dataLength(),
		  m_dataPos(),
		  m_msg()
		{
		}

		bool isSending() const
		{
			return m_msgType != EMessageType::NONE;
		}

		bool stop()
		{
			switch (m_msgType)
			{
				case EMessageType::NONE:
				{
					break;
				}
				case EMessageType::OWN:
				{
					if (m_dataPos != 0)
					{
						return false;
					}
					else
					{
						m_msgType = EMessageType::NONE;
						m_msg.clear();
					}
					break;
				}
				case EMessageType::SHARED:
				{
					if (m_dataPos != 0)
					{
						m_buffer.assign(m_data+m_dataPos, m_dataLength-m_dataPos);
						m_data = m_buffer.c_str();
						m_dataLength = m_buffer.length();
						m_dataPos = 0;
						m_msgType = EMessageType::BUFFERED;

						return false;
					}
					else
					{
						m_msgType = EMessageType::NONE;
					}
					break;
				}
				case EMessageType::BUFFERED:
				{
					return false;
				}
			}

			return true;
		}

		bool send(T && msg)
		{
			if (isSending())
			{
				return false;
			}

			m_msg = std::move(msg);
			m_data = m_msg.c_str();
			m_dataLength = m_msg.length();
			m_dataPos = 0;
			m_msgType = EMessageType::OWN;

			return true;
		}

		bool sendShared(const T & msg)
		{
			if (isSending())
			{
				return false;
			}

			m_data = msg.c_str();
			m_dataLength = msg.length();
			m_dataPos = 0;
			m_msgType = EMessageType::SHARED;

			return true;
		}

		bool doSend()
		{
			if (!isSending())
			{
				return true;
			}

			size_t length = m_socket->send(m_data+m_dataPos, m_dataLength-m_dataPos);

			m_dataPos += length;
			if (m_dataPos >= m_dataLength)
			{
				if (m_msgType == EMessageType::OWN)
				{
					m_msg.clear();
				}
				else if (m_msgType == EMessageType::BUFFERED)
				{
					m_buffer.clear();
				}

				m_msgType = EMessageType::NONE;

				return true;
			}

			return false;
		}
	};
}
