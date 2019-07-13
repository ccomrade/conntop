/**
 * @file
 * @brief EventSystem class.
 */

#pragma once

#include <memory>

#include "IEventCallback.hpp"
#include "EventWrapper.hpp"

namespace ctp
{
	class EventSystem
	{
		class Impl;
		std::unique_ptr<Impl> m_impl;

		using ExecutorFunction = void (*)( void *pCallback, const EventWrapper & eventWrapper );

		template<class T>
		static void Executor( IEventCallback<T> *pCallback, const EventWrapper & eventWrapper )
		{
			const T *pEvent = eventWrapper.get<T>();
			if ( pEvent )
			{
				pCallback->onEvent( *pEvent );
			}
		}

		void pushEvent( EventWrapper && eventWrapper );
		void addCallback( void *pCallback, void *pExecutor, int eventID );
		void delCallback( void *pCallback, int eventID );

	public:
		EventSystem();
		~EventSystem();

		template<class T>
		void dispatch( T && event )
		{
			pushEvent( EventWrapper( std::forward<T>( event ) ) );
		}

		template<class T, class... Args>
		void dispatch( Args &&... args )
		{
			EventWrapper event;
			event.emplace<T>( std::forward<Args>( args )... );
			pushEvent( std::move( event ) );
		}

		template<class T>
		void registerCallback( IEventCallback<T> *pCallback )
		{
			if ( pCallback )
			{
				addCallback( pCallback, reinterpret_cast<void*>( Executor<T> ), T::ID );
			}
		}

		template<class T>
		void removeCallback( IEventCallback<T> *pCallback )
		{
			if ( pCallback )
			{
				delCallback( pCallback, T::ID );
			}
		}

		void run();

		void stop();
	};
}
