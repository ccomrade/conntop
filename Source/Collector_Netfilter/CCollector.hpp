/**
 * @file
 * @brief Netfilter collector.
 */

#pragma once

#include <memory>

#include "ICollector.hpp"

namespace ctp
{
	class CCollector_Netfilter : public ICollector
	{
		class Impl;
		std::unique_ptr<Impl> m_impl;

	public:
		CCollector_Netfilter();
		~CCollector_Netfilter();

		KString getName() const override;

		void init(IConnectionUpdateCallback *callback) override;

		void onUpdate() override;

		bool isPaused() const override;
		void setPaused(bool paused) override;
	};
}
