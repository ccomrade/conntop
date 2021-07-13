/**
 * @file
 * @brief Client class.
 */

#pragma once

#include <memory>

#include "ClientEvent.hpp"
#include "ClientServerProtocol.hpp"

namespace ctp
{
	class ClientConnector;

	class Client final : public IClientSessionCallback
	{
		ClientContext m_context;
		ClientSession m_session;
		std::unique_ptr<ClientConnector> m_pConnector;
		int m_requestedDataFlags;
		int m_requestedDataUpdateFlags;
		int m_remainingDataFlags;
		bool m_isDataRequested;
		bool m_isSynchronized;
		bool m_isPaused;

		void requestData();
		void setSynchronized(bool isSynchronized);

		// IClientSessionCallback

		void onSessionConnectionEstablished(ClientSession *session) override;
		void onSessionEstablished(ClientSession *session) override;
		void onSessionDisconnect(ClientSession *session) override;
		void onSessionServerTick(ClientSession *session) override;
		void onSessionDataStatus(ClientSession *session, bool isDifferent) override;
		void onSessionData(ClientSession *session, int type, bool isUpdate, rapidjson::Value & data) override;

		friend class ClientSession;  // IClientSessionCallback functions are private

	public:
		Client();
		~Client();

		void init();

		void onUpdate();

		void disconnect();

		void setPaused(bool paused);

		bool isPaused() const
		{
			return m_isPaused;
		}

		bool isSynchronized() const
		{
			return m_isSynchronized;
		}

		bool isConnected() const
		{
			return m_session.getState() == ESessionState::CONNECTED;  // session is established
		}

		bool isConnecting() const
		{
			return m_pConnector || m_session.getState() == ESessionState::OPENING;
		}

		bool isDisconnecting() const
		{
			return m_session.getState() == ESessionState::CLOSING;
		}

		const ClientSession & getSession() const
		{
			return m_session;
		}
	};
}
