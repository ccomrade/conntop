/**
 * @file
 * @brief Implementation of Netfilter collector.
 */

#include "CCollector.hpp"
#include "Conntrack.hpp"

namespace ctp
{
	class CCollector_Netfilter::Impl
	{
		Conntrack m_conntrack;

	public:
		Impl()
		: m_conntrack()
		{
		}

		void init( IConnectionUpdateCallback *callback )
		{
			m_conntrack.init( callback );
		}

		void onUpdate()
		{
			m_conntrack.onUpdate();
		}

		bool isPaused() const
		{
			return m_conntrack.isPaused();
		}

		void setPaused( bool paused )
		{
			m_conntrack.setPaused( paused );
		}
	};

	CCollector_Netfilter::CCollector_Netfilter()
	: m_impl(std::make_unique<Impl>())
	{
	}

	CCollector_Netfilter::~CCollector_Netfilter()
	{
	}

	KString CCollector_Netfilter::getName() const
	{
		return "Netfilter (conntrack)";
	}

	void CCollector_Netfilter::init( IConnectionUpdateCallback *callback )
	{
		m_impl->init( callback );
	}

	void CCollector_Netfilter::onUpdate()
	{
		m_impl->onUpdate();
	}

	bool CCollector_Netfilter::isPaused() const
	{
		return m_impl->isPaused();
	}

	void CCollector_Netfilter::setPaused( bool paused )
	{
		m_impl->setPaused( paused );
	}
}
